#!/usr/bin/env python3
"""greedy_sub.py <unit> <regex> <replacement> [--define NAME] [--write]
Try each occurrence of <regex> in the unit as a single substitution; accept it when no fe-exact function is
lost and the unit's total diff (over all markers) drops. Run under `uv run python`."""
import sys,re,os
S=os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0,S); import fe
unit=sys.argv[1]; unit=unit if unit.startswith('src/') else 'src/'+unit
rx=re.compile(sys.argv[2]); rep=sys.argv[3]
define=sys.argv[sys.argv.index('--define')+1] if '--define' in sys.argv else None
path=fe.R+'/'+unit; text=open(path,encoding='latin-1').read()
addrs=[int(m.group(1),16) for m in re.finditer(r'// ORIGINAL: (0x[0-9A-F]{8}) \S+ 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+',text)]
tmp=fe.SCR+'/greedy_%s.cpp'%os.path.basename(unit).replace('.','_')
def score(t):
    open(tmp,'w',encoding='latin-1').write(t)
    c,err=fe.compile_unit(unit,tmp,'23')
    if c is None: return None
    ex=set(); tot=0; per={}
    for a in addrs:
        r=fe.evaluate(c,t,a)
        if 'error' in r: continue
        per[a]=r['diff']
        if r['exact']: ex.add(a)
        tot+=r['diff']
    return ex,tot,per
if define and ('#define '+define) not in text:
    i=text.index('#include "stdafx.h"'); text=text[:i]+'#define '+define+'\n'+text[i:]
    b0=score(open(path,encoding='latin-1').read()); b1=score(text)
    print('define effect: exact %d -> %d, total diff %d -> %d, lost %s'%(len(b0[0]),len(b1[0]),b0[1],b1[1],['%08X'%a for a in b0[0]-b1[0]]))
    if b0[0]-b1[0]: print('define loses exact functions; abort'); sys.exit(1)
base=score(text); cur=text; acc=0
print('base exact',len(base[0]),'total diff',base[1])
pos=0
while True:
    m=rx.search(cur,pos)
    if not m: break
    cand=cur[:m.start()]+m.expand(rep)+cur[m.end():]
    s=score(cand)
    line=cur.count('\n',0,m.start())+1
    if s and not (base[0]-s[0]) and s[1]<base[1]:
        ch=[('%08X'%a,base[2][a],s[2][a]) for a in s[2] if s[2][a]!=base[2].get(a)]
        print('ACCEPT line',line,'total',base[1],'->',s[1],ch,'new exact',['%08X'%a for a in s[0]-base[0]])
        cur=cand; base=s; acc+=1; pos=m.start()+len(m.expand(rep))
    else:
        print('reject line',line,None if not s else (s[1],['%08X'%a for a in base[0]-s[0]]))
        pos=m.end()
print('accepted',acc,'final exact',len(base[0]),'total diff',base[1])
if '--write' in sys.argv and acc: open(path,'w',encoding='latin-1').write(cur); print('written')
