#!/usr/bin/env python3
"""Restricted agent-channel regression. Stdlib unit tests; optional isolated live pair.

python diagnostics/AgentPlayTests.py
python diagnostics/AgentPlayTests.py --package DATA --exe BUILD/Game.exe --output NEW_DIRECTORY
"""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import time
import unittest
import urllib.error
import urllib.request
import zlib

SCRIPT = Path(__file__).resolve().parents[1] / 'tools' / 'agent_play.py'
spec = importlib.util.spec_from_file_location('agent_play', SCRIPT)
agent = importlib.util.module_from_spec(spec)
spec.loader.exec_module(agent)


class CommandBoundaryTests(unittest.TestCase):
    def test_debug_lifecycle_and_multiline_requests_are_rejected(self):
        for command in ('netstatus', 'console @GetUnit(0)', 'rng 0', 'quit', 'close',
                        'observe\nend_turn', 'observe\x00', 'preview_screenshot', 'preview_focus 0',
                        'shoot 0 nan 1 1', 'shoot 0 inf 1 1', 'move 0 1 2 extra', 'attack 0'):
            with self.subTest(command=command), self.assertRaises((ValueError, TypeError)):
                agent.validate_command(command)

    def test_player_actions_and_previews_are_available(self):
        for command in ('observe', 'screenshot', 'attack 0 1', 'preview_attack 0 1',
                        'move 0 30 40', 'camera 1 -2 .5 -3', 'equip 0 1 0', 'floor -1',
                        'shoot 0 1.5 2.5 6', 'hover 500 300', 'click 500 300'):
            self.assertEqual(agent.validate_command(command), command)

    def test_screenshot_conversion_preserves_pixels_and_orientation(self):
        # Bottom-up 24-bit BMP, two rows, with padding. Expected RGB is red/green then blue/white.
        pixels = b'\xff\x00\x00\xff\xff\xff\x00\x00' + b'\x00\x00\xff\x00\xff\x00\x00\x00'
        header = bytearray(54)
        header[:2] = b'BM'
        struct.pack_into('<I', header, 10, 54)
        struct.pack_into('<IiiHHI', header, 14, 40, 2, 2, 1, 24, 0)
        with tempfile.TemporaryDirectory() as temporary:
            source, destination = Path(temporary) / 'a.bmp', Path(temporary) / 'a.png'
            source.write_bytes(header + pixels)
            agent.png_from_bmp(source, destination)
            png = destination.read_bytes()
            self.assertEqual(png[:8], b'\x89PNG\r\n\x1a\n')
            at = 8
            while at < len(png):
                size = struct.unpack_from('>I', png, at)[0]
                if png[at + 4:at + 8] == b'IDAT':
                    self.assertEqual(zlib.decompress(png[at + 8:at + 8 + size]),
                                     b'\x00\xff\x00\x00\x00\xff\x00\x00\x00\x00\xff\xff\xff\xff')
                    break
                at += 12 + size
            else:
                self.fail('Missing PNG image data')


