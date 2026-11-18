"""psearch.py <unit> <0xADDR> <name> [--iters N] [--seed S] [--apply] [--deep]
Hill-climb dead /Ob2 budget probes over the statement gaps of one function; score = fe differing rows."""
import sys,os,re,random,argparse
os.environ.setdefault('FE_PFX_BASE','14'); os.environ.setdefault('FE_JOBS','5')
sys.path.insert(0,'recovery/tools')
import perm
ap=argparse.ArgumentParser(); ap.add_argument('unit'); ap.add_argument('addr'); ap.add_argument('name'); ap.add_argument('--iters',type=int,default=8); ap.add_argument('--seed',type=int,default=1); ap.add_argument('--apply',action='store_true'); ap.add_argument('--deep',action='store_true'); ap.add_argument('--n',type=int,default=60)
a=ap.parse_args()
t=open(a.unit,encoding='latin-1').read(); L=t.split('\n')
mi=[i for i,l in enumerate(L) if '// ORIGINAL: 0x%08X'%int(a.addr,16) in l][0]
# skip to end of comment block if inside /* */
st=None
for i in range(mi+1,len(L)):
    l=L[i]
    if l and not l.startswith(('//','/*',' *','*/','#',' ','\t','}')) and '(' in l:
        j=i
        while not L[j].rstrip().endswith('{'):
            j+=1
            if L[j].rstrip().endswith(';'): j=None; break
        if j is None: continue
        st=j; break
en=next(i for i in range(st+1,len(L)) if L[i].startswith('}'))
gaps=[]
for i in range(st+1,en+1):
    l=L[i]; ind=len(l)-len(l.lstrip())
    if not l.strip() or l.strip().startswith(('//','}','else','case ','default:','#','/*','*')): 
        if i!=en: continue
    if i!=en and (ind!=4 and not (a.deep and ind in (8,12))): continue
    k=i-1
    while k>st and (not L[k].strip() or L[k].strip().startswith('//')): k-=1
    p=L[k].rstrip()
    if k==st or p.endswith((';','}','{')) and not p.endswith(('= {',)):
        if i==en:
            # before closing brace only if last stmt is not return/goto
            if re.match(r'\s*(return|goto)\b',p): continue
        gaps.append(i)
print('function lines',st+1,en+1,'gaps',len(gaps))
P1='%s_budget_probe1'%a.name; P4='%s_budget_probe4'%a.name
defs=['static inline int %s(int x) { if (x == 0) x = 1; return x; }'%P1,
'static inline int %s(int x) {'%P4,'    if (x == 0) x = 1;','    if (x == 1) x = 4;','    if (x == 2) x = 7;','    if (x == 3) x = 10;','    return x;','}']
def build(vec,final=False):
    out=list(L); 
    for gi in sorted(vec,reverse=True):
        n1,n4=vec[gi]; i=gaps[gi]
        if n1==0 and n4==0: continue
        ind=' '*(len(L[i])-len(L[i].lstrip())) if i!=en else '    '
        if final:
            ins=[ind+'%s(5);  // WORKAROUND(inline): see %s'%(P4,P1)]*n4+[ind+'%s(5);  // WORKAROUND(inline): see %s'%(P1,P1)]*n1
        else:
            ins=[ind+('%s(5);'%P4)*n4+('%s(5);'%P1)*n1]
        out[i:i]=ins
    hdr=defs
    if final:
        hdr=['// WORKAROUND(inline): dead /Ob2 budget probes, not recovered code; constant',
             '// arguments fold them away. They spend the budget the image spends before',
             '// this function\'s Popup teardowns (found by a probe hill-climb scored on the',
             '// image diff); the original\'s spend is not identified.']+defs
    out[st_decl:st_decl]=hdr
    return '\n'.join(out)
# declaration start: walk back from st to the line starting the signature
st_decl=st
while st_decl>0 and (L[st_decl].startswith((' ','\t')) or not L[st_decl].strip()): st_decl-=1
random.seed(a.seed)
def key(v): return tuple(sorted((g,n) for g,n in v.items() if n!=(0,0)))
seen={}
best={}; 
res=perm.run_deck(a.unit,a.addr,[('base',t),(key(best),build(best))])
for w,dd,ex,k in res: print('start',dd,k if k=='base' else 'with-defs')
seen[key(best)]=[r for r in res if r[3]!='base'][0][1]
bestd=seen[key(best)]
for it in range(a.iters):
    cands={}
    tries=0
    while len(cands)<a.n and tries<5000:
        tries+=1
        v=dict(best)
        for _ in range(random.choice((1,1,1,2,2,3))):
            g=random.randrange(len(gaps)); n1,n4=v.get(g,(0,0))
            if random.random()<0.5: n1=max(0,n1+random.choice((-2,-1,1,1,2,3)))
            else: n4=max(0,n4+random.choice((-2,-1,1,1,2,3)))
            v[g]=(n1,n4)
        k=key(v)
        if k not in seen and k not in cands: cands[k]=v
    deck=[(k,build(v)) for k,v in cands.items()]
    res=perm.run_deck(a.unit,a.addr,deck)
    for w,dd,ex,k in res: seen[k]=dd
    bk=min(seen,key=lambda k:(seen[k],sum(n1+n4 for g,(n1,n4) in k)))
    best=dict(bk); bestd=seen[bk]
    print(it,bestd,[(gaps[g]+1,n) for g,n in bk],flush=True)
    if bestd==0: break
if a.apply and bestd<seen[()]:
    open(a.unit,'w',encoding='latin-1').write(build(best,final=True)); print('APPLIED',bestd)
