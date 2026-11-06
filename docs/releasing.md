# Публикация релизов

[English](releasing.en.md) · [Главная](../README.md)

## Движок

1. Поставьте на коммит тег, начинающийся с `v`, и отправьте тег в репозиторий. Дефис в теге обозначает предварительный релиз.
2. Дождитесь завершения [Engine CI and release](../.github/workflows/engine-release.yml). Workflow соберёт три настольных архива и Android APK, проверит их и опубликует вместе с `SHA256SUMS`.
3. Проверьте файлы и ссылки на инструкции на [странице релизов](https://github.com/mrartanis/silent-storm-remaster/releases). Если загрузка остановилась на черновике, перезапустите workflow.

Чтобы собрать архивы без публикации, запустите **Engine CI and release** вручную для нужной ветки. Архивы будут доступны в результатах запуска.

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
