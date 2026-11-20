import sys; from cj import *
import cj, p1
from p1 import fn
V=[]
def sub(name,a,b,**kw):
    return fn(name,1,1,**kw).replace(a,b)
V.append(('auto_inline', sub('au','static inline void','void')))
V.append(('inl_explicit_return', sub('er','dmap(1); }','dmap(1); return; }')))
V.append(('gotos_to_end', fn('ge',1,1,end='goto END;',after='  END:\n  if (gx & 0x40) { switch (c) { case 40: f0(); return; case 41: use(41); return; } }\n')))
V.append(('label_after_switch', fn('ls',1,1,after='  LBL: ;\n  if (gx & 0x40) { switch (c) { case 40: f0(); return; case 41: use(41); goto LBL; } }\n')))
# each toggle via a member of a class
run(V, show=sys.argv[1:])
