#!/usr/bin/env python3
"""spellsweep.py <unit> <0xADDR> [--write] [--rules a,b] [--json F] [--pairs]: per-function raw-vs-helper spelling screen for
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
OBJ=r'[A-Za-z_]\w*(?:\[\w+\])?(?:(?:\.|->)(?!right\b|left\b|top\b|bottom\b)[A-Za-z_]\w*(?:\[\w+\])?)*'
DEFAULT=['human_raw','alive_raw','alien_raw','tiles_raw','abs_raw','human_fn','alive_fn','tiles_fn','abs_fn']
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
 # RECT extents: raw subtraction against the win.h inlines (NewTechWin::on_redraw 0x00483950 became exact with
 # rect_width(&scratch)/rect_height(&scratch) in the call arguments; not in the default rule set)
 ('rw_fn',(r'(?<![\w.>])('+OBJ+r')(\.|->)right - \1\2left\b', lambda m:'rect_width(%s%s)'%('&' if m.group(2)=='.' else '',m.group(1)))),
 ('rh_fn',(r'(?<![\w.>])('+OBJ+r')(\.|->)bottom - \1\2top\b', lambda m:'rect_height(%s%s)'%('&' if m.group(2)=='.' else '',m.group(1)))),
 # strings.h / alpha.h header inlines (MATCHING_GUIDE 2.8 "late free helpers"; not default): each expands to exactly
 # the raw statement, and says() keeps the literal argument per site (a `$SG` of the caller, as the image's unpooled
 # per-site texts), unlike an argument-free helper whose literal is one COMDAT
 ('says_fn',(r'(?<![\w.>])strcat\(StringTemp, (?!label_get\()'+ARG+r'\)', lambda m:'says(%s)'%m.group(1))),
 ('sayl_fn',(r'(?<![\w.>])strcat\(StringTemp, label_get\('+ARG+r'\)\)', lambda m:'say_label(%s)'%m.group(1))),
 ('clear_fn',(r"(?<![\w.>])StringTemp\[0\] = (?:'\\0'|0);", lambda m:'::clear();')),
 ('table_fn',(r'(?<![\w.>])StringTable\.get\(', lambda m:'table_text(')),
 # the reverse: a table_text stand-in written as the direct call (probe 0x0059F120: the image hoists the shared
 # x*3 of both arms only with the direct call)
 ('table_raw',(r'(?<![\w.>])table_text\(', lambda m:'StringTable.get(')),
 ('treaty_fn',(r'(?<![\w.>])PlayersData\[([^\[\]]+)\]\.diplo_treaties\[([^\[\]]+)\] & (\w+)\b(?!\s*[\[(])', lambda m:'has_treaty(%s, %s, %s)'%(m.group(1),m.group(2),m.group(3)))),
 ('rw_raw',(r'\brect_width\(&('+OBJ+r')\)', lambda m:'(%s.right - %s.left)'%(m.group(1),m.group(1)))),
 ('rh_raw',(r'\brect_height\(&('+OBJ+r')\)', lambda m:'(%s.bottom - %s.top)'%(m.group(1),m.group(1)))),
])
def par(a):
    a=a.strip(); return a if re.fullmatch(r'[\w.\[\]>-]+',a) and '->' not in a.replace('->','') or re.fullmatch(r'\w+',a) else '('+a+')'
def span(t,addr):
    m=re.search(r'// ORIGINAL: 0x%08X (\S+)'%addr,t); s=m.start(); e=t.index('\n}\n',s)
    # a marker may sit above file-local helpers: end at the function's own body
    nm=re.match(r'\?([A-Za-z_]\w*)@',m.group(1))
    if nm:
        d=re.search(r'\n(?!static |inline |//)[A-Za-z_][^\n;]*\b%s\([^\n;]*\{?\n'%re.escape(nm.group(1)),t[s:])
        if d: e=t.index('\n}\n',s+d.start())
    return s,e
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
    rules=sys.argv[sys.argv.index('--rules')+1].split(',') if '--rules' in sys.argv else list(DEFAULT)
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
    if '--pairs' in sys.argv:
        # pairs of opposite toggles (one site to the helper, one written out) within SS_PAIR_LINES lines, on top of
        # the best selection: a swap where each alone is worse
        # (StatusWin::draw_active 0x004B4CF0: the strlen test direct, the strcat beside it through table_text)
        while True:
            sel=list(best[2]); rest=[x for x in S if x not in sel]
            near=int(os.environ.get('SS_PAIR_LINES','15'))
            ok=lambda x,y: x[3]!=y[3] and abs(src.count('\n',0,x[0])-src.count('\n',0,y[0]))<=near
            pd=[('%d,%d'%(i,j),apply(src,sel+[rest[i],rest[j]])) for i in range(len(rest)) for j in range(i+1,len(rest)) if ok(rest[i],rest[j])]
            if not pd: break
            pr=evaluate(unit,addr,pd); k=min((k for k in pr if pr[k] is not None),key=lambda k:pr[k][:2],default=None)
            if k is None or not better(pr[k],best[1]): break
            i,j=map(int,k.split(',')); best=('pair:'+best[0],pr[k],sel+[rest[i],rest[j]])
            print('  pair + %s %s -> calls=%d rows=%d'%(lab(rest[i]),lab(rest[j]),pr[k][0],pr[k][1])); sys.stdout.flush()
    print('BEST %s calls=%d rows=%d (base calls=%d rows=%d) sites=%s'%(best[0],best[1][0],best[1][1],b[0],b[1],[lab(x) for x in best[2]]))
    out=dict(unit=unit,addr='%08X'%addr,base=b[:2],best=best[1][:2],sites=[lab(x) for x in best[2]],calls=best[1][2],nsites=len(S))
    if '--write' in sys.argv and best[2]: open(R+'/'+unit,'w',encoding='latin-1').write(apply(src,best[2])); print('wrote')
    if '--json' in sys.argv: open(sys.argv[sys.argv.index('--json')+1],'w').write(json.dumps(out))
    return out
if __name__=='__main__': main()
