"""par-6 r54: blocker (3) host split by other join predecessors sharing the toggle tail's suffix
(`add esp,8; jmp`, `call dmap; add esp,8`) placed before/after the toggles; p28 harness, ii17 tail.
psave-calls 1 = merged (ours), 4 = kept (image)."""
import sys; from cj import *
import cj
from p1 import T as TG, toggle
V = []
TAIL = 'psave(0); dmap(1);'
def mk(n, before='', after=''):
    s = ''.join('static inline void tg%d_%s(int c) %s\n' % (i, n, toggle(i, *t, tail=TAIL)) for i, t in enumerate(TG))
    s += 'static inline void bn_%s(int c) { int f = gmp; int n; if (f & 0x1000) n = f & 0xFFFFCFFF; else if (f & 0x2000) n = f | 0x1000; else n = f | 0x2000; gmp = n; imp = n; %s }\n' % (n, TAIL)
    s += 'void t_%s(int c) {\n switch (c) {\n  case 1: f0(); break;\n%s' % (n, before) + ''.join('  case %d: tg%d_%s(c); break;\n' % (10 + i, i, n) for i in range(3))
    s += '  case 13: { gx ^= 0x40000; dmap(1); } break;\n  case 14: bn_%s(c); break;\n%s  case 30: use(30); break;\n  }\n' % (n, after)
    s += '  if (gx & 0x40) { switch (c) { case 40: f0(); return; case 41: use(41); return; } }\n}\n'
    V.append((n, s))
def cs(k0, bodies): return ''.join('  case %d: %s break;\n' % (k0 + i, b) for i, b in enumerate(bodies))
mk('base')
H2 = ['chk(%d, 0);' % i for i in (2, 1, 0, 14)]
mk('h2before', before=cs(200, H2))
mk('h2after', after=cs(200, H2))
mk('h2both', before=cs(200, H2[:2]), after=cs(210, H2[2:]))
D = ['use(5); dmap(1);', 'f0(); dmap(1);', 'psave(gx); dmap(1);', 'psave(gx); dmap(gp);']
for i, d in enumerate(D):
    mk('d%dbefore' % i, before=cs(300, [d]))
    mk('d%dafter' % i, after=cs(300, [d]))
mk('p0before', before=cs(300, ['psave(0); dmap(1);']))
mk('p0after', after=cs(300, ['psave(0); dmap(1);']))
mk('stp0before', before=cs(300, ['gx = 5; psave(0); dmap(1);']))
mk('d1twobefore', before=cs(300, ['f0(); dmap(1);', 'use(3); dmap(1);']))
run(V, show=sys.argv[1:])
