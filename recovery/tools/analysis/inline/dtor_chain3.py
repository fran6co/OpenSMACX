import sys; sys.path.insert(0,'.'); from deck import *
def ifs(n,base): return ' '.join('if (ga[%d]==1000) use(%d);'%(base+i,base+i) for i in range(n))
n1,n2,n3=[int(x) for x in sys.argv[1:4]]
P=['extern void dm2(); extern void dm3();',
   'struct C { int v; C(); ~C() { dm3(); %s } };'%ifs(n3,30),
   'struct B { C c; int v; B(); ~B() { dm2(); %s } };'%ifs(n2,20),
   'struct A { B b; int v; A(); ~A() { dm(); %s } };'%ifs(n1,10)]
def fn(name,M):
    body=' '.join('if (cond()) { at(%d); return; }'%i for i in range(1,M+1))
    return 'void %s() { A a; %s at(%d); }'%(name,body,M+1)
Ms=[int(x) for x in sys.argv[4:]] or [5,10,20,40]
for M in Ms: P.append(fn('r%d'%M,M))
show(run(P),['r%d'%M for M in Ms])
