#!/usr/bin/env python3
"""unitpad.py <unit> [--write] [--at ADDR]: whole-unit sweep of the unit's pad block (CANARY-PADS/HEADER-PADS: function NAME) 0..63.
Per count: number of fe-exact markers and the claimed markers that are not exact. --at ADDR sweeps pads inserted before that marker instead (no block needed)."""
import sys,os,re,collections
R=os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
sys.path.insert(0,R+'/recovery/tools'); import fe
from concurrent.futures import ThreadPoolExecutor
unit=sys.argv[1]; src=open(R+'/'+unit,encoding='latin-1').read()
base=int(os.environ.get('FE_PFX_BASE','2')); jobs=int(os.environ.get('FE_JOBS','5'))
if '--at' in sys.argv:
    a=sys.argv[sys.argv.index('--at')+1]; m0=src.index('// ORIGINAL: 0x%08X'%int(a,16)); c=src.rfind('/*',0,m0); e=src.rfind('*/',0,m0)
    if c>e: m0=c
    mk=lambda n: src[:m0]+''.join('void FePadDecl%d();\n'%i for i in range(n))+src[m0:]; cur=0
else:
    m=re.search(r'(// (?:CANARY|HEADER)-PADS: (function|extern) (\w+)\n\n?)((?:[^\n]*\b\3\d+[^\n]*;\n)*)',src)
    kind=m.group(2); name=m.group(3); lines=[l for l in m.group(4).split('\n') if l]; cur=len(lines)
    fmt=('void %s%d();\n' if kind=='function' else None)
    if fmt is None:
        ex=lines[0] if lines else sys.exit('extern block empty'); fmt=re.sub(r'%s\d+'%name,name+'%d',ex).replace('%d','\0').replace('%','%%').replace('\0','%d')+'\n'; fmt='%s'.join(fmt.split(name)) if False else fmt
        mk=lambda n: src[:m.start(4)]+''.join(fmt%i for i in range(n))+src[m.end(4):]
    else:
        mk=lambda n: src[:m.start(4)]+''.join(fmt%(name,i) for i in range(n))+src[m.end(4):]
marks=[(int(x.group(1),16),x.group(2),'BYTE_EXACT' in x.group(3)) for x in re.finditer(r'// ORIGINAL: 0x([0-9A-F]{8}) (\S+) [^\n]*?((?:BYTE_EXACT)?)\n',src)]
marks=[(int(x.group(1),16),x.group(2),'BYTE_EXACT' in x.group(0)) for x in re.finditer(r'// ORIGINAL: 0x([0-9A-F]{8}) (\S+)[^\n]*',src)]
if '--only' in sys.argv:
    keep={int(x,16) for x in sys.argv[sys.argv.index('--only')+1].split(',')}; marks=[m_ for m_ in marks if m_[0] in keep]
fe.compile_cmd(unit)
def lane(k):
    out=[]
    for n in range(k,64,jobs):
        t=mk(n); p='%s/build/recovery/fe/up%d.cpp'%(R,k); open(p,'w',encoding='latin-1').write(t)
        c,err=fe.compile_unit(unit,p,str(base+k))
        if c is None: out.append((n,None)); continue
        d={}
        for a,s,cl in marks:
            try: r=fe.evaluate(c,t,a); d[a]=r['diff'] if not r['exact'] else 0
            except Exception: d[a]=-1
        out.append((n,d))
    return out
res=[]
with ThreadPoolExecutor(jobs) as ex:
    for o in ex.map(lane,range(jobs)): res+=o
res.sort(); nm={a:s for a,s,c in marks}
always=set(a for a,_,_ in marks) if '--only' not in sys.argv else set()
for n,d in res:
    if d and '--only' not in sys.argv: always&={a for a in d if d[a]!=0}
groups=collections.OrderedDict()
for n,d in res:
    if d is None: key='FAIL'
    else: key=' '.join('%08X:%d'%(a,d[a]) for a in sorted(d) if d[a]!=0 and a not in always)
    groups.setdefault(key,[]).append(n)
print('current',cur,'never-exact',len(always))
for k,v in sorted(groups.items(),key=lambda kv:len(kv[0])): print(v,'->',k or 'ALL OTHERS EXACT')
if '--write' in sys.argv and '--at' not in sys.argv:
    best=min(groups.items(),key=lambda kv:(len(kv[0].split()),min(abs(x-cur) for x in kv[1])))
    n=min(best[1],key=lambda x:abs(x-cur)); open(R+'/'+unit,'w',encoding='latin-1').write(mk(n)); print('wrote',n)
