#!/usr/bin/env python3
"""Run the recovered executable where the game's own files are.

`build/OpenSMACX.exe` is the artifact; `.opensmacx/game/` is the install it is
a recovery OF - 653 files of `alpha.txt`, `.pcx` art, `.cvr` movies and the
shipped `terranx_original.exe` beside them. The executable reads those by
relative path, so running it out of the build directory is running it without
its data, and every failure after the first would be about the wrong thing.

    tools/run_recovered.py                 # build/OpenSMACX.exe in the game dir
    tools/run_recovered.py --exe path.exe  # some other build

WHY IT STAGES RATHER THAN JUST SETTING A WORKING DIRECTORY. Wine resolves a
DLL from the EXECUTABLE'S directory first, and VC6 links the C and C++
runtimes dynamically with no static option its own libraries agree on - so the
artifact imports MSVCRTD.DLL and MSVCP60D.DLL from a Debug build, neither of
which is redistributable and neither of which is in any prefix here. Without
them the process dies before its first instruction with wine's exit 53 and no
diagnostic beyond `c0000135`, which reads exactly like the recovered code
faulting immediately. For the one artifact whose job is to say how far the
recovery reaches, that is the worst possible failure mode.

So the executable and the runtimes it needs are copied in beside the data, and
the run happens there. Copies, not links: the game directory is gitignored
working state and this only ever adds to it.

WHAT SUCCESS LOOKS LIKE, TODAY. Not a game. The entry point is the recovered
`_WinMain@16`, and six of the eleven functions it calls are still reached
through `src/pending_bodies.cpp` - forwarders that jump to an address meaning
nothing in this process. So the expected outcome is a page fault inside the
first of them, and the address it faults at is the measurement: it names the
frontier. Measured 2026-08-15: `0x0062D3C2`, inside `jackal_init_real`, after
the command-line copy, the popup-allocator stores, both `prefs_get` calls,
`GetSystemMetrics` and `jackal_version_check` have all run.
"""

from __future__ import annotations

import argparse
import re
import shutil
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import byte_match  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
GAME_DIR = REPO_ROOT / ".opensmacx" / "game"
DEFAULT_EXE = REPO_ROOT / "build" / "OpenSMACX.exe"

# Both runtime models, because which pair the artifact imports depends on the
# build type and asking the PE is cheaper than reasoning about it.
CRT_DIRS = ("DEBUG", "REDIST")
FAULT = re.compile(r"page fault on (?P<how>[\w ]+) to (?P<target>[0-9A-Fa-f]+) "
                   r"at address (?P<address>[0-9A-Fa-f]+)")


def imported_runtimes(exe: Path) -> list:
    """The MSVC runtime DLLs `exe` imports, asked of the PE rather than guessed."""
    import pefile
    image = pefile.PE(str(exe))
    return [entry.dll.decode() for entry in image.DIRECTORY_ENTRY_IMPORT
            if entry.dll.decode().upper().startswith("MSVC")]


def stage(exe: Path, game: Path) -> list:
    staged = [shutil.copy(exe, game / exe.name)]
    for name in imported_runtimes(exe):
        for folder in CRT_DIRS:
            source = byte_match.VC6_ROOT / folder / name.upper()
            if source.is_file():
                staged.append(shutil.copy(source, game / name.upper()))
                break
        else:
            print(f"run-recovered: {name} is imported and was not found under "
                  f"{byte_match.VC6_ROOT}; the run will fail to start",
                  file=sys.stderr)
    return staged



# CLion, and any other IDE with a "Remote Debug" configuration, needs the port
# written into the configuration BEFORE the process exists. winedbg takes one.
GDB_PORT = 12345


