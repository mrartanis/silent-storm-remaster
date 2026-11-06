# Установка на Android

[English](android.en.md) · [Установка на компьютере](install.md) · [Главная](../README.md)

Нужен телефон или планшет ARM64 с Android 9 или новее и поддержкой Vulkan. Игра использует оригинальные текстуры.

1. Скачайте APK с окончанием `android-arm64.apk` из [последнего релиза](https://github.com/mrartanis/silent-storm-remaster/releases/latest) и установите его.
2. На компьютере найдите свою установленную копию **Silent Storm 1.2**. В Gold Edition выберите основную Silent Storm.
3. Установите [Android Platform Tools](https://developer.android.com/tools/releases/platform-tools), включите отладку по USB, подключите устройство и подтвердите подключение.
4. Откройте терминал в папке оригинальной игры и скопируйте ресурсы.

   ```sh
   adb shell mkdir -p /sdcard/Android/data/org.silentstorm.remaster/files
   adb push game.db start.cfg tree.mma cfg res scripts /sdcard/Android/data/org.silentstorm.remaster/files/
   ```

5. Откройте **Silent Storm** на устройстве. Для [сетевой игры](multiplayer.md) скопируйте в ту же папку `network-version.txt` из настольного архива той же версии.

## Управление

- Коснитесь цели, чтобы выбрать её. Удерживайте палец на карте, чтобы открыть контекстное меню.
- Перетащите карту одним пальцем, чтобы сдвинуть камеру. Двумя пальцами меняйте масштаб, поворачивайте камеру или ведите оба пальца вверх и вниз, чтобы изменить наклон.
- Кнопка **ALT** включает и выключает подсветку предметов на земле.
- При сворачивании игры или выключении экрана обновляется слот **«Автосохранение при сворачивании»**.

[Лицензии компонентов Android](../android/THIRD_PARTY.md)
