#!/usr/bin/env python3
"""callseq.py <unit> <addr> : align the image's and our call sequences (by symbol) and print the differences with image addresses / our offsets."""
import sys,os,re,glob,difflib
sys.path.insert(0,os.getcwd()+'/recovery/tools'); import fe
unit,addr=sys.argv[1],int(sys.argv[2],16)
R=fe.R
addr2sym={}
for p in glob.glob(R+'/src/**/*.cpp',recursive=True):
    t=open(p,encoding='latin-1').read()
    for m in re.finditer(r'// ORIGINAL: 0x([0-9A-F]{8}) (\S+) 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+[^\n]*\n((?://[^\n]*\n)*)',t):
        sym=m.group(2); m2=re.search(r'// symbol\s+(\S+)',m.group(3))
        addr2sym[int(m.group(1),16)]=m2.group(1) if m2 else sym
CRT={'sub_645470':'_strcat','sub_645460':'_strcpy','sub_64601d':'_rand','sub_6453e0':'_strlen','sub_644f3a':'_abs','sub_64fc88':'__itoa','sub_644ef2':'_free','sub_644dff':'_exit','sub_645660':'_strcmp','sub_645550':'__chkstk','sub_64558a':'??2@YAPAXI@Z','sub_6458c8':'__ftol','sub_647851':'_floor','sub_645da7':'_atoi','sub_645dd0':'_strchr','sub_6468d0':'_strstr','sub_645e90':'_strncpy','sub_6465f0':'_memset','sub_645930':'_memcpy'}
t=open(R+'/'+unit,encoding='latin-1').read()
SRC=sys.argv[4] if len(sys.argv)>4 else R+'/'+unit; t=open(SRC,encoding='latin-1').read(); c,err=fe.compile_unit(unit,SRC,sys.argv[3] if len(sys.argv)>3 else '9')
sym,lo,hi=fe.ident(t,addr)
s=c.function(sym); sec=c.sections[s.section-1]
rm={r[0]:r for r in sec.relocations}
ours=[]
for ins in fe.md.disasm(sec.data,0):
    if ins.mnemonic=='call' and ins.size==5 and (ins.address+1) in rm:
        sy=c.symbols.get(rm[ins.address+1][1]); ours.append((ins.address,sy.name if sy else '?'))
img=[]
for ins in fe.md.disasm(fe.imgbytes(lo,hi),lo):
    if ins.mnemonic=='call' and ins.size==5:
        try: tgt=int(ins.op_str,16)
        except ValueError: continue
        n=addr2sym.get(tgt,'sub_%x'%tgt); img.append((ins.address,CRT.get(n,n)))
A=[x[1] for x in img]; B=[x[1] for x in ours]
for tag,i1,i2,j1,j2 in difflib.SequenceMatcher(None,A,B,autojunk=False).get_opcodes():
    if tag=='equal': continue
    print(tag,'IMG',[('%x'%a,n[:28]) for a,n in img[i1:i2]],'OURS',[('%x'%a,n[:28]) for a,n in ours[j1:j2]])
