import sys,os,re,json,subprocess
from concurrent.futures import ThreadPoolExecutor
R=os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))); S=os.path.dirname(os.path.abspath(__file__))
rules_=sys.argv[1].split(','); extra=sys.argv[2:]
PFX=[i for i in range(0,39) if i not in (1,20)] if os.environ.get('RULE_WIDE')=='1' else list(range(21,37))
N=len(PFX)
cc=json.load(open(R+'/build/compile_commands.json'))
units=[e['file'][len(R)+1:] for e in cc if e['file'].startswith(R+'/src/')]
work=[]
for u in units:
    try: t=open(R+'/'+u,encoding='latin-1').read()
    except Exception: continue
    if any('BYTE_EXACT' not in m.group(1) for m in re.finditer(r'// ORIGINAL: 0x[0-9A-F]{8} \S+ 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)',t)): work.append(u)
work.sort(key=lambda u:-os.path.getsize(R+'/'+u))
def lane(k):
    out=[]
    for i in range(k,len(work),N):
        for rule in rules_:
            p=subprocess.run(['uv','run','python',S+'/rules.py',rule,work[i],str(PFX[k])]+extra,cwd=R,capture_output=True,text=True)
            out.append(p.stdout.strip().split('\n')[-1] if p.stdout.strip() else json.dumps(dict(unit=work[i],tried=-1,accepted=[],total=-1,err=p.stderr[-300:])))
    return out
res=[]
with ThreadPoolExecutor(N) as ex:
    for o in ex.map(lane,range(N)): res+=o
tried=0
for l in res:
    try: j=json.loads(l)
    except Exception: print(l[:200]); continue
    tried+=max(0,j['tried'])
    if j.get('err'): print(j['unit'],'ERR',j['err'][-150:])
    if j.get('exact_lost'): print(j['unit'],'EXACT_LOST',j['exact_lost'])
    if j['accepted']: print(j['unit'],'tried',j['tried']); [print('   ',a) for a in j['accepted']]
print('units',len(res),'sites tried',tried)
