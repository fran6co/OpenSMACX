"""Cross-jump blocker screen 7 (par-6 r53): inline toggles that RETURN a value (ignored by the case),
member inlines on `this`, and returns in the middle.  psave-calls 1 = merged, 4 = kept."""
import sys; from cj import *
import cj
from p1 import T as TG, toggle, BN
cj.HDR += 'extern Mn m;\n'
V = []
def mk(n, ret, retexpr, usebn=True, member=False):
    pre = 'struct C%s { int pad; void t_%s(int c);' % (n, n) if member else ''
    body = lambda i, t: toggle(i, *t)[:-1] + ' return %s; }' % retexpr if ret != 'void' else toggle(i, *t)
    s = ''
    if member:
        s += pre + ''.join(' %s tg%d() %s' % (ret, i, body(i, t)) for i, t in enumerate(TG)) + ' %s bn() %s };\n' % (ret, BN[:-1] + (' return %s; }' % retexpr if ret != 'void' else '}'))
        s += 'void C%s::t_%s(int c) {\n' % (n, n)
        call = lambda i: 'tg%d();' % i; bcall = 'bn();'
    else:
        s += ''.join('static inline %s tg%d_%s() %s\n' % (ret, i, n, body(i, t)) for i, t in enumerate(TG))
        s += 'static inline %s bn_%s() %s\n' % (ret, n, BN[:-1] + (' return %s; }' % retexpr if ret != 'void' else '}'))
        s += 'void t_%s(int c) {\n' % n
        call = lambda i: 'tg%d_%s();' % (i, n); bcall = 'bn_%s();' % n
    s += '  switch (c) {\n  case 1: f0(); break;\n' + ''.join('  case %d: %s break;\n' % (10 + i, call(i)) for i in range(3))
    s += '  case 13: { gx ^= 0x40000; dmap(1); } break;\n  case 14: %s break;\n  case 30: use(30); break;\n  }\n' % bcall
    s += '  if (gx & 0x40) { switch (c) { case 40: f0(); return; case 41: use(41); return; } }\n}\n'
    V.append((n, s))
mk('void', 'void', '')
mk('ret0', 'int', '0')
mk('ret1', 'int', '1')
mk('retg', 'int', 'gx')
mk('mvoid', 'void', '', member=True)
mk('mret1', 'int', '1', member=True)
run(V, show=sys.argv[1:])
