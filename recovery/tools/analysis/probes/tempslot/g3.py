#!/usr/bin/env python3
"""g3.py: named-only packing. X and Y overlap (both live in block 1), Z lives alone in block 2.
Weights kX, kY, kZ (extra reads). Which of X/Y does Z share with? (first-fit vs last-fit vs best-fit)"""
import sys, os, itertools
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import t as T
HDR = '''#include "probe.h"
void take(int *);
'''
def fn(name, kx, ky, kz, zfirst=False):
    b1 = '  { int X; int Y; take(&X); take(&Y); %s %s }\n' % (' '.join(['use(X);'] * kx), ' '.join(['use(Y);'] * ky))
    b2 = '  { int Z; take(&Z); %s }\n' % ' '.join(['use(Z);'] * kz)
    return 'void %s(int s) {\n%s}\n' % (name, (b2 + b1) if zfirst else (b1 + b2))
V = []
for kx, ky, kz in ((9, 5, 1), (9, 5, 7), (9, 5, 12), (5, 9, 1), (5, 9, 7), (5, 9, 12), (9, 9, 1), (5, 5, 5)):
    V.append(fn('n_%d_%d_%d' % (kx, ky, kz), kx, ky, kz))
    V.append(fn('z_%d_%d_%d' % (kx, ky, kz), kx, ky, kz, True))
if __name__ == '__main__':
    T.show(HDR + ''.join(V), asm_of=sys.argv[1] if len(sys.argv) > 1 else None)
