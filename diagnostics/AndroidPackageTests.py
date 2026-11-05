"""Verify the Android game APK as shipped, without a device or retail data."""
import argparse
import subprocess
from zipfile import ZipFile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('apk')
    parser.add_argument('--aapt', required=True)
    parser.add_argument('--version', required=True)
    args = parser.parse_args()
    badging = subprocess.check_output([args.aapt, 'dump', 'badging', args.apk], text=True)
    for expected in (
        "package: name='org.silentstorm.remaster'",
        f"versionName='{args.version}'",
        "minSdkVersion:'28'",
        "targetSdkVersion:'36'",
        "native-code: 'arm64-v8a'",
        "launchable-activity: name='org.libsdl.app.SDLActivity'",
        'application-icon-',
    ):
        assert expected in badging, (expected, badging)
    assert 'application-debuggable' not in badging
    resources = subprocess.check_output([args.aapt, 'dump', 'resources', args.apk], text=True)
    assert 'mipmap/ic_launcher' in resources
    assert 'drawable/silent_storm_icon' in resources
    with ZipFile(args.apk) as apk:
        paths = set(apk.namelist())
        for name in ('libmain.so', 'libSDL3.so', 'libc++_shared.so'):
            with apk.open(f'lib/arm64-v8a/{name}') as library:
                header = library.read(20)
                assert header[:6] == b'\x7fELF\x02\x01' and header[18:20] == b'\xb7\x00', name
        assert any(path.startswith('assets/fonts/') and path.endswith('.ttf') for path in paths)
        assert 'assets/licenses/THIRD_PARTY.md' in paths
        assert 'assets/licenses/ffmpeg-8.1/COPYING.LGPLv2.1' in paths
        assert not any(path.startswith('assets/') and
                       (path.endswith(('.res', '.db', '.mma')) or '/res-hd/' in path)
                       for path in paths)
    print('Android release package verified')


if __name__ == '__main__':
    main()
