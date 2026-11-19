#!/usr/bin/env python3
"""spellsweep.py <unit> <0xADDR> [--write] [--rules a,b] [--json F]: per-function raw-vs-helper spelling screen for
the invented budget-only helpers (MATCHING_GUIDE 3.1 "Auditing a header helper"): is_human / is_alive /
is_alien_faction / map_tiles() / MapAbstract().  Every site inside the function is a toggle (helper -> raw and
raw -> helper).  Screens BASE, each site alone, all helper sites raw, all raw sites helper, then a greedy pass that
applies the single toggles in order of gain and keeps each one that does not worsen the key.  Key = (named call
multiset distance to the image, fe rows): a variant is kept when the call distance drops (rows free, no ratchet)
or when it is equal and rows drop.  Prefixes FE_PFX_BASE.. (FE_JOBS lanes).  Screen only."""
import sys,os,re,json,collections
from concurrent.futures import ThreadPoolExecutor
T=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0,T); sys.path.insert(0,T+'/analysis'); import fe,cdeck
R=fe.R
M='GameDataStateStorage.fields_.lock_masks_.bytes_.'
ARG=r'((?:[^()]|\((?:[^()]|\([^()]*\))*\))+)'
RULES=collections.OrderedDict([
 ('human_raw',(r'\bis_human\('+ARG+r'\)', lambda m:'(%shuman_mask_ & (1 << %s))'%(M,par(m.group(1))))),
 ('alive_raw',(r'\bis_alive\('+ARG+r'\)', lambda m:'(%salive_mask_ & (1 << %s))'%(M,par(m.group(1))))),
 ('alien_raw',(r'\bis_alien_faction\('+ARG+r'\)', lambda m:'(ExpansionEnabled && (Players[%s].rule_flags & RFLAG_ALIEN))'%m.group(1))),
 ('tiles_raw',(r'(?<![\w.>:])map_tiles\(\)', lambda m:'MapTiles')),
 ('abs_raw',(r'(?<![\w.>:])MapAbstract\(\)', lambda m:'MapAbstractTiles')),
 ('human_fn',(re.escape(M)+r'human_mask_ & \(1u? << '+ARG+r'\)', lambda m:'is_human(%s)'%m.group(1))),
 ('alive_fn',(re.escape(M)+r'alive_mask_ & \(1u? << '+ARG+r'\)', lambda m:'is_alive(%s)'%m.group(1))),
 ('tiles_fn',(r'(?<![\w.>:])MapTiles\b(?!\w)', lambda m:'map_tiles()')),
 ('abs_fn',(r'(?<![\w.>:])MapAbstractTiles\b', lambda m:'MapAbstract()')),
])
def par(a):
    a=a.strip(); return a if re.fullmatch(r'[\w.\[\]>-]+',a) and '->' not in a.replace('->','') or re.fullmatch(r'\w+',a) else '('+a+')'
def span(t,addr):
    m=re.search(r'// ORIGINAL: 0x%08X '%addr,t); s=m.start(); e=t.index('\n}\n',s); return s,e
def sites(t,addr,rules):
    s,e=span(t,addr); out=[]; body=t[s:e]
    code=re.sub(r'//[^\n]*',lambda m:' '*len(m.group(0)),body)
    for r in rules:
        rx,rep=RULES[r]
        for m in re.finditer(rx,code):
            out.append((s+m.start(),s+m.end(),rep(m),r))
    out.sort(); return out
def apply(t,sel):
    for a,b,rep,r in sorted(sel,reverse=True): t=t[:a]+rep+t[b:]
    return t
