# varslot.py <unit> <addr> <name-prefix|''> : per named local (assigned at a source line), the image frame slot its aligned
# stores use next to ours. A local whose image stores land in ANOTHER local's home is a candidate original reuse
# (MATCHING_GUIDE 3.3); `none` = the image has no store there (register or folded). Uses prefix FE_PFX_BASE.
import sys,os,re,collections,subprocess
R=os.getcwd(); sys.path.insert(0,R+'/recovery/tools'); sys.path.insert(0,R+'/recovery/tools/analysis')
import fe,tailscan
unit,addr,pref=sys.argv[1],int(sys.argv[2],16),(sys.argv[3] if len(sys.argv)>3 else '')
PFX=os.environ.get('FE_PFX_BASE','27')
t=open(unit,encoding='latin-1').read(); L=t.split('\n'); sym,lo,hi=fe.ident(t,addr)
c,cod=tailscan.compile_listing(unit,unit,PFX)
lines=tailscan.lines_of(cod,sym,os.path.basename(unit).lower())
out=subprocess.run(['uv','run','python','recovery/tools/fe.py',unit,sys.argv[2],'--pfx',PFX,'--all'],capture_output=True,text=True).stdout
def slot(s):
    m=re.search(r'mov (?:dword|byte|word) ptr \[ebp ([-+]) (0x[0-9a-f]+|\d+)\], ',s)
    if not m: return None
    v=int(m.group(2),0); return -v if m.group(1)=='-' else v
res=collections.defaultdict(collections.Counter); ours=collections.defaultdict(collections.Counter)
for l in out.splitlines():
    if '|' not in l: continue
    a,b=l.split('|',1); sb=slot(b)
    if sb is None: continue
    m=re.match(r'\s*([0-9a-f]+)\s',b)
    if not m: continue
    ln=lines.get(int(m.group(1),16))
    if not ln or not ln.startswith('L'): continue
    src=L[int(ln[1:])-1]
    vs=set(re.findall(r'\b(%s\w+)\s*(?:=[^=]|\+\+|--|[-+|&]=)'%pref,src))
    if len(vs)!=1: continue
    v=vs.pop(); sa=slot(a)
    ours[v][sb]+=1
    res[v][sa if sa is not None else 'none']+=1
for v in sorted(res,key=lambda v:str(res[v].most_common(1)[0][0])):
    print('%-48s ours %-14s img %s'%(v,' '.join(hex(k) for k,_ in ours[v].most_common(2)),' '.join('%s:%d'%(hex(k) if isinstance(k,int) else k,n) for k,n in res[v].most_common(3))))
