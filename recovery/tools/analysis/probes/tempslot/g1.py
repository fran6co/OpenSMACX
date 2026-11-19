#!/usr/bin/env python3
"""g1.py: which dead named slot does a spilled compiler temporary take?
Region A: six named locals a0..a5 (pressure -> some memory-homed), each with a configurable number of
uses; all die before region B. Region B: a nested loop whose strength-reduced pointers spill (tv temps).
Prints each function's frame symbols; the unnamed offsets are the temporaries' slots."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import t as T
HDR = '''#include "probe.h"
struct V { int a, b, c, d, e, f, g, h; };
extern V Vs[];
struct W { int x[40]; };
extern W Ws[];
extern int n1, n2;
'''
LOOP = '''  for (int i = 0; i < n1; ++i) {
    for (int k = 0; k < n2; ++k) {
      if (Vs[i].a == Ws[k].x[s] && Vs[k].b == Ws[i].x[3]) { use(Vs[i].c); use(Ws[k].x[7]); }
    }
  }
'''
def fn(name, uses, lastorder=None):
    # uses: list of extra use counts per variable a0..a5
    n = len(uses)
    b = '  int %s;\n' % ', '.join('a%d = ga[s+%d]' % (j, j) for j in range(n))
    b += '  for (int z = 0; z < g; ++z) { call(); %s }\n' % ' '.join('use(a%d + z);' % j for j in range(n))
    order = lastorder or list(range(n))
    for j in order:
        b += '  ' + ' '.join('use(a%d);' % j for _ in range(uses[j])) + '\n'
    return 'void %s(int s) {\n%s%s}\n' % (name, b, LOOP)
V = {
 'flat': fn('flat', [1, 1, 1, 1, 1, 1]),
 'w0': fn('w0', [6, 1, 1, 1, 1, 1]),
 'w5': fn('w5', [1, 1, 1, 1, 1, 6]),
 'rev': fn('rev', [1, 1, 1, 1, 1, 1], [5, 4, 3, 2, 1, 0]),
 'inc': fn('inc', [1, 2, 3, 4, 5, 6]),
 'dec': fn('dec', [6, 5, 4, 3, 2, 1]),
}
if __name__ == '__main__':
    src = HDR + ''.join(V.values())
    T.show(src, asm_of=sys.argv[1] if len(sys.argv) > 1 else None)
