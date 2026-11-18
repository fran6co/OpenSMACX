#!/usr/bin/env python3
"""perm.py <unit> <addr> --lines A-B [--max N] [--jobs 14] [--win lo-hi] [--apply K]
Permute source lines A..B (1-based, each one whole statement) of the unit and fast-evaluate each order.
variants.py-style use: --deck FILE (python file defining DECK = [(name, full_source_text), ...])."""
import sys,os,re,json,subprocess,argparse,itertools,random,threading
from concurrent.futures import ThreadPoolExecutor
R=os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))); S=os.path.dirname(os.path.abspath(__file__))
def run_deck(unit,addr,deck,jobs=14,win=None):
    """deck: list of (key, source_text). Returns list of (win, diff, exact, key) sorted best first."""
    d=S+'/perm'; os.makedirs(d,exist_ok=True)
    def lane(k):
        cmd=['uv','run','python',S+'/fe.py',unit,addr,'--batch','--pfx',str(21+k)]
        if win: cmd+=['--win',win]
        p=subprocess.Popen(cmd,cwd=R,stdin=subprocess.PIPE,stdout=subprocess.PIPE,text=True)
        out=[]
        for i in range(k,len(deck),jobs):
            key,text=deck[i]; path='%s/l%d.cpp'%(d,k)
            open(path,'w',encoding='latin-1').write(text)
            p.stdin.write(path+'\n'); p.stdin.flush()
            o=p.stdout.readline().split()
            if len(o)<3 or o[1]=='FAIL': out.append(((9999,),9999,False,key)); continue
            ex=o[-1]=='True'; dd=int(o[-2]); w=tuple(int(x) for x in o[1:-2]) or (0,)
            out.append((w,dd,ex,key))
        p.stdin.close(); p.wait()
        return out
    res=[]
    with ThreadPoolExecutor(jobs) as ex:
        for o in ex.map(lane,range(jobs)): res+=o
    res.sort(key=lambda r:(r[0],r[1]))
    return res
if __name__=='__main__':
    ap=argparse.ArgumentParser(); ap.add_argument('unit'); ap.add_argument('addr'); ap.add_argument('--lines'); ap.add_argument('--max',type=int,default=5040)
    ap.add_argument('--jobs',type=int,default=14); ap.add_argument('--win'); ap.add_argument('--apply',type=int); ap.add_argument('--top',type=int,default=8); ap.add_argument('--deck')
    a=ap.parse_args()
    unit=a.unit if a.unit.startswith('src/') else 'src/'+a.unit
    src=open(R+'/'+unit,encoding='latin-1').read()
    if a.deck:
        g={}; exec(open(a.deck).read(),g); deck=g['DECK']
    else:
        L=src.split('\n'); A,B=[int(x) for x in a.lines.split('-')]
        block=L[A-1:B]; n=len(block)
        perms=list(itertools.permutations(range(n))) if n<=7 else None
        if perms is None or len(perms)>a.max:
            random.seed(1); seen=set(); perms=[tuple(range(n))]
            while len(perms)<a.max:
                p=list(range(n)); random.shuffle(p); p=tuple(p)
                if p not in seen: seen.add(p); perms.append(p)
        deck=[(p,'\n'.join(L[:A-1]+[block[j] for j in p]+L[B:])) for p in perms]
    res=run_deck(unit,a.addr,deck,a.jobs,a.win)
    print('tried',len(res))
    for w,dd,ex_,key in res[:a.top]: print(w,dd,ex_,key)
    if a.apply is not None:
        key=res[a.apply][3]; text=dict(deck)[key]
        open(R+'/'+unit,'w',encoding='latin-1').write(text); print('applied',key)