def debug(exe: Path, game: Path, port: int) -> int:
    """Start the executable under winedbg's gdb proxy on a FIXED port.

    `--port` IS ORDER-SENSITIVE, AND THAT COST AN HOUR. It must come after
    `--gdb`, and it takes its value as a separate argument:

        winedbg --gdb --port 12345 --no-start <exe>   binds 0.0.0.0:12345
        winedbg --gdb --no-start --port=12345 <exe>   usage message
        winedbg --port 12345 --gdb --no-start <exe>   usage message

    The usage message winedbg prints on the last two names none of its
    options, so a rejected `--port` looks exactly like a winedbg that does not
    support one - and `WINEDBG_PORT` and `WINE_GDB_PORT` are both ignored,
    which makes the wrong conclusion easy to reach and easy to keep. It was
    reached here, and a TCP relay was written to bridge a fixed port onto the
    random one winedbg was assumed to insist on. wine 9.0 needs no such thing.

    NO SOURCE-LEVEL DEBUGGING, and it is worth knowing before you try: the
    executable carries a CodeView debug directory pointing at a PDB, which gdb
    cannot read, so it reports `(No debugging symbols found)` and every frame
    is `?? ()`. What you get is the assembly, the registers and the fault
    address - which for a matching decompilation is most of what there is to
    want, because the ADDRESS is the catalogue key. `describe()` turns one
    into a function name.
    """
    windows_path = subprocess.run(
        ["winepath", "-w", str((game / exe.name).resolve())],
        env=byte_match.wine_environment(), capture_output=True,
        text=True).stdout.strip() or exe.name
    print(f"  gdb remote on localhost:{port}")
    print(f"  attach with: target remote localhost:{port}")
    server = subprocess.run(
        ["winedbg", "--gdb", "--port", str(port), "--no-start", windows_path],
        cwd=game, env={**byte_match.wine_environment(), "WINEDEBUG": "-all"})
    return server.returncode


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--game", type=Path, default=GAME_DIR)
    parser.add_argument("--timeout", type=int, default=120)
    parser.add_argument("--gdb", action="store_true",
                        help="start it under winedbg's gdb proxy and bridge "
                             f"that to localhost:{GDB_PORT}, so an IDE's "
                             "remote-debug configuration can hold a fixed "
                             "port")
    parser.add_argument("--port", type=int, default=GDB_PORT)
    arguments = parser.parse_args()

    if not arguments.exe.is_file():
        print(f"run-recovered: {arguments.exe} is absent; build it first",
              file=sys.stderr)
        return 2
    if not arguments.game.is_dir():
        print(f"run-recovered: {arguments.game} is absent, so there is no "
              f"game data to run against", file=sys.stderr)
        return 2

    staged = stage(arguments.exe, arguments.game)
    print(f"staged {len(staged)} file(s) into {arguments.game}")
    if arguments.gdb:
        return debug(arguments.exe, arguments.game, arguments.port)
    try:
        done = subprocess.run(["wine", arguments.exe.name],
                              cwd=arguments.game,
                              env=byte_match.wine_environment(),
                              capture_output=True, text=True,
                              timeout=arguments.timeout)
    except subprocess.TimeoutExpired:
        print(f"still running after {arguments.timeout}s - which is further "
              f"than a page fault, and worth looking at")
        return 0

    output = done.stdout + done.stderr
    print(f"exit {done.returncode}")
    fault = FAULT.search(output)
    if fault:
        address = int(fault.group("address"), 16)
        print(f"  page fault at 0x{address:08X} ({fault.group('how').strip()} "
              f"to 0x{fault.group('target')})")
        print(f"  {describe(address)}")
    for line in output.splitlines():
        if "err:module" in line:
            print(f"  {line.strip()}")
    return done.returncode


def describe(address: int) -> str:
    """Which catalogued function `address` lands in, and its recovery state."""
    try:
        import annotation_scan
        rows = byte_match.load_rows()
    except Exception as error:                       # pragma: no cover
        return f"(cannot name it: {error})"
    import emit_translation_unit as emit
    for at, row in rows.items():
        for low, high in emit.parse_body_ranges(row.get("body_ranges") or ""):
            if low <= address <= high:
                landed = {a.address: a for a in
                          annotation_scan.scan_tree(REPO_ROOT / "src")}
                note = landed.get(at)
                where = f", {note.path}" if note else ", not in the map"
                return (f"inside 0x{at:08X} {row.get('name', '')}{where} - "
                        f"reached through a src/pending_bodies.cpp forwarder "
                        f"if it is not compiled in")
    return "outside every catalogued span"


if __name__ == "__main__":
    sys.exit(main())
