#!/usr/bin/env python3
"""g9.py: packing order of a deep-loop temporary. X and Y overlap (block before the loop), both dead in
the loop; the loop's spilled temporary T joins X (T processed before X: weighted order) or Y (raw order)."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import t as T
import g6
def u(v, k): return ' '.join(['use(%s);' % v] * k)
L = {'d1': g6.LOOP,
     'd3': '  for (int o = 0; o < g; ++o) { for (int o2 = 0; o2 < g2; ++o2) {\n  ' + g6.LOOP + '  } }\n'}
V = []
for tag, lp in L.items():
    for kx, ky in ((10, 6), (16, 12), (5, 3), (3, 2)):
        V.append('void %s_%d_%d(int s) {\n  { int X; int Y; take(&X); take(&Y); %s %s }\n%s}\n' % (tag, kx, ky, u('X', kx), u('Y', ky), lp))
if __name__ == '__main__':
    T.show(g6.HDR + ''.join(V), asm_of=sys.argv[1] if len(sys.argv) > 1 else None)
