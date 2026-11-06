# Game testing

[Русский](testing.md) · [Task index](README.en.md)

1. Use the user's save or reproduce the conditions on a copy of the data. If the expected behavior is unclear, compare the same scene in the original Silent Storm 1.2. Use a separate profile and a fresh output directory.
2. Choose the smallest scenario that exercises the actual game logic. List registered tests with `ctest --test-dir <build> -N`. Set `S2_GAME_DIR` for runs that need retail assets. Tests independent of those assets must register without it. Confirm the scenario detects the intended defect before changing the code.
3. Repeat the scenario on the fixed build. Run the related regression checks and report the platform, configuration, scenario, and result. For visual defects, capture frames under the same conditions. If a manual run confirms correct behavior, check the inputs and expectations of the automated scenario.
4. Save the evidence outside the repository. Close only your own game processes, using `quit` for harness runs. Verify that the processes exited.

| Task | Tool |
| --- | --- |
| Behavior regressions | [UpstreamBugTests.cpp](../diagnostics/UpstreamBugTests.cpp) and [CMake test registrations](../CMakeLists.txt) |
| Replay input, events, and captures | [Stage6Scenario.py](../diagnostics/Stage6Scenario.py) and [New-LabRun.ps1](../diagnostics/New-LabRun.ps1) |
| Frame profiling and network frame drops | [ProfileFrames.py](../diagnostics/ProfileFrames.py) and [NetworkFPS.py](../diagnostics/NetworkFPS.py) |
| Local match | [NetworkMatchTests.cpp](../diagnostics/NetworkMatchTests.cpp) and [network harness](../diagnostics/NETWORK_MODE.md#проверки) |

For network changes, cover the affected host or client role, the transition from preparation to combat, and the ordinary quick match. Cover the campaign when changing shared game logic. The compatibility version comes from [network-version.txt](../network-version.txt).

For performance comparisons, keep the scene, camera, resolution, HD, and synchronization settings fixed. Compare frame times and long frames such as p99 and maximum with CPU, GPU, and replication activity. Report hardware GPU and software Vulkan measurements separately.
