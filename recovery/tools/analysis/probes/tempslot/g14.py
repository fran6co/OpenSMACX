#!/usr/bin/env python3
"""g14.py: which of two EQUAL-count units is created first (the tie order of the last-fit packer)? X and Y have
the same count and overlap in block 1; a light Z (1 access) lives alone in block 2 and joins the most recently
created slot, so Z's partner names the second-created of X/Y. Variants separate declaration order, first
lexical reference, first definition (store) and address-taking order."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import t as T
HDR = '''#include "probe.h"
void take(int *);
'''
K = 5
def u(v, k=K): return ' '.join(['use(%s);' % v] * k)
B2 = '  { int Z; take(&Z); }\n'
CASES = {
 # declared X,Y; address taken X,Y; uses X..,Y..
 'dXY_tXY_uXY': '  { int X; int Y; take(&X); take(&Y); %s %s }\n' % (u('X'), u('Y')),
 # declared Y,X (same take/use order as above)
 'dYX_tXY_uXY': '  { int Y; int X; take(&X); take(&Y); %s %s }\n' % (u('X'), u('Y')),
 # declared X,Y; take Y first
 'dXY_tYX_uXY': '  { int X; int Y; take(&Y); take(&X); %s %s }\n' % (u('X'), u('Y')),
 # declared X,Y; uses Y first
 'dXY_tXY_uYX': '  { int X; int Y; take(&X); take(&Y); %s %s }\n' % (u('Y'), u('X')),
 # declared X,Y; everything Y first
 'dXY_tYX_uYX': '  { int X; int Y; take(&Y); take(&X); %s %s }\n' % (u('Y'), u('X')),
 # declared in separate nested scopes: X outer, Y inner
 'nest_XoutYin': '  { int X; take(&X); { int Y; take(&Y); %s %s } }\n' % (u('X'), u('Y')),
 'nest_YoutXin': '  { int Y; take(&Y); { int X; take(&X); %s %s } }\n' % (u('X'), u('Y')),
 # interleaved uses
 # first use X, last use X (Y between): first-use rule -> X first, last-use rule -> Y first
 'sandwich_X': '  { int X; int Y; take(&X); take(&Y); use(X); %s %s }\n' % (u('Y'), u('X', K - 1)),
 'sandwich_Y': '  { int X; int Y; take(&X); take(&Y); use(Y); %s %s }\n' % (u('X'), u('Y', K - 1)),
 # a store before the uses: X stored first, Y used first
 'store_X_useY': '  { int X; int Y; take(&X); take(&Y); X = ga[1]; %s %s }\n' % (u('Y'), u('X', K - 1)),
 # address taken after uses
 'late_take': '  { int X; int Y; X = ga[1]; Y = ga[2]; %s %s take(&Y); take(&X); }\n' % (u('Y', K - 1), u('X', K - 1)),
 'dXY_tXY_mix': '  { int X; int Y; take(&X); take(&Y); %s }\n' % ' '.join(['use(X); use(Y);'] * K),
}
# not address-taken variables (kept in memory by a call-heavy body): definitions instead of take()
CASES2 = {
 'v_dXY_sXY': '  { int X, Y; X = ga[1]; Y = ga[2]; call(); %s call(); %s }\n' % (u('X'), u('Y')),
 'v_dYX_sXY': '  { int Y, X; X = ga[1]; Y = ga[2]; call(); %s call(); %s }\n' % (u('X'), u('Y')),
 'v_dXY_sYX': '  { int X, Y; Y = ga[2]; X = ga[1]; call(); %s call(); %s }\n' % (u('X'), u('Y')),
}
def fn(name, b1, zfirst=False):
    return 'void %s(int s) {\n%s}\n' % (name, (B2 + b1) if zfirst else (b1 + B2))
V = []
for k, b in list(CASES.items()) + list(CASES2.items()):
    V.append(fn(k, b))
    V.append(fn('z_' + k, b, True))
if __name__ == '__main__':
    T.show(HDR + ''.join(V), asm_of=sys.argv[1] if len(sys.argv) > 1 else None)
