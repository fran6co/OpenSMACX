#!/usr/bin/env python3
"""runs.py DIFF.txt [minlen=8]: runs of unpaired rows in a saved fe.py --all diff, image side and ours side
(start/end address, row index).  A long image-only run is code we lack (often a called-vs-expanded helper);
a long ours-only run is code we have extra."""
import sys,re
mn=int(sys.argv[2]) if len(sys.argv)>2 else 8
rows=[l.rstrip('\n') for l in open(sys.argv[1]) if '|' in l and l[:2] in ('!!','!>','  ')]
def runs(side):
    cur=[]
    for i,l in enumerate(rows):
        a,b=l.split('|',1); ia=a[3:9].strip(); ib=b.strip().split(' ')[0] if b.strip() else ''
        only = (side=='img' and ia and not ib) or (side=='ours' and ib and not ia)
        if l.startswith('!!') and only: cur.append((i,ia if side=='img' else ib))
        else:
            if len(cur)>=mn: yield cur
            cur=[]
    if len(cur)>=mn: yield cur
for side in ('img','ours'):
    for r in runs(side): print(side,len(r),r[0][1],r[-1][1],'row',r[0][0])
