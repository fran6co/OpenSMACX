#!/usr/bin/env python3
"""loopsweep.py [--rule rebind|sibling] [--json loopscan.json | --unit U --addr 0xADDR] [--write] [--min N]
Loop-form sweeps for loopscan.py's tags.
rebind: a loop body that binds
`T &r = A[i];` or `T *p = &A[i];` each iteration is rewritten to write `A[i]` at
each use (`r.f` -> `A[i].f`, `p->f` -> `A[i].f`, `*p` -> `A[i]`, `p` -> `&A[i]`) and the
declaration is dropped.  A site is skipped when the bound name or a variable of the
subscript is assigned after the declaration, or the subscript calls a function.
Each site is measured alone with fe.py; --write keeps every site that lowers its
function's differing rows by at least --min (default 1) without changing the frame
size or the loop-test multiset away from the image.  Screen only.
derive: a coordinate stepped in the for-clause (`x = X0 ... x += K`) beside a counter is written
`X0 + i * K` at each use (MATCHING_GUIDE 2.4; the image tests the derived coordinate).
basic: the reverse, `v = A + i * K;` at the top of the body moves into the for-clause as its own
induction variable (VC6 replaces a counter's test only with an IV derived from it).
--structure also keeps a site that brings the loop-test multiset closer to the image when rows rise.
sibling: a counter name declared by several non-nested `for (int NAME ...)` loops, and not used
outside them, becomes one function-scope `int NAME;` (one variant per name, one for all names).
Run from the worktree root: FE_PFX_BASE=27 FE_JOBS=5 uv run python recovery/tools/analysis/loopsweep.py --json loops.json"""
import sys,os,re,json,argparse
from collections import Counter
from concurrent.futures import ThreadPoolExecutor
S=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0,S); sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
import fe, loopscan
R=fe.R
DECL=re.compile(r'(?m)^(?P<indent>[ \t]*)(?:const\s+)?[A-Za-z_][\w:]*(?:\s+const)?\s*(?P<k>[*&])\s*(?P<name>[A-Za-z_]\w*)\s*=\s*(?P<amp>&?)\s*(?P<expr>[A-Za-z_][\w.:>-]*(?:\s*\[[^\]\n]*\])+(?:\.[A-Za-z_]\w*(?:\[[^\]\n]*\])*)*)\s*;[ \t]*\n')
def sites(text,clean,lo,hi):
    out=[]
    for m in DECL.finditer(clean,lo,hi):
        k,name,amp,expr=m.group('k'),m.group('name'),m.group('amp'),m.group('expr')
        if (k=='&')==bool(amp): continue           # T &r = &A[i] / T *p = A[i] are other shapes
        if '(' in expr: continue
        # scope: to the end of the enclosing block
        d=0; end=hi
        for j in range(m.end(),hi):
            c=clean[j]
            if c=='{': d+=1
            elif c=='}':
                if d==0: end=j; break
                d-=1
        body=clean[m.end():end]
        idx=set(re.findall(r'[A-Za-z_]\w*',expr))
        if re.search(r'\b%s\s*(?:=[^=]|\+\+|--|[+\-*/|&]=)|(?:\+\+|--)\s*%s\b|&\s*%s\b'%(name,name,name),body): continue
        if any(re.search(r'\b%s\s*(?:=[^=]|\+\+|--|[+\-*/|&]=)|(?:\+\+|--)\s*%s\b'%(v,v),body) for v in idx): continue
        e=re.sub(r'\s+',' ',expr)
        if k=='&': sub=[(r'\b%s\b'%name,e)]
        else: sub=[(r'\b%s\s*->'%name,e+'.'),(r'\*\s*%s\b'%name,e),(r'\b%s\s*\['%name,'(&'+e+')['),(r'\b%s\b'%name,'&'+e)]
        seg=text[m.end():end]; cseg=body
        # apply on code only: rebuild from the clean copy's token positions
        pieces=[]; pos=0
        for mm in re.finditer('|'.join('(?:%s)'%p for p,_ in sub),cseg):
            for p,r in sub:
                if re.fullmatch(p,mm.group(0)): pieces.append((mm.start(),mm.end(),r)); break
        new=''; pos=0
        for a,b,r in pieces: new+=seg[pos:a]+r; pos=b
        new+=seg[pos:]
        out.append(dict(line=clean.count('\n',0,m.start())+1,name=name,expr=e,text=text[:m.start()]+text[m.end():m.end()]+new+text[end:]))
    return out
