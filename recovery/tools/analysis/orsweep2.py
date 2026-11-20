#!/usr/bin/env python3
"""orsweep2.py <unit.cpp> <addr> [--write KEY] [--list]: value-form variants orsweep.py does not try:
W  while/for conditions with a top-level && / ||  -> (cond) != 0
N  if (!(A && B)) -> if ((A && B) == 0)
M  nested else-less ifs `if (A) { if (B) {...} }` -> if ((A && B) != 0) {...}
"""
import sys, os, re
W='/home/fran6co/code/OpenSMACX/.opensmacx/worktrees/par-5'
sys.path.insert(0, W+'/recovery/tools'); sys.path.insert(0, W+'/recovery/tools/analysis')
import orsweep
from orsweep import span, close, toplevel

def match_brace(t, i):
    # t[i] == '{'; returns index of matching '}' (skips strings, chars, comments)
    d=0; j=i; n=len(t)
    while j<n:
        c=t[j]
        if c=='/' and t[j+1:j+2]=='/': j=t.index('\n',j); continue
        if c=='/' and t[j+1:j+2]=='*': j=t.index('*/',j)+2; continue
        if c in '"\'':
            q=c; j+=1
            while t[j]!=q:
                j+= 2 if t[j]=='\\' else 1
            j+=1; continue
        if c=='{': d+=1
        elif c=='}':
            d-=1
            if d==0: return j
        j+=1
    return -1

def line(t,a): return t.count('\n',0,a)+1

def variants(t, addr):
    s,e=span(t,addr); out=[]
    ops=['&&','||']
    for m in re.finditer(r'\b(while|for) \(', t[s:e]):
        a=s+m.end()-1; b=close(t,a); inner=t[a+1:b]
        if '//' in inner: continue
        if m.group(1)=='for':
            parts=inner.split(';')
            if len(parts)!=3: continue
            c=parts[1]
            if not any(toplevel(c,op) for op in ops): continue
            cs=a+1+len(parts[0])+1; ce=cs+len(c)
            lead=len(c)-len(c.lstrip())
            out.append(('W%d'%line(t,a), t[:cs]+c[:lead]+'('+c[lead:]+') != 0'+t[ce:]))
        else:
            if not any(toplevel(inner,op) for op in ops): continue
            out.append(('W%d'%line(t,a), t[:a+1]+'('+inner+') != 0'+t[b:]))
    for m in re.finditer(r'\bif \(', t[s:e]):
        a=s+m.end()-1; b=close(t,a); cond=t[a+1:b]
        if '//' in cond: continue
        if cond.startswith('!(') and close(cond,1)==len(cond)-1:
            inn=cond[2:-1]
            if any(toplevel(inn,op) for op in ops):
                out.append(('N%d'%line(t,a), t[:a+1]+'('+inn+') == 0'+t[b:]))
        # nested
        if t[b:b+3]!=') {': continue
        ob=b+2; oe=match_brace(t,ob)
        if oe<0: continue
        after=t[oe+1:oe+40].lstrip()
        if after.startswith('else'): continue
        body=t[ob+1:oe].strip()
        mm=re.match(r'if \(', body)
        if not mm: continue
        bi=t.index('if (',ob); ba=bi+3; bb=close(t,ba); c2=t[ba+1:bb]
        if '//' in c2 or t[bb:bb+3]!=') {': continue
        ib=bb+2; ie=match_brace(t,ib)
        if t[ie+1:oe].strip(): continue  # inner if must be the only statement (no else)
        def par(x): return '('+x+')' if toplevel(x,'||') or toplevel(x,'?') else x
        nc='('+par(cond)+' && '+par(c2)+') != 0'
        new=t[:a+1]+nc+') {'+t[ib+1:ie]+'}'+t[oe+1:]
        out.append(('M%d'%line(t,a), new))
        out.append(('Mp%d'%line(t,a), t[:a+1]+par(cond)+' && '+par(c2)+') {'+t[ib+1:ie]+'}'+t[oe+1:]))
    return out

if __name__=='__main__':
    unit,addr=sys.argv[1],int(sys.argv[2],16)
    path=W+'/src/'+unit; t=open(path,encoding='latin-1').read()
    deck=variants(t,addr)
    if '--list' in sys.argv:
        for k,_ in deck: print(k)
        sys.exit(0)
    if '--write' in sys.argv:
        k=sys.argv[sys.argv.index('--write')+1]
        open(path,'w',encoding='latin-1').write(dict(deck)[k]); print('wrote',k); sys.exit(0)
    import fe, sdeck
    _,lo,hi=fe.ident(t,addr)
    sdeck.run(unit,addr,lo,hi,[('BASE',t)]+deck)