def live(args):
    root = args.output.resolve()
    subprocess.run([sys.executable, str(SCRIPT), 'start', '--package', str(args.package), '--exe', str(args.exe),
                    '--output', str(root), '--hidden'], check=True)
    capabilities = {role: json.loads((root / (role + '-player.json')).read_text())['endpoint'] for role in ('host', 'client')}
    records = []
    def call(role, command):
        result = agent.request(capabilities[role], {'command': command, 'settle': True})
        records.append({'role': role, 'command': command, 'response': result})
        return result
    def raw(role, command, sequence):
        game = root / role
        path = game / '_agent_request.txt'
        assert not path.exists(), 'Raw test collided with a game request'
        temporary = game / '_agent_request.test.tmp'
        temporary.write_text(f'{sequence} {command}', encoding='ascii')
        temporary.replace(path)
        deadline = time.monotonic() + 10
        while time.monotonic() < deadline:
            try:
                result = json.loads((game / '_agent_response.json').read_text())
                if result['id'] == sequence:
                    records.append({'role': role, 'raw': command, 'response': result})
                    return result
            except (OSError, ValueError):
                pass
            time.sleep(.05)
        raise TimeoutError(command)
    try:
        initial = {role: call(role, 'observe')['observation'] for role in capabilities}
        assert initial['host']['side'] == 0 and initial['client']['side'] == 1
        for role, observation in initial.items():
            assert observation['visible_enemies'] == [], 'Fixture must initially have hidden opponents'
            assert len(observation['units']) == 1 and 'backpack' in observation['units'][0]
            assert 'teams' not in observation and 'enemy_units' not in observation
            capture = call(role, 'screenshot')
            assert capture['ok'] and Path(capture['screenshot']).read_bytes().startswith(b'\x89PNG')
        own_preview = call('host', 'preview_reload 0')
        assert 'result' in own_preview and own_preview['result'] != 21, 'Host must use synchronous native preview'
        assert own_preview['observation']['units'] == initial['host']['units'], 'Preview changed player state'
        for index, command in enumerate(('netstatus', 'unitpos', 'console @ExitGame()', 'rng 1',
                                         'load quick', 'save quick', 'nettestdamage 1 0 100',
                                         'attack 0 99999', 'move -1 20 20', 'observe trailing',
                                         'move 0 30.5 20')):
            result = raw('host', command, 900000 + index)
            assert result['ok'] is False, command
            assert result['observation']['units'] == initial['host']['units'], command
            assert result['observation']['visible_enemies'] == [], command
        # Out-of-turn orders cannot mutate the replica or consume AP/ammo.
        assert call('client', 'move 0 18 55')['ok'] is False
        assert call('client', 'end_turn')['ok'] is False
        assert call('client', 'observe')['observation']['units'] == initial['client']['units']
        for command in ('quit', 'console @GetUnits()', 'observe\nend_turn'):
            assert call('host', command)['ok'] is False
        # Even writing the old developer channel cannot activate it in this mode.
        harness = root / 'host' / '_harness_cmd.txt'
        harness.write_text('unitpos', encoding='ascii')
        time.sleep(.4)
        assert harness.exists() and not (root / 'host' / '_saveload.log').exists()
        harness.unlink()
        bad_url = capabilities['host'].rsplit('/', 1)[0] + '/not-a-capability'
        req = urllib.request.Request(bad_url, data=b'{"command":"observe"}')
        try:
            urllib.request.urlopen(req, timeout=5)
            raise AssertionError('Wrong capability was accepted')
        except urllib.error.HTTPError as error:
            assert error.code == 403
        for command in ('camera 1 0 .1 -1', 'floor 0', 'hover 512 384', 'focus 0'):
            assert call('host', command)['ok'], command
        for role in capabilities:
            assert call(role, 'screenshot')['ok']
        grenade = next(item for item in initial['host']['units'][0]['backpack'] if item['kind'] == 'grenade')
        equipped = call('host', f"equip 0 {grenade['item']} 1")['observation']['units'][0]
        assert equipped['active_slot'] == 1 and any(item['kind'] == 'grenade' and item['slot'] == 1 for item in equipped['slots'])
        restored = call('host', 'slot 0 0')['observation']['units'][0]
        assert restored['active_slot'] == 0 and restored['ammo'] == initial['host']['units'][0]['ammo']
        unit = initial['host']['units'][0]
        x, y = unit['grid']
        moved = False
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            result = call('host', f'move 0 {x + dx} {y + dy}')
            actual = result['observation']['units'][0]
            if actual['grid'] != [x, y]:
                assert actual['ap'] < unit['ap'], 'Movement did not spend ordinary AP'
                moved = True
                break
        assert moved, 'No ordinary legal movement around spawn'
        assert call('host', 'end_turn')['ok']
        assert call('client', 'observe')['observation']['turn'] == 1
        for _ in range(6):
            result = call('client', 'preview_reload 0')
            if result.get('result') != 21:
                break
            time.sleep(.1)
        assert 'result' in result and 'start_ap' in result
        agent.save_json(root / 'regression.json', {'passed': True, 'records': records})
        print('LIVE restricted channel, visibility, capabilities, AP, previews, screenshots and harness isolation passed')
    finally:
        subprocess.run([sys.executable, str(SCRIPT), 'close', '--session', str(root / 'session.json')], check=True)


