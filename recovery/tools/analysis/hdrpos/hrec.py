"""hrec.py unit addr N first_header : recursive boundary search. Prints the chain of boundaries."""
import sys,os,re,subprocess
R='/home/fran6co/code/OpenSMACX/.opensmacx/worktrees/par-5'; HB='/tmp/claude-1000/-home-fran6co-code-OpenSMACX/be235c8c-71b6-49f3-abea-875f51ac16da/scratchpad/hp2/hbis.py'
unit,addr,N,h=sys.argv[1:5]
def find(name,cur):
    for d in [os.path.dirname(cur),'','jackal','caviar','uv']:
        p=os.path.normpath(os.path.join(d,name))
        if os.path.exists(R+'/src/'+p): return p
for depth in range(8):
    out=subprocess.run(['uv','run','python',HB,unit,addr,h,N],cwd=R,capture_output=True,text=True).stdout
    print('==',h); print(out.strip())
    L=[l for l in out.split('\n') if l.startswith('L')]
    pv=re.findall(r'prev cand L(\d+)',out)
    if len(L)<2: print('no boundary'); break
    H=open(R+'/src/'+h,encoding='latin-1').read().split('\n')
    a=int(pv[0])-1; b=int(L[1].split()[0][1:])-1
    seg=H[a:b]; print('   boundary segment:'); [print('     |',x) for x in seg]
    inc=[re.search(r'#include\s+"([^"]+)"',x) for x in seg]; inc=[m.group(1) for m in inc if m]
    if not inc: break
    h=find(inc[0],h)
