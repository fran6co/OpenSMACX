import sys; sys.path.insert(0,'.'); from deck import *
# Popup-like chain: P : Bs { S s; } ; S : GW { FB f1, f2; }
HDR='''
extern void dm2(); extern void dm3();
struct GW { virtual ~GW(); int x[4]; };
struct FB : GW { FB(); void close(); virtual ~FB() { close(); } };
struct S : GW { FB f1; FB f2; S(); void close(); virtual ~S() { dm2(); close(); } };
struct Bs : GW { FB b1; FB b2; Bs(); void close(); virtual ~Bs() { dm3(); close(); } };
struct P : Bs { S s; P(); void close(); virtual ~P() { dm(); close(); } };
'''
def fn(name,M,filler=''):
    body=' '.join('if (cond()) { %s at(%d); return; }'%(filler,i) for i in range(1,M+1))
    return 'void %s() { P p; %s at(%d); }'%(name,body,M+1)
Ms=[int(x) for x in sys.argv[1:]] or [5,10,20,40]
P=[HDR]
for M in Ms: P.append(fn('r%d'%M,M))
show(run(P),['r%d'%M for M in Ms])
