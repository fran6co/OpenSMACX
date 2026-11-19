"""mv.py unit addr header 'decl line' [N_pads_in_cpp]: move the declaration line to every depth-0 position of the header."""
import sys,re
sys.path.insert(0,'/tmp/claude-1000/-home-fran6co-code-OpenSMACX/be235c8c-71b6-49f3-abea-875f51ac16da/scratchpad/hp2'); import hv
unit,addr,hdr,decl=sys.argv[1:5]
R=hv.R; cpp=hv.strip_hp(open(R+'/'+unit,encoding='latin-1').read())
H=open(R+'/src/'+hdr,encoding='latin-1').read().split('\n')
k=[i for i,l in enumerate(H) if l.strip()==decl.strip()]; assert len(k)==1,k; k=k[0]
G=H[:k]+H[k+1:]
cand=[];depth=0;prev=True
for i,l in enumerate(G):
    s=l.strip(); code=re.sub(r'//.*','',s)
    if depth==0 and prev: cand.append(i)
    depth+=code.count('{')-code.count('}')
    if s and not s.startswith('//') and not s.startswith('*') and not s.startswith('/*'):
        prev=code.endswith(';') or code.endswith('}') or (s.startswith('#') and not s.endswith('\\'))
lo=int(sys.argv[5]) if len(sys.argv)>5 else 0
cand=[i for i in cand if i>=lo-1]
V=[(i,{hdr:'\n'.join(G[:i]+[H[k]]+G[i:])},cpp) for i in cand]
last=None
for i,r in hv.run(unit,[int(addr,16)],V):
    if isinstance(r,str): r='ERR'
    if r!=last: print('at L%-5d %s   %s'%(i+1,r,G[i].strip()[:80] if i<len(G) else '')); last=r
