import sys; sys.path.insert(0,'.'); import ib
HDR='#include "stdafx.h"\n#include "popup.h"\nextern void at(int); extern int cond(); extern void use(int); extern int ga[];\n'
def fn(name,M,pre=''):
    body=' '.join('if (cond()) { at(%d); return; }'%i for i in range(1,M+1))
    return 'void %s() { Popup p; %s %s at(%d); }'%(name,pre,body,M+1)
Ms=[int(x) for x in sys.argv[1:]]
asm=ib.compile_in_unit('src/probe.cpp',HDR+'\n'.join(fn('r%d'%M,M) for M in Ms),'21')
F={n:l for n,s,l in ib.functions(asm)}
for M in Ms:
    tl=ib.tails(F['r%d'%M])
    def cls(t):
        if 'c~Popup' in t or 'cPopup' in t: return 'P'
        if 'cScroll' in t: return 's'
        if any(x.startswith('?close@Scroll') or x=='close' for x in t): pass
        return 'S' if any('close@Scroll' in x for x in t) else '?'
    print('%3d'%M, ''.join(cls(tl.get(i,[])) for i in range(1,M+2)))
