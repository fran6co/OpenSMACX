#!/usr/bin/env python3
"""addr.py: prag.py with the global's address taken elsewhere in the unit.  Usage: uv run python prag.py"""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))) + '/inline')
import ib
PFX = os.environ.get('FE_PFX_BASE', '21')
HDR = '''#include "probe.h"
struct P { int pad[248]; int a, b, c, d; int pad2[400]; };
extern P Pl[];
extern int GI;
int x_pop(const char *, int, int);
void synch(int);
void takep(int *);
void t_addr() { takep(&GI); }
'''
BODY = '''void f_%s(int id) {
  int *a = &Pl[id].a; int *b = &Pl[id].b; int *c = &Pl[id].c; int *d = &Pl[id].d;
  int fl = (*a != 0) ? 1 : 0;
  if (*b) fl |= 2; else fl &= ~2;
  GI = fl;
  if (x_pop("X", 1, 0) >= 0) { %s synch(id); }
}
'''
U = {
 'cast': '*a = (char)GI & 1u; *b = ((char)GI & 2u) >> 1; *c = ((char)GI & 4u) >> 2; *d = ((char)GI & 8u) >> 3;',
 'direct': 'Pl[id].a = (char)GI & 1u; Pl[id].b = ((char)GI & 2u) >> 1; Pl[id].c = ((char)GI & 4u) >> 2; Pl[id].d = ((char)GI & 8u) >> 3;',
 'loc': 'char s = (char)GI; *a = s & 1u; *b = (s & 2u) >> 1; *c = (s & 4u) >> 2; *d = (s & 8u) >> 3;',
}
PR = {'none': ('', '')}
NOPE = {'g_off': ('#pragma optimize("g", off)\n', '#pragma optimize("", on)\n'),
      'a_on': ('#pragma optimize("a", on)\n', '#pragma optimize("", on)\n'),
      'w_on': ('#pragma optimize("w", on)\n', '#pragma optimize("", on)\n')}
src = HDR
names = []
for p, (pre, post) in PR.items():
    for u, b in U.items():
        n = '%s_%s' % (p, u); names.append(n)
        src += pre + BODY % (n, b) + post
asm, _ = ib.compile_asm(None, PFX, text=src)
F = {f[0].split('(')[0].strip(): f[2] for f in ib.functions(asm)}
for n in names:
    L = [l for k, l in F.items() if k.endswith('f_' + n)][0]
    i = max(k for k, l in enumerate(L) if 'x_pop' in l)
    print('%-12s movsx=%d | %s' % (n, sum('movsx' in l for l in L[i:]), ' ; '.join(L[i + 1:])[:330]))
