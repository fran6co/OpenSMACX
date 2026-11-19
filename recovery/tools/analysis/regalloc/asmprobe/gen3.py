# MATCHING_GUIDE 2.7 4h: writes pr<N>.cpp here; run with ../run.py --oy pr<N>.cpp
out=['#include "../probe.h"']
fns=[]
ARM={'defna':'  if (cond()) { p = def_p(); }','def':'  if (cond()) { p = def_p(); __asm { nop } }','defafter':'  if (cond()) { __asm { nop } p = def_p(); }',
     'defasmp':'  if (cond()) { __asm { mov eax, p } p = def_p(); }','asmw':'  if (cond()) { __asm { mov p, eax } }','na':'  if (cond()) { call(); }'}
for mode in ARM:
  for T in (2,4,6,8,10,12):
    P=8
    name='t%d_%s'%(T,mode)
    fns.append(name)
    b=['void W::%s(int p) {'%name,'  D(a); D(b);']
    for i in range(10):
        if i==5: b.append(ARM[mode])
        b.append('  U(a); U(b);')
        if i< T: b.append('  f[%d] = def_c();'%(i%8))
        if i< P: b.append('  U(p);')
    for i in range(10,T): b.append('  f[%d] = def_c();'%(i%8))
    b.append('  U(a); U(b); U(p); f[0] = 1;')
    b.append('}')
    out+=b
out.insert(1,'struct W { int f[8];'+''.join(' void %s(int p);'%n for n in fns)+' };')
open('pr3.cpp','w').write('\n'.join(out)+'\n')
