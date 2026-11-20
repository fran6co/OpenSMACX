"""Cross-jump then copy-back (par-6 r53): the JJ-merged block's size.  Toggle tails `psave(A); dmap(B);`
with A/B a register (the live parameter c, 1-byte push) or an immediate (2 bytes): merged block
push+call+push+call+add esp,8 = 15 bytes (both registers), 16 or 17.  psave-calls 1 = merged."""
import sys; from cj import *
import cj
from p1 import T as TG, toggle
V = []
def mk(n, tail):
    s = ''.join('static inline void tg%d_%s(int c) %s\n' % (i, n, toggle(i, *t, tail=tail)) for i, t in enumerate(TG))
    s += 'static inline void bn_%s(int c) { int f = gmp; int n; if (f & 0x1000) n = f & 0xFFFFCFFF; else if (f & 0x2000) n = f | 0x1000; else n = f | 0x2000; gmp = n; imp = n; %s }\n' % (n, tail)
    s += 'void t_%s(int c) {\n  switch (c) {\n  case 1: f0(); break;\n' % n + ''.join('  case %d: tg%d_%s(c); break;\n' % (10 + i, i, n) for i in range(3))
    s += '  case 13: { gx ^= 0x40000; dmap(1); } break;\n  case 14: bn_%s(c); break;\n  case 30: use(30); break;\n  }\n' % n
    s += '  if (gx & 0x40) { switch (c) { case 40: f0(); return; case 41: use(41); return; } }\n}\n'
    V.append((n, s))
mk('rr15', 'psave(c); dmap(c);')
mk('ri16', 'psave(c); dmap(1);')
mk('ir16', 'psave(0); dmap(c);')
mk('ii17', 'psave(0); dmap(1);')
if "--more" not in sys.argv and "--size" not in sys.argv: run(V, show=sys.argv[1:])
if '--more' in sys.argv:
    V.clear()
    for k, t in [('rr15', 'psave(c); dmap(c);'), ('f0x3', 'f0(); f0(); f0();'), ('rf0f0', 'psave(c); f0(); f0();'),
                 ('rrr21', 'psave(c); dmap(c); use(c);'), ('strr', 'gx = 1; psave(c); dmap(c);'),
                 ('rrst', 'psave(c); dmap(c); gx = c;'), ('gr', 'psave(gx); dmap(c);'), ('r1', 'psave(c);'),
                 ('rf0', 'psave(c); f0();'), ('ri', 'psave(c); dmap(1);'), ('rrst4', 'psave(c); dmap(c); gx = c; gmp = c;')]:
        mk(k, t)
    run(V, show=[a for a in sys.argv[1:] if a != '--more'])
if '--size' in sys.argv:
    # one pushing call but over 15 bytes; no pushes at all; 15 vs 16 with a store
    V.clear()
    for k, t in [('one18', 'psave(0x100); f0();'), ('one14', 'psave(c); f0();'), ('calls15', 'f0(); f0(); f0();'),
                 ('calls20', 'f0(); f0(); f0(); f0();'), ('sto15', 'gmp = c; f0(); f0();'), ('sto16', 'gmp = 1; f0(); f0();'),
                 ('gxr16', 'gx = c; f0(); f0();'), ('gxi20', 'gx = 1; f0(); f0();'), ('u19', 'use(c); f0(); f0();'),
                 ('u14b', 'f0(); use(c);'), ('u19b', 'f0(); f0(); use(c);'), ('gxr_u', 'gx = c; use(c);'), ('gxi_u', 'gx = 1; use(c);')]:
        mk(k, t)
    run(V)
