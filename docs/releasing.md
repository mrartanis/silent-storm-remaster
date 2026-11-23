# Публикация релизов

[English](releasing.en.md) · [Главная](../README.md)

## Движок

1. Поставьте на коммит тег, начинающийся с `v`, и отправьте тег в репозиторий. Дефис в теге обозначает предварительный релиз.
2. Дождитесь завершения [Engine CI and release](../.github/workflows/engine-release.yml). Workflow соберёт и проверит три настольных архива движка, Android APK и три архива [лаунчера](launcher.md). Все файлы публикуются в одном релизе вместе с `SHA256SUMS`.
3. Проверьте файлы и ссылки на инструкции на [странице релизов](https://github.com/mrartanis/silent-storm-remaster/releases). Если загрузка остановилась на черновике, перезапустите workflow.

Чтобы получить проверочные архивы, запустите вручную **Engine CI and release** для движка или **Desktop launcher** для лаунчера. Архивы будут доступны в результатах запуска.

При несовместимых изменениях сетевого формата или репликации увеличьте номер протокола во второй строке [network-version.txt](../network-version.txt). Третья строка обозначает семейство игровых данных.

## HD-пак

1. Подготовьте утверждённые текстуры по [инструкции по восстановлению исходников](hd-authoring.md). Установите Python 3.11+, Pillow и 7-Zip.
2. Выполните команды из корня репозитория. Используйте новую папку для сборки и замените `vNEXT` будущей версией.

```powershell
python assets/terrain-hd/build_pack.py --output ../hd-release-stage/res-hd
python assets/terrain-hd/validate_pack.py --pack ../hd-release-stage/res-hd --no-update-queue
Copy-Item LICENSE.md ../hd-release-stage/HD-LICENSE.md
Set-Location ../hd-release-stage
7z a -t7z -mx=9 -m0=Delta:4 -m1=LZMA2:d=1024m:fb=273:mf=bt4:mc=1024 -ms=on -mmt=2 -mtm=off -v1900m silent-storm-hd-vNEXT.7z 'res-hd/Textures*.res' res-hd/manifest.json HD-LICENSE.md
7z t silent-storm-hd-vNEXT.7z.001
```

3. Создайте индекс `hd-vNEXT-index.json` по [примеру формата](../data/hd-runtime/hd-v0.1-index.json). Укажите тег, имена и размеры томов, SHA-256 томов, манифеста и файлов `res-hd`. Сверьте контрольные суммы с распакованным архивом и подготовьте `SHA256SUMS`.
4. Создайте черновик с тегом `hd-vNEXT` в `mrartanis/silent-storm-remaster`. Загрузите все тома, индекс, лицензию и `SHA256SUMS`. Опубликуйте стабильный релиз с `--latest=false`, чтобы ссылка на последний релиз вела к движку.

## Исходники HD

Замороженные PNG публикуются отдельно от игрового пака, в релизах `assets-hd-sources-*`.
Не используйте префикс `hd-`: установщик считает такие релизы готовыми игровыми паками.

1. Сохраните резервную копию всех ZIP, проверьте их размеры, SHA-256 и содержимое по `data/hd-sources/manifest.json`. Команда `python packaging/restore_hd_sources.py --archive-dir /путь/к/архивам --verify-only` проверяет каждый PNG без восстановления.
2. Оставьте манифест с путями и контрольными суммами в Git. В `data/hd-sources/release.json` закрепите имя репозитория, тег релиза и SHA-256 манифеста. ZIP должны быть меньше 2 GiB каждый и не должны попадать в Git или LFS.
3. Создайте черновик релиза `assets-hd-sources-vNEXT`. Загрузите все ZIP, `manifest.json`, `release.json`, двуязычный `HD-SOURCES-README.md` из `data/hd-sources/README.md`, `HD-LICENSE.md` из `LICENSE.md` и `SHA256SUMS`. Включите в контрольные суммы все файлы, кроме самого `SHA256SUMS`.
4. Сверьте размеры и SHA-256 загруженных файлов через Releases API по ID релиза (`/repos/{owner}/{repo}/releases/{id}`): поиск черновика по тегу может вернуть 404. Затем опубликуйте стабильный релиз с `--latest=false`. Проверьте скачивание и восстановление до удаления старых указателей LFS.

Удаление указателей или правил LFS не освобождает серверную квоту: старые объекты остаются в хранилище GitHub. Для их очистки обратитесь в GitHub Support; не удаляйте репозиторий ради очистки. Сохраните проверенную резервную копию исходников.
