# MATCHING_GUIDE 2.7 4h: writes pr<N>.cpp here; run with ../run.py --oy pr<N>.cpp
import sys
out=['#include "../probe.h"','struct W { int f[8]; void fn(int p);'+''.join(' void f_%s(int p);'%k for k in []) +' };']
fns=[]
for asm in (0,1):
  for T in (2,4,6,8,10,12,16,20):
    P=8
    name='t%d_%s'%(T,'asm' if asm else 'na')
    fns.append(name)
    b=['void W::%s(int p) {'%name,'  D(a); D(b);']
    if asm: b.append('  __asm { nop }')
    for i in range(10):
        b.append('  U(a); U(b);')
        if i< T: b.append('  f[%d] = def_c();'%(i%8))
        if i< P: b.append('  U(p);')
    for i in range(10,T): b.append('  f[%d] = def_c();'%(i%8))
    b.append('  U(a); U(b); U(p); f[0] = 1;')
    b.append('}')
    out+=b
out[1]='struct W { int f[8];'+''.join(' void %s(int p);'%n for n in fns)+' };'
open('pr1.cpp','w').write('\n'.join(out)+'\n')
