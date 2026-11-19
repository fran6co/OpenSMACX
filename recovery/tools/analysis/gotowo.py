"""gotowo.py <unit> <0xADDR>: MATCHING_GUIDE 2.7 rule (b) screen. For every label whose block ends in a
return (or the function end, <= 12 statements), write that block out at subsets of its goto sites
(all subsets up to five sites, else each site alone and all) and fe each variant. Env: FE_PFX_BASE,
FE_JOBS, MAXV (variant cap, default 40), OUT (file for the best variant when it beats the base)."""
import sys,os,re,itertools
R=os.getcwd()
sys.path.insert(0,R+'/recovery/tools'); os.chdir(R)
import perm
unit,addr=sys.argv[1],sys.argv[2]
MAXV=int(os.environ.get('MAXV','40'))
t=open(unit,encoding='latin-1').read()
m=re.search(r'// ORIGINAL: 0x%s '%addr[2:].upper().zfill(8),t)
start=m.start()
# function body: first '{' at line end after a line not starting with //
i=start
lines=t[start:].split('\n')
off=start; body_start=None
for ln in lines:
    if not ln.startswith('//') and ln.rstrip().endswith('{') and body_start is None and not ln.startswith(' '):
        body_start=off+len(ln); break
    off+=len(ln)+1
depth=1; j=body_start
while depth:
    j+=1
    c=t[j]
    if c=='{': depth+=1
    elif c=='}': depth-=1
body_end=j  # index of closing brace
body=t[body_start:body_end]
isvoid=bool(re.search(r'\bvoid\b[^\n]*\(', t[start:body_start].split('\n')[-1]))
labels=re.findall(r'^\s*([A-Za-z_]\w*):\s*$',body,re.M)
out=[]
for L in labels:
    lm=re.search(r'^(\s*)%s:\s*\n'%L,body,re.M)
    rest=body[lm.end():]
    blk=[];ok=False;bal=0
    for ln in rest.split('\n'):
        s=ln.strip()
        if re.match(r'^[A-Za-z_]\w*:\s*$',s): break
        bal+=ln.count('{')-ln.count('}')
        if bal<0:
            # leaving enclosing block: only ok if this is the function end
            break
        blk.append(s)
        if s.startswith('return') and bal==0: ok=True; break
    else:
        ok=True  # reached function end
        blk=[b for b in blk if b]
        if isvoid: blk.append('return;')
    if bal<0 and lm.end()+len('\n'.join(blk))+1>=len(body)-2: ok=True; blk=[b for b in blk if b]+(['return;'] if isvoid else [])
    if not ok or not blk or len(blk)>12: print('skip',L,ok,len(blk)); continue
    code='{ '+' '.join(b for b in blk if b)+' }'
    sites=[mm.start() for mm in re.finditer(r'goto\s+%s\s*;'%L,body)]
    print('label',L,'sites',len(sites),'block',code[:120])
    n=len(sites)
    combos=list(itertools.product([0,1],repeat=n)) if n<=5 else [tuple(1 if k==s else 0 for k in range(n)) for s in range(n)]+[tuple([1]*n)]
    for cb in combos:
        if not any(cb): continue
        nb=body; 
        for k in reversed(range(n)):
            if cb[k]:
                mm=list(re.finditer(r'goto\s+%s\s*;'%L,nb))[k]
                nb=nb[:mm.start()]+code+nb[mm.end():]
        out.append(('%s:%s'%(L,''.join(map(str,cb))),t[:body_start]+nb+t[body_end:]))
out=out[:MAXV]
deck=[('base',t)]+out
res=perm.run_deck(unit,addr,deck,jobs=int(os.environ.get('FE_JOBS','1')))
res.sort(key=lambda r:r[1])
for r in res[:6]: print(r[1],r[2],r[3])
base=[r for r in res if r[3]=='base'][0][1]
print('BASE',base,'BEST',res[0][1],res[0][3])
if res[0][1]<base:
    open(os.environ.get('OUT','/dev/null'),'w',encoding='latin-1').write(dict(deck)[res[0][3]])
