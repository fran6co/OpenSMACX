#!/bin/bash
# info.sh <unit> <addr> <src>... : per whole-file variant: fe rows, frame size, the register holding this, ebx uses,
# norm.py summary. Writes <src>.diff beside each variant. Prefix FE_PFX_BASE (default 27).
unit=$1; addr=$2; shift 2
for s in "$@"; do
  uv run python recovery/tools/fe.py $unit $addr --pfx ${FE_PFX_BASE:-27} --src $s --all > $s.diff 2>&1
  rows=$(grep '^FE' $s.diff | sed 's/.*diff=\([0-9]*\).*/\1/')
  frame=$(cut -d'|' -f2 $s.diff | grep -m1 'sub esp' | awk '{print $NF}')
  thisr=$(cut -d'|' -f2 $s.diff | head -30 | grep -m1 'mov e.., ecx$' | awk '{print $3}')
  z=$(cut -d'|' -f2 $s.diff | grep -c ', \(ebx\|bl\)$')
  echo "$(basename $s) rows=$rows frame=$frame this=$thisr ebxuses=$z $(python3 recovery/tools/analysis/norm.py $s.diff | cut -d' ' -f2-)"
done
