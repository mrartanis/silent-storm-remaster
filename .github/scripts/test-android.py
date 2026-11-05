"""Exercise SDL input and Vulkan surface recreation on Android via adb."""
import argparse
import struct
import subprocess
import time


def until(description, predicate, timeout=20):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        result = predicate()
        if result:
            return result
        time.sleep(0.15)
    raise RuntimeError(f'Timed out waiting for {description}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('apk')
    parser.add_argument('--serial')
    args = parser.parse_args()
    adb = ['adb'] + (['-s', args.serial] if args.serial else [])
    package = 'org.silentstorm.remaster.probe'
    activity = package + '/org.libsdl.app.SDLActivity'

    def run(*argv):
        return subprocess.check_output([*adb, *argv], timeout=35)

    def pid():
        result = subprocess.run([*adb, 'shell', 'pidof', package], timeout=10,
                                check=False, capture_output=True, text=True)
        return result.stdout.strip()

    def count(message):
        return run('logcat', '-d', f'--pid={original_pid}', '-s', 'SDL').count(message.encode())

    def presented():
        frame = run('exec-out', 'screencap')
        if len(frame) < 16:
            return None
        width, height, pixel_format = struct.unpack_from('<III', frame)
        header_size = len(frame) - width * height * 4
        if width <= height or height == 0 or pixel_format not in (1, 2) or header_size not in (12, 16):
            return None
        x, y = width // 2, height // 2
        offset = header_size + 4 * (y * width + x)
        color = frame[offset:offset + 3]
        if all(abs(got - want) < 20 for got, want in zip(color, (0x25, 0x31, 0x4B))):
            return x, y
        return None

    if pid():
        raise RuntimeError('Close the existing probe before running the smoke test')
    run('install', '-r', args.apk)
    original_pid = None
    try:
        run('shell', 'am', 'start', '-W', '-n', activity)
        original_pid = until('probe process', pid)
        until('native Vulkan startup', lambda: count('bgfx renderer=Vulkan'))
        x, y = until('the first rendered Vulkan frame', presented)
        run('shell', 'input', 'tap', str(x), str(y))
        until('tap in SDL', lambda: count('Android probe tap') == 1)
        with subprocess.Popen([*adb, 'shell', 'input', 'swipe', str(x), str(y), str(x), str(y), '2400']) as hold:
            until('context hold before finger release',
                  lambda: hold.poll() is None and count('Android probe hold') == 1, timeout=4)
            assert hold.wait(timeout=5) == 0
        assert count('Android probe tap') == 1, 'Long press also triggered a tap'

        run('shell', 'input', 'keyevent', 'KEYCODE_HOME')
        until('Android background transition', lambda: count('Android probe background') == 1)
        time.sleep(2)
        run('shell', 'am', 'start', '-W', '-n', activity)
        until('Android foreground transition', lambda: count('Android probe resumed') == 1)
        assert pid() == original_pid, 'Probe restarted on resume'
        until('a visible Vulkan frame after resume', presented)
        run('shell', 'input', 'tap', str(x), str(y))
        until('touch works after resume', lambda: count('Android probe tap') == 2)
        print('Android Vulkan, touch, hold and resume smoke passed')
    finally:
        if original_pid and pid() == original_pid:
            run('shell', 'input', 'keyevent', 'KEYCODE_BACK')
            until('normal probe exit', lambda: count('Android probe normal exit'))


if __name__ == '__main__':
    main()
