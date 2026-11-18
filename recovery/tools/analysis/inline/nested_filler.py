import sys; sys.path.insert(0,'.'); from deck import *
# case B family under growing top filler: outer no ifs + inner ni ifs, N sites, filler K stores at top
ni=int(sys.argv[1]); no=int(sys.argv[2]); N=int(sys.argv[3]); Ks=range(int(sys.argv[4]),int(sys.argv[5]),int(sys.argv[6]))
P=['inline void inn(int k) { mk2(k); %s }'%' '.join('if (ga[%d]==1000) use(%d);'%(i,i) for i in range(ni)),
   'inline void out(int k) { mk(k); %s inn(k); }'%' '.join('if (ga[%d]==1000) use(%d);'%(50+i,i) for i in range(no))]
for K in Ks: P.append('void k%d() { %s %s }'%(K,' '.join('g3 = %d;'%(5000+i) for i in range(K)),' '.join('out(%d);'%i for i in range(1,N+1))))
r=run(P)
for K in Ks:
    l=r['k%d'%K][2]; o={};inn={}
    for k,c,x in ib.sites(l):
        if c=='mk': o[k]='O'
        elif c=='out': o[k]='o'
        elif c=='mk2': inn[k]='I'
        elif c=='inn': inn[k]='i'
    print('%4d'%K,''.join(o.get(i,'?') for i in range(1,N+1)),''.join(inn.get(i,'-') for i in range(1,N+1)))
