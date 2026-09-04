#!/usr/bin/env bash
# winedbg_gdb.sh - run winedbg's GDB stub in the foreground for a debugger to
# connect to, surviving the ways wine fights its caller: a stripped IDE
# environment, the start.exe re-exec that orphans the stub when the launcher
# is killed, and the suspended debuggee that keeps the exe locked afterwards.
#
# CLion wiring (2026.2 debug profiles): Settings > Debugger > Debug Profiles >
# Add > GDB Server, then pick it in the toolbar "Debug | <profile>" switcher
# and debug the ordinary CMake Application configuration.
#   GDB Server tab   Command line:
#                      <repo>/tools/winedbg_gdb.sh <build-dir>/OpenSMACX.exe
#                    (exe falls back to WINEDBG_EXE from the environment)
#                    Working directory: whatever the debuggee needs - it opens
#                    its assets by relative path, and winedbg's child inherits
#                    it (.opensmacx/game for OpenSMACX).
#   Device Settings  Upload executable to device: Never. Reset command:
#                    cleared - the stub has no `monitor reset`.
#   Debugger tab     Connection: Custom script -
#                      file <build-dir>/OpenSMACX.sym
#                      target remote localhost:31337
#                    Custom, not Automatic: symbols must come from the ELF
#                    sidecar (tools/gdb_sidecar.py), never from the VC6 PE the
#                    CMake configuration would otherwise hand gdb. Persistent
#                    session: off - the stub dies with its first client.
#
# Manual terminal debugging: run this script in one terminal, then in another:
#   gdb -ex "file <build-dir>/OpenSMACX.sym" -ex "target remote localhost:31337"
#
# Env: WINEDBG_EXE (unless passed as the argument), WINEDBG_PORT (31337),
#   WINEPREFIX (~/opt/vc6/.wineprefix), WINEDBG_BIN (winedbg from PATH, else
#   /usr/bin/winedbg), WINEDBG_KILL_STALE (1).
set -euo pipefail

die() { echo "winedbg_gdb: $*" >&2; exit 2; }

# IDE-spawned processes get a stripped environment: resolve winedbg with a
# system fallback and default the prefix.
WINEDBG_BIN="${WINEDBG_BIN:-$(command -v winedbg || echo /usr/bin/winedbg)}"
PORT="${WINEDBG_PORT:-31337}"
EXE="${1:-${WINEDBG_EXE:-}}"
export WINEPREFIX="${WINEPREFIX:-$HOME/opt/vc6/.wineprefix}"

[ -n "$EXE" ] || die "no exe: pass it as the argument or export WINEDBG_EXE"
[ -f "$EXE" ] || die "$EXE not found"

# winedbg cannot start a RELATIVE path that has a directory component -
# "build/OpenSMACX.exe" from the repo root dies with "Couldn't start
# process" while ./OpenSMACX.exe from inside build/ works - so hand it an
# absolute, symlink-resolved path. No cd here: the debuggee must inherit
# the CALLER's working directory for its relative asset paths.
EXE="$(cd -- "$(dirname -- "$EXE")" && pwd -P)/$(basename -- "$EXE")"
[ -f "$EXE" ] || die "could not resolve $1 to an absolute path"

[ -d "$WINEPREFIX" ] || die "WINEPREFIX $WINEPREFIX does not exist"
[ -x "$WINEDBG_BIN" ] || die "winedbg not found on PATH or at /usr/bin/winedbg (set WINEDBG_BIN)"

# Wine re-execs winedbg through `start.exe /exec winedbg.exe ...`, so killing
# the unix launcher (what CLion does at session end) leaves the wine tree and
# the suspended debuggee alive - and the debuggee keeps the exe locked, which
# fails the next relink with LNK1104. Reap by pattern instead: `winedbg.exe`
# as well as the unix launcher, and the debuggee by exact cmdline (-x, since
# a substring match would also hit this script, whose argv carries the path).
reap_wine() {
    pkill -f "winedbg(\.exe)? --gdb.*--port $PORT" 2>/dev/null || true
    pkill -xf -- "$EXE" 2>/dev/null || true
}

# Reap a stub orphaned by a previous crashed session, then make sure the port
# is actually free before binding it. The probe connection is harmless here:
# a stale stub dies on its first client hanging up, which is the goal.
port_open() { (exec 3<>"/dev/tcp/127.0.0.1/$PORT") 2>/dev/null; }
if [ "${WINEDBG_KILL_STALE:-1}" = 1 ]; then
    reap_wine
fi
for _ in $(seq 1 30); do
    if port_open; then sleep 0.1; else break; fi
done
if port_open; then
    die "port $PORT still busy after reaping; run: pkill -f 'winedbg(\.exe)? --gdb'"
fi

# Stay in the foreground as the process the caller kills, and take the whole
# wine tree along on the way out. The caller owns connecting and its retries,
# so there is no readiness wait - but a winedbg that dies before it could
# serve anything (bad exe path, prefix problems) used to exit this script 0,
# which reads as success to an IDE profile with no server behind it.
#
# wine 10.0's dbghelp page-faults, and then crashes winedbg outright, parsing
# this PE's lone debug-directory entry (CodeView NB10 -> the VC6 PDB) -
# deterministic "winedbg: Internal crash" before the stub ever serves. gdb's
# symbols come from the ELF sidecar, never the PE, so the entry is dead
# weight: run a scratch copy with the debug data directory zeroed. Code
# bytes are untouched; WINEDBG_KEEP_PE_DEBUG=1 opts out.
EXE_TO_RUN="$EXE"
if [ "${WINEDBG_KEEP_PE_DEBUG:-0}" != 1 ]; then
    EXE_TO_RUN="$(mktemp --suffix=.exe "${TMPDIR:-/tmp}/winedbg_gdb.XXXXXX")"
    python3 - "$EXE" "$EXE_TO_RUN" <<'PYEOF'
import struct, sys
src, dst = sys.argv[1], sys.argv[2]
d = bytearray(open(src, "rb").read())
pe = struct.unpack_from("<I", d, 0x3C)[0]
opt = pe + 24
magic = struct.unpack_from("<H", d, opt)[0]
if magic not in (0x10B, 0x20B):
    sys.exit(f"not a PE optional header: magic 0x{magic:x}")
dd = opt + (96 if magic == 0x10B else 112) + 6 * 8  # data dir 6 = debug
struct.pack_into("<II", d, dd, 0, 0)
open(dst, "wb").write(d)
PYEOF
    [ -s "$EXE_TO_RUN" ] || die "failed to write the scrubbed exe copy"
fi
trap 'rm -f -- "$EXE_TO_RUN" 2>/dev/null; reap_wine' EXIT
trap 'exit 143' TERM INT
"$WINEDBG_BIN" --gdb --no-start --port "$PORT" "$EXE_TO_RUN" &
STUB=$!
sleep 1
if ! kill -0 "$STUB" 2>/dev/null; then
    wait "$STUB" || true
    die "winedbg exited before serving (see its output above); if it printed 'Couldn't start process', the exe path could not be launched"
fi
wait "$STUB" || true
