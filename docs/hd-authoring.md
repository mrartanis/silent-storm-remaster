# HD-текстуры для сборки и разработки

[English](hd-authoring.en.md) · [Сборка](build.md) · [Установка для игры](install.md#добавьте-hd-текстуры)

## Подготовьте готовый пак для CMake

Установите Git LFS, Python 3.11+ и 7-Zip. Выполните команды из корня репозитория.

```sh
git lfs install
git lfs pull --include="data/hd-runtime/*" --exclude=""
python packaging/prepare_hd.py
```

Скрипт проверит архивы и распакует их в `data/res-hd`. При `S2_BUILD_HD_TEXTURES=ON` CMake добавляет пак рядом с движком и повторно использует кеш. На одном диске файлы кеша и сборки могут быть жёсткими ссылками на одни и те же данные. Изменяйте исходники и пересобирайте пак в отдельной папке.

## Восстановите исходники для работы над текстурами

Семь ZIP-архивов с PNG хранятся в отдельном релизе [assets-hd-sources-v0.1](https://github.com/mrartanis/silent-storm-remaster/releases/tag/assets-hd-sources-v0.1), а не в Git LFS. Они занимают 12,54 GiB и нужны только для работы над текстурами, не для игры или обычной сборки движка. Манифест в `data/hd-sources` сопоставляет PNG исходным путям и содержит контрольные суммы. `release.json` закрепляет релиз и SHA-256 манифеста. Точные промпты и метаданные хранятся в [assets/terrain-hd](../assets/terrain-hd).

```sh
python packaging/restore_hd_sources.py --download --verify-only
python packaging/restore_hd_sources.py
```

Скрипт скачивает недостающие архивы только с `--download`, кеширует их в `data/hd-sources` и проверяет размеры, SHA-256 ZIP и каждого PNG. Без `--download` сеть не используется. Можно скачать ZIP вручную или использовать резервную копию: `python packaging/restore_hd_sources.py --archive-dir /путь/к/архивам --verify-only`. Повреждённый ZIP не перезаписывается автоматически; переместите его в сторону перед повторным скачиванием.

Скачанные ZIP и восстановленные PNG игнорируются Git. Изменённые PNG не перезаписываются. Для восстановления в отдельную папку укажите `--root /абсолютный/путь`. Готовый пак обновляется отдельно по [инструкции по публикации HD](releasing.md#hd-пак).
