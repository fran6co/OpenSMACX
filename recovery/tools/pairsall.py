#!/usr/bin/env python3
"""pairsall.py <feall.json> [maxdiff] [--write] : run pairs.py on every non-exact function with diff <= maxdiff."""
import sys,os,json,subprocess,collections
from concurrent.futures import ThreadPoolExecutor
R=os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))); S=os.path.dirname(os.path.abspath(__file__))
r=[x for x in json.load(open(sys.argv[1])) if not x['exact'] and not x.get('claimed') and 0<x['diff']<=int(sys.argv[2])]
by=collections.defaultdict(list)
for x in r: by[x['unit']].append(x['addr'])
groups=sorted(by.items(),key=lambda kv:-len(kv[1]))
extra=[a for a in sys.argv[3:]]
def lane(k):
    out=[]
    for i in range(k,len(groups),14):
        u,addrs=groups[i]
        for a in addrs:
            p=subprocess.run(['uv','run','python',S+'/pairs.py',u,'0x'+a,str(21+k)]+extra,cwd=R,capture_output=True,text=True)
            out.append(p.stdout.strip().split('\n')[-1] if p.stdout.strip() else json.dumps(dict(unit=u,addr=a,err=p.stderr[-200:])))
    return out
res=[]
with ThreadPoolExecutor(14) as ex:
    for o in ex.map(lane,range(14)): res+=o
for l in res:
    try: j=json.loads(l)
    except Exception: print(l[:160]); continue
    if j.get('err') or j.get('best',0)<j.get('base',0): print(l[:260])
print('functions',len(res))
