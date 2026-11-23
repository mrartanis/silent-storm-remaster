# Frozen HD sources / Замороженные исходники HD

## Русский

Семь ZIP-архивов с PNG опубликованы в отдельном релизе
[assets-hd-sources-v0.1](https://github.com/mrartanis/silent-storm-remaster/releases/tag/assets-hd-sources-v0.1).
Это материалы для редактирования HD, а не готовый игровой пак. Для игры и обычной
сборки движка они не нужны. Архивы занимают 12,54 GiB и содержат 20 307 уникальных
изображений, восстанавливаемых по 27 707 исходным путям.

Из корня копии репозитория, с Python 3.11+:

```sh
python packaging/restore_hd_sources.py --download --verify-only
python packaging/restore_hd_sources.py
```

Скачивание происходит только с `--download`; существующие ZIP повторно не
скачиваются. Проверяются размеры, SHA-256 архивов и каждого PNG. Изменённые PNG
не перезаписываются. ZIP и восстановленные PNG игнорируются Git. Можно скачать
ZIP вручную и использовать `--archive-dir /путь/к/архивам`; для восстановления
PNG в отдельную папку укажите `--root /путь/к/рабочей/папке`.

`manifest.json` сохраняет исходные пути и контрольные суммы. `release.json`
закрепляет релиз и SHA-256 манифеста. Промпты и метаданные остаются в
`assets/terrain-hd`. Условия использования приведены в `LICENSE.md` в репозитории
и в `HD-LICENSE.md` среди файлов релиза.

## English

The seven PNG ZIP archives are published in the separate
[assets-hd-sources-v0.1 release](https://github.com/mrartanis/silent-storm-remaster/releases/tag/assets-hd-sources-v0.1).
These are HD authoring materials, not the ready-to-play pack. They are not needed
to play or build the engine normally. The archives total 12.54 GiB and contain
20,307 unique images restored to 27,707 original paths.

From the root of a repository checkout, using Python 3.11+:

```sh
python packaging/restore_hd_sources.py --download --verify-only
python packaging/restore_hd_sources.py
```

Downloads require `--download`; existing ZIPs are reused. Archive sizes and the
SHA-256 hashes of each archive and PNG are checked. Modified PNGs are never
overwritten. Git ignores downloaded ZIPs and restored PNGs. You can download the
ZIPs manually and pass `--archive-dir /path/to/archives`; use `--root /work/folder`
to restore the PNGs outside the checkout.

`manifest.json` preserves original paths and checksums. `release.json` pins the
release and manifest SHA-256. Prompts and metadata remain in `assets/terrain-hd`.
See the repository's `LICENSE.md`, also attached to the release as `HD-LICENSE.md`,
for the applicable terms.
