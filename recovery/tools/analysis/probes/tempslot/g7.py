#!/usr/bin/env python3
"""g7.py: as g6 with the loop nested (depth 2) or followed by extra temp uses; crossover of X's read count."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import t as T
import g6
def u(v, k): return ' '.join(['use(%s);' % v] * k)
L1 = g6.LOOP
L2 = '  for (int o = 0; o < g; ++o) {\n  ' + g6.LOOP + '  }\n'
L3 = '  for (int o = 0; o < g; ++o) { for (int o2 = 0; o2 < g2; ++o2) {\n  ' + g6.LOOP + '  } }\n'
V = []
for tag, L in (('d1', L1), ('d2', L2), ('d3', L3)):
    for k in (1, 2, 3, 4, 6, 8, 12, 16, 24, 32):
        V.append('void %s_%d(int s) {\n  int X; take(&X);\n%s  %s\n}\n' % (tag, k, L, u('X', k)))
if __name__ == '__main__':
    T.show(g6.HDR + ''.join(V), asm_of=sys.argv[1] if len(sys.argv) > 1 else None)
