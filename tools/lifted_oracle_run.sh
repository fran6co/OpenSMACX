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
# Overridable so a diagnostic can run its own copy while a sweep is in
# progress. The sweep's watchdog reaps with `pkill -f
# 'build.oracle.lifted_oracle.exe'`, which matches any process holding that
# path, so a concurrent probe MUST come from somewhere else or the sweep will
# kill it on its first stall.
ORACLE="${ORACLE:-$ROOT/build/oracle/lifted_oracle.exe}"
# Captured BEFORE the default is applied: afterwards REPORT is always set and
# the short-run guard below could never fire.
REPORT_EXPLICIT="${REPORT+yes}"
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
# A SHORT RUN MUST NOT WRITE THE CANONICAL REPORT. The driver truncates its
# report, so `--only 0x004031a0` with the default path replaces a 5,674-row
# whole-image sweep with two lines. That happened: three diagnostic probes cost
# the default arm's report, which then had to be re-swept.
#
# So a short run gets probe-report.tsv unless REPORT was set explicitly. The
# whole-plan path is unaffected - it is refused above and sweep.sh sets REPORT
# itself.
case " $* " in
    *" --only "*|*" --limit "*)
        if [ -z "$REPORT_EXPLICIT" ]; then
            REPORT="$ROOT/build/oracle/probe-report.tsv"
            echo "lifted_oracle_run.sh: short run -> $REPORT" >&2
            echo "  (set REPORT= explicitly to write somewhere else)" >&2
        fi
        ;;
esac

exec "$WINE" "$ORACLE" \
    --exe "$(win "$EXE")" --list "$(win "$PLAN")" --report "$(win "$REPORT")" "$@"
