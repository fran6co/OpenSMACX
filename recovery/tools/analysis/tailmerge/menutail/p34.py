"""par-6 r54: EH state at the tail's start (p28 harness, ii17 tail).  Q has a non-inline dtor; `oneQ` wraps one
toggle in `{ Q q; ...; }` (its tail runs in a different state, dtor after), `tailQ` puts the object's scope
around the toggles' tails only in two of them, `allQ` wraps every toggle, `topQ` declares Q before the switch.
psave-calls 1 = merged (ours), 4 = kept (image)."""
import sys; from cj import *
import cj
from p1 import T as TG, toggle
cj.HDR += 'struct Q { Q(); ~Q(); int x; };\n'
V = []
TAIL = 'psave(0); dmap(1);'
def mk(n, wrap=(), top=''):
    s = ''.join('static inline void tg%d_%s(int c) %s\n' % (i, n, toggle(i, *t, tail=TAIL)) for i, t in enumerate(TG))
    s += 'static inline void bn_%s(int c) { int f = gmp; int n; if (f & 0x1000) n = f & 0xFFFFCFFF; else if (f & 0x2000) n = f | 0x1000; else n = f | 0x2000; gmp = n; imp = n; %s }\n' % (n, TAIL)
    s += 'void t_%s(int c) {\n %s switch (c) {\n  case 1: f0(); break;\n' % (n, top)
    for i in range(3):
        b = 'tg%d_%s(c);' % (i, n)
        if i in wrap: b = '{ Q q; q.x = c; %s }' % b
        s += '  case %d: %s break;\n' % (10 + i, b)
    b = 'bn_%s(c);' % n
    if 3 in wrap: b = '{ Q q; q.x = c; %s }' % b
    s += '  case 13: { gx ^= 0x40000; dmap(1); } break;\n  case 14: %s break;\n  case 30: use(30); break;\n  }\n' % b
    s += '  if (gx & 0x40) { switch (c) { case 40: f0(); return; case 41: use(41); return; } }\n}\n'
    V.append((n, s))
mk('base'); mk('oneQ', (0,)); mk('twoQ', (0, 1)); mk('allQ', (0, 1, 2, 3)); mk('bnQ', (3,)); mk('topQ', top='Q q; q.x = c;')
mk('topQ_one', (1,), top='Q q; q.x = c;')
run(V, show=sys.argv[1:])
