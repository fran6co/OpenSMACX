#!/usr/bin/env python3
"""pairs.py <unit> <addr> <pfx> [--write] : try every single site and every pair of non-overlapping sites of
the behaviour-preserving rules (ifswap, earlyret, declswap, stmtswap, cmpswap, ternary) inside ONE function;
keep the best candidate if it lowers that function's difference. Run under `uv run python`."""
import sys,os,re,json,itertools
S=os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0,S); import fe,rules
unit=sys.argv[1]; unit=unit if unit.startswith('src/') else 'src/'+unit; addr=int(sys.argv[2],16); pfx=sys.argv[3]
path=fe.R+'/'+unit; text=open(path,encoding='latin-1').read()
mt=rules.mask(text)
ms=[m.start() for m in rules.MARK.finditer(text)]
me=[m for m in rules.MARK.finditer(text) if int(m.group(1),16)==addr][0]
lo=me.start(); hi=min([x for x in ms if x>lo]+[len(text)])
sites=[]
for name in ('ifswap','earlyret','declswap','stmtswap','cmpswap','ternary'):
    for c in rules.RULES[name](text,mt,lo):
        if c[0]>=hi: break
        if c[1]<=hi: sites.append(c[:4])
    if len(sites)>60: break
sites=sites[:60]
tmp=fe.SCR+'/pairs_%s_%s.cpp'%(os.path.basename(unit).replace('.','_'),pfx)
def score(t):
    open(tmp,'w',encoding='latin-1').write(t)
    c,err=fe.compile_unit(unit,tmp,pfx)
    if c is None: return None
    r=fe.evaluate(c,t,addr)
    return None if 'error' in r else (r['diff'],r['exact'])
def apply(cs):
    t=text
    for s,e,rep,_ in sorted(cs,key=lambda c:-c[0]): t=t[:s]+rep+t[e:]
    return t
base=score(text); best=(base[0],None,None)
cands=[(c,) for c in sites]+[p for p in itertools.combinations(sites,2) if p[0][1]<=p[1][0] or p[1][1]<=p[0][0]]
for cs in cands:
    sc=score(apply(cs))
    if sc and sc[0]<best[0]:
        best=(sc[0],cs,sc[1])
        if sc[1]: break
out=dict(unit=unit,addr='%08X'%addr,sites=len(sites),tried=len(cands),base=base[0],best=best[0],exact=bool(best[2]),labels=[c[3] for c in best[1]] if best[1] else [])
print(json.dumps(out))
if '--write' in sys.argv and best[1] and best[0]<base[0]: open(path,'w',encoding='latin-1').write(apply(best[1]))
