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
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
WINE="${WINE:-/Applications/Wine Staging.app/Contents/Resources/wine/bin/wine}"
EXE="${EXE:-$ROOT/.opensmacx/game/terranx_original.exe}"
PLAN="${PLAN:-$ROOT/build/oracle/plan.tsv}"
REPORT="${REPORT:-$ROOT/build/oracle/report.tsv}"
LOG="${LOG:-$ROOT/build/oracle/sweep.log}"
STALL="${STALL:-45}"     # seconds without a new report line before a kill
win() { printf 'Z:%s' "$(printf '%s' "$1" | tr '/' '\\')"; }

: > "$LOG"
rm -f "$REPORT"
resume=""
append=""
hangs=0
while : ; do
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
            pkill -f lifted_oracle.exe || true
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
    printf '%s\tHANG\t0\t0\tthe run stopped making progress here\t%s\n' \
        "$hung_address" "$hung_name" >> "$REPORT"
    echo "sweep: hang at $hung_address ($hung_name); resuming after it" | tee -a "$LOG"
    resume="--resume-after $hung_address"
    append="--append"
done
echo "sweep: finished, $hangs hang(s)" | tee -a "$LOG"