def solo(args):
    """Existing single-player harness still loads/renders; agent files stay inactive."""
    root = args.output.resolve()
    agent.stage(args.package.resolve(), args.exe.resolve(), root, 'solo', True)
    game = root / 'solo'
    startup = subprocess.STARTUPINFO()
    startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW
    startup.wShowWindow = 0
    with (root / 'solo-process.log').open('w') as output:
        process = subprocess.Popen([str(game / 'Game.exe'), '-windowed', '-1024', '-harness', '-harness-active', '-cfg', 'agent.cfg'],
                                   cwd=game, env=dict(os.environ, S2_USER_DATA_DIR=str(root / 'solo-user-data')),
                                   startupinfo=startup, stdout=output, stderr=subprocess.STDOUT)
    def command(text):
        path = game / '_harness_cmd.txt'
        temporary = game / '_harness_cmd.tmp'
        temporary.write_text(text, encoding='ascii')
        temporary.replace(path)
        deadline = time.monotonic() + 40
        while path.exists():
            if process.poll() is not None or time.monotonic() >= deadline:
                raise RuntimeError('Single-player harness did not consume ' + text)
            time.sleep(.1)
        time.sleep(.2)
        return (game / '_saveload.log').read_text(errors='replace').rsplit('[harness] cmd: ' + text, 1)[-1]
    try:
        deadline = time.monotonic() + 60
        startup_log = root / 'solo-user-data' / 'startup.log'
        while not (startup_log.exists() and 'main-menu-presented' in startup_log.read_text(errors='replace')):
            if process.poll() is not None or time.monotonic() >= deadline:
                raise RuntimeError('Single-player menu did not start')
            time.sleep(.25)
        request = game / '_agent_request.txt'
        request.write_text('1 observe', encoding='ascii')
        command('console template 3589 730 with Day')
        deadline = time.monotonic() + 60
        while 'unitpos count=' not in command('unitpos'):
            if time.monotonic() >= deadline:
                raise RuntimeError('Single-player tutorial did not load')
            time.sleep(.5)
        assert 'screenshot ok=1' in command('screenshot')
        assert request.exists() and not (game / '_agent_response.json').exists(), 'Agent channel activated without flag'
        captures = root / 'solo-user-data' / 'screenshots'
        image = max(captures.glob('*.bmp'), key=lambda p: p.stat().st_mtime_ns)
        agent.png_from_bmp(image, root / 'solo-tutorial.png')
        agent.save_json(root / 'solo-regression.json', {'passed': True, 'scenario': 'template 3589, persona 730, Day',
                                                     'agent_channel_inactive': True})
        print('SINGLE PLAYER existing menu/tutorial/render/harness and inactive agent channel passed')
    finally:
        if process.poll() is None:
            command('quit')
        assert process.wait(timeout=20) == 0, 'Single-player exit failed'


def human_window(args):
    import ctypes
    from ctypes import wintypes
    root = args.output.resolve() / 'human-window'
    subprocess.run([sys.executable, str(SCRIPT), 'start', '--package', str(args.package), '--exe', str(args.exe),
                    '--output', str(root)], check=True)
    try:
        pids = {role: json.loads((root / (role + '-process.json')).read_text())['pid'] for role in ('host', 'client')}
        visible = {role: [] for role in pids}
        user32 = ctypes.WinDLL('user32', use_last_error=True)
        callback_type = ctypes.WINFUNCTYPE(wintypes.BOOL, wintypes.HWND, wintypes.LPARAM)
        user32.EnumWindows.argtypes = [callback_type, wintypes.LPARAM]
        user32.GetWindowThreadProcessId.argtypes = [wintypes.HWND, ctypes.POINTER(wintypes.DWORD)]
        user32.IsWindowVisible.argtypes = [wintypes.HWND]
        @callback_type
        def inspect(window, _):
            pid = wintypes.DWORD()
            user32.GetWindowThreadProcessId(window, ctypes.byref(pid))
            for role, expected in pids.items():
                if pid.value == expected:
                    visible[role].append(bool(user32.IsWindowVisible(window)))
            return True
        user32.EnumWindows(inspect, 0)
        assert any(visible['host']), 'The human host window is hidden'
        assert visible['client'] and not any(visible['client']), 'Agent client window is visible'
        agent.save_json(root / 'window-regression.json', {'passed': True, 'visible_windows': visible})
        print('HUMAN WINDOW host visible, agent client hidden, automatically connected')
    finally:
        subprocess.run([sys.executable, str(SCRIPT), 'close', '--session', str(root / 'session.json')], check=True)



if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--package', type=Path)
    parser.add_argument('--exe', type=Path)
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    suite = unittest.defaultTestLoader.loadTestsFromTestCase(CommandBoundaryTests)
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    if not result.wasSuccessful():
        sys.exit(1)
    if any((args.package, args.exe, args.output)):
        if not all((args.package, args.exe, args.output)):
            parser.error('Live checks require --package, --exe and --output together')
        live(args)
        solo(args)
        human_window(args)
