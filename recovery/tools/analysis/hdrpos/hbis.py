"""hbis.py unit addr header N [--others]: remove the unit's HEADER-PADS block (if any), then put N pads at every
depth-0 line of <header> (src-relative path) in a mirrored tree; report target rows per line."""
import sys,os,re,shutil,subprocess,threading
from concurrent.futures import ThreadPoolExecutor
R='/home/fran6co/code/OpenSMACX/.opensmacx/worktrees/par-5'
sys.path.insert(0,R+'/recovery/tools'); import fe
from match_recovery.pe_coff import Coff
unit,addr,hdr,N=sys.argv[1],int(sys.argv[2],16),sys.argv[3],int(sys.argv[4])
txt=open(R+'/'+unit,encoding='latin-1').read()
m=re.search(r'// HEADER-PADS: function \w+\n((?:void \w+\(\);\n)*)',txt)
if m and '--keep' not in sys.argv: txt=txt[:m.start(1)]+txt[m.end(1):]
H=open(R+'/src/'+hdr,encoding='latin-1').read().split('\n')
# candidate lines: brace depth 0, previous significant line ends a declaration
cand=[];depth=0;prev_ok=True;incomment=False
for i,l in enumerate(H):
    s=l.strip()
    if depth==0 and prev_ok and not incomment: cand.append(i)
    code=re.sub(r'//.*','',s); code=re.sub(r'"(\\.|[^"\\])*"','""',code); code=re.sub(r"'(\\.|[^'\\])*'","''",code)
    if '/*' in code and '*/' not in code: incomment=True
    if incomment and '*/' in code: incomment=False
    depth+=code.count('{')-code.count('}')
    if s=='' or s.startswith('//') or s.startswith('#'): prev_ok = prev_ok if s=='' or s.startswith('//') else (not s.endswith('\\'))
    else: prev_ok = code.endswith(';') or code.endswith('}')
pads=''.join('void HbPad%d();'%q for q in range(N))
lk=threading.Lock(); ctr=[0]; tls=threading.local(); cmd0=fe.compile_cmd(unit)
def lane():
    if not hasattr(tls,'p'):
        with lk: tls.p=27+ctr[0]; ctr[0]+=1
        D='%s/build/recovery/fe/hm%d'%(R,tls.p)
        shutil.rmtree(D,ignore_errors=True); shutil.copytree(R+'/src',D+'/src')
        tls.D=D
        shutil.copy(R+'/src/'+hdr,D+'/src/'+hdr)
    return tls.p,tls.D
def comp(D,p,src):
    obj='%s/o.obj'%D; out=[]
    for t in cmd0:
        if t.startswith('/Fo'): t='/Fo'+obj
        elif t.startswith('/Fd') or t.startswith('--repro-root') or t=='/W4': continue
        elif t==R+'/'+unit: t=src
        else: t=t.replace(R+'/src',D+'/src')
        out.append(t)
    if os.path.exists(obj): os.remove(obj)
    env=dict(os.environ, VC6_PREFIX=f'{fe.MAIN}/.opensmacx/wine/worker-{p}')
    subprocess.run(out,cwd=D,env=env,capture_output=True,text=True)
    return Coff(open(obj,'rb').read()) if os.path.exists(obj) else None
others=[int(x.group(1),16) for x in re.finditer(r'// ORIGINAL: 0x([0-9A-F]{8}) ',txt)] if '--others' in sys.argv else []
def job(i):
    p,D=lane()
    M=H[:]
    if i>=0: M.insert(i,pads)
    open(D+'/src/'+hdr,'w',encoding='latin-1').write('\n'.join(M))
    src=D+'/'+unit; open(src,'w',encoding='latin-1').write(txt)
    c=comp(D,p,src)
    if c is None: return i,None,None
    r=fe.evaluate(c,txt,addr).get('diff')
    o=[(a,fe.evaluate(c,txt,a).get('diff')) for a in others]
    return i,r,o
res=list(ThreadPoolExecutor(5).map(job,[-1]+cand))
base=res[0][1]; print('no pads in header:',base,'candidates',len(cand),'first',cand[0]+1,'last',cand[-1]+1,'of',len(H))
last=None
for i,r,o in res[1:]:
    if r!=last: print('   (prev cand L%d)'%(pv+1) if last is not None else '','\nL%-5d rows=%s  %s'%(i+1,r,H[i].strip()[:90])); last=r
    pv=i
