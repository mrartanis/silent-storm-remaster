#pragma once

// Opt-in player-facing channel. The developer harness is never enabled by it.
namespace S2Agent {
bool Poll(); // false requests ordinary application shutdown
void Done(); // release borrowed world references before engine teardown
}
