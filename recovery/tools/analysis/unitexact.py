#!/usr/bin/env python3
"""unitexact.py <unit.cpp> [ALT.cpp ...]: fe row counts of every marker of a unit, for the unit's
source and each ALT whole-file variant side by side; lists only the functions whose rows differ
between the columns (and the exact counts per column).  Prefix FE_PFX_BASE (default 2).  Screen only."""
import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import fe
unit = sys.argv[1]; srcs = [os.path.join(fe.R, unit)] + [os.path.abspath(p) for p in sys.argv[2:]]
pfx = os.environ.get('FE_PFX_BASE', '2')
cols = []
for s in srcs:
    t = open(s, encoding='latin-1').read()
    c, err = fe.compile_unit(unit, s, pfx)
    if c is None:
        sys.exit('compile failed: %s\n%s' % (s, err[:2000]))
    d = {}
    for m in re.finditer(r'// ORIGINAL: 0x([0-9A-F]{8}) (\S+)', t):
        a = int(m.group(1), 16)
        try:
            r = fe.evaluate(c, t, a)
            d[a] = (m.group(2), r['diff'], r['exact'])
        except Exception:
            pass
    cols.append(d)
print('exact per column:', [sum(1 for v in d.values() if v[2]) for d in cols])
for a in sorted(cols[0]):
    row = [d.get(a, ('', -1, False)) for d in cols]
    if len(set(r[1] for r in row)) > 1:
        print('%08X %-40s %s' % (a, row[0][0][:40], ' '.join('%5d%s' % (r[1], '*' if r[2] else ' ') for r in row)))
