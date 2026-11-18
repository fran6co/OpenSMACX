#!/usr/bin/env python3
"""fescan.py : dump diff rows of every non-exact function to scratchpad/fescan.json (one compile per unit)."""
import sys,os,re,json,subprocess
from concurrent.futures import ThreadPoolExecutor
R=os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))); S=os.path.dirname(os.path.abspath(__file__))
WORKER=r'''
import sys,json
sys.path.insert(0,%r); import fe
unit=sys.argv[1]; addrs=[int(x,16) for x in sys.argv[2].split(',')]; pfx=sys.argv[3]
c,err=fe.compile_unit(unit,fe.R+'/'+unit,pfx)
out=[]
if c is not None:
    text=open(fe.R+'/'+unit,encoding='latin-1').read()
    for a in addrs:
        r=fe.evaluate(c,text,a)
        if 'error' in r: continue
        rows=[(k,(o[0]+r['lo'],o[1]) if o else None,(c_[0],c_[1]) if c_ else None) for k,o,c_ in r['rows']]
        out.append(dict(unit=unit,addr='%%08X'%%a,sym=r['sym'],diff=r['diff'],img=r['img'],ours=r['ours'],rows=rows))
print(json.dumps(out))
''' % S
cc=json.load(open(R+'/build/compile_commands.json'))
units=[e['file'][len(R)+1:] for e in cc if e['file'].startswith(R+'/src/')]
work=[]
for u in units:
    try: t=open(R+'/'+u,encoding='latin-1').read()
    except Exception: continue
    addrs=[m.group(1) for m in re.finditer(r'// ORIGINAL: (0x[0-9A-F]{8}) \S+ 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)',t) if 'BYTE_EXACT' not in m.group(2)]
    if addrs: work.append((u,addrs))
def lane(k):
    res=[]
    for i in range(k,len(work),14):
        u,addrs=work[i]
        p=subprocess.run(['uv','run','python','-c',WORKER,u,','.join(addrs),str(21+k)],cwd=R,capture_output=True,text=True)
        try: res+=json.loads(p.stdout.strip().split('\n')[-1])
        except Exception: pass
    return res
out=[]
with ThreadPoolExecutor(14) as ex:
    for o in ex.map(lane,range(14)): out+=o
json.dump(out,open(S+'/fescan.json','w'))
print('functions',len(out))
