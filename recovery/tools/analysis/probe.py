"""probe.py <base.cpp> <stmt> <needle-regex> <line-from> <line-to> [step]: insert stmt before each top-level
statement line in range; report whether needle (regex on our disassembly) still occurs."""
import sys,os,re,subprocess,concurrent.futures as cf
base,stmt,needle,lo,hi=sys.argv[1],sys.argv[2],sys.argv[3],int(sys.argv[4]),int(sys.argv[5])
step=int(sys.argv[6]) if len(sys.argv)>6 else 1
T=open(base).read().split('\n')
pts=[n for n in range(lo,hi) if re.match(r' {4,8}(if|for|do|while|switch|[a-z_]+ = |[A-Za-z_]+\()',T[n-1]) and not T[n-2].rstrip().endswith((',','&&','||','('))]
pts=pts[::step]
d=os.path.dirname(base)+'/probe'; os.makedirs(d,exist_ok=True)
def run(a):
    k,n=a
    p='%s/p%d.cpp'%(d,n); open(p,'w').write('\n'.join(T[:n-1]+['    '+stmt]+T[n-1:]))
    env=dict(os.environ,FE_PFX_BASE=str(2+k%5))
    o=subprocess.run([sys.executable,'recovery/tools/objdis.py','src/enemy.cpp',p,'0x0056B5B0'],capture_output=True,text=True,env=env).stdout
    os.remove(p)
    return n,len(re.findall(needle,o)),o.count('\n')
import threading
lock=[threading.Lock() for _ in range(5)]
def runl(a):
    with lock[a[0]%5]: return run(a)
with cf.ThreadPoolExecutor(5) as ex:
    for n,c,ln in ex.map(runl,list(enumerate(pts))): print(n,c,ln,T[n-1][:70])
