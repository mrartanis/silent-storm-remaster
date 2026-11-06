# Agent guide

[Русский](README.md) · [Repository rules in Russian](../AGENTS.md)

- Work in the `silent-storm-remaster` repository. The main branch is `develop`. Check the remote, branch, and working changes when the workspace contains multiple checkouts. Treat Nival's sources, the former fork, and the installed retail game as references. Use a separate worktree for parallel changes.
- Preserve Silent Storm 1.2 behavior, balance, and resource and save compatibility. Reproduce bugs through the normal game path. When practical, use an existing automated scenario to verify the failure before the fix and the result afterward.
- Test on a copy of the game data with a separate `S2_USER_DATA_DIR` per process. Save useful screenshots and evidence, then close only the test processes you started and verify they exited. Leave a game open when the user asks. In LLM matches, follow the play skill and use only player-visible information.
- Regular builds reuse the frozen pack in `data/res-hd`. Restore PNGs and rebuild the pack only for an explicit authoring task. Publish the engine and HD pack separately. Players supply their own original game data.
- Maintain both Russian and English player guides in `docs/`. Keep the README short and use `releases/latest` or `releases` links.
- Keep test host addresses, personal paths, credentials, and private evidence out of Git. Use the ignored `.codeagents/local/` directory for machine settings. Check the staged files before committing.

| Task | Guide |
| --- | --- |
| Reproduce a bug, test graphics or networking, or measure frame times | [Game testing](testing.en.md) |
| Review and import upstream changes | [Import fixes](upstream.en.md) |
| Check a build, CI, a release, or the HD pack | [Builds and releases](releases.en.md) |
| Start a match with an agent or change the player tooling | [LLM play](agent-play.en.md) |

For example, keep the locations of the game data copy, build, and test output in `local/environment.md`. Keep passwords, keys, and tokens in your credential store.
