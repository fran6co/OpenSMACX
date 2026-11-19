"""cdeck.py: call-multiset deck runner.

cdeck.run(unit, addr, [(key, whole_file_text), ...], focus=('on_map',)) compiles each variant on
prefixes FE_PFX_BASE.. (FE_JOBS parallel) and prints per variant: fe row diff, exact, the callee
counts that differ from the image ({callee: (image, ours)}), our offsets of calls whose name
contains a focus string, and our frame size (F0x..). Judge /Ob2 experiments by the call multiset
first (MATCHING_GUIDE 2.8), then frame, then rows.  Screen only.
"""
import sys,os,re,glob,collections
from concurrent.futures import ThreadPoolExecutor
T=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0,T); import fe
R=fe.R
addr2sym={}
for p in glob.glob(R+'/src/**/*.cpp',recursive=True):
    t=open(p,encoding='latin-1').read()
    for m in re.finditer(r'// ORIGINAL: 0x([0-9A-F]{8}) (\S+) 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+[^\n]*\n((?://[^\n]*\n)*)',t):
        sym=m.group(2); m2=re.search(r'// symbol\s+(\S+)',m.group(3))
        addr2sym[int(m.group(1),16)]=m2.group(1) if m2 else sym
CRT={'sub_645470':'_strcat','sub_645460':'_strcpy','sub_64601d':'_rand','sub_6453e0':'_strlen','sub_644f3a':'_abs','sub_64fc88':'__itoa','sub_644ef2':'_free','sub_644dff':'_exit','sub_645660':'_strcmp','sub_645550':'__chkstk','sub_64558a':'??2@YAPAXI@Z','sub_6458c8':'__ftol','sub_647851':'_floor','sub_645da7':'_atoi','sub_645dd0':'_strchr','sub_6468d0':'_strstr','sub_645e90':'_strncpy','sub_6465f0':'_memset','sub_645930':'_memcpy','sub_645398':'_atexit','sub_6457c2':'??_L@YGXPAXIHP6EX0@Z1@Z','sub_6456e4':'??_M@YGXPAXIHP6EX0@Z@Z'}
def frame(data):
    import re as _r
    for ins in fe.md.disasm(data[:40],0):
        if ins.mnemonic=='sub' and ins.op_str.startswith('esp'): return ins.op_str.split(',')[1].strip()
    return '?'
def calls_img(lo,hi):
    out=[]
    for ins in fe.md.disasm(fe.imgbytes(lo,hi),lo):
        if ins.mnemonic=='call' and ins.size==5:
            try: t=int(ins.op_str,16)
            except ValueError: continue
            out.append((ins.address,addr2sym.get(t,'sub_%x'%t)))
    return out
def calls_ours(c,sym):
    s=c.function(sym); sec=c.sections[s.section-1]; rm={r[0]:r for r in sec.relocations}; out=[]
    for ins in fe.md.disasm(sec.data,0):
        if ins.mnemonic=='call' and ins.size==5 and (ins.address+1) in rm:
            sy=c.symbols.get(rm[ins.address+1][1]); out.append((ins.address,sy.name if sy else '?'))
    return out
def run(unit,addr,deck,focus=(),base=int(os.environ.get('FE_PFX_BASE','2')),jobs=int(os.environ.get('FE_JOBS','5'))):
    addr=int(addr,16) if isinstance(addr,str) else addr
    fe.compile_cmd(unit)
    def one(k):
        res=[]
        for i in range(k,len(deck),jobs):
            key,text=deck[i]; path='%s/build/recovery/fe/cd%d.cpp'%(R,k); open(path,'w',encoding='latin-1').write(text)
            c,err=fe.compile_unit(unit,path,str(base+k))
            if c is None: res.append((key,None,err)); continue
            r=fe.evaluate(c,text,addr); sym,lo,hi=fe.ident(text,addr)
            A=collections.Counter(n for a,n in calls_img(lo,hi)); B=collections.Counter(n for a,n in calls_ours(c,r['sym']))
            A=collections.Counter({CRT.get(k_,k_):v for k_,v in A.items()}); d={k_:(A[k_],B[k_]) for k_ in set(A)|set(B) if A[k_]!=B[k_]}
            pos=[ '%x:%s'%(a,n.lstrip('?').split('@')[0]) for a,n in calls_ours(c,r['sym']) if any(f in n for f in focus)]
            res.append((key,r,d,pos+['F'+frame(c.sections[c.function(r['sym']).section-1].data)]))
        return res
    fe.compile_cmd(unit); out=[]
    with ThreadPoolExecutor(jobs) as ex:
        for o in ex.map(one,range(jobs)): out+=o
    for x in out:
        if x[1] is None: print(x[0],'FAIL',x[2][:200]); continue
        key,r,d,pos=x; print('%-28s diff=%d exact=%s calls=%s %s'%(key,r['diff'],r['exact'],d,' '.join(pos)))
    return out
