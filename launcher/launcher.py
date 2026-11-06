"""Silent Storm Remaster desktop launcher."""

import argparse
import json
from pathlib import Path
import sys
from threading import Event, Thread
from time import monotonic

# Source and packaged builds both use the engine's HD index helpers.
if not getattr(sys, "frozen", False):
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "packaging"))

from PySide6.QtCore import QLocale, QLockFile, QSize, QStandardPaths, Qt, QTimer, QUrl, Signal
from PySide6.QtGui import QColor, QDesktopServices, QIcon, QLinearGradient, QPainter, QPixmap
from PySide6.QtWidgets import (
    QApplication, QCheckBox, QFileDialog, QFrame, QHBoxLayout, QInputDialog, QLabel, QMessageBox,
    QProgressBar, QPushButton, QVBoxLayout, QWidget,
)

import discovery
import install


HERE = Path(__file__).resolve().parent
ART = HERE / "embers.webp"
RELEASE_PAGE = install.REPOSITORY + "/releases"


class Hero(QFrame):
    def __init__(self):
        super().__init__()
        self.art = QPixmap(str(ART))
        self.setMinimumHeight(236)

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.SmoothPixmapTransform)
        painter.fillRect(self.rect(), QColor("#0d0908"))
        if not self.art.isNull():
            start = round(self.width() * 0.28)
            area = QSize(self.width() - start, self.height())
            photo = self.art.scaled(area, Qt.AspectRatioMode.KeepAspectRatioByExpanding,
                                    Qt.TransformationMode.SmoothTransformation)
            painter.drawPixmap(start + (area.width() - photo.width()) // 2,
                               round((area.height() - photo.height()) * 0.60), photo)
        shade = QLinearGradient(0, 0, self.width(), 0)
        shade.setColorAt(0.0, QColor("#0d0908"))
        shade.setColorAt(0.28, QColor("#0d0908"))
        shade.setColorAt(0.37, QColor(13, 9, 8, 221))
        shade.setColorAt(0.55, QColor(13, 9, 8, 6))
        shade.setColorAt(1.0, QColor(13, 9, 8, 4))
        painter.fillRect(self.rect(), shade)
        painter.setPen(QColor("#624425"))
        painter.drawRect(self.rect().adjusted(0, 0, -1, -1))


