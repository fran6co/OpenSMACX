# symbolic executor for caviar_build_projection_tables: image and ours
import sys,os,re,struct
W='/home/fran6co/code/OpenSMACX/.opensmacx/worktrees/par-5'
sys.path.insert(0,W+'/recovery/tools'); import fe
BASIS={0x9C0A40:'X0',0x9C0A42:'X1',0x9C0A44:'X2',0x9C0A46:'Y0',0x9C0A48:'Y1',0x9C0A4A:'Y2',0x9C0A4C:'Z0',0x9C0A50:'Z1',0x9C0A54:'Z2'}
def cellname(a):
    off=a-0x9C0840
    if 0<=off<26*16 and off%16 in (0,4,8): return 'c%d.%s'%(off//16,'xyz'[off%16//4])
    return None
def run(code,org,absaddr):
    """absaddr(ins_addr, disp)->absolute address for memory operand"""
    reg={}; slot={}; esp=0; out=[]; events=[]
    import capstone
    md=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32); md.detail=True
    from capstone import x86 as X
    for ins in md.disasm(code,org):
        m=ins.mnemonic; ops=ins.operands
        def rn(o): return ins.reg_name(o.reg)
        def mem(o):
            b=ins.reg_name(o.mem.base) if o.mem.base else None; i=ins.reg_name(o.mem.index) if o.mem.index else None
            return b,i,o.mem.disp
        if m.startswith('f'):
            b,i,d=mem(ops[0]) if ops and ops[0].type==X.X86_OP_MEM else (None,None,0)
            a=absaddr(ins,d) if b is None else 0
            out.append((m, cellname(a) or hex(a))); continue
        if m=='sub' and ops[0].type==X.X86_OP_REG and rn(ops[0])=='esp': esp-=ops[1].imm; continue
        if m=='push': esp-=4; continue
        if m=='pop': esp+=4; continue
        if m in('mov','movsx') and ops[0].type==X.X86_OP_REG and ops[1].type==X.X86_OP_MEM:
            b,i,d=mem(ops[1])
            if b=='esp': reg[rn(ops[0])]=slot.get(esp+d,'?slot%x'%(esp+d)); events.append(('ld',esp+d)); continue
            if b is None and i is None:
                a=absaddr(ins,d)
                if a in BASIS: reg[rn(ops[0])]=BASIS[a]; continue
                c=cellname(a)
                if c: reg[rn(ops[0])]='['+c+']'; continue
            print('STOPa %x %s %s'%(ins.address,m,ins.op_str)); break
        if m=='mov' and ops[0].type==X.X86_OP_MEM and ops[1].type==X.X86_OP_REG:
            b,i,d=mem(ops[0])
            if b=='esp': slot[esp+d]=reg.get(rn(ops[1]),'?'); events.append(('st',esp+d,reg.get(rn(ops[1])))); continue
            if b is None:
                a=absaddr(ins,d); c=cellname(a)
                if c: out.append((c,reg.get(rn(ops[1]),'?'))); continue
            print('STOPb %x %s %s'%(ins.address,m,ins.op_str)); break
        if m=='mov' and ops[0].type==X.X86_OP_REG and ops[1].type==X.X86_OP_REG: reg[rn(ops[0])]=reg.get(rn(ops[1]),'?'); continue
        if m=='lea':
            b,i,d=mem(ops[1]); reg[rn(ops[0])]='(%s+%s)'%(reg.get(b,'?'),reg.get(i,'?')); continue
        if m in('add','sub') and ops[1].type==X.X86_OP_REG:
            reg[rn(ops[0])]='(%s%s%s)'%(reg.get(rn(ops[0]),'?'),'+' if m=='add' else '-',reg.get(rn(ops[1]),'?')); continue
        if m in('add','sub') and ops[1].type==X.X86_OP_MEM:
            b,i,d=mem(ops[1])
            if b=='esp': v=slot.get(esp+d,'?')
            else:
                a=absaddr(ins,d); v=BASIS.get(a,'?m')
            reg[rn(ops[0])]='(%s%s%s)'%(reg.get(rn(ops[0]),'?'),'+' if m=='add' else '-',v); continue
        if m=='neg': reg[rn(ops[0])]='-'+reg.get(rn(ops[0]),'?'); continue
        if m=='shl': reg[rn(ops[0])]=reg.get(rn(ops[0]),'?')+'<<8'; continue
        print('STOP %x %s %s'%(ins.address,m,ins.op_str)); break
    return out,slot,esp,events
def image():
    lo,hi=0x6404F0,0x640A42
    return run(fe.imgbytes(lo,hi),lo,lambda ins,d:d&0xffffffff)
def ours(src,pfx='32'):
    c,err=fe.compile_unit('src/caviar/caviar_raster.cpp',src,pfx)
    if c is None: raise SystemExit(err)
    s=c.function('?caviar_build_projection_tables@@YAXXZ'); sec=c.sections[s.section-1]
    rm={r[0]:r for r in sec.relocations}
    def absaddr(ins,d):
        for off in range(ins.address,ins.address+ins.size):
            if off in rm:
                sy=c.symbols.get(rm[off][1]); n=sy.name
                base={'?CaviarFixedPointProjection@@3UCaviarFixedPointProjectionRecord@@A':0x9C0A40}.get(n)
                if base is None:
                    if 'FixedPoint' in n: base=0x9C0A40
                    else: base=0x9C0840
                    NAMES.add(n)
                return base+(d&0xffffffff)
        return d&0xffffffff
    return run(sec.data,0,absaddr)
NAMES=set()
if __name__=='__main__':
    io,islot,iesp,iev=image()
    oo,oslot,oesp,oev=ours(sys.argv[1] if len(sys.argv)>1 else W+'/src/caviar/caviar_raster.cpp')
    print('img stores',len(io),'ours',len(oo), NAMES)
    import difflib
    A=['%s %s'%x for x in io]; B=['%s %s'%x for x in oo]
    n=0
    for tag,i1,i2,j1,j2 in difflib.SequenceMatcher(None,A,B,autojunk=False).get_opcodes():
        if tag=='equal':
            for x in A[i1:i2]: print('   ',x)
        else:
            n+=max(i2-i1,j2-j1)
            for x in A[i1:i2]: print('IMG ',x)
            for x in B[j1:j2]: print('OURS',x)
    print('differing',n,'frame img',islot and min(islot),'ours',oslot and min(oslot), 'nslots',len(islot),len(oslot))
