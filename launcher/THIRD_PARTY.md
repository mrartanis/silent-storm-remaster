# Bundled archive tools

## GOG installer import

The desktop launcher includes innoextract 1.9 by Daniel Scharrer:
https://constexpr.org/innoextract/

innoextract is distributed under the zlib/libpng license. Its copyright and
license, together with the bundled libraries' notices, are included in
`innoextract/license` inside the launcher package. `innoextract/BUILD.json`
records the build inputs. Original Windows and Linux binaries are downloaded
from the author's website with pinned SHA-256 hashes. The macOS executable is
the Homebrew build, including its upstream compatibility patches; its shared
libraries are collected and relocated into the application by PyInstaller.

No GOG installer or original game data is included in the launcher.

## HD archive extraction

The launcher includes official, unmodified standalone 7-Zip 26.06 executables
by Igor Pavlov: https://www.7-zip.org/

7-Zip is distributed under the GNU LGPL 2.1 or later, with the additional
notices specified by its upstream license. The applicable notices are included
in `sevenzip/license`. The complete matching source archive is provided in
`sevenzip/source/7z2606-src.tar.xz`, including its build instructions and licenses.
`sevenzip/BUILD.json` records the official download hashes and launcher source
commit. Windows uses standalone `7za.exe`; Linux and macOS use `7zz`.
PyInstaller collects runtime dependencies and selects the native macOS
architecture when packaging the app. These utilities are separate processes;
they are not linked into the launcher.
