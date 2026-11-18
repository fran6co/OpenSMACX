import sys; sys.path.insert(0,'.'); from deck import *
# Case A variants: outer free (mk + inn), inner 4 ifs; plus top-level costly f4 calls at given positions
P=['inline void inn(int k) { mk2(k); %s }'%' '.join('if (ga[%d]==1000) use(%d);'%(i,i) for i in range(4)),
   'inline void out(int k) { mk(k); inn(k); }', F('f4',4,mark='mk(k);')]
def fn(name, seq):
    return 'void %s() { %s }'%(name,' '.join(seq))
tests={
 'base': ['out(%d);'%i for i in range(1,17)],
 'f4first': ['f4(100);']*0+['f4(%d);'%(200+i) for i in range(5)]+['out(%d);'%i for i in range(1,17)],
 'f4last': ['out(%d);'%i for i in range(1,17)]+['f4(%d);'%(200+i) for i in range(5)],
 'f4mid': ['out(%d);'%i for i in range(1,9)]+['f4(%d);'%(200+i) for i in range(5)]+['out(%d);'%i for i in range(9,17)],
}
P+= [fn(n,s) for n,s in tests.items()]
r=run(P)
for n in tests:
    l=r[n][2]; o={};inn={};f={}
    for k,c,x in ib.sites(l):
        if c=='mk' and k>=200: f[k]='F'
        elif c=='f4': f[k]='f'
        elif c=='mk': o[k]='O'
        elif c=='out': o[k]='o'
        elif c=='mk2': inn[k]='I'
        elif c=='inn': inn[k]='i'
    print('%-8s'%n,''.join(o.get(i,'?') for i in range(1,17)),''.join(inn.get(i,'-') for i in range(1,17)),''.join(f.get(200+i,'.') for i in range(5)))