def sibling_sites(text,clean,lo,hi):
    found={}
    for m in re.finditer(r'\bfor\s*\(\s*int\s+([A-Za-z_]\w*)\s*=',clean[:hi]):
        if m.start()<lo: continue
        p0=clean.index('(',m.start()); p1=loopscan.match(clean,p0,'(',')'); s_,e=loopscan.stmt_end(clean,p1+1)
        found.setdefault(m.group(1),[]).append((m.start(),e,m))
    names=[]
    for name,L in found.items():
        if len(L)<2: continue
        if any(a<c<b for a,b,_ in L for c,d,_ in L if (a,b)!=(c,d)): continue      # nested reuse
        outside=clean[lo:hi]
        for a,b,_ in sorted(L,reverse=True): outside=outside[:a-lo]+outside[b+1-lo:]
        if re.search(r'\b%s\b'%name,outside): continue
        names.append((name,L))
    def apply(sel):
        t=text; ms=sorted([x for n,L in sel for x in L],key=lambda x:-x[0])
        for a,b,m in ms: t=t[:m.start()]+re.sub(r'int\s+','',t[m.start():m.end()],count=1)+t[m.end():]
        decl=''.join('\n    int %s;'%n for n,_ in sel)
        return t[:lo+1]+decl+t[lo+1:]
    out=[dict(line=clean.count('\n',0,L[0][0])+1,name=n,expr='%d loops'%len(L),text=apply([(n,L)])) for n,L in names]
    if len(names)>1: out.append(dict(line=0,name='+'.join(n for n,_ in names)[:12],expr='all',text=apply(names)))
    return out
def for_loops(clean,lo,hi):
    """yield (start, p0, p1, body_s, body_e, init, cond, incr) for each for-loop in [lo,hi)"""
    for m in re.finditer(r'\bfor\s*\(',clean[:hi]):
        if m.start()<lo: continue
        p0=m.end()-1; p1=loopscan.match(clean,p0,'(',')')
        parts=loopscan.split_top(clean[p0+1:p1],';')
        if len(parts)!=3: continue
        s_,e=loopscan.stmt_end(clean,p1+1)
        yield m.start(),p0,p1,s_,e,parts[0],parts[1],parts[2]
def counter_of(init,cond,incr):
    inc=[x.strip() for x in loopscan.split_top(incr,',')]
    for x in inc:
        m=re.fullmatch(r'(?:\+\+\s*([A-Za-z_]\w*)|([A-Za-z_]\w*)\s*\+\+)',x)
        if not m: continue
        c=m.group(1) or m.group(2)
        mi=re.search(r'(?:^|,)\s*(?:int\s+)?%s\s*=\s*(-?\d+)\s*(?:,|$)'%c,init)
        if mi and re.search(r'\b%s\b\s*(<|<=|!=)'%c,cond): return c,int(mi.group(1))
    return None,None
def modified(v,txt): return re.search(r'\b%s\s*(?:=[^=]|\+\+|--|[-+*/|&^]=)|(?:\+\+|--)\s*%s\b|&\s*%s\b'%(v,v,v),txt)
def used_after(v,clean,e,hi):
    m=re.search(r'\b%s\b'%v,clean[e+1:hi])
    if not m: return False
    rest=clean[e+1+m.start():hi]
    return not re.match(r'%s\s*=[^=]'%v,rest)
