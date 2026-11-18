import sys; sys.path.insert(0,'.'); from deck import *
# One level-2 candidate: P { S s; ~P(){dm(); close();} }, S { FB f1,f2; ~S(){dm2(); close();} }
HDR='''
extern void dm2(); extern void dm3();
struct GW { virtual ~GW(); int x[4]; };
struct FB : GW { FB(); void close(); virtual ~FB() { close(); } };
struct S : GW { FB f1; FB f2; S(); void close(); virtual ~S() { dm2(); close(); } };
struct P : GW { S s; P(); void close(); virtual ~P() { dm(); close(); } };
'''
def fn(name,M):
    body=' '.join('if (cond()) { at(%d); return; }'%i for i in range(1,M+1))
    return 'void %s() { P p; %s at(%d); }'%(name,body,M+1)
Ms=[int(x) for x in sys.argv[1:]] or list(range(1,17))
P=[HDR]+[fn('r%d'%M,M) for M in Ms]
r=run(P)
for M in Ms:
    st,tl,l=r['r%d'%M]
    print('%3d'%M, ''.join('S' if 'E2' in tl.get(i,[]) else ('s' if 'cS' in tl.get(i,[]) else ('P' if 'cP' in tl.get(i,[]) else '?')) for i in range(1,M+2)))
