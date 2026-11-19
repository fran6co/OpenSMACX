"""sdeck.py: structural deck evaluator. `sdeck.run(unit, addr, lo, hi, deck)`, deck = [(key, whole-file text)].
Per variant: (EH unwind-depth distance, named call multiset distance, jcc/jmp/call/ret count distance, control-flow hunks, fe rows) plus
the counts of callees named in WATCH; sorted by that key, so a variant is judged by its decisions before its rows.
Lanes FE_PFX_BASE..FE_PFX_BASE+FE_JOBS-1."""
import sys,os,difflib,collections,hashlib
from concurrent.futures import ThreadPoolExecutor
S=os.path.dirname(os.path.abspath(__file__)); T=os.path.dirname(S); R=os.path.dirname(os.path.dirname(T))
sys.path.insert(0,T); import fe
sys.path.insert(0,S); import callees as CL, ehcensus as EH
def depths(fi):
    if not isinstance(fi,dict): return []
    d=[]
    for p in fi['par']: d.append(0 if p<0 or p>=len(d) else d[p]+1)
    return d
def ehdist(lo,c,sym):
    # edit distance between the unwind-state depth sequences (image FuncInfo against ours)
    a,b=depths(EH.img_fi(lo)),depths(EH.our_fi(c,sym))
    row=list(range(len(b)+1))
    for i,x in enumerate(a,1):
        prev,row[0]=row[0],i
        for j,y in enumerate(b,1):
            cur=min(row[j]+1,row[j-1]+1,prev+(x!=y)); prev,row[j]=row[j],cur
    return row[-1]
WATCH=[]
SCR=fe.SCR+'/sdeck'
os.makedirs(SCR,exist_ok=True)
def toks(ins): return [('jcc' if m[0]=='j' and m!='jmp' else m) for m in ins]
def evaluate(unit,addr,lo,hi,text,pfx,callnames=False):
    path='%s/v%s.cpp'%(SCR,pfx); open(path,'w',encoding='latin-1').write(text)
    c,err=fe.compile_unit('src/'+unit,path,str(pfx))
    if c is None: return None
    r=fe.evaluate(c,text,addr)
    if 'error' in r: return None
    sym,_,_=fe.ident(text,addr); s=c.function(sym); sec=c.sections[s.section-1]
    b=[i.mnemonic for i in fe.md.disasm(sec.data,0) if i.mnemonic[0]=='j' or i.mnemonic in('call','ret')]
    a=[i.mnemonic for i in fe.md.disasm(fe.imgbytes(lo,hi),lo) if i.mnemonic[0]=='j' or i.mnemonic in('call','ret')]
    ca=collections.Counter(toks(a)); cb=collections.Counter(toks(b))
    cnt=sum(abs(ca[k]-cb[k]) for k in set(ca)|set(cb))
    sm=difflib.SequenceMatcher(None,a,b,autojunk=False)
    h=sum(1 for op in sm.get_opcodes() if op[0]!='equal')
    d=CL.diff(c,text,addr); named=None; w=()
    if d:
        ci,co=d; named=sum(((ci-co)+(co-ci)).values())
        w=tuple('%s:%d/%d'%(k[:14],ci[k],co[k]) for k in sorted(set(ci)|set(co)) if any(x in k for x in WATCH) and ci[k]!=co[k])
    return (ehdist(lo,c,sym),named,cnt,h,r['diff'])+w
def run(unit,addr,lo,hi,deck,base=None,jobs=None):
    base=int(base or fe.PFX); jobs=int(jobs or os.environ.get('FE_JOBS','5'))
    res0=evaluate(unit,addr,lo,hi,deck[0][1],base)  # warm caches serially
    fe.compile_cmd("src/"+unit)
    res={}
    def lane(k):
        for i in range(k,len(deck),jobs):
            key,text=deck[i]; res[key]=evaluate(unit,addr,lo,hi,text,base+k)
    with ThreadPoolExecutor(jobs) as ex: list(ex.map(lane,range(jobs)))
    out=sorted(res.items(),key=lambda kv:(kv[1] is None, kv[1] or ()))
    for k,v in out: print(v,k)
    return out
