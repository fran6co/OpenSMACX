import sys; from cj import *
import cj, p1
from p1 import fn
V=[]
b=fn('l0',1,1)
V.append(('base',b))
V.append(('lab_after_ifelse', fn('l1',1,1).replace('m.uncheck(2, 0x209); ','m.uncheck(2, 0x209); LA: ; ').replace('m.uncheck(2, 0x20a); ','m.uncheck(2, 0x20a); LA: ; ').replace('m.uncheck(2, 0x20b); ','m.uncheck(2, 0x20b); LA: ; ')))
V.append(('lab_before_psave', fn('l2',1,1).replace('psave(0);','LA: ; psave(0);')))
V.append(('lab_after_dmap', fn('l3',1,1).replace('dmap(1); }','dmap(1); LA: ; }')))
V.append(('lab_case_host', fn('l4',0,0).replace('psave(0);','LA%d: ; psave(0);'%0,1)))
run(V, show=sys.argv[1:])
