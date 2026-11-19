#!/usr/bin/env python3
"""g6.py: weight of a spilled temporary against a named address-taken X that is live across the loop.
Readout: offset order (group creation order). X with k reads; loop with pointer IVs over 4 tables."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import t as T
HDR = '''#include "probe.h"
void take(int *);
struct V { int a, b, c, d, e, f, g, h; };
extern V Vs[];
struct W { int x[40]; }; extern W Ws[];
struct Q { int a, b, c; }; extern Q Qs[];
struct P { int a[5]; }; extern P Ps[];
extern int n1;
'''
def u(v, k): return ' '.join(['use(%s);' % v] * k)
LOOP = '  for (int i = 0; i < n1; ++i) { call(); use(Vs[i].a); use(Ws[i].x[2]); use(Qs[i].c); use(Ps[i].a[1]); }\n'
def fn(name, k):
    return 'void %s(int s) {\n  int X; take(&X);\n%s  %s\n}\n' % (name, LOOP, u('X', k))
V = [fn('k%d' % k, k) for k in (1, 2, 3, 4, 5, 6, 8, 10, 12, 16)]
if __name__ == '__main__':
    T.show(HDR + ''.join(V), asm_of=sys.argv[1] if len(sys.argv) > 1 else None)
