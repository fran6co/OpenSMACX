"""Cross-jump blocker screen 3 (par-6 r53): which natural constructs give each tail its own jump target
(p21 `labels` keeps all four copies).  psave-calls 1 = merged, 4 = kept."""
import sys; from cj import *
import cj
from p1 import INL, INLBN, T as TG, toggle, BN
cj.HDR += 'struct TT { int v; ~TT() {} }; struct DD { DD(); ~DD(); };\n'
V = []
def mk(name, cases, pre='', inl=None, bn=None, after=None):
    s = (inl if inl is not None else INL.replace('%s', name)) + (bn if bn is not None else INLBN % name) + pre
    s += 'void t_%s(int c) {\n  switch (c) {\n  case 1: f0(); break;\n%s  case 30: use(30); break;\n  }\n%s}\n' % (
        name, cases, after if after is not None else '  if (gx & 0x40) { switch (c) { case 40: f0(); return; case 41: use(41); return; } }\n')
    V.append((name, s))
def std(n, fmt='  case %d: %s break;\n'):
    return ''.join(fmt % (10 + i, 'tg%d_%s();' % (i, n)) for i in range(3)) + fmt % (14, 'bn_%s();' % n)
mk('base', std('base'))
mk('braces', std('braces', '  case %d: { %s } break;\n'))
mk('scope_tt', std('scope_tt', '  case %d: { TT t; t.v = c; %s } break;\n'))
mk('scope_dd', std('scope_dd', '  case %d: { DD d; %s } break;\n'))
mk('label_end', ''.join('  case %d: tg%d_label_end(); E%d: break;\n' % (10 + i, i, i) for i in range(3)) + '  case 14: bn_label_end(); E3: break;\n')
# early return inside each inline (on a global flag)
er = ''.join('static inline void tg%d_er() { if (gx & 0x800) return; %s }\n' % (i, toggle(i, *t)) for i, t in enumerate(TG))
mk('early_ret', std('er'), inl=er, bn='static inline void bn_er() { if (gx & 0x800) return; %s }\n' % BN)
V[-1] = ('early_ret', V[-1][1].replace('t_early_ret', 't_er'))
# void return of a nested inline: tails in a shared inline `save()` called by each toggle
sv = 'static inline void sv_s() { psave(0); dmap(1); }\n'
inl = sv + ''.join('static inline void tg%d_sh() %s\n' % (i, toggle(i, *t, tail='sv_s();')) for i, t in enumerate(TG))
mk('shared_inline', std('sh'), inl=inl, bn='static inline void bn_sh() ' + BN.replace('psave(0); dmap(1);', 'sv_s();') + '\n')
V[-1] = ('shared_inline', V[-1][1].replace('t_shared_inline', 't_sh'))
# the switch is the then-arm of an if with an else
mk('if_else', std('if_else'), after='')
V[-1] = ('if_else', V[-1][1].replace('  switch (c) {\n  case 1', '  if (c != 999) {\n  switch (c) {\n  case 1').replace('  }\n}\n', '  }\n  } else use(999);\n  if (gx & 0x40) use(41);\n}\n'))
# tails as the last statement of an if inside the case
mk('if_in_case', std('if_in_case', '  case %d: if (gx != 7) { %s } break;\n'))
run(V, show=sys.argv[1:])
