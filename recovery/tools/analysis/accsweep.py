#!/usr/bin/env python3
"""accsweep.py FEALL.json [unit-substr ...] : accessor respelling screen for every non-exact function.
Families: T treaties->has_treaty, A agenda->has_agenda, N hand-expanded get_noun/get_title/get_name,
H human mask->is_human, L alive mask->is_alive, X all.  Prints rows and named-call distance (image vs ours) per family
for functions where a family changes either.  FEALL.json comes from feall.py --out; prefix FE_PFX_BASE.  Screen only."""
import sys,os,re,json,collections
S=os.path.dirname(os.path.abspath(__file__))
T=os.path.dirname(S)
R=os.path.dirname(os.path.dirname(T))
sys.path.insert(0,T); sys.path.insert(0,S)
import fe,cdeck,nat
ID=r'((?:[A-Za-z_][\w.]*(?:\[[^\[\]]+\])?(?:\.\w+)*)|\d+)'
HM=r'GameDataStateStorage\.fields_\.lock_masks_\.bytes_\.human_mask_'
AM=r'GameDataStateStorage\.fields_\.lock_masks_\.bytes_\.alive_mask_'
K=r'(\w+|\([^()]*\))'
def fam_T(g): return re.sub(r'PlayersData\[([^\[\]]+)\]\.diplo_treaties\[([^\[\]]+)\] & '+K+r'(?!\s*=[^=])',r'has_treaty(\1, \2, \3)',g)
def fam_A(g): return re.sub(r'PlayersData\[([^\[\]]+)\]\.diplo_agenda\[([^\[\]]+)\] & '+K+r'(?!\s*=[^=])',r'has_agenda(\1, \2, \3)',g)
def fam_H(g):
    g=re.sub(r'\(%s & \(1 << ([^()]+)\)\) != 0'%HM,r'is_human(\1)',g)
    g=re.sub(r'\(%s & \(1 << ([^()]+)\)\) == 0'%HM,r'!is_human(\1)',g)
    g=re.sub(r'%s & \(1 << ([^()]+)\)'%HM,r'is_human(\1)',g)
    g=re.sub(r'\(1 << ([^()]+)\) & %s'%HM,r'is_human(\1)',g)
    return g
def fam_L(g):
    g=re.sub(r'\(%s & \(1 << ([^()]+)\)\) != 0'%AM,r'is_alive(\1)',g)
    g=re.sub(r'\(%s & \(1 << ([^()]+)\)\) == 0'%AM,r'!is_alive(\1)',g)
    g=re.sub(r'%s & \(1 << ([^()]+)\)'%AM,r'is_alive(\1)',g)
    g=re.sub(r'\(1 << ([^()]+)\) & %s'%AM,r'is_alive(\1)',g)
    return g
FAM=[('T',fam_T),('A',fam_A),('N',nat.nat),('H',fam_H),('L',fam_L)]
def span(t,addr):
    a=t.index('// ORIGINAL: 0x%08X'%addr); a=t.index('\n',a)
    m=re.compile(r'\n[A-Za-z_][^\n;]*\)\s*(?:const\s*)?\{\n').search(t,a)
    b=t.index('\n}\n',m.start()); return m.start(),b
def dist(c,t,addr):
    r=fe.evaluate(c,t,addr); sym,lo,hi=fe.ident(t,addr)
    A=collections.Counter(cdeck.CRT.get(n,n) for a,n in cdeck.calls_img(lo,hi)); B=collections.Counter(n for a,n in cdeck.calls_ours(c,r['sym']))
    return r['diff'],sum(abs(A[k]-B[k]) for k in set(A)|set(B)), r['exact']
if __name__=='__main__':
    pfx=os.environ.get('FE_PFX_BASE','2')
    J=[e for e in json.load(open(sys.argv[1])) if not e['exact'] and e['diff']>0]
    by=collections.defaultdict(list)
    for e in J: by[e['unit']].append(int(e['addr'],16))
    for unit in sorted(by):
        if len(sys.argv)>2 and not any(s in unit for s in sys.argv[2:]): continue
        t=open(R+'/'+unit,encoding='latin-1').read()
        cb,err=fe.compile_unit(unit,R+'/'+unit,pfx)
        if cb is None: continue
        for addr in by[unit]:
            try: a,b=span(t,addr); base=dist(cb,t,addr)
            except Exception as ex: continue
            g=t[a:b]; out=[]; allg=g
            for k,f in FAM:
                g2=f(g)
                if g2==g: continue
                allg=f(allg)
                out.append((k,t[:a]+g2+t[b:]))
            if len(out)>1: out.append(('X',t[:a]+allg+t[b:]))
            res=[]
            for k,t2 in out:
                p='%s/build/recovery/fe/acc%s.cpp'%(R,pfx); open(p,'w',encoding='latin-1').write(t2)
                c,err=fe.compile_unit(unit,p,pfx)
                if c is None: res.append((k,'FAIL')); continue
                try: res.append((k,)+dist(c,t2,addr))
                except Exception as ex: res.append((k,'ERR'))
            if res: print('%s %08X %s base rows=%d calls=%d | %s'%(unit,addr,fe.ident(t,addr)[0][:30],base[0],base[1],'  '.join('%s:%s'%(r[0],'/'.join(map(str,r[1:]))) for r in res)),flush=True)
