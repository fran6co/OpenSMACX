import sys; from cj import *
import cj, p1
from p1 import fn
V=[]
def var(name, tail, pre=''):
    s=fn(name,1,1)
    s=s.replace('psave(0); dmap(1);', tail)
    return pre%{'n':name}+s if pre else s
V.append(('base', var('b0','psave(0); dmap(1);')))
V.append(('dmap_inline_helper', var('b1','psave(0); rd_b1();', 'static inline void rd_%(n)s() { dmap(1); }\n')))
V.append(('both_inline_helper', var('b2','sv_b2();', 'static inline void sv_%(n)s() { psave(0); dmap(1); }\n')))
V.append(('psave_inline_helper', var('b3','ps_b3(); dmap(1);', 'static inline void ps_%(n)s() { psave(0); }\n')))
V.append(('label_between', var('b4','psave(0); { L: ; } dmap(1);')))
V.append(('empty_if_between', var('b5','psave(0); if (0) {} dmap(1);')))
V.append(('block_scopes', var('b6','{ psave(0); } { dmap(1); }')))
V.append(('local0', var('b7','int z = 0; psave(z); dmap(1);')))
V.append(('do_while0', var('b8','do { psave(0); } while (0); dmap(1);')))
V.append(('cond_between', var('b9','psave(0); if (gx == 77) f0(); dmap(1);')))
run(V, show=sys.argv[1:])
