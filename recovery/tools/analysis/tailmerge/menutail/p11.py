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
    V.append((k,s))
add('chk1_b','  case 1000: chk(100, 1); break;\n')
add('chk1_a','  case 1000: chk(100, 1); break;\n','after')
add('h2_1_a','  case 1000: h2(100, 1); break;\n','after')
add('h2_0_a','  case 1000: h2(100, 0); break;\n','after')
add('chk0_a','  case 1000: chk(100, 0); break;\n','after')
add('chk20_a',''.join('  case %d: chk(%d, 1); break;\n'%(1000+j,100+j) for j in range(20)),'after')
add('h2_20_a',''.join('  case %d: h2(%d, 1); break;\n'%(1000+j,100+j) for j in range(20)),'after')
run(V, show=sys.argv[1:])
