"""par-6 r54: does the 15-byte JJ limit see a frame reference's displacement size?  Toggles written in the
cases (cj.toggle) plus base_names, tails storing into a function-level volatile local: `L[0] = c; f0(); f0();`
is 3+10 = 13 bytes with a disp8 slot (small frame) and 6+10 = 16 with a disp32 slot (L[k] past 0x80).
Prints the number of stores into L (4 = every tail kept, 1 = merged)."""
import sys, os, re
from cj import *
import cj, ib
cj.HDR += 'extern Mn m;\n'
from p1 import T as TG
V = []
def mk(n, decl, tail):
    s = 'void t_%s(int c) {\n  %s\n  switch (c) {\n  case 1: f0(); break;\n' % (n, decl)
    s += ''.join('  case %d: %s break;\n' % (10 + i, toggle(i, *t, tail=tail)) for i, t in enumerate(TG))
    s += '  case 13: { gx ^= 0x40000; dmap(1); } break;\n  case 14: { int f = gmp; int n; if (f & 0x1000) n = f & 0xFFFFCFFF; else if (f & 0x2000) n = f | 0x1000; else n = f | 0x2000; gmp = n; imp = n; %s } break;\n  case 30: use(30); break;\n  }\n' % tail
    s += '  if (gx & 0x40) { switch (c) { case 40: f0(); return; case 41: use(L[1]); return; } }\n}\n'
    V.append((n, s))
mk('s_reg13', 'volatile int L[2];', 'L[0] = c; f0(); f0();')
mk('b_reg16', 'volatile int L[400];', 'L[0] = c; f0(); f0();')
mk('b_hi_reg13', 'volatile int L[400];', 'L[399] = c; f0(); f0();')
mk('s_imm17', 'volatile int L[2];', 'L[0] = 1; f0(); f0();')
mk('g_reg16', 'volatile int L[2];', 'gx = c; f0(); f0();')
mk('s_reg13x', 'volatile int L[2];', 'f0(); L[0] = c; f0();')
mk('b_reg16x', 'volatile int L[400];', 'f0(); L[0] = c; f0();')
src = cj.HDR + ''.join(v for _, v in V)
asm, out = ib.compile_asm(None, os.environ.get('FE_PFX_BASE', '33'), text=src)
if not asm: print(out); sys.exit(1)
for k, _ in V:
    f = [x for x in ib.functions(asm) if ('t_%s@@' % k) in x[0] or ('t_%s@@' % k) in x[1]][0]
    L = f[2]
    n = sum(1 for l in L if l.startswith('mov') and ('_L$' in l.split(',')[0] or '?gx@@' in l.split(',')[0]) and 'xor' not in l)
    d = [l for l in L if l.startswith('mov') and ('_L$' in l.split(',')[0])][:1]
    print('%-12s stores %d   %s' % (k, n, d[0] if d else ''))
    if k in sys.argv[1:]: print('\n'.join(L))
