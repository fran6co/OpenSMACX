#!/usr/bin/env python3
"""deck.py <unit> <addr> <sig> <template> <slots.py> : template has @A@ @B@ placeholders; slots.py defines SLOTS={'A':[...],...}; full product evaluated."""
import sys,os,itertools
R=os.getcwd(); sys.path.insert(0,R+'/recovery/tools')
os.environ.setdefault('FE_PFX_BASE','8'); os.environ.setdefault('FE_JOBS','5')
import perm
unit,addr,sig,tpl,slots=sys.argv[1:6]
t=open(unit,encoding='latin-1').read()
i=t.index('\n'+sig)+1; j=t.index('\n}\n',i)+3
body=open(tpl,encoding='latin-1').read().rstrip('\n')+'\n'
g={}; exec(open(slots).read(),g); SL=g['SLOTS']
keys=sorted(SL); deck=[]
for combo in itertools.product(*[range(len(SL[k])) for k in keys]):
    b=body
    for k,c in zip(keys,combo): b=b.replace('@%s@'%k,SL[k][c])
    deck.append((combo,t[:i]+b+t[j:]))
res=perm.run_deck(unit,addr,deck,int(os.environ['FE_JOBS']),sys.argv[6] if len(sys.argv)>6 and not sys.argv[6].startswith('--') else None)
print('keys',keys,'tried',len(res))
for w,dd,ex,key in res[:12]: print(w,dd,ex,key)
if '--write' in sys.argv:
    key=res[0][3]; open(unit,'w',encoding='latin-1').write(dict(deck)[key]); print('wrote',key)
