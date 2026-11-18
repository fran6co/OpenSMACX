import sys; sys.path.insert(0,'.'); from deck import *
# outer body given as text (k = site), inner ni ifs; N list
ob=sys.argv[1]; ni=int(sys.argv[2]); Ns=[int(x) for x in sys.argv[3:]]
P=['inline void inn(int k) { mk2(k); %s }'%' '.join('if (ga[%d]==1000) use(%d);'%(i,i) for i in range(ni)),
   'inline void out(int k) { mk(k); %s inn(k); }'%ob]
for N in Ns: P.append('void n%d() { %s }'%(N,' '.join('out(%d);'%i for i in range(1,N+1))))
r=run(P)
for N in Ns:
    l=r['n%d'%N][2]; o={};inn={}
    for k,c,x in ib.sites(l):
        if c=='mk': o[k]='O'
        elif c=='out': o[k]='o'
        elif c=='mk2': inn[k]='I'
        elif c=='inn': inn[k]='i'
    print('%3d'%N,''.join(o.get(i,'?') for i in range(1,N+1)),''.join(inn.get(i,'-') for i in range(1,N+1)))
