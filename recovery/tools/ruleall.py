import sys,os,re,json,subprocess
from concurrent.futures import ThreadPoolExecutor
R=os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))); S=os.path.dirname(os.path.abspath(__file__))
rule=sys.argv[1]; extra=sys.argv[2:]
cc=json.load(open(R+'/build/compile_commands.json'))
units=[e['file'][len(R)+1:] for e in cc if e['file'].startswith(R+'/src/')]
work=[]
for u in units:
    try: t=open(R+'/'+u,encoding='latin-1').read()
    except Exception: continue
    if any('BYTE_EXACT' not in m.group(1) for m in re.finditer(r'// ORIGINAL: 0x[0-9A-F]{8} \S+ 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)',t)): work.append(u)
def lane(k):
    out=[]
    for i in range(k,len(work),14):
        p=subprocess.run(['uv','run','python',S+'/rules.py',rule,work[i],str(21+k)]+extra,cwd=R,capture_output=True,text=True)
        out.append(p.stdout.strip().split('\n')[-1] if p.stdout.strip() else json.dumps(dict(unit=work[i],tried=-1,accepted=[],total=-1,err=p.stderr[-300:])))
    return out
res=[]
with ThreadPoolExecutor(14) as ex:
    for o in ex.map(lane,range(14)): res+=o
tried=0
for l in res:
    try: j=json.loads(l)
    except Exception: print(l[:200]); continue
    tried+=max(0,j['tried'])
    if j.get('err'): print(j['unit'],'ERR',j['err'][-150:])
    if j['accepted']: print(j['unit'],'tried',j['tried']); [print('   ',a) for a in j['accepted']]
print('units',len(res),'sites tried',tried)
