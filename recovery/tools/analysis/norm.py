import sys,re,collections
# classify '!!' rows of an fe --all diff: slot-only (same after masking frame operands), reg-only, other
f=sys.argv[1]
R=r'\b(e[abcd]x|e[sd]i|[abcd][lh]|[abcd]x|[sd]i)\b'
k=collections.Counter(); reg=[]
for l in open(f):
    if '|' not in l or not l.startswith('!!'): continue
    a,b=l.split('|',1)
    a=a[9:].strip(); b=b[6:].strip()
    if not a or not b: k['unpaired']+=1; continue
    ma=re.sub(r'ebp [-+] (0x[0-9a-f]+|\d+)','S',a); mb=re.sub(r'ebp [-+] (0x[0-9a-f]+|\d+)','S',b)
    if ma==mb: k['slot-only']+=1; continue
    ra=re.sub(R,'R',ma); rb=re.sub(R,'R',mb)
    if ra==rb: k['reg(+slot)']+=1; continue
    k['other-paired']+=1
print(f.split('/')[-1],dict(k))
