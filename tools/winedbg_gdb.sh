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
# so there is no readiness wait.
trap 'reap_wine' EXIT
trap 'exit 143' TERM INT
"$WINEDBG_BIN" --gdb --no-start --port "$PORT" "$EXE" &
wait $! || true