def evaluate(unit,addr,deck):
    base=int(os.environ.get('FE_PFX_BASE','2')); jobs=int(os.environ.get('FE_JOBS','5'))
    fe.compile_cmd(unit); res={}
    def one(k):
        for i in range(k,len(deck),jobs):
            key,text=deck[i]; path='%s/build/recovery/fe/ss%d.cpp'%(R,k); open(path,'w',encoding='latin-1').write(text)
            c,err=fe.compile_unit(unit,path,str(base+k))
            if c is None: res[key]=None; continue
            try:
                r=fe.evaluate(c,text,addr); sym,lo,hi=fe.ident(text,addr)
                A=collections.Counter(cdeck.CRT.get(n,n) for a,n in cdeck.calls_img(lo,hi)); B=collections.Counter(n for a,n in cdeck.calls_ours(c,r['sym']))
                d={k_:(A[k_],B[k_]) for k_ in set(A)|set(B) if A[k_]!=B[k_]}
                res[key]=(sum(abs(a-b) for a,b in d.values()),0 if r['exact'] else r['diff'],d)
            except Exception as ex: res[key]=None
    with ThreadPoolExecutor(jobs) as ex: list(ex.map(one,range(jobs)))
    return res
def better(n,o): return n is not None and (n[0]<o[0] or (n[0]==o[0] and n[1]<o[1]))
def main():
    unit=sys.argv[1]; addr=int(sys.argv[2],16)
    rules=sys.argv[sys.argv.index('--rules')+1].split(',') if '--rules' in sys.argv else list(RULES)
    src=open(R+'/'+unit,encoding='latin-1').read(); S=sites(src,addr,rules)
    if not S: print('no sites'); return None
    lab=lambda x:'%s@L%d'%(x[3],src.count('\n',0,x[0])+1)
    deck=[('BASE',src)]+[(lab(x),apply(src,[x])) for x in S]
    raw=[x for x in S if x[3].endswith('_raw')]; fn=[x for x in S if x[3].endswith('_fn')]
    if len(raw)>1: deck.append(('ALLRAW',apply(src,raw)))
    if len(fn)>1: deck.append(('ALLFN',apply(src,fn)))
    for r in rules:
        g=[x for x in S if x[3]==r]
        if 1<len(g)<max(len(raw),len(fn)): deck.append(('ALL:'+r,apply(src,g)))
    res=evaluate(unit,addr,deck); b=res['BASE']
    if b is None: print('BASE failed'); return None
    print('BASE calls=%d rows=%d %s'%(b[0],b[1],b[2]))
    for k,_ in deck[1:]:
        v=res[k]; print('  %-22s %s'%(k,'FAIL' if v is None else 'calls=%d rows=%d%s'%(v[0],v[1],'  <' if better(v,b) else '')))
    best=('BASE',b,[])
    groups={'ALLRAW':raw,'ALLFN':fn}
    for r in rules: groups['ALL:'+r]=[x for x in S if x[3]==r]
    for k in groups:
        if k in res and better(res[k],best[1]): best=(k,res[k],groups[k])
    # greedy over improving singles, best first, from BASE and from the best group
    order=sorted([x for x in S if better(res[lab(x)],b)],key=lambda x:res[lab(x)][:2])
    starts=[('BASE',b,[])]+([best] if best[0]!='BASE' else [])
    for nm,cur,sel in starts:
        sel=list(sel)
        cand=order if nm=='BASE' else sorted([x for x in S if x not in sel],key=lambda x:(res[lab(x)] or (9e9,9e9))[:2])[:10]
        for x in cand:
            if x in sel: continue
            v=evaluate(unit,addr,[('g',apply(src,sel+[x]))])['g']
            if better(v,cur): sel.append(x); cur=v; print('  greedy[%s] + %s -> calls=%d rows=%d'%(nm,lab(x),v[0],v[1])); sys.stdout.flush()
        if better(cur,best[1]): best=('greedy:'+nm,cur,sel)
    print('BEST %s calls=%d rows=%d (base calls=%d rows=%d) sites=%s'%(best[0],best[1][0],best[1][1],b[0],b[1],[lab(x) for x in best[2]]))
    out=dict(unit=unit,addr='%08X'%addr,base=b[:2],best=best[1][:2],sites=[lab(x) for x in best[2]],calls=best[1][2],nsites=len(S))
    if '--write' in sys.argv and best[2]: open(R+'/'+unit,'w',encoding='latin-1').write(apply(src,best[2])); print('wrote')
    if '--json' in sys.argv: open(sys.argv[sys.argv.index('--json')+1],'w').write(json.dumps(out))
    return out
if __name__=='__main__': main()
