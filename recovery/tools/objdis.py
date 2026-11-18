# objdis.py <unit> <src> <addr> : raw disassembly of our function with real jump targets
import sys,os
S=os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0,S); import fe
unit,srcp,addr=sys.argv[1],sys.argv[2],int(sys.argv[3],16)
unit=unit if unit.startswith('src/') else 'src/'+unit
c,err=fe.compile_unit(unit,srcp,'22')
t=open(srcp,encoding='latin-1').read(); sym,lo,hi=fe.ident(t,addr)
s=c.function(sym); sec=c.sections[s.section-1]
for i in fe.md.disasm(sec.data,0):
    print('%4x %s %s'%(i.address,i.mnemonic,i.op_str))
    if i.address>len(sec.data)-2: break
