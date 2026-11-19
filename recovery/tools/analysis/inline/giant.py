#!/usr/bin/env python3
"""giant.py N STRIDE [kind] [--charged]: is there a caller-size stop on /Ob2 expansion (MATCHING_GUIDE 2.8,
round 16)?  One caller of N filler statements of `kind` (store: `ga[i] = i;`, if: `if (ga[i] == i) use(i);`,
call: `use(i);`) with a FREE inline site `fr(k)` (`mk(k);`) after every STRIDE fillers (k = filler index), or a
charged 4-if site with --charged.  Prints the run string of E/c per site and the first called site's filler index."""
import sys; sys.path.insert(0, '.'); import ib, os
N = int(sys.argv[1]); ST = int(sys.argv[2]); kind = sys.argv[3] if len(sys.argv) > 3 and not sys.argv[3].startswith('-') else 'if'
charged = '--charged' in sys.argv
mixed = '--mixed' in sys.argv  # also a free fr(k) site halfway between the charged ones
F = {'store': 'ga[%d] = %d;', 'if': 'if (ga[%d] == %d) use(%d);', 'call': 'use(%d);'}[kind]
fill = lambda i: F % ((i,) * F.count('%d'))
src = ['#include "probe.h"',
       'inline void fr(int k) { mk(k); }',
       'inline void fc(int k) { if (g == 1000) mk(k); if (g2 == 1000) mk(k); if (g3 == 1000) mk(k); if (g4 == 1000) mk(k); }']
body = []
for i in range(1, N + 1):
    body.append(fill(i))
    if i % ST == 0:
        body.append('%s(%d);' % ('fc' if charged else 'fr', i))
    if mixed and i % ST == ST // 2:
        body.append('fr(%d);' % i)
if '--front' in sys.argv:  # every site first, then the fillers: is the stop a running total in source order?
    body = [x for x in body if not x.startswith(('fr(', 'fc('))] and \
        [x for x in body if x.startswith(('fr(', 'fc('))] + [x for x in body if not x.startswith(('fr(', 'fc('))]
src.append('void caller() {\n' + '\n'.join(body) + '\n}')
r = ib.report(None, os.environ.get('FE_PFX_BASE', '13'), text='\n'.join(src), quiet=True)
s = r['caller'][0]
st = {}
for k, c, x in s:
    if k % ST == 0 and (c in ('mk', 'fr', 'fc')):
        st[k] = 'E' if c == 'mk' else 'c'
ks = sorted(st)
out = ''.join(st[k] for k in ks)
if mixed:
    fs = {}
    for k, c, x in s:
        if k % ST == ST // 2 and c in ('mk', 'fr'):
            fs[k] = 'E' if c == 'mk' else 'c'
    fo = ''.join(fs[k] for k in sorted(fs))
    print('free sites: E=%d c=%d first_called=%s %s' % (fo.count('E'), fo.count('c'), next((k for k in sorted(fs) if fs[k] == 'c'), None), fo[:120]))
first = next((k for k in ks if st[k] == 'c'), None)
print('N=%d stride=%d %s%s sites=%d E=%d first_called=%s  %s' % (N, ST, kind, ' charged' if charged else '', len(ks), out.count('E'), first, out if len(out) < 200 else out[:200] + '...'))
