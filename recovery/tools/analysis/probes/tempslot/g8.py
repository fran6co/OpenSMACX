#!/usr/bin/env python3
"""g8.py: frame offset order: founder count or group total? X live over all blocks (kx reads),
n disjoint block-locals with ky reads each pack into one group."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import t as T
HDR = '#include "probe.h"\nvoid take(int *);\n'
def u(v, k): return ' '.join(['use(%s);' % v] * k)
def fn(name, kx, ky, n):
    b = '  int X; take(&X); %s\n' % u('X', kx)
    for j in range(n):
        b += '  { int Y%d; take(&Y%d); %s use(X); }\n' % (j, j, u('Y%d' % j, ky))
    return 'void %s(int s) {\n%s}\n' % (name, b)
V = [fn('f_%d_%d_%d' % k, *k) for k in ((10, 5, 1), (10, 5, 3), (10, 5, 6), (10, 8, 4), (20, 8, 4), (4, 3, 8))]
if __name__ == '__main__':
    T.show(HDR + ''.join(V), asm_of=sys.argv[1] if len(sys.argv) > 1 else None)
