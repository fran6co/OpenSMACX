#!/usr/bin/env python3
"""unbind.py [--json loopscan.json] [--addr 0xADDR] [--write] [--min N]
Loop-form sweep for the `rebind` tag of loopscan.py: a loop body that binds
`T &r = A[i];` or `T *p = &A[i];` each iteration is rewritten to write `A[i]` at
each use (`r.f` -> `A[i].f`, `p->f` -> `A[i].f`, `*p` -> `A[i]`, `p` -> `&A[i]`) and the
declaration is dropped.  A site is skipped when the bound name or a variable of the
subscript is assigned after the declaration, or the subscript calls a function.
Each site is measured alone with fe.py; --write keeps every site that lowers its
function's differing rows by at least --min (default 1) without changing the frame
size or the loop-test multiset away from the image.  Screen only.
Run from the worktree root: FE_PFX_BASE=27 FE_JOBS=5 uv run python recovery/tools/analysis/unbind.py --json loops.json"""
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
def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--json'); ap.add_argument('--addr'); ap.add_argument('--unit'); ap.add_argument('--write',action='store_true'); ap.add_argument('--min',type=int,default=1)
    ap.add_argument('--jobs',type=int,default=int(os.environ.get('FE_JOBS','14')))
    a=ap.parse_args(); base=int(os.environ.get('FE_PFX_BASE','21'))
    if a.addr: targets=[(a.unit if a.unit.startswith('src/') else 'src/'+a.unit,int(a.addr,16))]
    else: targets=[(f['unit'],f['addr']) for f in json.load(open(a.json)) if any(t.startswith('rebind') for x in f['src'] for t in x['tags'])]
    jobs=[]
    for unit,addr in targets:
        t=open(R+'/'+unit,encoding='latin-1').read(); clean=loopscan.strip_comments(t)
        fb=loopscan.func_body(t,clean,addr)
        if not fb: continue
        for s in sites(t,clean,fb[0],fb[1]+1): jobs.append((unit,addr,t,s))
    if not jobs: print('no sites'); return
    fe.compile_cmd(jobs[0][0])
    D=R+'/build/recovery/fe/unbind'; os.makedirs(D,exist_ok=True)
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
        ok=r['diff']<=b['diff']-a.min and abs(r['fo']-r['fi'])<=abs(b['fo']-b['fi']) and dl(r)<=dl(b)
        if ok and ((unit,addr) not in keep or r['diff']<keep[(unit,addr)][1]['diff']): keep[(unit,addr)]=(s,r)
    if a.write:
        for (unit,addr),(s,r) in keep.items():
            t=open(R+'/'+unit,encoding='latin-1').read()
            if t!=[j for j in jobs if j[0]==unit and j[1]==addr][0][2]: print('changed since scan, skipped',unit); continue
            open(R+'/'+unit,'w',encoding='latin-1').write(s['text']); print('wrote %s %08X L%d %s'%(unit,addr,s['line'],s['name']))
if __name__=='__main__': main()
