# LLM play

[Русский](agent-play.md) · [Task index](README.en.md)

For a match, use [silent-storm-play](../.agents/skills/silent-storm-play/SKILL.md) and the [launch guide](../docs/agent-play.en.md). Find the configured game data, engine, and output paths in local settings. Use a suitable existing build and a fresh session directory. Launch through binary arguments without adding a game menu item.

The coordinator keeps `session.json` and closes its own pair of processes. Give each player only its own `*-player.json` and use `call` for ordinary actions and observations. Give each side its own working directory and profile. The player channel provides screenshots, visible opponents, and the game's normal action previews. Game memory, other players' transcripts, saves, and the debug harness are outside that channel.

The launcher targets Windows with Python 3.11+. Create a virtual environment with `python -m venv .venv-agent` if needed. The launcher uses the standard library. The C++ channel is part of the `Game` target. Before launch, check that the executable, libraries, fonts, and `network-version.txt` come from the same build.

When changing the channel, check the [launcher](../tools/agent_play.py), [command handler](../Game/AgentPlay.cpp), and [AgentPlayTests.py](../diagnostics/AgentPlayTests.py). `-agent-play` is separate from `-harness`. `-agent-hidden` hides the window and keeps normal rendering. The HTTP broker listens only on loopback and issues a separate token per side. Actions use the game's ordinary validation. Confirm the result from observed state such as AP, ammo, or position.
