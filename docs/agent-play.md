# Игра с LLM-агентом

[English](agent-play.en.md) · [Сетевая игра](multiplayer.md) · [Главная](../README.md)

Навык [silent-storm-play](../.agents/skills/silent-storm-play/SKILL.md) запускает локальный матч на двух экземплярах игры. Агент видит кадры игры, получает сведения о видимых противниках и действует обычными игровыми командами.

1. [Соберите движок для Windows x64](build.md#windows-x64) и установите Python 3.11+.
2. Подготовьте папку с [оригинальными данными](install.md#установите-движок). Положите в неё `res-hd` или подготовьте [кеш HD в репозитории](hd-authoring.md).
3. Из корня репозитория запустите матч. Укажите свои пути к данным и движку и выберите новый каталог для сессии.

```powershell
python tools/agent_play.py start `
  --package C:/Games/SilentStorm-data `
  --exe build/Release/Game.exe `
  --output C:/Games/SilentStorm-match `
  --squad 1
```

По умолчанию человек играет в окне хоста, агент управляет клиентом. Укажите `--human-side client`, чтобы поменять стороны, или `--hidden` для матча двух агентов.

Закройте оба экземпляра по окончании матча.

```powershell
python tools/agent_play.py close --session C:/Games/SilentStorm-match/session.json
```
