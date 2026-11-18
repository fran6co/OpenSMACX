import sys; sys.path.insert(0,'.'); from deck import *
from dtor_nested_hdr import HDR
M=int(sys.argv[1]); where=sys.argv[2] if len(sys.argv)>2 else 'top'
Ks=range(0,int(sys.argv[3]) if len(sys.argv)>3 else 60,2)
def fn(name,M,K):
    fill=' '.join('g3 = %d;'%(5000+i) for i in range(K))
    body=' '.join('if (cond()) { at(%d); return; }'%i for i in range(1,M+1))
    if where=='top': return 'void %s() { P p; %s %s at(%d); }'%(name,fill,body,M+1)
    return 'void %s() { P p; %s %s at(%d); }'%(name,body,fill,M+1)
P=[HDR]+[fn('k%d'%K,M,K) for K in Ks]
r=run(P)
prev=set()
for K in Ks:
    st,tl,l=r['k%d'%K]
    s=''.join('S' if 'E2' in tl.get(i,[]) else ('s' if 'cS' in tl.get(i,[]) else ('P' if 'cP' in tl.get(i,[]) else '?')) for i in range(1,M+2))
    cur={i+1 for i,c in enumerate(s) if c=='S'}
    print('%3d'%K, s, 'gained',sorted(cur-prev),'lost',sorted(prev-cur)); prev=cur
