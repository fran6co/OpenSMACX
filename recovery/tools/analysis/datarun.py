#!/usr/bin/env python3
"""datarun.py [--all] [--lo A --hi B]
Data-order evidence for unit membership: a unit's .data and .bss contributions are contiguous and the units
follow link order (the .text order of their main runs). Every bound object (RECOVERY_OBJECT) is ranked by its
owner file's link position; within one section the ranks must not fall. Objects outside the longest
non-decreasing subsequence are definitions in the wrong unit; the neighbours on both sides bound the unit
that really defines them."""
import sys,os,re,struct,bisect,argparse
HERE=os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0,'recovery/tools'); sys.path.insert(0,'recovery/tools/analysis')
import fe,tumap
ap=argparse.ArgumentParser(); ap.add_argument('--all',action='store_true'); ap.add_argument('--lo',default='0'); ap.add_argument('--hi',default='ffffffff')
a=ap.parse_args()
runs={}
for lo,hi,unit,name,exact,path in tumap.load():
    runs.setdefault(unit,[]).append((lo,hi))
# main run: merge adjacent spans per unit, largest
main={}
order=sorted((lo,hi,u) for u,r in runs.items() for lo,hi in r)
cur=None
for lo,hi,u in order:
    if cur and cur[2]==u: cur=(cur[0],hi,u)
    else:
        if cur and cur[1]-cur[0]>main.get(cur[2],(0,0))[0]: main[cur[2]]=(cur[1]-cur[0],cur[0])
        cur=(lo,hi,u)
if cur and cur[1]-cur[0]>main.get(cur[2],(0,0))[0]: main[cur[2]]=(cur[1]-cur[0],cur[0])
rank={u:v[1] for u,v in main.items()}
objs=[]
for root,_,fs in os.walk('src'):
    for f in fs:
        if not f.endswith('.cpp'): continue
        p=os.path.join(root,f)
        for l in open(p,encoding='latin-1'):
            m=re.match(r'\s*// RECOVERY_OBJECT: v\d (0x[0-9A-F]{8}) (?:(0x[0-9A-F]{8}) )?(\S+)',l)
            if m and p in rank: objs.append((int(m.group(1),16),p,m.group(3)))
objs.sort()
import struct as S_
img=fe._img; pe=fe._pe; nsec=S_.unpack_from('<H',img,pe+6)[0]; opt=S_.unpack_from('<H',img,pe+20)[0]; base=S_.unpack_from('<I',img,pe+52)[0]
secs=[]
for i in range(nsec):
    o=pe+24+opt+40*i; nm=img[o:o+8].rstrip(b'\0').decode(); vs,va,rs,ro=S_.unpack_from('<IIII',img,o+8)
    secs.append((nm,base+va,base+va+min(vs,rs) if rs else base+va,base+va+vs))
parts=[]
for nm,va,mid,end in secs:
    if nm in ('.rdata','.data'):
        parts.append((nm+' init',va,mid)); 
        if end>mid: parts.append((nm+' bss',mid,end))
lo_,hi_=int(a.lo,16),int(a.hi,16)
for nm,p0,p1 in parts:
    xs=[o for o in objs if p0<=o[0]<p1]
    if not xs: continue
    r=[rank[o[1]] for o in xs]
    # LIS non-decreasing
    tails=[]; idx=[]; prev=[-1]*len(r)
    for i,v in enumerate(r):
        k=bisect.bisect_right(tails,v)
        if k==len(tails): tails.append(v); idx.append(i)
        else: tails[k]=v; idx[k]=i
        prev[i]=idx[k-1] if k else -1
    keep=set(); i=idx[-1]
    while i>=0: keep.add(i); i=prev[i]
    print('== %s %08X-%08X objects=%d in-order=%d out-of-order=%d'%(nm,p0,p1,len(xs),len(keep),len(xs)-len(keep)))
    ks=sorted(keep)
    for i,o in enumerate(xs):
        if i in keep and not a.all: continue
        if not (lo_<=o[0]<hi_): continue
        j=bisect.bisect_left(ks,i)
        before=xs[ks[j-1]] if j>0 else None; after=xs[ks[j]] if j<len(ks) else None
        if i in keep: print('   %08X %-30s %s'%(o[0],o[1][4:],o[2][:50]))
        else: print(' ! %08X %-28s %-44s between %s .. %s'%(o[0],o[1][4:],o[2][:44],before[1][4:] if before else '-',after[1][4:] if after else '-'))
