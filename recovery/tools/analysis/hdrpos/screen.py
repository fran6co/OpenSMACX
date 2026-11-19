"""screen.py OUT.jsonl unit [unit...] : pads after stdafx.h (pos=s) or after last include (pos=e), N=0..63.
Env HP_POS=s|e|both, HP_NMAX=64. Per unit and N: diff rows of every marker."""
import sys,os,re,json,threading,hashlib
CACHE={}
from concurrent.futures import ThreadPoolExecutor
R='/home/fran6co/code/OpenSMACX/.opensmacx/worktrees/par-5'
sys.path.insert(0,R+'/recovery/tools'); import fe
DONE=set()
if os.path.exists(sys.argv[1]):
    import collections; _c=collections.Counter(json.loads(l)['unit'] for l in open(sys.argv[1])); DONE={u for u,k in _c.items() if k>=64}
out=open(sys.argv[1],'a'); units=[u for u in sys.argv[2:] if u not in DONE]
POS=os.environ.get('HP_POS','s'); NMAX=int(os.environ.get('HP_NMAX','64'))
PB=27; J=5
lk=threading.Lock()
def variant(text,pos,n):
    L=text.split('\n')
    if pos=='s':
        idx=[j for j,l in enumerate(L) if re.match(r'\s*#\s*include\s+"stdafx\.h"',l)]
        if not idx: idx=[min(j for j,l in enumerate(L) if l.startswith('#include'))-1]
        at=idx[0]+1
    else:
        at=max(j for j,l in enumerate(L[:3000]) if l.startswith('#include'))+1
    pads=''.join('void HpScreenPad%d();'%q for q in range(n))
    L.insert(at,pads)   # one line: keeps line numbers after it shifted by exactly 1
    return '\n'.join(L)
tls=threading.local(); ctr=[0]
def job(args):
    unit,pos,n,text,addrs=args
    if not hasattr(tls,'pfx'):
        with lk: tls.pfx=PB+ctr[0]; ctr[0]+=1
    d=os.path.dirname(R+'/'+unit); path='%s/__hp%d.cpp'%(d,tls.pfx)
    vt=variant(text,pos,n); open(path,'w',encoding='latin-1').write(vt)
    c,err=fe.compile_unit(unit,path,tls.pfx)
    os.remove(path)
    if c is None: return dict(unit=unit,pos=pos,n=n,error=(err or '')[:300])
    r={}
    for a in addrs:
        try:
            idn=fe.ident(vt,a)
            key=None
            try:
                fs=c.function(idn[0]); sec=c.sections[fs.section-1]
                key=(a,hashlib.sha1(bytes(sec.data)+repr([(x[0],x[2],c.symbols[x[1]].section==fs.section and c.symbols[x[1]].value) for x in sec.relocations]).encode()).hexdigest())
            except Exception: pass
            if key and key in CACHE: r['%08X'%a]=CACHE[key]; continue
            e=fe.evaluate(c,vt,a)
            if key and 'error' not in e: CACHE[key]=e['diff']
            if 'error' not in e: r['%08X'%a]=e['diff']
        except Exception as ex: pass
    return dict(unit=unit,pos=pos,n=n,rows=r)
tasks=[]
for u in units:
    t=open(R+'/'+u,encoding='latin-1').read()
    addrs=[int(m.group(1),16) for m in re.finditer(r'// ORIGINAL: 0x([0-9A-F]{8}) ',t)]
    for pos in (['s','e'] if POS=='both' else [POS]):
        for n in range(NMAX): tasks.append((u,pos,n,t,addrs))
with ThreadPoolExecutor(J) as ex:
    for res in ex.map(job,tasks):
        out.write(json.dumps(res)+'\n'); out.flush()
