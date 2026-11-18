import sys,os,re,json,subprocess
from concurrent.futures import ThreadPoolExecutor
R=os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))); S=os.path.dirname(os.path.abspath(__file__))
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
        p=subprocess.run(['uv','run','python',S+'/accsub.py',work[i],str(21+k)]+sys.argv[1:],cwd=R,capture_output=True,text=True)
        out.append(p.stdout.strip().split('\n')[-1] if p.stdout.strip() else work[i]+' NOOUT '+p.stderr[-200:])
    return out
res=[]
with ThreadPoolExecutor(14) as ex:
    for o in ex.map(lane,range(14)): res+=o
for l in res:
    try: j=json.loads(l)
    except Exception: print(l[:200]); continue
    if j['accepted']: print(j['unit'],'tried',j['tried']); [print('   ',a) for a in j['accepted']]
print('units',len(res))
