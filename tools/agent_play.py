#!/usr/bin/env python3
"""Local two-player launcher and restricted, per-player HTTP/CLI game tools (stdlib only)."""
import argparse
import contextlib
import http.server
import json
import math
import os
from pathlib import Path
import secrets
import shutil
import socket
import struct
import subprocess
import sys
import threading
import time
import urllib.error
import urllib.request
import zlib


def save_json(path, value):
    temporary = path.with_suffix('.tmp')
    temporary.write_text(json.dumps(value, indent=2), encoding='utf-8')
    temporary.replace(path)


def free_port():
    with socket.socket() as sock:
        sock.bind(('127.0.0.1', 0))
        return sock.getsockname()[1]


def png_from_bmp(source, destination):
    """Convert the game's uncompressed 24/32-bit screenshots without dependencies."""
    data = source.read_bytes()
    offset = struct.unpack_from('<I', data, 10)[0]
    width, height, planes, bits, compression = struct.unpack_from('<iiHHI', data, 18)
    if data[:2] != b'BM' or planes != 1 or bits not in (24, 32) or compression != 0:
        raise ValueError('Unsupported game screenshot BMP format')
    stride = ((width * bits + 31) // 32) * 4
    rows = bytearray()
    for y in range(abs(height)):
        row = y if height < 0 else height - 1 - y
        pixels = data[offset + row * stride:offset + (row + 1) * stride]
        rows.append(0)
        for x in range(width):
            start = x * (bits // 8)
            rows.extend((pixels[start + 2], pixels[start + 1], pixels[start]))
    def chunk(kind, payload):
        return struct.pack('>I', len(payload)) + kind + payload + struct.pack('>I', zlib.crc32(kind + payload))
    destination.write_bytes(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', width, abs(height), 8, 2, 0, 0, 0))
                           + chunk(b'IDAT', zlib.compress(rows, 3)) + chunk(b'IEND', b''))


ARITIES = {'observe': 0, 'screenshot': 0, 'end_turn': 0, 'stop': 0,
           'focus': 1, 'move': 3, 'look': 3, 'attack': 2, 'shoot': 4,
           'reload': 1, 'continue': 1, 'mode': 2, 'pose': 2, 'hide': 2, 'slot': 2, 'heal': 2,
           'equip': 3, 'camera': 4, 'floor': 1, 'hover': 2, 'click': 2, 'right_click': 2}


def validate_command(command):
    if not isinstance(command, str) or len(command) > 256 or any(c in command for c in '\r\n\x00'):
        raise ValueError('Expected one game command')
    words = command.split()
    if not words:
        raise ValueError('Missing command')
    verb = words[0]
    if verb.startswith('preview_') and verb[8:] not in ('observe', 'screenshot', 'end_turn', 'stop', 'focus', 'camera', 'floor', 'hover', 'click', 'right_click'):
        verb = verb[8:]
    if verb not in ARITIES or len(words) != ARITIES[verb] + 1:
        raise ValueError('Unknown command or wrong argument count')
    for value in words[1:]:
        number = float(value)
        if not (-4096 <= number <= 4096):
            raise ValueError('Argument outside game coordinate limits')
    return ' '.join(words)


class Player:
    def __init__(self, root, role, process):
        self.root, self.role, self.process = root, role, process
        self.game = root / role
        self.lock = threading.Lock()
        self.sequence = 0

    def rpc(self, command, timeout=30):
        if self.process.poll() is not None:
            raise RuntimeError(f'{self.role} game exited ({self.process.returncode})')
        self.sequence += 1
        request = self.game / '_agent_request.txt'
        response = self.game / '_agent_response.json'
        deadline = time.monotonic() + timeout
        # Never overwrite an in-flight request, including one whose caller timed out.
        while request.exists():
            if time.monotonic() >= deadline:
                raise TimeoutError('Previous game request still pending')
            time.sleep(.05)
        temporary = self.game / '_agent_request.tmp'
        temporary.write_text(f'{self.sequence} {command}', encoding='ascii')
        temporary.replace(request)
        while time.monotonic() < deadline:
            try:
                value = json.loads(response.read_text(encoding='utf-8'))
                if value['id'] == self.sequence:
                    return value
            except (OSError, ValueError):
                pass
            if self.process.poll() is not None:
                raise RuntimeError(f'{self.role} game exited')
            time.sleep(.05)
        raise TimeoutError('Game did not answer')

    def call(self, command, settle=False, wait_turn=0):
        command = validate_command(command)
        with self.lock:
            if wait_turn:
                deadline = time.monotonic() + min(max(float(wait_turn), 0), 30)
                while True:
                    value = self.rpc('observe')
                    observation = value['observation']
                    if (observation['phase'] == 'finished' or
                        (observation.get('turn') == observation.get('side') and not observation.get('executing')) or
                            time.monotonic() >= deadline):
                        break
                    time.sleep(.25)
            before = {p: p.stat().st_mtime_ns for p in (self.root / (self.role + '-user-data') / 'screenshots').glob('*.bmp')}
            result = self.rpc(command)
            if settle and result['ok'] and command.split()[0] not in ('observe', 'screenshot') and not command.startswith('preview_'):
                time.sleep(.5)
                deadline = time.monotonic() + 20
                while True:
                    state = self.rpc('observe')['observation']
                    if not state.get('executing') or time.monotonic() >= deadline:
                        result['observation'] = state
                        break
                    time.sleep(.2)
            if command == 'screenshot' and result['ok']:
                directory = self.root / (self.role + '-user-data') / 'screenshots'
                deadline = time.monotonic() + 10
                while time.monotonic() < deadline:
                    files = [p for p in directory.glob('*.bmp') if before.get(p) != p.stat().st_mtime_ns]
                    if files:
                        source = max(files, key=lambda p: p.stat().st_mtime_ns)
                        image = self.root / (self.role + '-captures') / (str(self.sequence) + '.png')
                        image.parent.mkdir(exist_ok=True)
                        png_from_bmp(source, image)
                        result['screenshot'] = str(image)
                        break
                    time.sleep(.1)
                else:
                    raise TimeoutError('Screenshot file missing')
            with (self.root / (self.role + '-transcript.jsonl')).open('a', encoding='utf-8') as trace:
                trace.write(json.dumps({'time': time.time(), 'command': command, 'response': result}) + '\n')
            return result


def stage(package, executable, root, role, hd):
    game = root / role
    game.mkdir()
    # Data files are copied, resource directories linked read-only by convention.
    for source in package.iterdir():
        if source.is_file() and source.suffix.lower() in ('.db', '.mma', '.cfg'):
            shutil.copy2(source, game / source.name)
    for source in executable.parent.iterdir():
        if source.is_file() and source.suffix.lower() == '.dll':
            shutil.copy2(source, game / source.name)
    shutil.copy2(executable, game / 'Game.exe')
    shutil.copy2(executable.parent / 'network-version.txt', game / 'network-version.txt')
    for name in ('cfg', 'scripts'):
        shutil.copytree(package / name, game / name)
    fonts = executable.parent / 'fonts'
    if not fonts.exists():
        fonts = package / 'fonts'
    shutil.copytree(fonts, game / 'fonts')
    for name in ('res', 'res-hd'):
        source = package / name
        if name == 'res-hd' and not source.exists():
            source = Path(__file__).resolve().parents[1] / 'data' / 'res-hd'
        if not source.exists():
            if name == 'res' or hd:
                raise FileNotFoundError(source)
            continue
        if os.name == 'nt':
            # Native PowerShell performs the file operation without shell interpolation.
            environment = dict(os.environ, S2_AGENT_LINK=str(game / name), S2_AGENT_TARGET=str(source.resolve()))
            subprocess.run(['powershell.exe', '-NoProfile', '-NonInteractive', '-Command',
                            'New-Item -ItemType Junction -Path $env:S2_AGENT_LINK -Target $env:S2_AGENT_TARGET | Out-Null'],
                           env=environment, check=True, capture_output=True)
        else:
            (game / name).symlink_to(source.resolve(), target_is_directory=True)
    (game / 'agent.cfg').write_text(f'setvar gfx_hd_textures = {int(hd)}\nmainmenu\n', encoding='ascii')


def launch_game(config, root, role):
    game = root / role
    arguments = [str(game / 'Game.exe'), '-agent-play', '-windowed', '-1024', '-cfg', 'agent.cfg']
    if role == 'host':
        arguments += ['-net-host', str(config['game_port']), '-net-squad', str(config['squad'])]
        if config.get('map'):
            arguments += ['-net-map', str(config['map'])]
    else:
        arguments += ['-net-connect', '127.0.0.1', str(config['game_port'])]
    if config['hidden'] or config['human_side'] != role:
        arguments += ['-agent-hidden']
    options = {}
    if os.name == 'nt':
        startup = subprocess.STARTUPINFO()
        startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW
        startup.wShowWindow = 0 if config['hidden'] or config['human_side'] != role else 5
        options['startupinfo'] = startup
    with (root / (role + '-process.log')).open('w') as log:
        return subprocess.Popen(arguments, cwd=game, env=dict(os.environ, S2_USER_DATA_DIR=str(root / (role + '-user-data'))),
                                stdout=log, stderr=subprocess.STDOUT, **options)


def serve(path):
    config = json.loads(path.read_text())
    root = path.parent
    players = {}
    stop = threading.Event()
    class Handler(http.server.BaseHTTPRequestHandler):
        def log_message(self, *_):
            pass

        def do_POST(self):
            try:
                length = int(self.headers.get('Content-Length', '0'))
                if not 0 < length < 2048:
                    raise ValueError('Invalid request length')
                payload = json.loads(self.rfile.read(length))
                if not isinstance(payload, dict):
                    raise ValueError('Expected a JSON object')
                wait_turn = payload.get('wait_turn', 0)
                if not isinstance(wait_turn, (int, float)) or not math.isfinite(wait_turn) or not 0 <= wait_turn <= 30:
                    raise ValueError('wait_turn must be a number from 0 to 30')
                token = self.path.strip('/')
                if secrets.compare_digest(token, config['admin_token']):
                    if payload != {'command': 'close'}:
                        raise ValueError('Admin endpoint only supports close')
                    result = {'ok': True, 'status': 'closing'}
                    stop.set()
                else:
                    role = next((r for r, t in config['tokens'].items() if secrets.compare_digest(token, t)), None)
                    if role is None:
                        self.send_error(403)
                        return
                    if role not in players:
                        raise RuntimeError('Game is starting')
                    result = players[role].call(payload.get('command'), payload.get('settle', True), wait_turn)
                data = json.dumps(result).encode()
                self.send_response(200)
            except (ValueError, RuntimeError, TimeoutError, OSError) as error:
                data = json.dumps({'ok': False, 'error': str(error)}).encode()
                self.send_response(400)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Content-Length', str(len(data)))
            self.end_headers()
            with contextlib.suppress(BrokenPipeError):
                self.wfile.write(data)

    server = http.server.ThreadingHTTPServer(('127.0.0.1', config['api_port']), Handler)
    threading.Thread(target=server.serve_forever, daemon=True).start()
    failure = None
    try:
        for role in ('host', 'client'):
            process = launch_game(config, root, role)
            players[role] = Player(root, role, process)
            save_json(root / (role + '-process.json'), {'pid': process.pid})
            if role == 'host':
                deadline = time.monotonic() + 180
                log = root / 'host-user-data' / 'network.log'
                while not (log.exists() and 'listen port=' in log.read_text(errors='replace')):
                    if stop.is_set():
                        return
                    if process.poll() is not None or time.monotonic() >= deadline:
                        raise RuntimeError('Host failed to listen; see host startup/process logs')
                    time.sleep(.25)
        deadline = time.monotonic() + 180
        while not all(p.call('observe')['observation']['phase'] == 'playing' for p in players.values()):
            if stop.is_set():
                return
            for role in players:
                network_log = root / (role + '-user-data') / 'network.log'
                if network_log.exists() and ' FAIL ' in network_log.read_text(errors='replace'):
                    raise RuntimeError(role + ' connection failed; see its network.log')
            if time.monotonic() >= deadline:
                raise TimeoutError('Players failed to connect; see network logs')
            time.sleep(.5)
        save_json(root / 'ready.json', {'ok': True, 'players': list(players)})
        while not stop.wait(.5):
            if any(p.process.poll() is not None for p in players.values()):
                break
    except Exception as error:
        failure = str(error)
        save_json(root / 'error.json', {'error': failure})
    finally:
        for player in players.values():
            if player.process.poll() is None:
                with player.lock, contextlib.suppress(Exception):
                    player.rpc('quit', timeout=10)
        exit_codes = {}
        for role, player in players.items():
            try:
                exit_codes[role] = player.process.wait(timeout=20)
            except subprocess.TimeoutExpired:
                # Preserve a stuck process for diagnosis; never silently kill another game.
                exit_codes[role] = 'did_not_exit'
        save_json(root / 'closed.json', {'exit_codes': exit_codes, 'error': failure})
        server.shutdown()
        server.server_close()


def request(endpoint, payload):
    req = urllib.request.Request(endpoint, data=json.dumps(payload).encode(), headers={'Content-Type': 'application/json'})
    try:
        with urllib.request.urlopen(req, timeout=65) as response:
            return json.load(response)
    except urllib.error.HTTPError as error:
        return json.load(error)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest='operation', required=True)
    start = sub.add_parser('start', help='Launch an isolated local network match')
    start.add_argument('--package', type=Path, required=True, help='Game data with game.db/res/cfg/scripts')
    start.add_argument('--exe', type=Path, required=True, help='Built Game.exe, alongside DLLs/fonts/network-version.txt')
    start.add_argument('--output', type=Path, required=True, help='New isolated session directory')
    start.add_argument('--squad', type=int, choices=range(1, 7), default=1)
    start.add_argument('--map', type=int, default=0, help='Existing network random encounter variant; 0 uses default')
    start.add_argument('--human-side', choices=('host', 'client'), default='host')
    start.add_argument('--hidden', action='store_true', help='Hide both rendered windows for agent-vs-agent')
    call = sub.add_parser('call', help='Use one player capability, never the admin session')
    call.add_argument('--player', type=Path, required=True)
    call.add_argument('--wait-turn', type=float, default=0, help='Wait up to 30s for our idle turn before command')
    call.add_argument('command', nargs=argparse.REMAINDER)
    close = sub.add_parser('close', help='Gracefully close only this session and verify exit')
    close.add_argument('--session', type=Path, required=True)
    broker = sub.add_parser('_serve')
    broker.add_argument('--session', type=Path, required=True)
    args = parser.parse_args()
    if args.operation == '_serve':
        serve(args.session.resolve())
    elif args.operation == 'start':
        if os.name != 'nt':
            parser.error('This launcher currently stages Windows binaries')
        root = args.output.resolve()
        root.mkdir(parents=True, exist_ok=False)
        config = {'game_port': free_port(), 'api_port': free_port(), 'tokens': {r: secrets.token_urlsafe(24) for r in ('host', 'client')},
                  'admin_token': secrets.token_urlsafe(24), 'human_side': args.human_side, 'hidden': args.hidden,
                  'squad': args.squad, 'map': args.map}
        while config['game_port'] == config['api_port']:
            config['api_port'] = free_port()
        for role in ('host', 'client'):
            stage(args.package.resolve(), args.exe.resolve(), root, role, True)
            save_json(root / (role + '-player.json'), {'endpoint': f"http://127.0.0.1:{config['api_port']}/{config['tokens'][role]}"})
        session = root / 'session.json'
        save_json(session, config)
        flags = subprocess.CREATE_NO_WINDOW | subprocess.DETACHED_PROCESS
        with (root / 'broker.log').open('w') as log:
            process = subprocess.Popen([sys.executable, str(Path(__file__).resolve()), '_serve', '--session', str(session)],
                                       creationflags=flags, stdin=subprocess.DEVNULL, stdout=log, stderr=subprocess.STDOUT)
        deadline = time.monotonic() + 370
        while not (root / 'ready.json').exists():
            if (root / 'error.json').exists():
                raise RuntimeError((root / 'error.json').read_text())
            if process.poll() is not None or time.monotonic() >= deadline:
                raise RuntimeError('Broker did not become ready; inspect ' + str(root / 'broker.log'))
            time.sleep(.5)
        print(json.dumps({'session': str(session), 'host_player': str(root / 'host-player.json'),
                          'client_player': str(root / 'client-player.json'), 'ready': True}, indent=2))
    elif args.operation == 'call':
        capability = json.loads(args.player.read_text())
        print(json.dumps(request(capability['endpoint'], {'command': ' '.join(args.command), 'wait_turn': args.wait_turn}), indent=2))
    elif args.operation == 'close':
        path = args.session.resolve()
        config = json.loads(path.read_text())
        closed = path.parent / 'closed.json'
        if not closed.exists():
            request(f"http://127.0.0.1:{config['api_port']}/{config['admin_token']}", {'command': 'close'})
        deadline = time.monotonic() + 55
        while not closed.exists() and time.monotonic() < deadline:
            time.sleep(.2)
        if not closed.exists():
            raise TimeoutError('Session shutdown not confirmed')
        value = json.loads(closed.read_text())
        print(json.dumps(value, indent=2))
        if any(code != 0 for code in value['exit_codes'].values()):
            raise RuntimeError('A game did not exit cleanly')


if __name__ == '__main__':
    main()
