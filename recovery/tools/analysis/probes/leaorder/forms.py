#!/usr/bin/env python3
"""forms.py: which natural source forms count as member references for the lea-order counter (MATCHING_GUIDE 3.4).
Each case X is compiled as `X; T` and as `X; u(z9); T` (one extra distinct load); the pair of patterns gives the
counter phase mod 4 unambiguously (0: TiTi/iTiT, 1: iTiT/iiii, 2: iiii/iiii, 3: iiii/TiTi).  Loads count 1,
stores 2 (run.py).  Prints the phase of every case; the delta from `none` is what the form contributes.
Usage: uv run python forms.py   (FE_PFX_BASE, default 21)"""
import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))) + '/inline')
import ib
PFX = os.environ.get('FE_PFX_BASE', '21')
H = '''#include <string.h>
struct RC { int l, t, r, b; };
struct E { virtual void redraw(); char pad[0xa10]; char text_[256]; int b18; int f1,f2,f3,f4,f5,f6,f7,f8;
  int b40; int b44; E *nx; RC rc; RC rc2; int z9;
  int get1() { return f1; } int &ref1() { return f1; } void set1(int v) { f1 = v; } int flag1() { return f1 & 1; }
  %s };
void g(char*); void u(int); extern int G1; extern RC GR; void ur(RC*);
'''
T = ' g(&text_[b44]); g(&text_[b40]); g(&text_[b44]); g(&text_[b40]); return 1;'
CASES = [
    ('none', ''),
    ('ld1 u(f1)', 'u(f1);'),
    ('st1 f1=k', 'f1 = k;'),
    ('st1 f1=1', 'f1 = 1;'),
    ('f1=0;f1|=1', 'f1 = 0; f1 |= 1;'),
    ('f1|=1', 'f1 |= 1;'),
    ('f1|=k', 'f1 |= k;'),
    ('f1&=~1', 'f1 &= ~1;'),
    ('f1+=k', 'f1 += k;'),
    ('f1++', 'f1++;'),
    ('f1=k;f1=G1 dead st', 'f1 = k; f1 = G1;'),
    ('f1=f2;f1=k', 'f1 = f2; f1 = k;'),
    ('f1=f1', 'f1 = f1;'),
    ('f1=0;if(f1)u', 'f1 = 0; if (f1) u(1);'),
    ('if(f1)u', 'if (f1) u(1);'),
    ('t=f1;t=k;u(t)', 'int t = f1; t = k; u(t);'),
    ('get1();', 'get1();'),
    ('(void)f1', '(void)f1;'),
    ('f1;', 'f1;'),
    ('u(get1())', 'u(get1());'),
    ('ref1()=k', 'ref1() = k;'),
    ('set1(k)', 'set1(k);'),
    ('if(flag1())', 'if (flag1()) u(1);'),
    ('rc=GR', 'rc = GR;'),
    ('rc.l..b=GR', 'rc.l = GR.l; rc.t = GR.t; rc.r = GR.r; rc.b = GR.b;'),
    ('rc=rc2', 'rc = rc2;'),
    ('ur(&rc)', 'ur(&rc);'),
    ('RC r=rc;ur(&r)', 'RC r = rc; ur(&r);'),
    ('p=this;u(p->f1)', 'E *p = this; u(p->f1);'),
    ('this->f1', 'u(this->f1);'),
    ('q->f1', 'u(q->f1);'),
    ('q->f1=k', 'q->f1 = k;'),
    ('u(f1);u(f1)', 'u(f1); u(f1);'),
    ('u(f1+f1)', 'u(f1 + f1);'),
    ('f1=k;u(f1)', 'f1 = k; u(f1);'),
    ('u(f1);f1=k', 'u(f1); f1 = k;'),
    ('f1=f2=k', 'f1 = f2 = k;'),
    ('u(f1?f2:f3)', 'u(f1 ? f2 : f3);'),
    ('u(f1&&f2)', 'u(f1 && f2);'),
    ('u(text_[0])', 'u(text_[0]);'),
    ('u(text_[b44])', 'u(text_[b44]);'),
    ('memset rc', 'memset(&rc, 0, sizeof rc);'),
    ('u(&f1==0)', 'u(&f1 == 0);'),
]
def phase(a, b):
    return {('TiTi', 'iTiT'): 0, ('iTiT', 'iiii'): 1, ('iiii', 'iiii'): 2, ('iiii', 'TiTi'): 3}.get((a, b), '?%s/%s' % (a, b))
if __name__ == '__main__':
    decl, body = '', ''
    for j, (k, p) in enumerate(CASES):
        for x, extra in ((0, ''), (1, 'u(z9);')):
            decl += 'int m%d_%d(int k, E *q);' % (j, x)
            body += 'int E::m%d_%d(int k, E *q) { %s%s%s }\n' % (j, x, p, extra, T)
    asm, _ = ib.compile_asm(None, PFX, text=H % decl + body)
    F = {n.split('(')[0].split('::')[-1]: L for n, s, L in ib.functions(asm)}
    pat = lambda name: ''.join('T' if m.group(1) == 'esi' else 'i' for m in
                               (re.match(r'lea\s+\w+, DWORD PTR \[(\w+)\+(\w+)\+2580\]', l) for l in F[name]) if m)
    for j, (k, p) in enumerate(CASES):
        print('%-22s %s' % (k, phase(pat('m%d_0' % j), pat('m%d_1' % j))))
