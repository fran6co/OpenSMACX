#!/usr/bin/env python3
"""run.py: base/index order of `lea r,[a+b+disp]` for &this->text_[this->field] (MATCHING_GUIDE 3.4).
Body T = g(&text_[b44]); g(&text_[b40]); g(&text_[b44]); g(&text_[b40]); after a prefix P.  Prints, per lea
of T, `T` when `this` is printed first ([esi+idx]) and `i` when the index is ([idx+esi]).
Findings (2026-11-20 par-5): the order never follows the spelling of the address expression; it follows how
many memory references through a pointer (this->f, q->f, loads and stores, also ones the optimizer folds away
like `f = f`) the function creates before it: period 4 for distinct loads (TiTi, iTiT, iiii, iiii, TiTi ...).
Plain global and parameter loads, dead `{int t = f;}` and `if (f) {}` and code after T do not count.
Usage: uv run python run.py   (FE_PFX_BASE, default 21)"""
import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))) + '/inline')
import ib
PFX = os.environ.get('FE_PFX_BASE', '21')
H = '''#include <string.h>
struct E { virtual void redraw(); char pad[0xa10]; char text_[256]; int b18; int f1,f2,f3,f4,f5,f6,f7,f8;
  int b40; int b44; E *nx; int f(int k, E *q); };
void g(char*); void u(int); extern int G1; struct R { int a, b; }; void ur(R*);
'''
T = ' g(&text_[b44]); g(&text_[b40]); g(&text_[b44]); g(&text_[b40]); return 1;'
CASES = {'none': '', 'u(G1)': 'u(G1);', 'u(k)': 'u(k);', 'local R': 'R r; r.a = k; ur(&r);', 'dead': '{int t = f1;}',
         'if(f1){}': 'if (f1) {}', 'f1=f1': 'f1 = f1;', 'f1=0': 'f1 = 0;', 'q->f1': 'u(q->f1);', 'nx->f1': 'u(nx->f1);',
         'u(f1)x2': 'u(f1); u(f1);'}
for n in range(1, 6):
    CASES['ld%d' % n] = ''.join('u(f%d);' % (i + 1) for i in range(n))
    CASES['st%d' % n] = ''.join('f%d = 0;' % (i + 1) for i in range(n))
if __name__ == '__main__':
    src = H
    for j, (k, p) in enumerate(CASES.items()):
        src += 'int E::m%d(int k, E *q) { %s%s }\n' % (j, p, T)
        src = src.replace('int f(int k, E *q);', 'int f(int k, E *q); int m%d(int k, E *q);' % j)
    asm, _ = ib.compile_asm(None, PFX, text=src)
    F = {n.split('(')[0].split('::')[-1]: L for n, s, L in ib.functions(asm)}
    for j, (k, p) in enumerate(CASES.items()):
        o = ''.join('T' if m.group(1) == 'esi' else 'i' for m in
                    (re.match(r'lea\s+\w+, DWORD PTR \[(\w+)\+(\w+)\+2580\]', l) for l in F['m%d' % j]) if m)
        print('%-10s %s' % (k, o))
