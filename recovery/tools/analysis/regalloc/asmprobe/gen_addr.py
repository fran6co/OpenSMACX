# MATCHING_GUIDE 2.7 "Constant candidates and phantom registers": a member-object address (receiver of DD::m) against 0-4 int candidates; writes pr5.cpp, run ../run.py [--oy] --asm pr5.cpp
out=['#include "../probe.h"','struct DD { int k[4]; void m(); };']
fns=[]
for comp in (0,2,3,4):
  for N in (2,4,8):
    name='n%d_c%d'%(N,comp); fns.append(name)
    b=['void P::%s() {'%name]
    vs='abcd'[:comp]
    for v in vs: b.append('  D(%s);'%v)
    for i in range(N):
        b.append('  dd.m();')
        for v in vs: b.append('  U(%s);'%v)
    for i in range(10-N):
        for v in vs: b.append('  U(%s);'%v)
    for v in vs: b.append('  U(%s);'%v)
    b.append('  big[3] = 1;')
    b.append('}')
    out+=b
out.insert(2,'struct P { int big[0x800]; DD dd;'+''.join(' void %s();'%n for n in fns)+' };')
open('pr5.cpp','w').write('\n'.join(out)+'\n')
