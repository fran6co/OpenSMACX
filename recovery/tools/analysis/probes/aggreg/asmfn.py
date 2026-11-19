"""asmfn.py: the run.py union loop inside a function that also contains inline __asm (the sub_5cd560
situation: media_fixed_mul is an __asm helper expanded into it), under the game flags and the uv
library flags (/Oi /Oy /Ob1).  Hypothesis: a function containing __asm loses loop optimisation and
VC6 then keeps the union in a register (`mov bx,dx; shl ebx,16; mov bx,dx`)."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import run as R
ASM = '''#pragma warning(disable : 4035)
inline int fmul(int factor, int value) { __asm { push edx
 mov edx, factor
 mov eax, value
 imul edx
 shl edx, 16
 shr eax, 16
 adc eax, edx
 pop edx } }
'''
BODIES = {k: R.V[k] for k in ('union', 'union_h16' if 'union_h16' in R.V else 'union_short_fill', 'struct2', 'dword_mask', 'word_ptr')}
for flags_key, extra in (('game', ()), ('uv', ('/Oi', '/Oy', '/Ob1'))):
    for k, (decl, fill, val) in BODIES.items():
        for with_asm in (False, True):
            pre = '  fill = fmul(fill, ga[0]) + ga[1];\n' if with_asm else ''
            text = R.HDR + ASM + R.fn('t_' + k, decl, fill, val, pre=pre)
            asm, _ = R.ib.compile_asm(None, R.PFX, extra=extra, text=text)
            for name, sym, lines in R.ib.functions(asm):
                if name.startswith('t_'):
                    w, regw, shl, memw = R.where(lines)
                    print('%-5s %-16s asm=%-5s %-4s n=%-3d %s | %s' % (flags_key, k, with_asm, w, len(lines), regw, memw[:2]))
                    if '--show' in sys.argv and with_asm and k == 'union': print('\n'.join('    ' + l for l in lines))
