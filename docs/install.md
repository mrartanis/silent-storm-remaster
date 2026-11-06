# Установка на компьютере

[English](install.en.md)

## Установите движок

1. Откройте [последний релиз](https://github.com/mrartanis/silent-storm-remaster/releases/latest) и скачайте архив для своей платформы.
2. Найдите установленную **Silent Storm 1.2** из вашей копии Steam или GOG. В Gold Edition выберите основную Silent Storm. В Steam папку игры можно открыть через **Свойства → Установленные файлы → Обзор**.
3. Создайте отдельную папку для ремастера. Скопируйте в неё файлы `game.db`, `start.cfg`, `tree.mma` и папки `cfg`, `res`, `scripts` целиком.
4. Распакуйте содержимое папки из архива движка рядом с `game.db`. Перенесите все библиотеки, шрифты и `network-version.txt`.

| Платформа | Архив | Запуск |
| --- | --- | --- |
| Windows 10/11 x64 | `*-windows-x64.zip` | Откройте `Play.cmd`. |
| Linux x64, Ubuntu 24.04 или совместимая система с glibc 2.39+ | `*-linux-x64.tar.gz` | Выполните `./Play.sh` в папке игры. |
| macOS 15+ на Apple silicon | `*-macos-arm64.tar.gz` | Откройте `Play.command` в Finder. |

На Linux установите Vulkan-драйвер для своей видеокарты. В Ubuntu 24.04 установите также системные библиотеки.

```sh
sudo apt install libgsf-1-114 libpng16-16t64 zlib1g libx11-6 libxi6 libgl1 libvulkan1
```

## Добавьте HD-текстуры

Закройте игру. Установите Python 3.11+ и [7-Zip](https://www.7-zip.org/). Для загрузки и распаковки текущего пака нужно около 12 ГБ свободного места. Откройте терминал в папке игры. Если в архиве движка нет `prepare_hd.py`, сохраните туда [скрипт установки](https://raw.githubusercontent.com/mrartanis/silent-storm-remaster/HEAD/packaging/prepare_hd.py).

```powershell
py -3 prepare_hd.py --game-dir .
```

На Linux и macOS используйте `python3 prepare_hd.py --game-dir .`. Скрипт находит последний стабильный HD-релиз, скачивает все тома, сверяет контрольные суммы и устанавливает `res-hd`. Переключатель **HD текстуры** находится в настройках графики.

## Обновления и сохранения

Закройте игру и распакуйте новый архив движка поверх установленного, включая `network-version.txt`. Для обновления HD-пака снова запустите скрипт.

| Платформа | Сохранения и настройки |
| --- | --- |
| Windows | `%LOCALAPPDATA%\Silent Storm Reconstruction` |
| Linux | `$XDG_DATA_HOME/silent-storm-reconstruction` или `~/.local/share/silent-storm-reconstruction` |
| macOS | `~/Library/Application Support/Silent Storm Reconstruction` |

Переменная `S2_USER_DATA_DIR` задаёт другую папку сохранений и настроек.

Разрешение, режим окна, сглаживание и вертикальная синхронизация находятся в настройках графики. Масштаб интерфейса и векторные шрифты можно изменить в разделе **Экран и интерфейс**.
