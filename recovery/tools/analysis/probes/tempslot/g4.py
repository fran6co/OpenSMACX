#!/usr/bin/env python3
"""g4.py: last-created group vs smallest-total group. X (function-scope, live over b1 and b2), Y in b1,
Q in b2 (joins Y's group), Z alone in b3."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import t as T
HDR = '''#include "probe.h"
void take(int *);
'''
def u(v, k): return ' '.join(['use(%s);' % v] * k)
def fn(name, kx, ky, kq, kz):
    return '''void %s(int s) {
  int X; take(&X);
  { int Y; take(&Y); %s %s }
  { int Q; take(&Q); %s %s }
  { int Z; take(&Z); %s }
}
''' % (name, u('X', kx), u('Y', ky), u('Q', kq), u('X', 1), u('Z', kz))
V = []
for k in ((9, 5, 4, 1), (9, 5, 6, 1), (9, 5, 1, 1), (9, 5, 4, 3), (12, 5, 4, 1), (12, 3, 1, 2)):
    V.append(fn('f_%d_%d_%d_%d' % k, *k))
if __name__ == '__main__':
    T.show(HDR + ''.join(V), asm_of=sys.argv[1] if len(sys.argv) > 1 else None)
