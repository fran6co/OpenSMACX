"""jseq.py <unit.cpp> <src> <addr> [<imglo> <imghi>] : control-flow token diff (jcc/jmp/call/ret), image vs ours.
Prints both token counts and the first SHOW (default 8) differing hunks with addresses/offsets. The span defaults to the ORIGINAL marker's. Lane FE_PFX_BASE."""
import sys,os,difflib,collections
S=os.path.dirname(os.path.abspath(__file__)); T=os.path.dirname(S); R=os.path.dirname(os.path.dirname(T))
sys.path.insert(0,T); import fe
unit,src,addr=sys.argv[1],os.path.abspath(sys.argv[2]),int(sys.argv[3],16)
pfx=fe.PFX
c,err=fe.compile_unit('src/'+unit,src,pfx)
if c is None: print('FAIL',err); sys.exit(1)
t=open(src,encoding='latin-1').read(); sym,lo,hi=fe.ident(t,addr)
if len(sys.argv)>5: lo,hi=int(sys.argv[4],16),int(sys.argv[5],16)
s=c.function(sym); sec=c.sections[s.section-1]
b=[(i.address,i.mnemonic) for i in fe.md.disasm(sec.data,0) if i.mnemonic[0]=='j' or i.mnemonic in('call','ret')]
a=[(i.address,i.mnemonic) for i in fe.md.disasm(fe.imgbytes(lo,hi),lo) if i.mnemonic[0]=='j' or i.mnemonic in('call','ret')]
ca=collections.Counter('jcc' if m[0]=='j' and m!='jmp' else m for _,m in a); cb=collections.Counter('jcc' if m[0]=='j' and m!='jmp' else m for _,m in b)
print('img',dict(ca),'ours',dict(cb))
sm=difflib.SequenceMatcher(None,[x[1] for x in a],[x[1] for x in b],autojunk=False)
n=0
for op,i1,i2,j1,j2 in sm.get_opcodes():
    if op!='equal':
        n+=1
        if n<=int(os.environ.get('SHOW','8')): print(op,['%x %s'%x for x in a[i1:i2][:5]],['%x %s'%x for x in b[j1:j2][:5]])
print('hunks',n)
