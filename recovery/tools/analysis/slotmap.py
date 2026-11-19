#!/usr/bin/env python3
"""slotmap.py DIFF.txt: from a saved fe.py --all diff, which of our [ebp+-N] slots pair with which image slot
(image -> ours with counts, then ours slots that pair with several image slots: packed/shared locals)."""
import sys,re,collections
pairs=collections.defaultdict(collections.Counter); rev=collections.defaultdict(collections.Counter)
S=r'ebp ([-+]) (0x[0-9a-f]+|\d+)\]'
for l in open(sys.argv[1]):
    if '|' not in l: continue
    a,b=l.split('|',1)
    ma=re.findall(S,a); mb=re.findall(S,b)
    if len(ma)==1 and len(mb)==1:
        x=int(ma[0][1],0)*(1 if ma[0][0]=='+' else -1); y=int(mb[0][1],0)*(1 if mb[0][0]=='+' else -1)
        pairs[x][y]+=1; rev[y][x]+=1
for x in sorted(pairs):
    print('%6s'%hex(x),' '.join('%s:%d'%(hex(y),n) for y,n in pairs[x].most_common()))
print('--- ours -> img')
for y in sorted(rev):
    if len(rev[y])>1: print('%6s'%hex(y),' '.join('%s:%d'%(hex(x),n) for x,n in rev[y].most_common()))
