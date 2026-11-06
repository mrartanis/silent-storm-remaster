# Play against an LLM agent

[Русский](agent-play.md) · [Multiplayer](multiplayer.en.md) · [Home](../README.en.md)

The [silent-storm-play skill](../.agents/skills/silent-storm-play/SKILL.md) runs a local match with two game instances. The agent sees game screenshots and information about visible opponents and plays using the standard game commands.

1. [Build the Windows x64 engine](build.en.md#windows-x64) and install Python 3.11+.
2. Prepare a folder with the [original game data](install.en.md#install-the-engine). Place `res-hd` in that folder or prepare the [HD cache in the repository](hd-authoring.en.md).
3. Start a match from the repository root. Use your own paths to the game data and engine and choose a new directory for the session.

```powershell
python tools/agent_play.py start `
  --package C:/Games/SilentStorm-data `
  --exe build/Release/Game.exe `
  --output C:/Games/SilentStorm-match `
  --squad 1
```

By default, the human plays in the host window and the agent controls the client. Use `--human-side client` to swap sides or `--hidden` for a match between two agents.

Use `--starting-level 1..20` and `--supply-level 1..14` to set the fighter and supply levels. Levels above one or `--prepare` enable the preparation stage, when both sides choose equipment and perks. For two fighters at the maximum levels, use `--squad 2 --starting-level 20 --supply-level 14`. The agent marks its own side ready with `ready`, and the human marks theirs ready in the game.

Close both game instances when the match ends.

```powershell
python tools/agent_play.py close --session C:/Games/SilentStorm-match/session.json
```
