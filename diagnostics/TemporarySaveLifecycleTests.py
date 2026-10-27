"""Exercise the real game startup/normal-exit lifetime with isolated save files."""
import argparse
import os
from pathlib import Path
import subprocess
import sys


def link_input(source, target, directory=False):
    if target.exists():
        if not os.path.samefile(source, target):
            raise RuntimeError(f"fixture input does not refer to its shared source: {target}")
        return
    if directory and os.name == "nt":
        env = os.environ.copy()
        env["S2_TEST_RESOURCE_LINK"] = str(target)
        env["S2_TEST_RESOURCE_TARGET"] = str(source)
        subprocess.run(["powershell.exe", "-NoProfile", "-NonInteractive", "-Command",
            "$ErrorActionPreference='Stop'; New-Item -ItemType Junction "
            "-Path $env:S2_TEST_RESOURCE_LINK -Target $env:S2_TEST_RESOURCE_TARGET | Out-Null"],
            env=env, check=True)
    elif directory:
        target.symlink_to(source, target_is_directory=True)
    else:
        try:
            os.link(source, target)
        except OSError:
            target.symlink_to(source)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--game", required=True, type=Path)
    parser.add_argument("--database", required=True, type=Path)
    parser.add_argument("--resources", required=True, type=Path)
    parser.add_argument("--work", required=True, type=Path)
    parser.add_argument("--case", choices=["normal", "startup-failure"], required=True)
    args = parser.parse_args()
    game, database, resources, work = args.game.resolve(), args.database.resolve(), args.resources.resolve(), args.work.resolve()
    if not game.is_file() or not database.is_file() or not resources.is_dir():
        raise RuntimeError("native game/shared database/resource inputs are unavailable")
    if not work.name.startswith("upstream-temp-save-"):
        raise RuntimeError("fixture work directory must have its dedicated test name")
    cwd, user = work / "game", work / "user"
    (cwd / "cfg").mkdir(parents=True, exist_ok=True)
    user.mkdir(parents=True, exist_ok=True)
    profile = user / "save" / "lifecycle-test"
    temporary = profile / "temp"
    (temporary / "nested").mkdir(parents=True, exist_ok=True)
    (profile / "named-save").mkdir(parents=True, exist_ok=True)
    (temporary / "snapshot.bin").write_bytes(b"owned temporary snapshot")
    (temporary / "nested" / "snapshot.bin").write_bytes(b"owned nested snapshot")
    named = profile / "named-save" / "game.sav"
    named_payload = b"named save must survive ordinary game shutdown"
    named.write_bytes(named_payload)
    (user / ".legacy-save-imported").write_bytes(b"fixture suppresses legacy import")
    for name in ["startup.log", "network.log"]:
        (user / name).unlink(missing_ok=True)
    (cwd / "cfg" / "autoexec.cfg").write_text('game_profile "lifecycle-test"\n', encoding="ascii")
    (cwd / "cleanup-empty.cfg").write_text("// isolated intermission; quit through the native harness\n", encoding="ascii")
    if args.case == "normal":
        link_input(database, cwd / "game.db")
        link_input(resources, cwd / "res", directory=True)
        (cwd / "_harness_cmd.txt").write_text("quit\n", encoding="ascii")
    elif (cwd / "game.db").exists():
        raise RuntimeError("startup-failure fixture unexpectedly contains a game database")
    env = os.environ.copy()
    env["S2_USER_DATA_DIR"] = str(user)
    env["S2_AUDIO_HEADLESS"] = "1"
    env["PATH"] = str(game.parent) + os.pathsep + str(game.parent.parent / "_implibs") + os.pathsep + env.get("PATH", "")
    command = [str(game), "-harness", "-harness-active", "-nosound", "-windowed", "-640", "-cfg", "cleanup-empty.cfg"]
    with (work / "process.log").open("wb") as output:
        child = subprocess.Popen(command, cwd=cwd, env=env, stdout=output, stderr=subprocess.STDOUT)
        try:
            code = child.wait(timeout=60)
        except subprocess.TimeoutExpired:
            (cwd / "_harness_cmd.txt").write_text("quit\n", encoding="ascii")
            try:
                child.wait(timeout=5)
            except subprocess.TimeoutExpired:
                child.terminate()
                try:
                    child.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    child.kill()
                    child.wait()
            raise RuntimeError("native game failed to complete its ordinary startup/quit lifetime")
    startup = (user / "startup.log").read_text(encoding="utf-8", errors="replace")
    if named.read_bytes() != named_payload:
        raise AssertionError("native shutdown modified a named save")
    if args.case == "startup-failure":
        if code != 1 or "database-ready" in startup or "normal-exit" in startup:
            raise RuntimeError(f"fixture did not take the intended early database failure: exit={code}")
        if not (temporary / "snapshot.bin").is_file():
            raise AssertionError("failed startup removed the previous temporary snapshot")
        print("native early startup failure: previous temporary snapshot and named save retained")
        return 0
    harness = (cwd / "_saveload.log").read_text(encoding="utf-8", errors="replace")
    if code != 0 or "loading-screen-ready" not in startup or "normal-exit" not in startup or "cmd: quit" not in harness:
        raise RuntimeError(f"fixture did not complete native initialized harness shutdown: exit={code}")
    print(f"native normal quit: exit={code} temporaryExists={int(temporary.exists())} namedSaveUnchanged=1", flush=True)
    if temporary.exists():
        raise AssertionError("temporary save workspace survives a successful native game shutdown")
    print("native temporary snapshots removed after ordinary game quit; named save retained")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, RuntimeError, AssertionError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        sys.exit(1)
