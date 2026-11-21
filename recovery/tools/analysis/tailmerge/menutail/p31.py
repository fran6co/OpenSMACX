"""par-6 r54: does an `||` site (MATCHING_GUIDE 3.7 orgoto rule) split or move the JJ host of the toggle tails?
p28's ii17 harness (3 toggles + base_names, 17-byte `psave(0); dmap(1)` tail).  psave-calls 1 = merged (ours), 4 = kept (image)."""
import sys; from cj import *
import cj
from p1 import T as TG, toggle
V = []
TAIL = 'psave(0); dmap(1);'
def mk(n, pre='', c1='f0(); break;', bnpre='', post='', tgpre=''):
    s = ''.join('static inline void tg%d_%s(int c) %s\n' % (i, n, toggle(i, *t, tail=TAIL)) for i, t in enumerate(TG))
    s += 'static inline void bn_%s(int c) { %s int f = gmp; int n; if (f & 0x1000) n = f & 0xFFFFCFFF; else if (f & 0x2000) n = f | 0x1000; else n = f | 0x2000; gmp = n; imp = n; %s }\n' % (n, bnpre, TAIL)
    s += 'void t_%s(int c) {\n %s switch (c) {\n  case 1: %s\n' % (n, pre, c1) + ''.join('  case %d: %s tg%d_%s(c); break;\n' % (10 + i, tgpre, i, n) for i in range(3))
    s += '  case 13: { gx ^= 0x40000; dmap(1); } break;\n  case 14: bn_%s(c); break;\n  case 30: use(30); break;\n  }\n%s' % (n, post)
    s += '  if (gx & 0x40) { switch (c) { case 40: f0(); return; case 41: use(41); return; } }\n}\n'
    V.append((n, s))
mk('base')
mk('preor', pre='if (gx == 3 || gmp == 4) return;')
mk('preand', pre='if (gx == 3 && gmp == 4) return;')
mk('c1or', c1='if (gx == 3 || gmp == 4) f0(); break;')
mk('c1orret', c1='if (gx == 3 || gmp == 4) return; f0(); break;')
mk('c1orgo', c1='if (gx == 3 || gmp == 4) goto L; f0(); break; L: use(5); break;')
mk('bnor', bnpre='if (gx == 3 || gmp == 4) return;')
mk('tgor', tgpre='if (gx == 3 || gmp == 4) break;')
mk('postor', post='  if (gx == 3 || gmp == 4) return;\n')
mk('preorgo', pre='if (gx == 3 || gmp == 4) goto X; use(9); X:')
run(V, show=sys.argv[1:])
