import sys; from cj import *
import cj, p1
from p1 import fn
V=[]
for n in (60,150,300):
    ex=''.join('  case %d: use(%d); break;\n'%(1000+j,1000+j) for j in range(n))
    s=fn('pb%d'%n,1,1).replace('  case 1: f0(); break;\n','  case 1: f0(); break;\n'+ex)
    V.append(('before%d'%n, s))
    ex2=''.join('  case %d: psave(%d); dmap(%d); break;\n'%(1000+j,j+2,j+2) for j in range(n))
    s=fn('pc%d'%n,1,1).replace('  case 1: f0(); break;\n','  case 1: f0(); break;\n'+ex2)
    V.append(('before_pairs%d'%n, s))
    s=fn('pd%d'%n,1,1,extra=ex2.replace('case 1','case 5'))
    V.append(('after_pairs%d'%n, s))
run(V, show=sys.argv[1:])
