# rdeck.py UNIT ADDR VARIANTS.py [KEY,..]  -- frame-aware replacement deck.
# VARIANTS.py defines V = {key: [(old, new), ...]}; each old string must occur
# exactly once in the unit. Ranks the variants with perm.run_deck (differing
# rows, exact flag); KEY,.. writes those variants' sources to
# build/recovery/fe/rd/; RDF=1 also prints each variant's `sub esp` frame line.
import sys,os,re,subprocess
R=os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','..','..'))
sys.path.insert(0,R+'/recovery/tools'); os.chdir(R)
import perm
unit,addr,vf=sys.argv[1:4]
unit=unit if unit.startswith('src/') else 'src/'+unit
src=open(unit,encoding='latin-1').read()
g={}; exec(open(vf).read(),g); V=g['V']
deck=[('base',src)]
for k,reps in V.items():
    t=src; ok=True
    for o,n in reps:
        if t.count(o)!=1: print('BAD',k,t.count(o),repr(o[:60])); ok=False; break
        t=t.replace(o,n)
    if ok: deck.append((k,t))
res=perm.run_deck(unit,addr,deck)
# frame per variant
d=R+'/build/recovery/fe/rd'; os.makedirs(d,exist_ok=True)
txt=dict(deck)
for w,dd,ex,k in sorted(res,key=lambda r:r[1]):
    print(dd,ex,k)
if len(sys.argv)>4:
    for k in sys.argv[4].split(','):
        p=d+'/'+k+'.cpp'; open(p,'w',encoding='latin-1').write(txt[k])
        print('wrote',p)
if os.environ.get('RDF'):
    from concurrent.futures import ThreadPoolExecutor
    def fr(i_k):
        i,k=i_k
        p=d+'/f_'+str(i)+'.cpp'; open(p,'w',encoding='latin-1').write(txt[k])
        pf=str(int(os.environ.get('FE_PFX_BASE','27'))+i%5)
        o=subprocess.run(['uv','run','python','recovery/tools/fe.py',unit[4:],addr,'--pfx',pf,'--src',p,'--all'],capture_output=True,text=True).stdout
        m=[l for l in o.split('\n') if 'sub esp' in l][:1]
        return k,(m[0] if m else '?')
    keys=[k for k,_ in deck]
    with ThreadPoolExecutor(5) as ex:
        for k,l in ex.map(fr,list(enumerate(keys))): print('FRAME',k,l)
