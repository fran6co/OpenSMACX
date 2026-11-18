#!/usr/bin/env python3
"""padany.py <unit> <addr> : insert 0..63 dummy declarations right before the function's ORIGINAL marker; report diff per count."""
import sys,re,collections,os
S=os.path.dirname(os.path.abspath(__file__))
R=os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0,S); import perm
unit=sys.argv[1] if sys.argv[1].startswith('src/') else 'src/'+sys.argv[1]; addr=sys.argv[2]
src=open(R+'/'+unit,encoding='latin-1').read()
m=src.index('// ORIGINAL: 0x%08X'%int(addr,16))
# back up to the start of an enclosing block comment if the marker sits inside one
c=src.rfind('/*',0,m); e=src.rfind('*/',0,m)
if c>e: m=c
deck=[(n, src[:m]+''.join('void FePadDecl%d();\n'%i for i in range(n))+src[m:]) for n in range(64)]
res=perm.run_deck(unit,addr,deck)
by=collections.defaultdict(list)
for w,d,ex,k in res: by[d].append(k)
for d in sorted(by): print('diff',d,'pads',sorted(by[d]))
