import sys; sys.path.insert(0,'.'); import ib
def body(nif): return ' '.join('if (g%s == 1000) mk(k);'%('' if i==0 else str(i+1)) for i in range(nif))
src=['#include "probe.h"']
N=int(sys.argv[1]) if len(sys.argv)>1 else 120
for nif in (1,2,3,4,6,8):
    src.append('inline void f%d(int k) { %s }'%(nif,body(nif)))
    src.append('void c%d() { %s }'%(nif,' '.join('f%d(%d);'%(nif,i) for i in range(1,N+1))))
r=ib.report(None,'21',text='\n'.join(src),quiet=True)
def runs(st):
    out=[];prev=None
    for k in range(1,N+1):
        v=st.get(k,'?')
        if out and out[-1][0]==v: out[-1][2]=k
        else: out.append([v,k,k])
    return ' '.join('%s%d-%d'%(v,a,b) if a!=b else '%s%d'%(v,a) for v,a,b in out)
for n,(s,t,l) in r.items():
    if not n.startswith('c'): continue
    st={}
    for k,c,x in s: st[k]=x
    print(n,runs(st))
