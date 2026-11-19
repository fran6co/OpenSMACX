"""partial.py: which spelling makes VC6 write a 16-bit value into the low half of a 32-bit register
(`mov bx, dx`)?  The 16-bit value h varies per iteration (loaded from ga[]) so nothing is hoisted."""
import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import run as R
X = {
 'shift_or':    ('DWORD p = 0;', 'p = h; p <<= 16; p |= h;'),
 'mask_or':     ('DWORD p = 0;', 'p = (p & 0xFFFF0000) | h; p <<= 16; p = (p & 0xFFFF0000) | h;'),
 'mask_or_ul':  ('DWORD p = 0;', 'p = (p & 0xFFFF0000UL) | (DWORD)h; p <<= 16; p = (p & 0xFFFF0000UL) | (DWORD)h;'),
 'union':       ('union { DWORD word; WORD half; } p; p.word = 0;', 'p.half = h; p.word <<= 16; p.half = h;'),
 'word_ptr':    ('DWORD p = 0;', '*(WORD *)&p = h; p <<= 16; *(WORD *)&p = h;'),
 'mask_short':  ('DWORD p = 0;', 'p = (p & ~0xFFFF) | h; p <<= 16; p = (p & ~0xFFFF) | h;'),
 'int_mask':    ('int p = 0;', 'p = (p & 0xFFFF0000) | h; p <<= 16; p = (p & 0xFFFF0000) | h;'),
}
BODY = '''void %s(WORD *row, unsigned int pitch) {
  %s
  for (int outer = 8; outer != 0; --outer) {
    DWORD *cells = (DWORD *)row;
    WORD h = (WORD)ga[outer];
    row += 8;
    %s
    cells[0] = p; cells[1] = p; cells[2] = p; cells[3] = p;
    row += pitch / 2 - 8;
  }
}
'''
for k, (decl, ops) in X.items():
    val = 'p.word' if k == 'union' else 'p'
    body = BODY % ('t_' + k, decl, ops).replace('= p;', '= %s;' % val) if False else BODY % ('t_' + k, decl, ops)
    if k == 'union': body = body.replace('cells[0] = p; cells[1] = p; cells[2] = p; cells[3] = p;', 'cells[0] = p.word; cells[1] = p.word; cells[2] = p.word; cells[3] = p.word;')
    asm, _ = R.ib.compile_asm(None, R.PFX, text=R.HDR + body)
    for name, sym, lines in R.ib.functions(asm):
        if name.startswith('t_'):
            loop = [l for l in lines if not l.startswith(('push', 'pop', 'ret'))]
            r16 = [l for l in lines if re.match(r'mov\s+[a-d]x, [a-d]x$|mov\s+[sd]i, ', l)]
            print('%-12s n=%-3d 16bit=%s' % (k, len(lines), r16))
            if '--show' in sys.argv: print('\n'.join('    ' + l for l in lines))
