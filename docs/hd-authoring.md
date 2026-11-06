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

Архивы в `data/hd-sources` содержат PNG. Манифест сопоставляет их исходным путям и содержит контрольные суммы. Точные промпты и метаданные хранятся в [assets/terrain-hd](../assets/terrain-hd).

```sh
git lfs pull --include="data/hd-sources/*" --exclude=""
python packaging/restore_hd_sources.py --verify-only
python packaging/restore_hd_sources.py
```

Восстановленные PNG игнорируются Git. Для восстановления в отдельную папку укажите `--root /абсолютный/путь`. Готовый пак обновляется отдельно по [инструкции по публикации HD](releasing.md#hd-пак).
