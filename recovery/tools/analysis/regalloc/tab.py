#!/usr/bin/env python3
"""tab.py <g_family.cpp> <vars> [--oy]: one row per probe family prefix, one column per K, cells = homes of <vars>
(e.g. `tab.py g_weights.cpp cd` prints for each duel which of c, d holds a register: 'c', 'd', 'cd' or '-')."""
import sys, re, io, contextlib, collections
import run
src, vs = sys.argv[1], sys.argv[2]
with contextlib.redirect_stdout(io.StringIO()):
    res = run.report(src, run.os.environ.get('FE_PFX_BASE', '2'), '--oy' in sys.argv)
rows = collections.OrderedDict()
for fn, h in res.items():
    m = re.fullmatch(r'(.*)_k(\d+)', fn)
    key, k = (m.group(1), int(m.group(2))) if m else (fn, 0)
    cell = ''
    for v in vs:
        hh = h.get(v, ['?'])
        if '-v' in sys.argv:
            cell += v + '=' + '/'.join(x[1:] if x[0] == 'e' else 'm' for x in hh) + ' '
        elif not all(x.startswith('[') for x in hh):
            cell += v + ('~' if any(x.startswith('[') for x in hh) else '')
    rows.setdefault(key, {})[k] = cell.strip() or '-'
ks = sorted({k for r in rows.values() for k in r})
w = max(len(c) for r in rows.values() for c in r.values()) + 1
print('%-12s' % 'K=' + ''.join(('%-' + str(w) + 's') % k for k in ks))
for key, r in rows.items():
    print('%-12s' % key + ''.join(('%-' + str(w) + 's') % r.get(k, '') for k in ks))
