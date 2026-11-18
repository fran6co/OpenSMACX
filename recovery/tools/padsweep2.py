#!/usr/bin/env python3
"""padsweep2.py <unit> <addr> : sweep the CANARY-PADS count 0..63 with the fast evaluator; print exact counts."""
import sys,re,os
S=os.path.dirname(os.path.abspath(__file__))
R=os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0,S); import perm
unit=sys.argv[1] if sys.argv[1].startswith('src/') else 'src/'+sys.argv[1]; addr=sys.argv[2]
src=open(R+'/'+unit,encoding='latin-1').read()
m=re.search(r'(// CANARY-PADS: function (\w+)\n)((?:void \w+\(\);\n)*)',src)
name=m.group(2); cur=len(m.group(3).strip().split('\n')) if m.group(3).strip() else 0
deck=[(n, src[:m.start(3)]+''.join('void %s%d();\n'%(name,i) for i in range(n))+src[m.end(3):]) for n in range(64)]
res=perm.run_deck(unit,addr,deck)
ok=sorted(k for w,d,ex,k in res if ex)
print('current',cur,'exact counts',ok)
if '--write' in sys.argv and ok:
    best=min(ok,key=lambda k:(abs(k-cur),k))
    open(R+'/'+unit,'w',encoding='latin-1').write(dict(deck)[best]); print('wrote',best)
