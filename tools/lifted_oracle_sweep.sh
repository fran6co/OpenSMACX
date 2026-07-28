#!/bin/sh
# Sweeps the whole plan through the differential oracle, surviving hangs.
#
# A case can hang the harness outright, in a way neither guard covers: the
# original side runs with the whole process address space in reach, so a seeded
# pointer can land on the HOST's stack and overwrite the registers the runner
# saved there. The harness then returns into its own code with corrupted
# callee-saved registers and spins. Nothing inside the process can defend
# against that, because the damage is already done by the time control returns.
#
# So the supervision is external: watch the report stop growing, kill the run,
# record the function it died on as HANG, and start again after it. Everything
# already measured is kept - the report is appended to, not rewritten.
#
# THERE IS A SECOND WAY A RUN DIES AND IT IS NOT A HANG. This host is arm64 and
# Wine is an x86_64 Mach-O under Rosetta 2, and Rosetta sometimes refuses an
# instruction outright:
#
#     rosetta error: unsupported privilege level: 0
#
# The process is gone before any guard runs, no guest fault occurred, and no
# report row is written. Since the plan's blocking flags became a scheduling
# hint rather than a refusal (see lifted_oracle_main.cpp), the sweep attempts
# functions that were previously never executed, and some of them land on this.
# Measured over the first 40 `iat`-flagged functions: 5 die this way, each
# reproducible on its own with --only. Scoring it HANG would say the harness
# watched the run make no progress; it never got to watch anything. So the log
# segment is checked and the row is written as KILLED-host-refused, which is in
# no evidence set anywhere and is classified `no-evidence` by
# tools/lifted_oracle_mutate.py.
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
WINE="${WINE:-/Applications/Wine Staging.app/Contents/Resources/wine/bin/wine}"
EXE="${EXE:-$ROOT/.opensmacx/game/terranx_original.exe}"
PLAN="${PLAN:-$ROOT/build/oracle/plan.tsv}"
REPORT="${REPORT:-$ROOT/build/oracle/report.tsv}"
LOG="${LOG:-$ROOT/build/oracle/sweep.log}"
# Seconds without a new report line before a kill. 45 was enough when a
# function was four cases; at sixteen, a function whose original loops forever
# costs sixteen watchdog periods, and 45 turned slow-but-healthy functions into
# false HANG rows. The harness now gives up on a function after two timeouts
# (see lifted_oracle_main.cpp), which bounds the honest worst case well inside
# this.
STALL="${STALL:-120}"
win() { printf 'Z:%s' "$(printf '%s' "$1" | tr '/' '\\')"; }

: > "$LOG"
rm -f "$REPORT"
resume=""
append=""
hangs=0
while : ; do
    # Where this segment of the log starts, so the host-refusal check below
    # reads only what THIS child printed. The log is appended to across
    # restarts, so grepping the whole file would make one Rosetta death label
    # every later kill for the rest of the sweep.
    log_mark=$(wc -l < "$LOG" 2>/dev/null || echo 0)
    # shellcheck disable=SC2086
    "$WINE" "$ROOT/build/oracle/lifted_oracle.exe" \
        --exe "$(win "$EXE")" --list "$(win "$PLAN")" --report "$(win "$REPORT")" \
        $resume $append "$@" >> "$LOG" 2>&1 &
    child=$!
    last=0
    stalled=0
    while kill -0 "$child" 2>/dev/null; do
        sleep 5
        now=$(wc -l < "$REPORT" 2>/dev/null || echo 0)
        if [ "$now" -eq "$last" ]; then
            stalled=$((stalled + 5))
        else
            stalled=0
            last=$now
        fi
        if [ "$stalled" -ge "$STALL" ]; then
            # Matched on the PATH, not the basename. A sibling agent runs its
            # own copy of this executable out of a scratch directory, and
            # `pkill -f lifted_oracle.exe` killed that too. The dots match
            # either slash, because the process shows a Windows path.
            pkill -f 'build.oracle.lifted_oracle.exe' || true
            hangs=$((hangs + 1))
            break
        fi
    done
    wait "$child" 2>/dev/null || true

    done_count=$(wc -l < "$REPORT" 2>/dev/null || echo 0)
    planned=$(grep -c '^0x' "$PLAN")
    if [ "$done_count" -ge "$((planned + 1))" ]; then
        break
    fi
    tail_address=$(tail -1 "$REPORT" | cut -f1)
    [ -n "$tail_address" ] || break
    # The function that hung is the next one in the plan after the last result.
    hung=$(grep -A1 "^$tail_address	" "$PLAN" | tail -1)
    hung_address=$(printf '%s' "$hung" | cut -f1)
    hung_name=$(printf '%s' "$hung" | cut -f3)
    [ -n "$hung_address" ] || break
    if tail -n "+$((log_mark + 1))" "$LOG" 2>/dev/null | grep -q 'rosetta error'; then
        verdict="KILLED-host-refused"
        detail="rosetta refused to emulate an instruction; the process died before any guard ran"
    else
        verdict="HANG"
        detail="the run stopped making progress here"
    fi
    printf '%s\t%s\t0\t0\t%s\t%s\n' \
        "$hung_address" "$verdict" "$detail" "$hung_name" >> "$REPORT"
    echo "sweep: $verdict at $hung_address ($hung_name); resuming after it" | tee -a "$LOG"
    resume="--resume-after $hung_address"
    append="--append"
done
echo "sweep: finished, $hangs hang(s)" | tee -a "$LOG"
