import sys; from cj import *
import cj, p1
from p1 import fn
cj.HDR+='extern void h2(int,int); extern void u2(int); extern void u3(int);\n'
V=[]
def add(k, ex, pos='before'):
    if pos=='before':
        s=fn('q_'+k,1,1).replace('  case 1: f0(); break;\n','  case 1: f0(); break;\n'+ex)
    else:
        s=fn('q_'+k,1,1,extra=ex)
    s=s.replace('t_%s('%('q_'+k),'t_%s('%('q_'+k))
    V.append((k,s))
for n in (1,2,4,14):
    add('pairs%d'%n, ''.join('  case %d: psave(%d); dmap(%d); break;\n'%(1000+j,j+2,j+2) for j in range(n)))
    add('h2_%d'%n, ''.join('  case %d: h2(%d, 0); break;\n'%(1000+j,j+2) for j in range(n)))
    add('otherpair%d'%n, ''.join('  case %d: u2(%d); u3(%d); break;\n'%(1000+j,j+2,j+2) for j in range(n)))
add('pairs1_after', '  case 1000: psave(5); dmap(5); break;\n', 'after')
add('same_args1', '  case 1000: f0(); psave(0); dmap(1); break;\n')
run(V, show=sys.argv[1:])