def derive_sites(text,clean,lo,hi):
    out=[]
    for st,p0,p1,bs,be,init,cond,incr in for_loops(clean,lo,hi):
        c,c0=counter_of(init,cond,incr)
        if not c: continue
        ip=loopscan.split_top(init,','); cp=loopscan.split_top(incr,',')
        for x in cp:
            m=re.fullmatch(r'\s*([A-Za-z_]\w*)\s*([+-])=\s*(\w+)\s*',x)
            if not m or m.group(1)==c: continue
            v,sg,k=m.groups()
            vi=[y for y in ip if re.fullmatch(r'\s*%s\s*=\s*([^=].*)'%v,y,re.S)]
            if len(vi)!=1 or '(' in vi[0]: continue
            v0=vi[0].split('=',1)[1].strip()
            body=clean[bs:be+1]
            if re.search(r'\b%s\b'%v,cond) or modified(v,body) or used_after(v,clean,be,hi): continue
            cc=c if c0==0 else '(%s - %d)'%(c,c0)
            expr='(%s %s %s * %s)'%(v0,sg,cc,k) if v0!='0' else ('%s * %s'%(cc,k) if sg=='+' else '-%s * %s'%(cc,k))
            ni=','.join(y for y in ip if y is not vi[0]).strip(); nc=','.join(y for y in cp if y is not x).strip()
            newhead=text[p0:p0+1]+' '+ni+'; '+cond.strip()+'; '+nc+')'
            nb=re.sub(r'\b%s\b'%v,lambda _:expr,text[bs:be+1])
            t=text[:p0]+newhead.replace('( ','(')+text[p1+1:bs]+nb+text[be+1:]
            out.append(dict(line=clean.count('\n',0,st)+1,name=v,expr='%s -> %s'%(x.strip(),expr),text=t))
    return out
def basic_sites(text,clean,lo,hi):
    out=[]
    for st,p0,p1,bs,be,init,cond,incr in for_loops(clean,lo,hi):
        c,c0=counter_of(init,cond,incr)
        if not c or clean[bs]!='{': continue
        body=clean[bs+1:be]
        m=re.match(r'\s*([A-Za-z_]\w*)\s*=\s*(?:(-?\d+)\s*\+\s*%s\s*\*\s*(\d+)|%s\s*\*\s*(\d+)\s*\+\s*(-?\d+)|(-?\d+)\s*\+\s*(\d+)\s*\*\s*%s)\s*;[ \t]*\n?'%(c,c,c),body)
        if not m: continue
        v=m.group(1); a=int(m.group(2) or m.group(5) or m.group(6)); k=int(m.group(3) or m.group(4) or m.group(7))
        rest=body[m.end():]
        if v==c or modified(v,rest): continue
        v0=a+c0*k
        for order in (0,1):
            ni=('%s = %d, %s'%(v,v0,init.strip())) if order==0 else ('%s, %s = %d'%(init.strip(),v,v0))
            nc=('%s += %d, %s'%(v,k,incr.strip())) if order==0 else ('%s, %s += %d'%(incr.strip(),v,k))
            t=text[:p0+1]+ni+'; '+cond.strip()+'; '+nc+text[p1:bs+1]+text[bs+1+m.end():]
            out.append(dict(line=clean.count('\n',0,st)+1,name=v+('<' if order==0 else '>'),expr='%s = %d (+%d)'%(v,v0,k),text=t))
    return out
