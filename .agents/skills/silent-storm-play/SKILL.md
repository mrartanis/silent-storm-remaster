---
name: silent-storm-play
description: Launch and play Silent Storm Remaster with a human or another LLM over the local network using fair player observations, screenshots, and normal game actions. Use for requests such as «поиграем вдвоём» or testing how an agent learns to play; not for game debugging or cheats.
---

Play through the restricted player channel. All setup uses binary parameters; no new game menu is needed. The Windows launcher starts an authoritative host and a rendered replica on the same computer, connects them automatically, and isolates their profiles from the user's saves.

## Find and launch

Find the checkout containing `tools/agent_play.py` and this repository skill at `.agents/skills/silent-storm-play/SKILL.md`. Run commands from that checkout. Use the configured paths to the user's game data and a suitable binary. Machine-specific settings can be stored in the ignored `.codeagents/local/` directory.

Check that the existing data and binary are usable. Keep the binary, runtime DLLs, fonts, and `network-version.txt` from the same build. The launcher uses the existing `data/res-hd` pack when the data directory lacks it. Setup and tooling details are in [.codeagents/agent-play.en.md](../../../.codeagents/agent-play.en.md).

For a human saying «поиграем вдвоём», run:

```powershell
python tools/agent_play.py start --package "GAME_DATA_DIR" --exe "PATH_TO_GAME_EXE" --output "NEW_SESSION_DIR" --squad 1
```

Use a fresh output directory. The host window belongs to the human; the agent controls `client-player.json`. Tell the human the match is connected and which side starts. A one-fighter squad is the default; `--squad 1..6` changes both squads. `--human-side client` reverses the visible window. `--map VARIANT` selects an existing random encounter; omit it to use the usual network default. Startup can take several minutes while checking resources. Inspect startup/process logs only for a failed launch, before playing.

For an expressly requested agent-versus-agent match, add `--hidden` and give each player only its own player capability file plus these instructions. If the user specifies a model, use it for the player agents. Neither player should see the other's observations or tactical messages. A coordinator may watch progress, but should not feed opponents' information to players.

The launcher returns paths for `session.json`, `host-player.json`, and `client-player.json`. Keep `session.json` for lifecycle management; it is not a player tool. If an already-running match and a player capability were provided, use those instead of launching another match.

## Player tools

```powershell
python tools/agent_play.py call --player "PLAYER_JSON" observe
python tools/agent_play.py call --player "PLAYER_JSON" screenshot
python tools/agent_play.py call --player "PLAYER_JSON" --wait-turn 20 observe
```

Every response contains `ok` and an `observation`. `phase` is `connecting`, `playing`, or `finished`. During play, `side` is yours, `turn` is the active side, and `executing` means an action is animating. `units` lists only your squad: zero-based `id`, grid/world coordinates, AP, HP, pose, ammo, shoot mode and pending command. `visible_enemies` contains only currently visible contacts; enemy HP is included only when the normal UI allows it. Contact IDs are local to this player. A contact disappears when no longer visible; remembered positions must come from your own previous observations.

Read screenshot images using your image viewing tool. The `screenshot` field is an absolute PNG path to your rendered view. Use `focus UNIT` to center the camera on one of your fighters before taking a picture.

| Command after `call --player PLAYER_JSON` | Meaning |
| --- | --- |
| `move UNIT GRID_X GRID_Y` | Move on the unit's current map layer; absolute grid coordinates |
| `look UNIT GRID_X GRID_Y` | Turn toward a grid coordinate |
| `attack UNIT CONTACT` | Attack a currently visible enemy contact with the held weapon |
| `shoot UNIT WORLD_X WORLD_Y WORLD_Z` | Shoot/throw at a world point, including a remembered location |
| `reload UNIT` | Reload through the normal inventory/ammo rules |
| `mode UNIT MODE` | 0 snap, 1 aimed, 2 careful, 3 short burst, 4 long burst; weapon support/AP apply |
| `pose UNIT POSE` | 0 prone, 1 crouch, 2 stand |
| `hide UNIT 0_OR_1` | Normal stealth action |
| `slot UNIT SLOT` | Activate an equipped inventory slot |
| `equip UNIT BACKPACK_ITEM SLOT` | Exchange a backpack item into a slot and activate it; use current backpack indices |
| `heal UNIT OWN_TARGET_UNIT` | Normal first aid with the held medical item |
| `continue UNIT` | Resume a pending movement/order on a later turn |
| `stop` | Stop your current action |
| `end_turn` | Pass the turn normally |
| `focus UNIT` | Select and center your own fighter |
| `camera DX DY YAW_DELTA ZOOM_DELTA` | Pan in world units, rotate in radians, adjust camera distance |
| `floor -3..4` | Change the normal cutaway floor |
| `hover PIXEL_X PIXEL_Y` | Move the in-game cursor; inspect the resulting ordinary tooltip with a screenshot |
| `click PIXEL_X PIXEL_Y` / `right_click PIXEL_X PIXEL_Y` | Normal pointer clicks for objects and existing UI; hover first and then click on a later call |

Prefix an action with `preview_` (e.g. `preview_move 0 30 40` or `preview_attack 0 1`) to ask the game's ordinary AP/rule preview. Attack preview also returns the ordinary hit chance. The replica may first return result 21 (`pending`); retry the same preview after a short interval. Common result codes: 0 allowed, 5 insufficient AP, 6 no path, 7 reload needed, 8 missing equipment, 9 jammed, 11 out of range, 19 attack replaced by reload, 20 cannot see target. A submitted action still goes through authoritative game validation. Observe its actual AP/ammo/position afterward; `submitted` alone does not prove it succeeded.

Your unit's `slots`, `active_slot`, `backpack` and `supported_modes` describe only your own equipment. To throw a grenade or heal, equip the appropriate backpack item into a hand slot, then `shoot` at a world point or `heal` an owned fighter. Slot numbers come from the returned inventory, and backpack indices can change after an exchange.

Grid and world coordinates are different. Use the fields returned by observations for their respective commands. Movement can stop on AP exhaustion with `command:true`; continue next turn or issue a new destination. Explore using small steps, camera focus, and screenshots; account for walls, terrain and facing. Do not keep repeating a rejected command unchanged. If you cannot act, end your turn. While waiting, use `--wait-turn 20 observe` instead of busy polling. If your shell tool yields a running session, await/poll that session to retrieve its JSON output before sending another request.

## Fair play boundary

During play use only your capability through `call` (or its equivalent HTTP POST). Do not read the game process, source, world dumps, save files, `_saveload.log`, debug harness, other player directories, session secrets, network traces, or another player's transcript/screenshots. Do not invoke Lua, `netstatus`, debug commands, teleportation, AP/HP/RNG edits, or hidden-target probes. `-agent-play` does not enable the developer harness and rejects its activation. This is an API information boundary, not an OS sandbox against a malicious local process.

The player HTTP capability accepts POST JSON `{"command":"observe","settle":true,"wait_turn":0}` at the endpoint in its player file. It binds only to loopback. Each role is independently serialized. Actions normally wait for animation to settle before returning; observations and previews remain read-only apart from ordinary selection/preview caches. Transcripts are saved separately per role for review after the match.

## Finish

When `phase:finished`, record the public `status` (winner/draw/failure), report what you learned, and do not issue tactical actions. Before closing, keep useful screenshots. Unless the user asked to leave the game running, the coordinator closes only its own session:

```powershell
python tools/agent_play.py close --session "SESSION_JSON"
```

Require both exit codes to be zero. Preserve logs/captures/transcripts. Never close an unrelated game. If a process fails to exit, report its owned session and the failure instead of killing arbitrary game processes.
