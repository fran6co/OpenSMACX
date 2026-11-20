import sys; from cj import *
import cj, p1
from p1 import fn
V=[]
for n in (300,1000,3000):
    fill=''.join('  if (gx == %d) use(%d);\n'%(i+1000,i+1000) for i in range(n))
    V.append(('fill_after%d'%n, fn('fa%d'%n,1,1,after=fill)))
    V.append(('fill_before%d'%n, fn('fb%d'%n,1,1).replace('  switch (c) {\n  case 1','%s  switch (c) {\n  case 1'%fill,1)))
    vars_=''.join('  int v%d = cond();\n'%i for i in range(n//10))+'  use('+'+'.join('v%d'%i for i in range(n//10))+');\n'
    V.append(('vars%d'%(n//10), fn('fv%d'%n,1,1,after=vars_)))
run(V, show=sys.argv[1:])
