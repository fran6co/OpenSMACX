import sys,re,collections
# slot consistency of an fe --all diff: rows with one frame operand on both sides
def score(f):
    c=collections.Counter(); n=0; eq=0; rows=0; bang=0
    for l in open(f):
        if '|' not in l: continue
        rows+=1
        if l.startswith('!!'): bang+=1
        a,b=l.split('|',1)
        sa=re.findall(r'ebp [-+] 0x[0-9a-f]+|ebp [-+] \d+',a); sb=re.findall(r'ebp [-+] 0x[0-9a-f]+|ebp [-+] \d+',b)
        if len(sa)==1 and len(sb)==1:
            ma=re.sub(r'ebp [-+] (0x[0-9a-f]+|\d+)','S',a[9:].strip()); mb=re.sub(r'ebp [-+] (0x[0-9a-f]+|\d+)','S',b[6:].strip())
            if ma!=mb: continue
            c[(sa[0],sb[0])]+=1; n+=1
    byo=collections.defaultdict(collections.Counter); byi=collections.defaultdict(collections.Counter)
    for (i,o),v in c.items(): byo[o][i]+=v; byi[i][o]+=v
    good=sum(max(x.values()) for x in byo.values())
    good2=sum(max(x.values()) for x in byi.values())
    return bang,n,n-good,n-good2
for f in sys.argv[1:]:
    print(f.split('/')[-1],'bang %d sloterows %d off-by-ours %d off-by-img %d'%score(f))
