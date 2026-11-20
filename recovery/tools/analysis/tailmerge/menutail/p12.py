import sys; from cj import *
import cj, p1
from p1 import fn
cj.HDR+='extern void h2(int,int);\n'
V=[]
def add(k, ex):
    s=fn('q_'+k,1,1,extra=ex)
    V.append((k,s))
add('none','')
for cv in (40,100,200,1000,5000):
    add('h2_case%d'%cv,'  case %d: h2(100, 1); break;\n'%cv)
    add('use_case%d'%cv,'  case %d: use(100); break;\n'%cv)
run(V, show=sys.argv[1:])
