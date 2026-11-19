"""ucmp.py <unit.cpp> [<unit.cpp>...] : per-marker fe rows of the HEAD version against the worktree version of each
unit (headers from the worktree); prints every marker that moved and the exact counts. Lanes FE_PFX_BASE..+FE_JOBS-1."""
import sys,os,re,subprocess
from concurrent.futures import ThreadPoolExecutor
S=os.path.dirname(os.path.abspath(__file__)); R=os.path.dirname(os.path.dirname(os.path.dirname(S)))
P=os.environ.get('FE_SCRATCH',R+'/build/recovery/fe')+'/ucmp'; os.makedirs(P,exist_ok=True)
BASE=int(os.environ.get('FE_PFX_BASE','21')); J=int(os.environ.get('FE_JOBS','5'))
def run(u,src,pfx):
    t=open(src,encoding='latin-1').read()
    addrs=re.findall(r'// ORIGINAL: (0x[0-9A-F]{8}) \S+ 0x[0-9A-Fa-f]+-0x',t)
    p=subprocess.run(['uv','run','python',R+'/recovery/tools/fe.py','src/'+u,','.join(addrs),'--quiet','--pfx',str(pfx),'--src',src],capture_output=True,text=True,cwd=R)
    return {m.group(1):(m.group(2)=='True',int(m.group(3))) for m in re.finditer(r'FE ([0-9A-F]{8}) \S+ exact=(\w+) diff=(\d+)',p.stdout)}
jobs=[]
for i,u in enumerate(sys.argv[1:]):
    h='%s/head_%s'%(P,u.replace('/','_')); open(h,'w',encoding='latin-1').write(subprocess.run(['git','show',os.environ.get('UCMP_REV','HEAD')+':src/'+u],capture_output=True,text=True,encoding='latin-1',cwd=R).stdout)
    jobs.append((u,h)); jobs.append((u,R+'/src/'+u))
res=[None]*len(jobs)
def lane(k):  # one wine prefix per lane, each lane serial
    for n in range(k,len(jobs),J): res[n]=run(*jobs[n],BASE+k)
with ThreadPoolExecutor(J) as ex: list(ex.map(lane,range(J)))
for i,u in enumerate(sys.argv[1:]):
    a,b=res[2*i],res[2*i+1]
    for k in sorted(set(a)|set(b)):
        if a.get(k)!=b.get(k): print(u,k,'head',a.get(k),'cur',b.get(k))
    print(u,'markers',len(b),'exact head',sum(1 for v in a.values() if v[0]),'cur',sum(1 for v in b.values() if v[0]))
