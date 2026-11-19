#!/usr/bin/env python3
"""g2.py: two dead address-taken block-locals A, B (kA, kB extra reads) before a loop whose
strength-reduced pointers spill; which slot do the temporaries take?"""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import t as T
HDR = '''#include "probe.h"
void take(int *);
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
def blk(v, k, addr=True):
    if addr:
        return '  { int %s; take(&%s); %s }\n' % (v, v, ' '.join('use(%s);' % v for _ in range(k)))
    return '  { int %s = ga[s+5]; for (int z%s = 0; z%s < g; ++z%s) { call(); use(%s); } %s }\n' % (v, v, v, v, v, ' '.join('use(%s);' % v for _ in range(k)))
def fn(name, parts):
    return 'void %s(int s) {\n%s%s}\n' % (name, ''.join(parts), LOOP)
V = []
for kA, kB in ((1, 1), (5, 1), (1, 5), (10, 1), (1, 10)):
    V.append(fn('AB_%d_%d' % (kA, kB), [blk('A', kA), blk('B', kB)]))
    V.append(fn('BA_%d_%d' % (kA, kB), [blk('B', kB), blk('A', kA)]))
if __name__ == '__main__':
    T.show(HDR + ''.join(V), asm_of=sys.argv[1] if len(sys.argv) > 1 else None)
