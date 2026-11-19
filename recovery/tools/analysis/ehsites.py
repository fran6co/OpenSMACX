# eh.py unit addr [--src ALT] [--pfx N]: FuncInfo unwind toState tuples image vs ours, and per-site depth runs
import sys,os,struct,re,argparse
R='/home/fran6co/code/OpenSMACX/.opensmacx/worktrees/par-3'
sys.path.insert(0,R+'/recovery/tools'); import fe
ap=argparse.ArgumentParser(); ap.add_argument('unit'); ap.add_argument('addr'); ap.add_argument('--src'); ap.add_argument('--pfx',default='14'); a=ap.parse_args()
text=open(a.src or R+'/'+a.unit,encoding='latin-1').read()
sym,lo,hi=fe.ident(text,int(a.addr,16))
def rd(va,n): return fe.imgbytes(va,va+n)
def img():
    ins=list(fe.md.disasm(rd(lo,32),lo))
    h=None
    for x in ins[:6]:
        if x.mnemonic=='push' and x.op_str.startswith('0x'): h=int(x.op_str,16)
    if h is None: return None
    b=rd(h,5); assert b[0]==0xB8
    fi=struct.unpack('<I',b[1:])[0]
    _,ms,pu=struct.unpack('<III',rd(fi,12))
    return [struct.unpack('<i',rd(pu+8*i,4))[0] for i in range(ms)]
def ours():
    c,err=fe.compile_unit(a.unit,a.src or R+'/'+a.unit,a.pfx)
    if c is None: print(err); return None
    s=c.function(sym); sec=c.sections[s.section-1]
    # find relocation in first 16 bytes -> handler symbol
    hs=None
    for x in fe.md.disasm(bytes(sec.data[:32]),0):
        if x.mnemonic=='push' and x.size==5:
            for off,si,ty in sec.relocations:
                if off==x.address+1: hs=c.symbols[si]
            break
    if hs is None: return None
    hsec=c.sections[hs.section-1]
    for off,si,ty in hsec.relocations:
        if off==hs.value+1: fis=c.symbols[si]; break
    fsec=c.sections[fis.section-1]; d=fsec.data
    ms=struct.unpack_from('<I',d,fis.value+4)[0]
    # unwind map pointer reloc at fis.value+8
    us=None
    for off,si,ty in fsec.relocations:
        if off==fis.value+8: us=c.symbols[si]; break
    if us is None: print('dbg',fis.name,fis.value,[(o,c.symbols[s].name) for o,s,t in fsec.relocations][:6]); return None
    usec=c.sections[us.section-1]
    return [struct.unpack_from('<i',usec.data,us.value+8*i)[0] for i in range(ms)]
def runs(p):
    s=[]
    for v in p:
        if v==-1: s.append(1)
        elif s: s[-1]+=1
        else: s.append(1)
    return s
I=img(); O=ours()
print('IMG ',len(I),I); print('OURS',len(O),O)
print('IMG  runs',runs(I)); print('OURS runs',runs(O))