class Launcher(QWidget):
    succeeded = Signal(object)
    failed = Signal(object)
    progress = Signal(str, int, object)

    def __init__(self, root, russian):
        super().__init__()
        self.root = root
        self.russian = russian
        self.target = install.desktop_target()
        self.config = root / "settings.json"
        self.settings = self.read_settings()
        data = self.settings.get("data")
        self.data = Path(data) if isinstance(data, str) and data else None
        self.engine_release = None
        self.hd_release = None
        self.job = None
        self.game = None
        self.cancelled = Event()
        install.recover_swap(root / "engine", shared="res-hd")
        install.recover_swap(root / "engine" / "res-hd")
        self.setWindowTitle("Silent Storm Remaster")
        self.setWindowIcon(QIcon(str(HERE / "insignia.svg")))
        self.setMinimumSize(880, 685)
        self.resize(1032, 704)
        self.setStyleSheet((HERE / "field-desk.qss").read_text(encoding="utf-8"))
        self.build()
        self.succeeded.connect(self.finish)
        self.failed.connect(self.failure)
        self.progress.connect(self.show_progress)
        self.timer = QTimer(self, interval=750)
        self.timer.timeout.connect(self.watch_game)
        self.refresh()
        if self.autocheck.isChecked():
            QTimer.singleShot(0, self.check)

    def t(self, ru, en):
        return ru if self.russian else en

    def read_settings(self):
        try:
            settings = json.loads(self.config.read_text(encoding="utf-8"))
            return settings if isinstance(settings, dict) else {}
        except (OSError, ValueError):
            return {}

    def save_settings(self):
        temporary = self.config.with_suffix(".tmp")
        temporary.write_text(json.dumps({
            "data": str(self.data) if self.data else "",
            "autocheck": self.autocheck.isChecked(),
            "windowed": self.windowed.isChecked(),
        }, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
        temporary.replace(self.config)

    def save_preferences(self):
        try:
            self.save_settings()
        except OSError as error:
            self.show_error(error)

    def label(self, text, name=""):
        label = QLabel(text)
        label.setTextFormat(Qt.TextFormat.PlainText)
        label.setObjectName(name)
        return label

    def button(self, text, action, name=""):
        button = QPushButton(text)
        button.setObjectName(name)
        button.setCursor(Qt.CursorShape.PointingHandCursor)
        button.clicked.connect(action)
        return button

    def build(self):
        tr = self.t
        layout = QVBoxLayout(self)
        layout.setContentsMargins(24, 15, 24, 18)
        layout.setSpacing(12)
        top = QHBoxLayout()
        crest = self.label("")
        crest.setPixmap(QIcon(str(HERE / "insignia.svg")).pixmap(30, 30))
        top.addWidget(crest)
        top.addSpacing(4)
        top.addWidget(self.label(tr("ПОЛЕВОЙ ШТАБ", "FIELD HEADQUARTERS"), "masthead"))
        top.addStretch()
        top.addWidget(self.label(self.target.label if self.target else tr("Другая платформа", "Other platform"), "platform"))
        top.addSpacing(8)
        top.addWidget(self.button(tr("Релизы GitHub ↗", "GitHub releases ↗"),
                                  lambda: QDesktopServices.openUrl(QUrl(RELEASE_PAGE)), "link"))
        layout.addLayout(top)

        hero = Hero()
        hero_layout = QVBoxLayout(hero)
        hero_layout.setContentsMargins(24, 17, 23, 15)
        hero_layout.setSpacing(1)
        hero_layout.addStretch()
        hero_layout.addWidget(self.label("NIVAL  /  2003", "heroKicker"))
        hero_layout.addWidget(self.label("SILENT STORM", "title"))
        hero_layout.addWidget(self.label("R E M A S T E R", "remaster"))
        hero_layout.addStretch()
        layout.addWidget(hero, 1)

        cards = QHBoxLayout()
        cards.setSpacing(12)
        data_card = QFrame(objectName="card")
        data_layout = QVBoxLayout(data_card)
        data_layout.setContentsMargins(16, 11, 16, 11)
        data_layout.setSpacing(6)
        data_layout.addWidget(self.label(tr("01   /   РЕСУРСЫ", "01   /   RESOURCES"), "eyebrow"))
        data_layout.addWidget(self.label(tr("Игровые данные", "Game data"), "section"))
        data_layout.addWidget(self.label(tr("Папка оригинальной Silent Storm 1.2",
                                            "Original Silent Storm 1.2 folder"), "muted"))
        self.data_path = self.label("", "path")
        self.data_path.setTextInteractionFlags(Qt.TextInteractionFlag.TextSelectableByMouse)
        self.data_path.setWordWrap(True)
        data_layout.addWidget(self.data_path)
        data_layout.addStretch()
        folder_row = QHBoxLayout()
        self.browse = self.button(tr("Выбрать папку…", "Choose folder…"), self.choose_data)
        folder_row.addWidget(self.browse)
        self.find_button = self.button(tr("Найти игру", "Find game"), self.find_data)
        self.find_button.setToolTip(tr("Поиск Silent Storm в Steam и GOG", "Find Silent Storm in Steam and GOG"))
        folder_row.addWidget(self.find_button)
        self.windowed = QCheckBox(tr("Запуск в окне", "Windowed mode"))
        self.windowed.setChecked(bool(self.settings.get("windowed")))
        self.windowed.toggled.connect(self.save_preferences)
        folder_row.addStretch()
        folder_row.addWidget(self.windowed)
        data_layout.addLayout(folder_row)
        cards.addWidget(data_card, 5)

        downloads_card = QFrame(objectName="card")
        downloads = QVBoxLayout(downloads_card)
        downloads.setContentsMargins(16, 11, 16, 11)
        downloads.setSpacing(6)
        downloads.addWidget(self.label(tr("02   /   СНАБЖЕНИЕ", "02   /   SUPPLY"), "eyebrow"))
        for label, status_attr, action_attr, action in (
            (tr("Движок", "Engine"), "engine_status", "engine_button", self.engine_action),
            (tr("HD-текстуры", "HD textures"), "hd_status", "hd_button", self.hd_action),
        ):
            row = QHBoxLayout()
            info = QVBoxLayout()
            info.setSpacing(2)
            info.addWidget(self.label(label, "component"))
            status = self.label("", "muted")
            setattr(self, status_attr, status)
            info.addWidget(status)
            row.addLayout(info, 1)
            button = self.button("", action)
            button.setMinimumWidth(122)
            setattr(self, action_attr, button)
            row.addWidget(button)
            downloads.addLayout(row)
        downloads.addStretch()
        self.autocheck = QCheckBox(tr("Проверять при запуске", "Check on startup"))
        self.autocheck.setChecked(bool(self.settings.get("autocheck", True)))
        self.autocheck.toggled.connect(self.save_preferences)
        downloads.addWidget(self.autocheck)
        cards.addWidget(downloads_card, 4)
        layout.addLayout(cards)

        operation = QHBoxLayout()
        self.status = self.label(tr("Проверка обновлений доступна через GitHub.",
                                    "Release checks are available through GitHub."), "status")
        self.status.setWordWrap(True)
        operation.addWidget(self.status, 1)
        self.progress_bar = QProgressBar()
        self.progress_bar.setFixedWidth(144)
        self.progress_bar.hide()
        operation.addWidget(self.progress_bar)
        self.cancel_button = self.button(tr("Отменить", "Cancel"), self.cancelled.set, "link")
        self.cancel_button.hide()
        operation.addWidget(self.cancel_button)
        self.check_button = self.button(tr("Проверить обновления", "Check for updates"), self.check)
        operation.addWidget(self.check_button)
        layout.addLayout(operation)

        footer = QHBoxLayout()
        footer.addWidget(self.button(tr("Сохранения ↗", "Saves ↗"), self.open_saves, "link"))
        self.log_button = self.button(tr("Журнал ↗", "Game log ↗"), self.open_log, "link")
        footer.addWidget(self.log_button)
        self.install_folder = self.button(tr("Папка установки ↗", "Installation folder ↗"),
                                          self.open_installation, "link")
        footer.addWidget(self.install_folder)
        footer.addStretch()
        self.play = self.button(tr("▶   Играть", "▶   Play"), self.play_game, "primary")
        self.play.setMinimumSize(195, 49)
        footer.addWidget(self.play)
        layout.addLayout(footer)

    def refresh(self):
        tr = self.t
        engine = self.root / "engine"
        engine_tag = install.release_at(engine)
        hd_tag = install.release_at(engine / "res-hd")
        engine_ready = bool(self.target and (engine / self.target.executable).is_file())
        hd_ready = (engine / "res-hd" / "Textures.res").is_file()
        data_ready = not install.missing_data(self.data)
        idle = self.job is None
        changing = self.job in ("engine", "hd")
        playing = self.game is not None
        online_engine = self.engine_release["tag_name"] if self.engine_release else ""
        online_hd = self.hd_release["tag_name"] if self.hd_release else ""
        engine_update = install.version(online_engine) > install.version(engine_tag)
        hd_update = online_hd and online_hd != hd_tag
        absent = tr("Не установлен", "Not installed")
        self.engine_status.setText(
            tr("Платформа не поддерживается", "Unsupported platform") if not self.target
            else (engine_tag or tr("Установлен", "Installed")) if engine_ready else absent)
        if online_engine and (not engine_ready or engine_update):
            self.engine_status.setText(f"{self.engine_status.text()}  ·  {online_engine}")
        self.engine_button.setText(
            tr("Скачать", "Download") if not engine_ready else
            tr("Обновить", "Update") if engine_update else tr("Восстановить", "Repair"))
        self.engine_button.setEnabled(idle and not playing and bool(online_engine)
                                      and install.version(online_engine) >= install.version(engine_tag))
        local_hd = hd_tag.removeprefix("hd-") or tr("Установлен", "Installed")
        self.hd_status.setText(local_hd if hd_ready else absent)
        if hd_update:
            self.hd_status.setText(f"{self.hd_status.text()}  ·  {online_hd.removeprefix('hd-')}")
        self.hd_button.setText(
            tr("Скачать", "Download") if not hd_ready else
            tr("Обновить", "Update") if hd_update else tr("Проверить", "Verify"))
        self.hd_button.setEnabled(idle and not playing and engine_ready and bool(online_hd))
        self.hd_button.setToolTip(tr("Для распаковки нужен 7-Zip. Переключатель HD есть в игре.",
                                    "Extraction requires 7-Zip. Toggle HD in the game settings."))
        path = str(self.data) if self.data else tr("Папка пока не выбрана", "No folder selected")
        self.data_path.setText(path)
        self.data_path.setToolTip(path)
        self.browse.setEnabled(not playing and not changing and self.job != "discover")
        self.find_button.setVisible(sys.platform == "win32" and self.data is None)
        self.find_button.setEnabled(idle and not playing)
        self.check_button.setEnabled(idle)
        self.play.setEnabled(engine_ready and data_ready and not playing and not changing)
        self.play.setText(tr("Игра запущена", "Game running") if playing else tr("▶   Играть", "▶   Play"))
        self.play.setToolTip(
            tr("Выберите папку оригинальной игры.", "Choose the original game folder.") if not data_ready
            else tr("Скачайте движок.", "Download the engine.") if not engine_ready else "")
        self.log_button.setEnabled((self.root / "game.log").is_file())
        self.install_folder.setEnabled(not changing)
        self.cancel_button.setVisible(changing)
        self.progress_bar.setVisible(changing)

    def choose_data(self):
        folder = QFileDialog.getExistingDirectory(self, self.t("Папка Silent Storm 1.2", "Silent Storm 1.2 folder"),
                                                   str(self.data or Path.home()))
        if not folder:
            return
        self.set_data(Path(folder).resolve(), self.t("Игровые данные найдены.", "Game data found."))

    def set_data(self, data, message):
        if data.is_relative_to(self.root):
            self.show_error(self.t("Выберите оригинальную игру вне папки лаунчера.",
                                   "Choose the original game outside launcher storage."))
            return
        if missing := install.missing_data(data):
            self.show_error(self.t("В папке отсутствуют", "The folder is missing") + "\n" + ", ".join(missing))
            return
        self.data = data
        self.save_preferences()
        self.status.setText(message)
        self.refresh()

    def find_data(self):
        if self.data is None and self.job is None:
            self.run_job("discover", discovery.find_windows_data,
                         self.t("Ищу Silent Storm в Steam и GOG…", "Looking for Silent Storm in Steam and GOG…"))

    def apply_found_data(self, matches):
        matches = [(path, store) for path, store in matches if not path.is_relative_to(self.root)]
        if not matches:
            self.status.setText(self.t("Копия Steam или GOG не найдена. Выберите папку вручную.",
                                       "No Steam or GOG copy found. Choose the folder manually."))
            return
        index = 0
        if len(matches) > 1:
            items = [f"{store}  ·  {path}" for path, store in matches]
            choice, accepted = QInputDialog.getItem(
                self, "Silent Storm Remaster",
                self.t("Выберите установленную копию.", "Choose an installed copy."),
                items, 0, False,
            )
            if not accepted:
                self.status.setText(self.t("Папка пока не выбрана.", "No folder selected."))
                return
            index = items.index(choice)
        path, store = matches[index]
        self.set_data(path, self.t(f"Найдена Silent Storm ({store}).", f"Silent Storm found ({store})."))

    def run_job(self, name, action, message):
        self.job = name
        self.status.setText(message)
        self.status.setToolTip("")
        self.cancelled.clear()
        self.progress_bar.setRange(0, 0)
        self.cancel_button.setEnabled(True)
        self.refresh()

        def work():
            try:
                self.succeeded.emit(action())
            except Exception as error:
                self.failed.emit(error)

        Thread(target=work, daemon=True).start()

    def check(self):
        self.run_job("check", lambda: install.check_releases(self.target),
                     self.t("Проверяю релизы GitHub…", "Checking GitHub releases…"))

    def begin_install(self, name, action):
        last_update = 0

        def report(phase, done=0, total=0):
            nonlocal last_update
            now = monotonic()
            if not total or done in (0, total) or now - last_update >= 0.1:
                self.progress.emit(phase, int(100 * done / total) if total else -1, (done, total))
                last_update = now

        def cancel():
            if self.cancelled.is_set():
                raise install.Cancelled()

        self.run_job(name, lambda: action(report, cancel),
                     self.t("Подготовка загрузки…", "Preparing download…"))

    def engine_action(self):
        self.begin_install("engine", lambda report, cancel:
                           install.install_engine(self.root, self.target, self.engine_release, report, cancel))

    def hd_action(self):
        self.begin_install("hd", lambda report, cancel:
                           install.install_hd(self.root, self.hd_release, report, cancel))

    def show_progress(self, phase, percent, byte_counts):
        if phase == "download":
            done, total = byte_counts
            self.progress_bar.setRange(0, 100)
            self.progress_bar.setValue(percent)
            self.status.setText(self.t("Загрузка", "Downloading") +
                                f"   {done / 1048576:,.1f} / {total / 1048576:,.1f} MiB")
        else:
            self.progress_bar.setRange(0, 0)
            self.status.setText(self.t("Распаковка архива…", "Extracting archive…") if phase == "extract"
                                else self.t("Проверка файлов HD…", "Verifying HD files…"))

    def finish(self, result):
        job = self.job
        self.job = None
        if job == "check":
            self.engine_release, self.hd_release = result
            self.status.setText(self.t("Релизы GitHub проверены.", "GitHub releases checked."))
        elif job == "engine":
            self.status.setText(self.t("Движок готов к запуску.", "Engine ready to launch."))
        elif job == "hd":
            self.status.setText(self.t("HD-пак готов. Переключатель находится в настройках графики.",
                                       "HD pack ready. Toggle HD in the game’s graphics settings."))
        elif job == "discover":
            self.apply_found_data(result)
        self.refresh()

    def failure(self, error):
        job = self.job
        self.job = None
        self.refresh()
        if isinstance(error, install.Cancelled):
            self.status.setText(self.t("Загрузка отменена.", "Download cancelled."))
        elif job == "check":
            self.status.setText(self.t("GitHub недоступен. Можно запустить установленную игру.",
                                       "GitHub is unavailable. An installed game can still be played."))
            self.status.setToolTip(str(error))
        elif job == "discover":
            self.status.setText(self.t("Поиск не удался. Выберите папку вручную.",
                                       "Search failed. Choose the folder manually."))
            self.status.setToolTip(str(error))
        else:
            self.status.setText(self.t("Установка прервана.", "Installation interrupted."))
            self.show_error(error)

    def show_error(self, error):
        dialog = QMessageBox(QMessageBox.Icon.Warning, "Silent Storm Remaster", str(error),
                             QMessageBox.StandardButton.Ok, self)
        dialog.setTextFormat(Qt.TextFormat.PlainText)
        dialog.exec()

    def open_saves(self):
        path = install.saves_root()
        try:
            path.mkdir(parents=True, exist_ok=True)
            QDesktopServices.openUrl(QUrl.fromLocalFile(str(path)))
        except OSError as error:
            self.show_error(error)

    def open_log(self):
        QDesktopServices.openUrl(QUrl.fromLocalFile(str(self.root / "game.log")))

    def open_installation(self):
        engine = self.root / "engine"
        path = engine if engine.is_dir() else self.root
        if not QDesktopServices.openUrl(QUrl.fromLocalFile(str(path))):
            self.show_error(self.t("Не удалось открыть папку установки.", "Could not open the installation folder."))

    def play_game(self):
        if install.missing_data(self.data):
            self.refresh()
            self.show_error(self.t("Проверьте путь к игровым данным.", "Check the game data folder."))
            return
        try:
            self.game = install.start_game(self.root, self.target, self.data, self.windowed.isChecked())
        except OSError as error:
            self.show_error(error)
            return
        self.status.setText(self.t("Silent Storm запущена.", "Silent Storm is running."))
        self.timer.start()
        self.refresh()

    def watch_game(self):
        if (code := self.game.poll()) is not None:
            self.game = None
            self.timer.stop()
            self.status.setText(self.t("Игра закрыта.", "Game closed.") if code == 0
                                else self.t(f"Игра завершилась с кодом {code}. Откройте журнал.",
                                            f"Game exited with code {code}. Open the game log."))
            self.refresh()

    def closeEvent(self, event):
        if self.job in ("engine", "hd") or self.game:
            self.status.setText(self.t("Завершите игру или отмените установку перед выходом.",
                                       "Close the game or cancel installation before exiting."))
            event.ignore()
        else:
            event.accept()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--storage", type=Path, help="launcher files, engine and HD pack")
    parser.add_argument("--language", choices=("ru", "en"))
    parser.add_argument("--smoke-test", type=Path, metavar="LOG", help=argparse.SUPPRESS)
    args = parser.parse_args()
    app = QApplication([sys.argv[0]])
    app.setApplicationName("Silent Storm Launcher")
    app.setStyle("Fusion")
    russian = (args.language == "ru" if args.language else
               QLocale.system().language() == QLocale.Language.Russian)
    if args.smoke_test:
        from smoke import smoke_test
        return smoke_test(app, Launcher, Hero, args.smoke_test)
    root = (args.storage or Path(QStandardPaths.writableLocation(
        QStandardPaths.StandardLocation.AppLocalDataLocation))).resolve()
    root.mkdir(parents=True, exist_ok=True)
    lock = QLockFile(str(root / "launcher.lock"))
    lock.setStaleLockTime(0)
    if not lock.tryLock(0):
        QMessageBox.information(None, "Silent Storm Remaster",
                                "Лаунчер уже открыт." if russian else "The launcher is already open.")
        return 1
    try:
        window = Launcher(root, russian)
        window.show()
        return app.exec()
    finally:
        lock.unlock()


if __name__ == "__main__":
    sys.exit(main())
