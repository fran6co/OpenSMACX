"""more.py: further spellings of the sub_5cd560 packed dword (see run.py)."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import run as R
X = {
 'u_arr': ('union { DWORD word; WORD half[2]; } p;', '    p.half[0] = (WORD)fill; p.word <<= 16; p.half[0] = (WORD)fill;', 'p.word'),
 'u_half_first': ('union { WORD half; DWORD word; } p;', '    p.half = (WORD)fill; p.word <<= 16; p.half = (WORD)fill;', 'p.word'),
 'u_int_short': ('union { int word; short half; } p;', '    p.half = (short)fill; p.word <<= 16; p.half = (short)fill;', 'p.word'),
 'u_h16': ('union { DWORD word; WORD half; } p; WORD h;', '    h = (WORD)fill; p.half = h; p.word <<= 16; p.half = h;', 'p.word'),
 'u_whole_first': ('union { DWORD word; WORD half; } p;', '    p.word = fill; p.word <<= 16; p.half = (WORD)fill;', 'p.word'),
 'u_named': ('PK p;', '    p.half = (WORD)fill; p.word <<= 16; p.half = (WORD)fill;', 'p.word'),
 'u_byte': ('union { DWORD word; unsigned char b; } p;', '    p.b = (unsigned char)fill; p.word <<= 8; p.b = (unsigned char)fill;', 'p.word'),
 'u_noloop_use': ('union { DWORD word; WORD half; } p;', '    p.half = (WORD)fill; p.word <<= 16; p.half = (WORD)fill; use(p.word);', 'p.word'),
}
for k, (decl, fill, val) in X.items():
    asm, _ = R.ib.compile_asm(None, R.PFX, text=R.HDR + 'union PK { DWORD word; WORD half; };\n' + R.fn('t_' + k, decl, fill, val))
    for name, sym, lines in R.ib.functions(asm):
        if name.startswith('t_'):
            w, regw, shl, memw = R.where(lines)
            print('%-16s %-4s n=%-3d %s | %s | %s' % (k, w, len(lines), regw, shl, memw))
            if '--show' in sys.argv: print('\n'.join('    ' + l for l in lines))
