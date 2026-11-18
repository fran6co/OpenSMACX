import sys; sys.path.insert(0,'.'); from deck import *
from dtor_nested_hdr import HDR
# dtor return sites (P{S}) with n flat-cost probes q(k) placed after return site `pos`
M=int(sys.argv[1]); n=int(sys.argv[2]); kind=sys.argv[3] if len(sys.argv)>3 else 'if4'
Q={'if4':'inline void q(int k) { mk(k); %s }'%' '.join('if (ga[%d]==1000) use(%d);'%(i,i) for i in range(4)),
   'if1':'inline int q(int x) { if (x == 0) x = 1; return x; }'}
def fn(name,pos):
    parts=['P p;']
    for i in range(1,M+1):
        if i-1==pos: parts.append(' '.join(('q(%d);' if kind=='if4' else 'g3 = q(%d);')%(900+j) for j in range(n)))
        parts.append('if (cond()) { at(%d); return; }'%i)
    if pos==M: parts.append(' '.join(('q(%d);' if kind=='if4' else 'g3 = q(%d);')%(900+j) for j in range(n)))
    parts.append('at(%d);'%(M+1))
    return 'void %s() { %s }'%(name,' '.join(parts))
P=[HDR,Q[kind]]+[fn('p%d'%pos,pos) for pos in range(0,M+1)]
r=run(P)
for pos in range(0,M+1):
    st,tl,l=r['p%d'%pos]
    s=''.join('S' if 'E2' in tl.get(i,[]) else ('s' if 'cS' in tl.get(i,[]) else ('P' if 'cP' in tl.get(i,[]) else '?')) for i in range(1,M+2))
    qe=sum(1 for k,v in st.items() if k>=900 and v=='E')
    print('probes after site %2d: %s  (q expanded %d/%d)'%(pos,s,qe,n))
