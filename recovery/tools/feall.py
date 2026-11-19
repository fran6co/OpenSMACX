#!/usr/bin/env python3
"""feall.py [--jobs 14] [unit ...] : fast-evaluate every non-BYTE_EXACT marker, one compile per unit.
Writes scratchpad/feall.json: [{unit, addr, sym, exact, diff, img, ours, ratio}]"""
import sys,os,re,json,subprocess,argparse
from concurrent.futures import ThreadPoolExecutor
R=os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))); S=os.path.dirname(os.path.abspath(__file__))
ap=argparse.ArgumentParser(); ap.add_argument('units',nargs='*'); ap.add_argument('--jobs',type=int,default=int(os.environ.get('FE_JOBS','14'))); ap.add_argument('--out',default=S+'/feall.json'); ap.add_argument('--all',action='store_true'); ap.add_argument('--base'); ap.add_argument('--root',default=R)
a=ap.parse_args()
MAIN=os.environ.get('OPENSMACX_MAIN', R if os.path.exists(R+'/.opensmacx/game/terranx_original.exe') else '/home/fran6co/code/OpenSMACX')
cc=json.load(open(MAIN+'/build/compile_commands.json'))
units=[e['file'][len(MAIN)+1:] for e in cc if e['file'].startswith(MAIN+'/src/')]
if R!=MAIN and os.path.exists(R+'/build/compile_commands.json'):  # units a regroup created in this worktree
    units+=[e['file'][len(R)+1:] for e in json.load(open(R+'/build/compile_commands.json')) if e['file'].startswith(R+'/src/') and e['file'][len(R)+1:] not in units]
if a.units: units=[u for u in units if u in a.units or os.path.basename(u) in a.units]
work=[]
for u in units:
    try: t=open(R+'/'+u,encoding='latin-1').read()
    except Exception: continue
    addrs=[m.group(1) for m in re.finditer(r'// ORIGINAL: (0x[0-9A-F]{8}) \S+ 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)',t) if a.all or 'BYTE_EXACT' not in m.group(2)]
    if addrs: work.append((u,addrs,'BYTE_EXACT' and {m.group(1) for m in re.finditer(r'// ORIGINAL: (0x[0-9A-F]{8}) \S+ 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)',t) if 'BYTE_EXACT' in m.group(2)}))
def run(i):
    u,addrs,claimed=work[i]; out=[]
    p=subprocess.run(['uv','run','python',S+'/fe.py',u,','.join(addrs),'--quiet','--pfx',str(int(os.environ.get('FE_PFX_BASE','21'))+i%a.jobs)],cwd=R,capture_output=True,text=True)
    for l in p.stdout.split('\n'):
        m=re.match(r'FE ([0-9A-F]{8}) (\S+) exact=(\w+) diff=(\d+) img=(\d+) ours=(\d+) ratio=([\d.]+)(?: jt=(\d+))?',l)
        if m: out.append(dict(unit=u,addr=m.group(1),sym=m.group(2),claimed=('0x'+m.group(1)) in claimed,exact=m.group(3)=='True',diff=int(m.group(4)),img=int(m.group(5)),ours=int(m.group(6)),ratio=float(m.group(7)),jt=int(m.group(8) or 0)))
        elif l.startswith('FE') or 'COMPILE FAIL' in l: out.append(dict(unit=u,addr=l[3:11] if l.startswith('FE') else '?',sym=l,exact=False,diff=-1,img=0,ours=0,ratio=0))
    if not out: out.append(dict(unit=u,addr='?',sym='NOOUT '+(p.stderr[-200:] if p.stderr else ''),exact=False,diff=-1,img=0,ours=0,ratio=0))
    return out
# keep each prefix serial: partition by i % jobs
def lane(k): 
    r=[]
    for i in range(k,len(work),a.jobs): r+=run(i)
    return r
res=[]
with ThreadPoolExecutor(a.jobs) as ex:
    for o in ex.map(lane,range(a.jobs)): res+=o
json.dump(res,open(a.out,'w'),indent=0)
ok=[r for r in res if r['diff']>=0]
if a.base:
    # keyed by address: a marker moved to another unit is compared with its old row (same unit preferred)
    B={}
    for x in json.load(open(a.base)): B.setdefault(x['addr'],[]).append(x)
    b={}
    for x in res:
        c=B.get(x['addr'],[])
        if c and x['addr']!='?': b[(x['unit'],x['addr'])]=next((y for y in c if y['unit']==x['unit']),c[0])
    seen={x['addr'] for x in res}
    gone=[y for k,c in B.items() if k not in seen and k!='?' for y in c]
    if gone and not a.units: print('NOT MEASURED NOW',len(gone)); [print('  ?',y['addr'],y['unit'],y['sym'][:50]) for y in gone[:40]]
    lost=[x for x in res if (x['unit'],x['addr']) in b and b[(x['unit'],x['addr'])]['exact'] and not x['exact']]
    gained=[x for x in res if (x['unit'],x['addr']) in b and not b[(x['unit'],x['addr'])]['exact'] and x['exact']]
    moved=[(x,b[(x['unit'],x['addr'])]['diff']) for x in res if (x['unit'],x['addr']) in b and x['diff']!=b[(x['unit'],x['addr'])]['diff'] and not x['exact'] and not b[(x['unit'],x['addr'])]['exact']]
    print('LOST',len(lost)); [print('  -',x['addr'],x['unit'],x['sym'][:50],'diff',x['diff']) for x in lost[:40]]
    print('GAINED',len(gained)); [print('  +',x['addr'],x['unit'],x['sym'][:50]) for x in gained[:40]]
    print('MOVED',len(moved)); [print('  ~',x['addr'],x['unit'],x['sym'][:40],'%d -> %d'%(d,x['diff'])) for x,d in sorted(moved,key=lambda t:t[0]['diff']-t[1])[:40]]
print('units',len(work),'functions',len(res),'evaluated',len(ok),'exact-by-fe',sum(r['exact'] for r in ok),'problems',len(res)-len(ok))
