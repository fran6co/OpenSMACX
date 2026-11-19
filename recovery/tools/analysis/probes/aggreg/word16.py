"""word16.py: the sub_5cd560 pack spelled with a WORD local and a DWORD local:
`WORD h = (WORD)fill; DWORD p = h; p <<= 16; p |= h;` (and variants) -- does VC6 emit
`mov dx, ax; mov bx, dx; shl ebx, 16; mov bx, dx` (the image) for it?"""
import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import run as R
X = {
 'in_loop':      ('', 'WORD h = (WORD)fill; DWORD p = h; p <<= 16; p |= h;'),
 'in_loop_or2':  ('', 'WORD h = (WORD)fill; DWORD p = h; p = (p << 16) | h;'),
 'in_loop_plus': ('', 'WORD h = (WORD)fill; DWORD p = h; p <<= 16; p += h;'),
 'out_h':        ('WORD h = (WORD)fill;', 'DWORD p = h; p <<= 16; p |= h;'),
 'out_both':     ('WORD h = (WORD)fill; DWORD p;', 'p = h; p <<= 16; p |= h;'),
 'expr':         ('', 'DWORD p = (WORD)fill; p <<= 16; p |= (WORD)fill;'),
 'union_in':     ('', 'union { DWORD word; WORD half; } u; u.half = (WORD)fill; u.word <<= 16; u.half = (WORD)fill; DWORD p = u.word;'),
}
BODY = '''void %s(int fill, WORD *row, unsigned int pitch) {
  %s
  for (int outer = 8; outer != 0; --outer) {
    DWORD *cells = (DWORD *)row;
    row += 8;
    %s
    cells[0] = p; cells[1] = p; cells[2] = p; cells[3] = p;
    row += pitch / 2 - 8;
  }
}
'''
for flags_key, extra in (('game', ()), ('uv', ('/Oi', '/Oy', '/Ob1'))):
    for k, (pre, ops) in X.items():
        asm, _ = R.ib.compile_asm(None, R.PFX, extra=extra, text=R.HDR + BODY % ('t_' + k, pre, ops))
        for name, sym, lines in R.ib.functions(asm):
            if name.startswith('t_'):
                r16 = [l for l in lines if re.match(r'mov\s+[a-d]x, [a-d]x$', l)]
                print('%-5s %-13s n=%-3d 16bit=%s' % (flags_key, k, len(lines), r16))
                if '--show' in sys.argv and k in sys.argv: print('\n'.join('    ' + l for l in lines))
