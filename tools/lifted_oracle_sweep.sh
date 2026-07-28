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
WINE="${WINE:-$(command -v wine \
    || echo '/Applications/Wine Staging.app/Contents/Resources/wine/bin/wine')}"
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
deaths=0
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
    # Set only where the watchdog actually kills the child. This, and not a
    # string in the log, is what separates the two ways a run dies - see the
    # verdict choice below.
    watchdog_killed=0
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
            watchdog_killed=1
            hangs=$((hangs + 1))
            break
        fi
    done
    if wait "$child" 2>/dev/null; then child_status=0; else child_status=$?; fi

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
    # WHICH OF THE TWO DEATHS WAS IT, and the answer comes from what this
    # script OBSERVED rather than from a string it went looking for.
    #
    # This used to ask "does the log segment contain `rosetta error`", which is
    # true only on an arm64 Mac. On Linux the process simply exits - silently,
    # no diagnostic of any kind - and every one of those was being recorded as
    # HANG. Measured on the first native sweep: eight rows said "the run
    # stopped making progress here" while the run's own closing line said
    # "finished, 0 hang(s)", because the watchdog had not fired once. A report
    # that contradicts its own summary is the shape of a verdict assigned by
    # something other than evidence.
    #
    # The watchdog flag is the honest discriminator and it needs no host
    # knowledge at all: if this script killed the child, it watched the report
    # stop growing for $STALL seconds and HANG is exactly what it saw. If the
    # child went away on its own, nothing was watched - the process was gone
    # before any guard ran - and that is KILLED-host-refused, which is in no
    # evidence set anywhere and is classified `no-evidence` by
    # tools/lifted_oracle_mutate.py. The Rosetta string still refines the
    # DETAIL where it appears, because naming the cause is worth doing; it just
    # no longer decides the verdict.
    if [ "$watchdog_killed" -eq 1 ]; then
        verdict="HANG"
        detail="the run stopped making progress here for ${STALL}s and the watchdog killed it"
    else
        verdict="KILLED-host-refused"
        deaths=$((deaths + 1))
        if tail -n "+$((log_mark + 1))" "$LOG" 2>/dev/null | grep -q 'rosetta error'; then
            detail="rosetta refused to emulate an instruction; the process died before any guard ran"
        else
            detail="the harness process exited (status $child_status) without writing a row; no guard ran"
        fi
    fi
    printf '%s\t%s\t0\t0\t%s\t%s\n' \
        "$hung_address" "$verdict" "$detail" "$hung_name" >> "$REPORT"
    echo "sweep: $verdict at $hung_address ($hung_name); resuming after it" | tee -a "$LOG"
    resume="--resume-after $hung_address"
    append="--append"
done
echo "sweep: finished, $hangs hang(s) and $deaths host death(s)" | tee -a "$LOG"
