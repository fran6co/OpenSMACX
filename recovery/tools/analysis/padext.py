#!/usr/bin/env python3
"""padext.py <unit> <addr[,addr]> [--write]: sweep an extern-kind CANARY-PADS block 0..63 for one or several functions and print the counts at which each (and all) are fe-exact."""
import sys,os,re
sys.path.insert(0,os.getcwd()+'/recovery/tools'); import perm
unit=sys.argv[1]; addrs=sys.argv[2].split(',')
src=open(unit,encoding='latin-1').read()
m=re.search(r'(// CANARY-PADS: extern (\w+)\n\n?)((?:extern int \w+;\n)*)',src)
name=m.group(2); cur=m.group(3).count('\n')
deck=[(n, src[:m.start(3)]+''.join('extern int %s%d;\n'%(name,i) for i in range(n))+src[m.end(3):]) for n in range(64)]
ok=None
for a in addrs:
    res=perm.run_deck(unit,a,deck)
    s=set(k for w,d,ex,k in res if ex); print(a,sorted(s)); ok=s if ok is None else ok&s
print('current',cur,'both',sorted(ok))
if '--write' in sys.argv and ok:
    best=min(ok,key=lambda k:(abs(k-cur),k)); open(unit,'w',encoding='latin-1').write(dict(deck)[best]); print('wrote',best)
