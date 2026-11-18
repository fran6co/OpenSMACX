#!/usr/bin/env python3
"""crtcalls.py OUT.json : per function, the multiset of calls to the CRT block/string routines
(memcpy memmove memset strlen strcpy strcat strcmp memcmp), ours (call relocations) against the image
(call targets). Prints every function where they differ. The game and Jackal have no intrinsics, so
a difference is a wrong spelling (memset where the image has a loop, a missing strcat, ...).
Uses prefixes FE_PFX_BASE..+4."""
import sys,os,json,re,struct,collections
S=os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0,S); import fe
R=fe.R; PB=int(os.environ.get('FE_PFX_BASE','21'))
from concurrent.futures import ThreadPoolExecutor
CRT={0x645930:'_memcpy',0x6474e0:'_memmove',0x6465f0:'_memset',0x6453e0:'_strlen',0x645460:'_strcpy',0x645470:'_strcat',0x645660:'_strcmp',0x645c70:'_memcmp'}
NAMES=set(CRT.values())
cc=json.load(open(fe.MAIN+'/build/compile_commands.json'))
units=[e['file'][len(fe.MAIN)+1:] for e in cc if e['file'].startswith(fe.MAIN+'/src/') and e['file'].endswith('.cpp')]
fe.compile_cmd('x')
def imgcalls(lo,hi):
    c=collections.Counter(); b=fe.imgbytes(lo,hi+1)
    for i in fe.md.disasm(b,lo):
        if i.mnemonic=='call' and i.op_str.startswith('0x') and int(i.op_str,16) in CRT: c[CRT[int(i.op_str,16)]]+=1
    return c
def one(k,u):
    t=open(R+'/'+u,encoding='latin-1').read()
    c,err=fe.compile_unit(u,R+'/'+u,PB+k)
    if c is None: return [(u,'FAIL',err[:200])]
    out=[]
    for m in re.finditer(r'// ORIGINAL: (0x[0-9A-F]{8}) ',t):
        addr=int(m.group(1),16); idn=fe.ident(t,addr)
        if not idn: continue
        sym,lo,hi=idn
        try: s=c.function(sym)
        except Exception: continue
        sec=c.sections[s.section-1]
        oc=collections.Counter()
        for r in sec.relocations:
            n=c.symbols[r[1]].name
            if n in NAMES and r[2]==0x14: oc[n]+=1
        ic=imgcalls(lo,hi)
        if oc!=ic: out.append((u,m.group(1),sym,dict(ic),dict(oc)))
    return out
def lane(k): 
    r=[]
    for j,u in enumerate(units):
        if j%5==k: r+=one(k,u)
    return r
res=[]
with ThreadPoolExecutor(5) as ex:
    for l in ex.map(lane,range(5)): res+=l
json.dump(res,open(sys.argv[1],'w'),indent=0)
for r in sorted(res): print(r[0],r[1],str(r[2])[:44],'img',r[3],'ours',r[4]) if r[1]!='FAIL' else print(r)