def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--json'); ap.add_argument('--addr'); ap.add_argument('--unit'); ap.add_argument('--write',action='store_true'); ap.add_argument('--min',type=int,default=1); ap.add_argument('--rule',default='rebind',choices=['rebind','sibling','derive','basic']); ap.add_argument('--structure',action='store_true',help='also keep a site that brings the loop tests closer to the image even when rows rise')
    ap.add_argument('--jobs',type=int,default=int(os.environ.get('FE_JOBS','14')))
    a=ap.parse_args(); base=int(os.environ.get('FE_PFX_BASE','21'))
    if a.addr: targets=[(a.unit if a.unit.startswith('src/') else 'src/'+a.unit,int(a.addr,16))]
    elif a.rule=='rebind': targets=[(f['unit'],f['addr']) for f in json.load(open(a.json)) if any(t.startswith('rebind') for x in f['src'] for t in x['tags'])]
    elif a.rule=='sibling': targets=[(f['unit'],f['addr']) for f in json.load(open(a.json)) if f.get('sibling')]
    elif a.rule=='derive': targets=[(f['unit'],f['addr']) for f in json.load(open(a.json)) if any(t.startswith('clause+=') for x in f['src'] for t in x['tags'])]
    else: targets=[(f['unit'],f['addr']) for f in json.load(open(a.json))]
    jobs=[]
    for unit,addr in targets:
        t=open(R+'/'+unit,encoding='latin-1').read(); clean=loopscan.strip_comments(t)
        fb=loopscan.func_body(t,clean,addr)
        if not fb: continue
        for s in dict(rebind=sites,sibling=sibling_sites,derive=derive_sites,basic=basic_sites)[a.rule](t,clean,fb[0],fb[1]+1): jobs.append((unit,addr,t,s))
    if not jobs: print('no sites'); return
    fe.compile_cmd(jobs[0][0])
    D=R+'/build/recovery/fe/loopsweep'; os.makedirs(D,exist_ok=True)
    def measure(unit,addr,text,lane,tag):
        p='%s/%s_%d.cpp'%(D,tag,lane); open(p,'w',encoding='latin-1').write(text)
        c,err=fe.compile_unit(unit,p,base+lane)
        if c is None: return None
        r=fe.evaluate(c,text,addr)
        if 'error' in r: return None
        O=[x[1] for x in r['rows'] if x[1]]; C=[x[2] for x in r['rows'] if x[2]]
        return dict(diff=r['diff'],exact=r['exact'],fi=loopscan.frame(O),fo=loopscan.frame(C),li=Counter(x['test'] for x in loopscan.loops(O)),lo=Counter(x['test'] for x in loopscan.loops(C)))
    basec={}
    def lane(k):
        out=[]
        for i in range(k,len(jobs),a.jobs):
            unit,addr,t,s=jobs[i]
            if (unit,addr) not in basec: basec[(unit,addr)]=measure(unit,addr,t,k,'b')
            out.append((i,basec[(unit,addr)],measure(unit,addr,s['text'],k,'s')))
        return out
    res=[]
    with ThreadPoolExecutor(a.jobs) as ex:
        for o in ex.map(lane,range(a.jobs)): res+=o
    res.sort()
    keep={}
    for i,b,r in res:
        unit,addr,t,s=jobs[i]
        if b is None or r is None: print('%08X %-24s L%-5d %-12s FAIL'%(addr,unit[4:],s['line'],s['name'])); continue
        dl=lambda m: sum(((m['li']-m['lo'])+(m['lo']-m['li'])).values())
        print('%08X %-24s L%-5d %-12s %4d -> %4d%s frame %#x/%#x->%#x loops %d->%d  %s'%(addr,unit[4:],s['line'],s['name'],b['diff'],r['diff'],' EXACT' if r['exact'] else '',b['fi'],b['fo'],r['fo'],dl(b),dl(r),s['expr'][:40]))
        fr=abs(r['fo']-r['fi'])<=abs(b['fo']-b['fi'])
        ok=fr and ((r['diff']<=b['diff']-a.min and dl(r)<=dl(b)) or (a.structure and dl(r)<dl(b)))
        if ok and ((unit,addr) not in keep or r['diff']<keep[(unit,addr)][1]['diff']): keep[(unit,addr)]=(s,r)
    if a.write:
        for (unit,addr),(s,r) in keep.items():
            t=open(R+'/'+unit,encoding='latin-1').read()
            if t!=[j for j in jobs if j[0]==unit and j[1]==addr][0][2]: print('changed since scan, skipped',unit); continue
            open(R+'/'+unit,'w',encoding='latin-1').write(s['text']); print('wrote %s %08X L%d %s'%(unit,addr,s['line'],s['name']))
if __name__=='__main__': main()
