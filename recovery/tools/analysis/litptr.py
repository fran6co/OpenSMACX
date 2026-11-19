"""litptr.py <unit.cpp> [--write]: `char T[] = "x"; char *p = T;` pairs -> `char *p = "x";` where the image proves the literal
form: the pointer cell holds the text's address and nothing else in .text/.data references the text."""
import sys,re,struct,bisect
sys.path.insert(0,'recovery/tools'); sys.path.insert(0,'recovery/tools/analysis')
import fe,tumap
unit=sys.argv[1]; write='--write' in sys.argv
t=open(unit,encoding='latin-1').read()
img=fe._img
tva,tvs,tro=[s for s in fe._secs if s[0]==0x401000][0]
text=img[tro:tro+tvs]
dlo,dhi=0x682000,0x6A7090
data=fe.imgbytes(dlo,dhi)
fs=tumap.load(); los=[f[0] for f in fs]
def refs(v):
    b=struct.pack('<I',v); c=[];d=[]
    i=text.find(b)
    while i>=0: c.append(tva+i); i=text.find(b,i+1)
    i=data.find(b)
    while i>=0:
        if i%4==0: d.append(dlo+i)
        i=data.find(b,i+1)
    return c,d
def fn_of(a):
    i=bisect.bisect_right(los,a)-1
    return fs[i][0] if i>=0 and fs[i][0]<=a<fs[i][1] else None
OBJ=r'// RECOVERY_OBJECT: v\d (0x[0-9A-F]{8})(?: (0x[0-9A-F]{8}))? (\S+)(?: (0x[0-9A-F]{8}))?[^\n]*\n'
# text definition: optional binding line directly above
pat=re.compile(r'((?:'+OBJ+r')?)((?:static )?char (\w+)\[\d*\] = ("(?:[^"\\]|\\.)*");[^\n]*\n)')
n=0
for m in list(pat.finditer(t)):
    name=m.group(7); lit=m.group(8)
    uses=[x for x in re.finditer(r'(?<![?\w])'+name+r'\b',t)]
    pm=re.search(r'()('+OBJ+r')((?:char \*|LPCSTR )(\w+) = '+name+r';)',t)
    if not pm: continue
    if len(uses)!=2: print('SKIP',name,'used',len(uses)); continue
    paddr=int(pm.group(3),16); wit=pm.group(6)
    cell=struct.unpack('<I',fe.imgbytes(paddr,paddr+4))[0]
    s=eval('b'+lit)
    if fe.imgbytes(cell,cell+len(s)+1)!=s+b'\0': print('SKIP',name,'image text differs at %08X'%cell); continue
    if m.group(2) and int(m.group(2),16)!=cell: print('SKIP',name,'binding',m.group(2),'cell %08X'%cell); continue
    c,d=refs(cell)
    if c or d!=[paddr]: print('SKIP',name,'other references',[hex(x) for x in c],[hex(x) for x in d]); continue
    if not wit:
        pc,pd=refs(paddr); f=[fn_of(a) for a in pc]; f=[x for x in f if x]
        if not f: print('SKIP',name,'no reader of the pointer'); continue
        wit='0x%08X'%f[0]
    print('CONVERT %-28s %08X -> %08X %s evidence %s'%(pm.group(8),paddr,cell,lit,wit))
    n+=1
    newp=pm.group(2)+'// RECOVERY_LITERAL: v1 0x%08X 0x%08X $SG%d %s\n'%(cell,cell+len(s)+1,90000+n,wit)+pm.group(7).replace('= '+name+';','= '+lit+';')
    t=t.replace(pm.group(2)+pm.group(7),newp,1)
    t=t.replace(m.group(0),'',1)
print(n,'converted')
if write: open(unit,'w',encoding='latin-1').write(t)
