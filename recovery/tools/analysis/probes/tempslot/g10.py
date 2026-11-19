#!/usr/bin/env python3
"""g10.py: does a strength-reduced loop counter's spilled pointer keep the counter variable's home?
J is a function-scope variable with its own uses before/after (memory-homed); the radius loop counts with
J (variant 'J'), with a separate function-scope `rad` ('rad') or a for-scoped int ('blk')."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import t as T
HDR = '''#include "probe.h"
void take(int *);
extern int RX[], RY[];
extern int xr(int);
struct V { int a, b, c, d, e, f, g, h; }; extern V Vs[];
struct W { int x[40]; }; extern W Ws[];
struct Q3 { int a, b, c; }; extern Q3 Qs[];
'''
def u(v, k): return ' '.join(['use(%s);' % v] * k)
def fn(name, ctr, decl):
    return '''int %s(int s, int x, int y) {
  int J, K, M, rad; int P = ga[s], Q = ga[s+1], W = ga[s+2];
  J = ga[3]; K = ga[4]; M = ga[5];
  for (int z = 0; z < g; z++) { for (int z2 = 0; z2 < g; z2++) { call(); use(J + z2); use(K + z2); use(M + z2); use(P + z2); } }
  call(); %s %s %s
  for (%s = 0; %s < 9; %s++) {
    int tx = xr(x + RX[%s]); int ty = y + RY[%s];
    for (int q = 0; q < g; q++) { call(); use(Vs[q].a + tx); use(Ws[q].x[2] + ty); use(Qs[q].c); }
  }
  J = ga[6]; K = ga[7]; M = ga[8];
  call(); %s %s %s use(P); use(Q); use(W); use(x); use(y);
  return 0;
}
''' % (name, u('J', 3), u('K', 3), u('M', 3), decl + ctr, ctr, ctr, ctr, ctr, u('J', 3), u('K', 3), u('M', 3))
V = [fn('vJ', 'J', ''), fn('vK', 'K', ''), fn('vM', 'M', ''), fn('vrad', 'rad', ''), fn('vblk', 'r2', 'int ')]
if __name__ == '__main__':
    T.show(HDR + ''.join(V), asm_of=sys.argv[1] if len(sys.argv) > 1 else None)
