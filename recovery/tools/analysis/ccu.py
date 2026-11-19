#!/usr/bin/env python3
"""ccu.py <unit.cpp> ... : every function of the units whose call multiset differs from the image's
(callee: (image, ours)).  Compiles on prefix FE_PFX_BASE (default 2).  Screen only."""
import sys,os,re,collections
sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
import cdeck,fe
R=cdeck.R
for unit in sys.argv[1:]:
    t=open(R+'/'+unit,encoding='latin-1').read()
    c,err=fe.compile_unit(unit,R+'/'+unit,os.environ.get('FE_PFX_BASE','2'))
    for m in re.finditer(r'// ORIGINAL: 0x([0-9A-F]{8}) ',t):
        a=int(m.group(1),16); idn=fe.ident(t,a)
        try: s=c.function(idn[0])
        except Exception: continue
        A=collections.Counter(cdeck.CRT.get(n,n) for _,n in cdeck.calls_img(idn[1],idn[2])); B=collections.Counter(n for _,n in cdeck.calls_ours(c,idn[0]))
        d={k:(A[k],B[k]) for k in set(A)|set(B) if A[k]!=B[k]}
        if d: print(unit,'%08X'%a,idn[0][:40],d)
