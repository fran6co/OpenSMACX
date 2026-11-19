# MATCHING_GUIDE 2.7 4h: writes pr<N>.cpp here; run with ../run.py --oy pr<N>.cpp
out=['#include "../probe.h"']
fns=[]
for mode in ('na','mid','midp'):
  for T in (4,6,8,10,12):
    P=8
    name='t%d_%s'%(T,mode)
    fns.append(name)
    b=['void W::%s(int p) {'%name,'  D(a); D(b);']
    for i in range(10):
        if i==5:
            if mode=='mid': b.append('  if (cond()) { __asm { nop } }')
            if mode=='midp': b.append('  if (cond()) { __asm { mov eax, p } }')
        b.append('  U(a); U(b);')
        if i< T: b.append('  f[%d] = def_c();'%(i%8))
        if i< P: b.append('  U(p);')
    for i in range(10,T): b.append('  f[%d] = def_c();'%(i%8))
    b.append('  U(a); U(b); U(p); f[0] = 1;')
    b.append('}')
    out+=b
out.insert(1,'struct W { int f[8];'+''.join(' void %s(int p);'%n for n in fns)+' };')
open('pr2.cpp','w').write('\n'.join(out)+'\n')
