"""armfirst.py FEALL.json [unit ...] (AF_SLOTS=1 abstracts frame slots, AF_OUT=F writes JSON) : image test/cmp separated from its jcc by flag-neutral instructions that
ours computes only after its jcc, at the head of one arm (fall-through or taken target).
=> the original computed that arm's first operation unconditionally (value form, if/else of calls, index local);
B = present at the head of both arms (a common prefix ours does not hoist: one arm uses another register). MATCHING_GUIDE 3.7 round 43."""
import sys,json,os,re,collections
from concurrent.futures import ThreadPoolExecutor
T=os.path.dirname(os.path.dirname(os.path.abspath(__file__))); sys.path.insert(0,T); import fe
R=fe.R
rows=json.load(open(sys.argv[1])); only=set(sys.argv[2:])
REG=re.compile(r'\b(e?[abcd]x|[abcd][lh]|e?si|e?di|ebx)\b')
def norm(s): return re.sub(r'\[(ebp|esp) [-+] 0x[0-9a-f]+\]','[S]',REG.sub('R',s)) if os.environ.get('AF_SLOTS') else REG.sub('R',s)
def gaps(L):
    """L = list of (off, text, size, tgt). -> list of (i_test, j_jcc, [gap idx])"""
    res=[]
    for i,x in enumerate(L):
        if re.match(r'^(test|cmp)\b',x[1]):
            j=i+1; g=[]
            # only flag-neutral moves may sit between; flag writers would end the test's life
            while j<len(L) and re.match(r'^(mov|lea|push|movsx|movzx|fld|fild|fst|fstp|fistp)\b',L[j][1]): g.append(j); j+=1
            if j<len(L) and re.match(r'^j(?!mp)',L[j][1]): res.append((i,j,g))
    return res
def run(u,addrs,pfx):
    c,err=fe.compile_unit(u,R+'/'+u,pfx)
    out=[]
    if c is None: return out
    t=open(R+'/'+u,encoding='latin-1').read()
    for a in addrs:
        e=fe.evaluate(c,t,a)
        if 'error' in e: continue
        O=[o for k,o,c_ in e['rows'] if o]; C=[c_ for k,o,c_ in e['rows'] if c_]
        oi={o[0]:n for n,o in enumerate(O)}; ci={c_[0]:n for n,c_ in enumerate(C)}
        # map image index -> our index via aligned rows (equal or replace rows paired)
        m={}
        for k,o,c_ in e['rows']:
            if o and c_: m[oi[o[0]]]=ci[c_[0]]
        cg={i:(j,g) for i,j,g in gaps(C)}
        for i,j,g in gaps(O):
            if not g: continue
            ci_=m.get(i)
            if ci_ is None or not re.match(r'^(test|cmp)\b',C[ci_][1]): continue
            # our jcc after our test
            k=ci_+1
            while k<len(C) and not re.match(r'^j',C[k][1]) and k-ci_<12: k+=1
            if k>=len(C) or not re.match(r'^j(?!mp)',C[k][1]): continue
            ours_gap=[norm(C[x][1]) for x in range(ci_+1,k)]
            missing=[norm(O[x][1]) for x in g]
            for s in ours_gap:
                if s in missing: missing.remove(s)
            if not missing or len(g)<=len(ours_gap): continue
            # where are they in ours? fall-through head / taken head
            fall=[norm(C[x][1]) for x in range(k+1,min(len(C),k+1+2*len(missing)+3))]
            tgt=C[k][3]; th=[]
            if tgt is not None and tgt in ci:
                ti=ci[tgt]; th=[norm(C[x][1]) for x in range(ti,min(len(C),ti+2*len(missing)+3))]
            where=[]
            for s in missing:
                if s in fall and s in th: where.append('B')
                elif s in fall: where.append('F')
                elif s in th: where.append('T')
                else: where.append('-')
            if all(w=='-' for w in where): continue
            # value used after the jcc on both image paths? (unconditional) - record image jcc target head
            it=O[j][3]; oi2={o[0]:n for n,o in enumerate(O)}
            ih=[norm(O[x][1]) for x in range(oi2[it],min(len(O),oi2[it]+4))] if it in oi2 else []
            ifl=[norm(O[x][1]) for x in range(j+1,min(len(O),j+4))]
            out.append(dict(ih=ih,ifl=ifl,unit=u,addr='%08X'%a,diff=e['diff'],img='%x'%(e['lo']+O[i][0]),ours='%x'%C[ci_][0],
                            ijcc=O[j][1],ojcc=C[k][1],missing=missing,where=''.join(where)))
    return out
by=collections.defaultdict(list)
for r in rows:
    if not r['exact'] and (not only or r['unit'] in only): by[r['unit']].append(int(r['addr'],16))
base=int(os.environ.get('FE_PFX_BASE','2')); nj=int(os.environ.get('FE_JOBS','5'))
units=sorted(by)
res=[]
with ThreadPoolExecutor(nj) as ex:
    import queue; Q=queue.Queue()
    for n in range(nj): Q.put(str(base+n))
    def job(u):
        p=Q.get()
        try: return run(u,by[u],p)
        finally: Q.put(p)
    futs=[ex.submit(job,u) for u in units]
    for f in futs:
        for d in f.result():
            res.append(d)
            print(d['unit'],d['addr'],'diff',d['diff'],'img',d['img'],'ours+'+d['ours'],d['where'],d['missing'],'|',d['ijcc'],'/',d['ojcc'],flush=True)
json.dump(res,open(os.environ.get('AF_OUT','/dev/null'),'w'),indent=1)
