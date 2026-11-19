#!/usr/bin/env python3
"""g12.py: are the spilled induction temporaries of two loops one symbol (summed count) when the loops
share a counter variable or a table? X (kx) and Y (ky) overlap before the loops and are dead in them.
Temps joining X's slot = processed before Y (count > ky): merged; joining Y's slot = separate."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import t as T
import g6
def u(v, k): return ' '.join(['use(%s);' % v] * k)
def loop(ctr, decl, tabs):
    a, b, c, d = tabs
    return '  for (%s%s = 0; %s < n1; ++%s) { call(); use(%s[%s].a); use(%s[%s].x[2]); use(%s[%s].c); use(%s[%s].a[1]); }\n' % (
        decl, ctr, ctr, ctr, a, ctr, b, ctr, c, ctr, d, ctr)
T1 = ('Vs', 'Ws', 'Qs', 'Ps')
V = []
for kx, ky in ((12, 6), (12, 3)):
    pre = '  int rad;\n  { int X; int Y; take(&X); take(&Y); %s %s }\n' % (u('X', kx), u('Y', ky))
    V.append('void shared_%d_%d(int s) {\n%s%s  call();\n%s}\n' % (kx, ky, pre, loop('rad', '', T1), loop('rad', '', T1)))
    V.append('void sep_%d_%d(int s) {\n%s%s  call();\n%s}\n' % (kx, ky, pre, loop('r1', 'int ', T1), loop('r2', 'int ', T1)))
    V.append('void one_%d_%d(int s) {\n%s%s}\n' % (kx, ky, pre, loop('r1', 'int ', T1)))
if __name__ == '__main__':
    T.show(g6.HDR + ''.join(V), asm_of=sys.argv[1] if len(sys.argv) > 1 else None)
