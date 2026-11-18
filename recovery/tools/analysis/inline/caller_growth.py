import sys; sys.path.insert(0,'.'); import ib
# caller = N calls of f4 + M filler statements of kind F (placed at end); count expansions
FILL={'store':'g3 = %d;','call':'use(%d);','if':'if (g4 == %d) use(1);','add':'g3 += %d;','ifs2':'if (g4 == %d) { use(1); use(2); }'}
def run(N,Ms,kind,nif=4,pos='end'):
    src=['#include "probe.h"','inline void f(int k) { %s }'%' '.join('if (g%s == 1000) mk(k);'%('' if i==0 else str(i+1)) for i in range(nif))]
    for M in Ms:
        fill=' '.join(FILL[kind]%(2000+i) for i in range(M))
        calls=' '.join('f(%d);'%i for i in range(1,N+1))
        src.append('void c_%d() { %s }'%(M, calls+' '+fill if pos=='end' else fill+' '+calls))
    r=ib.report(None,'21',text='\n'.join(src),quiet=True)
    out=[]
    for M in Ms:
        s,t,l=r['c_%d'%M]
        out.append((M,sum(1 for k,c,x in s if x=='E' and k<1000)//nif))
    return out
if __name__=='__main__':
    N=int(sys.argv[1]); kind=sys.argv[2]
    print(kind,N,run(N,[0,10,20,40,80,160,320],kind))
