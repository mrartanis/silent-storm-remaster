import sys
from pathlib import Path

from PyInstaller.utils.hooks import copy_metadata

here = Path(SPECPATH)
repo = here.parent
analysis = Analysis(
    [str(here / 'launcher.py')],
    pathex=[str(repo / 'packaging')],
    datas=[
        (str(here / 'field-desk.qss'), '.'),
        (str(here / 'insignia.svg'), '.'),
        (str(here / 'embers.webp'), '.'),
        (str(repo / 'LICENSE.md'), '.'),
    ] + copy_metadata('PySide6-Essentials') + copy_metadata('shiboken6'),
)
if sys.platform == 'win32':
    # Qt uses the ICU C API shipped with Windows. An unrelated ICU found
    # on the build machine's PATH can shadow it with versioned exports.
    analysis.binaries = [
        item for item in analysis.binaries
        if not (Path(item[0]).name.lower().startswith('icu') and item[0].lower().endswith('.dll'))
    ]
pyz = PYZ(analysis.pure)
exe = EXE(pyz, analysis.scripts, exclude_binaries=True, name='SilentStormLauncher',
          console=False, upx=False)
dist = COLLECT(exe, analysis.binaries, analysis.datas, upx=False, name='SilentStormLauncher')
if sys.platform == 'darwin':
    bundle = BUNDLE(dist, name='SilentStormLauncher.app',
                    bundle_identifier='io.github.mrartanis.silentstorm.launcher')
