#!/usr/bin/env python3
"""g11.py: frame offset order of packed slots: founder count or group total (non-address-taken symbols)?
Outer counter O (k extra reads) spans n sequential inner loops whose spilled counters c1..cn pack together."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import t as T
HDR = '''#include "probe.h"
struct V { int a, b, c, d, e, f, g, h; }; extern V Vs[];
struct W { int x[40]; }; extern W Ws[];
struct Q3 { int a, b, c; }; extern Q3 Qs[];
'''
def u(v, k): return ' '.join(['use(%s);' % v] * k)
def fn(name, k, n):
    b = '  for (int O = 0; O < g; O++) {\n    %s\n' % u('O', k)
    for j in range(n):
        b += '    for (int c%d = 0; c%d < g2; c%d++) { call(); use(Vs[c%d].a + O); use(Ws[c%d].x[2]); use(Qs[c%d].c); }\n' % ((j,) * 6)
    b += '  }\n'
    return 'void %s(int s) {\n%s}\n' % (name, b)
V = [fn('f_%d_%d' % (k, n), k, n) for k, n in ((0, 1), (0, 4), (4, 1), (4, 4), (8, 4), (8, 8), (16, 8))]
if __name__ == '__main__':
    T.show(HDR + ''.join(V), asm_of=sys.argv[1] if len(sys.argv) > 1 else None)
