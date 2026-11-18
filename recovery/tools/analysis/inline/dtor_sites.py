import sys; sys.path.insert(0,'.'); from deck import *
P=['struct D { int v; D(); ~D() { dm(); if (ga[1]==1000) use(1); if (ga[2]==1000) use(2); if (ga[3]==1000) use(3); if (ga[4]==1000) use(4);} };']
def fn(name,M,pre=''):
    body=' '.join('if (cond()) { at(%d); return; }'%i for i in range(1,M+1))
    return 'void %s() { D d; %s %s at(%d); }'%(name,pre,body,M+1)
for M in (5,10,20,40):
    P.append(fn('r%d'%M,M))
show(run(P),['r5','r10','r20','r40'])
