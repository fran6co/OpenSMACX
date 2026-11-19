#!/usr/bin/env python3
"""ctxscan.py [--pfx N] [--max-run K]
Emitting-unit screen for stray markers (MATCHING_GUIDE 2.5): a marker measured in unit U whose image span
lies in a short run (<= K functions) between the runs of other units is usually the out-of-line COMDAT copy
of a header inline, kept from the first unit in link order that emits it; that copy trails the emitting
unit's own functions. For each such marker this compiles the preceding and the following large-run unit and
reports whether they emit the symbol and whether fe reads it exact there:  `-> compile_context <unit>`."""
import sys,os,re,argparse,collections
sys.path.insert(0,'recovery/tools'); sys.path.insert(0,'recovery/tools/analysis')
import fe,tumap
ap=argparse.ArgumentParser(); ap.add_argument('--pfx',default='6'); ap.add_argument('--max-run',type=int,default=2); ap.add_argument('--lo',default='0'); ap.add_argument('--hi',default='ffffffff')
a=ap.parse_args()
mk=tumap.load()  # lo,hi,unit,name,exact,path
runs=[]
for m in mk:
    if runs and runs[-1][0]==m[2]: runs[-1][1].append(m)
    else: runs.append((m[2],[m]))
comp={}
def compiled(u):
    if u not in comp:
        comp[u]=fe.compile_unit(u,fe.R+'/'+u,a.pfx)[0] if fe.compile_cmd(u) else None
    return comp[u]
texts={}
def text(p):
    if p not in texts: texts[p]=open(fe.R+'/'+p,encoding='latin-1').read()
    return texts[p]
lo_,hi_=int(a.lo,16),int(a.hi,16)
for i,(u,ms) in enumerate(runs):
    if len(ms)>a.max_run or not (lo_<=ms[0][0]<hi_): continue
    prev=next((runs[j][0] for j in range(i-1,-1,-1) if len(runs[j][1])>a.max_run),None)
    nxt=next((runs[j][0] for j in range(i+1,len(runs)) if len(runs[j][1])>a.max_run),None)
    for lo,hi,unit,name,exact,path in ms:
        out=[]
        for cand in dict.fromkeys([unit,prev,nxt]):
            if not cand: continue
            c=compiled(cand)
            if c is None: out.append('%s:nocompile'%cand); continue
            r=fe.evaluate(c,text(path),lo)
            if 'error' in r: out.append('%s:%s'%(os.path.basename(cand),'nosym'))
            else: out.append('%s:%s'%(os.path.basename(cand),'EXACT' if r['exact'] else 'diff%d'%r['diff']))
        print('%08X %-34s now=%-26s file=%-24s %s'%(lo,name[:34],os.path.basename(unit),os.path.basename(path),'  '.join(out[1:]) + '   [now %s]'%out[0].split(':')[1]))
        sys.stdout.flush()
