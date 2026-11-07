# Bundled GOG extractor

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
