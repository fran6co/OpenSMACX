#!/usr/bin/env python3
"""inl.py: operand order of the `y * G` multiply inside an inlined map_loc-style helper, by how the caller
passes y: a named local (`const int y = S[v].y; loc(x, y)`) or the field expression itself
(`loc(S[v].x, S[v].y)`), with y used again after (live) or not.  Prints the instructions around imul."""
import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))) + '/inline')
import ib
PFX = os.environ.get('FE_PFX_BASE', '21')
H = ('#include "probe.h"\nstruct Tile { int v; int w; int z; };\nextern Tile T[];\nextern int G;\n'
     'struct SV { short x, y; int pad[11]; };\nextern SV S[];\nvoid sink2(int, int);\n'
     'inline Tile *loc(int x, int y) { return &T[(x >> 1) + y * G]; }\n')
V = {
 'named':      'int f(int v) { const int y = S[v].y; const int x = S[v].x; return loc(x, y)->v; }',
 'named_live': 'int f(int v) { const int y = S[v].y; const int x = S[v].x; if (loc(x, y)->v) sink2(x, y); return 0; }',
 'expr':       'int f(int v) { return loc(S[v].x, S[v].y)->v; }',
 'expr_live':  'int f(int v) { if (loc(S[v].x, S[v].y)->v) sink2(S[v].x, S[v].y); return 0; }',
 'int_named_live': 'int f(int v) { int y = ga[v]; int x = ga[v + 1]; if (loc(x, y)->v) sink2(x, y); return 0; }',
 'int_expr_live': 'int f(int v) { if (loc(ga[v + 1], ga[v])->v) sink2(ga[v + 1], ga[v]); return 0; }',
}
for k, body in V.items():
    asm, _ = ib.compile_asm(None, PFX, text=H + body.replace('int f(', 'int t_%s(' % k) + '\n')
    for name, sym, lines in ib.functions(asm):
        if name.startswith('t_'):
            i = next(j for j, l in enumerate(lines) if l.startswith('imul'))
            print('%-16s %s' % (k, ' ; '.join(re.sub(r'\s+', ' ', l) for l in lines[max(0, i - 4):i + 1])))
