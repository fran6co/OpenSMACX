import sys,os,re,subprocess,queue,threading
S='/tmp/claude-1000/-home-fran6co-code-OpenSMACX/be235c8c-71b6-49f3-abea-875f51ac16da/scratchpad'
def rep(t,a,b,n=1):
    assert a in t, a[:70]
    return t.replace(a,b,n)
def mrun(unit,addr,deck,lo=None,hi=None):
    """deck: [(key,text)] -> sorted [(masked,raw,key,path)]"""
    q=queue.Queue(); [q.put(p) for p in range(8,13)]
    out=[]
    def one(kt):
        k,t=kt; p=q.get()
        try:
            path=S+'/r4/m_%s.cpp'%re.sub(r'\W','_',k); open(path,'w',encoding='latin-1').write(t)
            cmd=['python3',S+'/r4/sm.py',unit,addr,path]+([lo,hi] if lo else [])
            o=subprocess.run(cmd,capture_output=True,text=True,env=dict(os.environ,PFX=str(p))).stdout
            m=re.search(r'masked-diff (\d+) raw (\d+)',o)
            if lo:
                rows=[l for l in o.splitlines() if l[:2] in('!!',)]
                out.append((len(rows),int(m.group(2)) if m else 9999,k,path))
            else: out.append((int(m.group(1)),int(m.group(2)),k,path) if m else (9999,9999,k,path))
        finally: q.put(p)
    from concurrent.futures import ThreadPoolExecutor
    with ThreadPoolExecutor(5) as ex: list(ex.map(one,deck))
    out.sort(); return out
def variants(base,E,combos=False,excl=()):
    import itertools
    keys=list(E); deck=[('base',base)]
    sets=[(k,) for k in keys]
    if combos: sets=[c for r in range(1,len(keys)+1) for c in itertools.combinations(keys,r) if not any(set(x)<=set(c) for x in excl)]
    for c in sets:
        t=base
        try:
            for k in c: t=E[k](t)
        except AssertionError as e: print('+'.join(c),'ASSERT',e); continue
        deck.append(('+'.join(c),t))
    return deck
