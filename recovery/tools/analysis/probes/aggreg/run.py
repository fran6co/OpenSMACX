#!/usr/bin/env python3
"""run.py: when does VC6 keep a 4-byte aggregate (union of DWORD/WORD, struct of two shorts, bitfields)
in a register?  Each probe fills a 16x8 block with a dword built from a 16-bit value, the sub_5cd560
shape (0x005CD560: `mov bx,dx; shl ebx,16; mov bx,dx` with the loop counter spilled).  The report
says where the aggregate lives: REG (a 16-bit `mov r16, r16` into a 32-bit register / no frame slot)
or MEM (stored to [ebp-..]) and lists the key instructions.
Usage: uv run python run.py [key ...] [--show]   (FE_PFX_BASE, default 21)"""
import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))) + '/inline')
import ib
PFX = os.environ.get('FE_PFX_BASE', '21')
HDR = '#include "probe.h"\ntypedef unsigned long DWORD; typedef unsigned short WORD;\n'

LOOP = '''  for (int outer = 8; outer != 0; --outer) {
    DWORD *cells = (DWORD *)row;
    row += 8;
%s
    cells[0] = %s; cells[1] = %s; cells[2] = %s; cells[3] = %s;
    row += pitch / 2 - 8;
  }
'''

def fn(name, decl, fill, val, params='int fill, WORD *row, unsigned int pitch', pre=''):
    return 'void %s(%s) {\n%s  %s\n%s}\n' % (name, params, pre, decl, LOOP % (fill, val, val, val, val))

V = {
 'union': ('union { DWORD word; WORD half; } p;',
           '    p.half = (WORD)fill; p.word <<= 16; p.half = (WORD)fill;', 'p.word'),
 'union_init': ('union { DWORD word; WORD half; } p; p.word = 0;',
           '    p.half = (WORD)fill; p.word <<= 16; p.half = (WORD)fill;', 'p.word'),
 'struct2': ('struct { WORD lo, hi; } p;',
           '    p.lo = (WORD)fill; p.hi = (WORD)fill;', '*(DWORD *)&p'),
 'dword_mask': ('DWORD p;',
           '    p = (WORD)fill; p = (p << 16) | (WORD)fill;', 'p'),
 'dword_mask2': ('DWORD p = 0;',
           '    p = (p & 0xFFFF0000) | (WORD)fill; p <<= 16; p = (p & 0xFFFF0000) | (WORD)fill;', 'p'),
 'bitfield': ('struct { DWORD lo : 16; DWORD hi : 16; } p;',
           '    p.lo = fill; p.hi = fill;', '*(DWORD *)&p'),
 'word_ptr': ('DWORD p;',
           '    *(WORD *)&p = (WORD)fill; p <<= 16; *(WORD *)&p = (WORD)fill;', 'p'),
 'union_short_fill': ('union { DWORD word; WORD half; } p; WORD f16 = (WORD)fill;',
           '    p.half = f16; p.word <<= 16; p.half = f16;', 'p.word'),
 'union_outside': ('union { DWORD word; WORD half; } p; p.half = (WORD)fill; p.word <<= 16; p.half = (WORD)fill;',
           '', 'p.word'),
}

def where(lines):
    regw = [l for l in lines if re.match(r'mov\s+(ax|bx|cx|dx|si|di), (ax|bx|cx|dx|si|di)$', l)]
    memw = [l for l in lines if re.match(r'mov\s+WORD PTR _p\$', l) or re.match(r'mov\s+DWORD PTR _p\$', l)]
    shl = [l for l in lines if l.startswith('shl') and '16' in l]
    return ('REG' if regw and not memw else 'MEM' if memw else '?'), regw[:3], shl[:2], memw[:3]

if __name__ == '__main__':
    sel = [a for a in sys.argv[1:] if not a.startswith('--')]
    for k, (decl, fill, val) in V.items():
        if sel and k not in sel:
            continue
        asm, _ = ib.compile_asm(None, PFX, text=HDR + fn('t_' + k, decl, fill, val))
        for name, sym, lines in ib.functions(asm):
            if name.startswith('t_'):
                w, regw, shl, memw = where(lines)
                print('%-16s %-4s n=%-3d %s | %s | %s' % (k, w, len(lines), regw, shl, memw))
                if '--show' in sys.argv:
                    print('\n'.join('    ' + l for l in lines))
