#!/usr/bin/env python3
"""g15.py: do a ?: result homed in memory and a spilled pointer induction temporary count LIGHTER than their
frame accesses when packed (MATCHING_GUIDE 3.3, boom's two residual temporaries)? X(12) and Y(ky) overlap and
are dead afterwards; the temporary joins X's slot when it is created before Y (its count > ky), Y's otherwise.
Parameters are address-taken at the end so their homes stay live (a parameter cached in a callee-saved
register leaves its home free and the temporary takes it).
Answer (2026-11-20): no. The inline ?: result spilled per arm (two arm stores + one reload, ~3) joins X at
Y=2 and Y at Y=3: count 3. A loop's spilled struct-pointer IV (~4) joins X at Y=3 and Y at Y=4: count 4.
Both count exactly their frame accesses. A ?: result stored ONCE after the join (boom 0x504F97) was not
reproduced: the probe's spill is always per arm."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import t as T
HDR = '''#include "probe.h"
void take(int *);
extern int MS, MD; struct MW { int pad[30556]; int zoom; }; extern MW *zp[];
inline int zs(int v, int z) { return MS > 0 ? MS * v / MD : (z + 16) * v / 16; }
struct V { int a, b, c, d, e, f, g, h; };
extern V Vs[];
struct W { int x[40]; }; extern W Ws[];
struct Q { int a, b, c; }; extern Q Qs[];
struct P { int a[5]; }; extern P Ps[];
extern int n1;
'''
def u(v, k): return ' '.join(['use(%s);' % v] * k)
XY = '  { int X; int Y; take(&X); take(&Y); %s %s }\n'
def tern(name, ky, body):
    return ('void %s(int s, int h, int w, int d) {\n  int px[4];\n' + XY +
            '  int p = ga[1], q = ga[2], r = ga[3];\n  call();\n%s'
            '  %s %s %s take(&s); take(&h); take(&w); take(&d); take(px);\n}\n') % (
        name, u('X', 12), u('Y', ky), body, u('p', 4), u('q', 4), u('r', 4))
def loop(name, ky, body):
    return ('void %s(int s) {\n' + XY + '%s}\n') % (name, u('X', 12), u('Y', ky), body)
TERN3 = '  ga[s] = ga[s] - zs(h, zp[s]->zoom) + zs(w, zp[s]->zoom) - zs(s, zp[s]->zoom);\n'
BOOM = '  px[s] = px[s] - zs(h, zp[s]->zoom) + zs(w * 60 / d, zp[s]->zoom);\n'
LOOP4 = '  for (int i = 0; i < n1; ++i) { call(); use(Vs[i].a); use(Ws[i].x[2]); use(Qs[i].c); use(Ps[i].a[1]); }\n'
V = []
for ky in range(1, 6):
    V.append(tern('tern3_%d' % ky, ky, TERN3))
    V.append(tern('boom_%d' % ky, ky, BOOM))
    V.append(loop('iv4_%d' % ky, ky, LOOP4))
if __name__ == '__main__':
    T.show(HDR + ''.join(V), asm_of=sys.argv[1] if len(sys.argv) > 1 else None)
