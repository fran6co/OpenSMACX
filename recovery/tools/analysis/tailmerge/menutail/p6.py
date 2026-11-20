import sys; from cj import *
import cj, p1
from p1 import fn
V=[]
V.append(('no_bn', fn('n0',1,1,order='TD')))
V.append(('no_bn_noD', fn('n1',1,1,order='T')))
V.append(('bn_distinct', fn('n2',1,1).replace('imp = n; psave(0); dmap(1);','imp = n; psave(0); dmap(1); use(3);')))
V.append(('bn_only_first', fn('n3',1,1,order='BD')))
run(V, show=sys.argv[1:])
