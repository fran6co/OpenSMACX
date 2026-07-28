#!/bin/sh
# Runs the differential oracle under Wine. Wine needs Windows paths, so this
# turns the repository's own into Z:\ form rather than making every caller do
# it. Extra arguments go through to the driver (--limit, --only, --cases ...).
#
# THIS SCRIPT IS FOR ONE FUNCTION OR A SHORT --limit, NOT FOR THE WHOLE PLAN.
# It has no supervision, and a whole-plan run WILL stop making progress: a
# seeded pointer can land on the host stack and overwrite the registers the
# runner saved there, after which the harness spins and nothing inside the
# process can recover. Reaching ?on_move@TutWin@@QAEXHH@Z at report line ~2215
# and hanging there is the reproducible case. Use tools/lifted_oracle_sweep.sh
# for the whole plan - it watches the report stop growing, kills the run,
# records the function as HANG and resumes after it.
#
# So this refuses a whole-plan run rather than appearing to work for 87 seconds
# and then hanging forever with no message.
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
WINE="${WINE:-$(command -v wine \
    || echo '/Applications/Wine Staging.app/Contents/Resources/wine/bin/wine')}"
EXE="${EXE:-$ROOT/.opensmacx/game/terranx_original.exe}"
PLAN="${PLAN:-$ROOT/build/oracle/plan.tsv}"
REPORT="${REPORT:-$ROOT/build/oracle/report.tsv}"
win() { printf 'Z:%s' "$(printf '%s' "$1" | tr '/' '\\')"; }
case " $* " in
    *" --only "*|*" --limit "*|*" --selfcheck "*|*" --selftest "*|*" --list "*)
        ;;
    *)
        echo "lifted_oracle_run.sh: a whole-plan run has no supervision and" >&2
        echo "  will hang. Use tools/lifted_oracle_sweep.sh, or pass --limit" >&2
        echo "  / --only to say you meant a short run." >&2
        exit 2
        ;;
esac
exec "$WINE" "$ROOT/build/oracle/lifted_oracle.exe" \
    --exe "$(win "$EXE")" --list "$(win "$PLAN")" --report "$(win "$REPORT")" "$@"
