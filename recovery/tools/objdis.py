# objdis.py <unit> <src> <addr> : raw disassembly of our function with real jump targets
import sys,os
S=os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0,S); import fe
unit,srcp,addr=sys.argv[1],sys.argv[2],int(sys.argv[3],16)
# accept `picktech`, `picktech.cpp`, `src/picktech.cpp` or a unit in a subdirectory (`listbox.cpp` -> src/jackal/listbox.cpp)
if not unit.endswith(('.cpp','.c')): unit+='.cpp'
unit=unit if unit.startswith('src/') else 'src/'+unit
if not fe.compile_cmd(unit):
    fe.compile_cmd(unit); hits=[k[len(fe.R)+1:] for k in fe._CC if os.path.basename(k)==os.path.basename(unit)]
    if len(hits)!=1: sys.exit('unit %s not in compile_commands (%s)'%(unit,', '.join(hits) or 'no basename match'))
    unit=hits[0]
c,err=fe.compile_unit(unit,os.path.abspath(srcp),fe.PFX)
if c is None: sys.exit('COMPILE FAIL '+(err or ''))
t=open(srcp,encoding='latin-1').read(); sym,lo,hi=fe.ident(t,addr)
s=c.function(sym); sec=c.sections[s.section-1]
for i in fe.md.disasm(sec.data,0):
    print('%4x %s %s'%(i.address,i.mnemonic,i.op_str))
    if i.address>len(sec.data)-2: break
