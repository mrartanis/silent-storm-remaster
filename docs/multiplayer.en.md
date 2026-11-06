# Multiplayer

[Русский](multiplayer.md) · [Home](../README.en.md)

Use the same engine release, game data, and active mods on both devices. Keep `network-version.txt` alongside `game.db`. The HD texture setting does not affect compatibility.

1. On both devices, open **Network Game**.
2. On the host, choose the map, preparation mode, and port, then select **Create**.
3. On the second device, enter the host's IPv4 address and the same port, then select **Connect**. The host must accept TCP connections on that port. The default is `7780`. Use `127.0.0.1` for two instances on the same computer.

In preparation mode, each player selects squad members, equipment, and perks at their own base. The host sets the squad size from 1 to 6, starting level from 1 to 20, and supply level from 1 to 14. The battle starts when both players select **Ready**. To start a quick match, turn off preparation when creating the match.

[Play against an LLM agent](agent-play.en.md)
