"""JJ merge of a 16-byte `push esi; call; push 1; call; add esp,8` tail (ri16 of p28) against the number
of tails and which predecessors share it (par-6 r53).  Count of psave calls = copies kept."""
import sys; from cj import *
import cj
from p1 import T as TG, toggle
V = []
def mk(n, k, bn, tail='psave(c); dmap(1);'):
    T3 = (TG * 3)[:k]
    s = ''.join('static inline void tg%d_%s(int c) %s\n' % (i, n, toggle(i, *t, tail=tail)) for i, t in enumerate(T3))
    s += 'static inline void bn_%s(int c) { int f = gmp; int n; if (f & 0x1000) n = f & 0xFFFFCFFF; else if (f & 0x2000) n = f | 0x1000; else n = f | 0x2000; gmp = n; imp = n; %s }\n' % (n, tail)
    s += 'void t_%s(int c) {\n  switch (c) {\n  case 1: f0(); break;\n' % n + ''.join('  case %d: tg%d_%s(c); break;\n' % (10 + i, i, n) for i in range(k))
    if bn: s += '  case 99: bn_%s(c); break;\n' % n
    s += '  case 98: { gx ^= 0x40000; dmap(1); } break;\n  case 97: use(30); break;\n  }\n'
    s += '  if (gx & 0x40) { switch (c) { case 140: f0(); return; case 141: use(41); return; } }\n}\n'
    V.append((n, s))
for k in (1, 2, 3, 4, 6):
    mk('k%dbn' % k, k, 1)
    mk('k%d' % k, k, 0)
run(V, show=sys.argv[1:])
