"""JJ merge against the shared tail's shape (par-6 r53), p28's harness (3 toggles + base_names, the
parameter c in esi).  Prints the number of copies of the tail's LAST call that remain (1 = all four
merged, 4 = none) and the tail's byte size."""
import sys, re, os
import cj, ib
from cj import *
import p28
TAILS = [('ii17', 'psave(0); dmap(1);', 17), ('ri16', 'psave(c); dmap(1);', 16), ('rr15', 'psave(c); dmap(c);', 15),
         ('one18', 'psave(0x100); f0();', 18), ('one14', 'psave(c); f0();', 14), ('calls15', 'f0(); f0(); f0();', 15),
         ('calls20', 'f0(); f0(); f0(); f0();', 20), ('calls25', 'f0(); f0(); f0(); f0(); f0();', 25),
         ('gxr16', 'gx = c; f0(); f0();', 16), ('gxi20', 'gx = 1; f0(); f0();', 20), ('u19', 'use(c); f0(); f0();', 19),
         ('u19b', 'f0(); f0(); use(c);', 19), ('u24b', 'f0(); f0(); f0(); use(c);', 24), ('rrr21', 'psave(c); dmap(c); use(c);', 21),
         ('r2i', 'psave(c); dmap(c); f0();', 20)]
p28.V.clear()
for k, t, _ in TAILS:
    p28.mk(k, t)
src = cj.HDR + ''.join(v for _, v in p28.V)
asm, out = ib.compile_asm(None, os.environ.get('FE_PFX_BASE', '33'), text=src)
F = {re.findall(r't_(\w+)@@', f[0] + ' ' + f[1])[0] if re.findall(r't_(\w+)@@', f[0] + ' ' + f[1]) else f[0]: f for f in ib.functions(asm)}
for k, t, size in TAILS:
    last = re.findall(r'(\w+)\(', t)[-1]
    f = [x for x in ib.functions(asm) if ('t_%s@@' % k) in x[0] or ('t_%s@@' % k) in x[1]][0]
    n = sum(1 for l in f[2] if l.startswith('call') and ('?%s@@' % last) in l)
    base = {'f0': 1, 'use': 2, 'dmap': 1}.get(last, 0)
    print('%-8s %-30s %2d bytes  last-call copies %d' % (k, t, size, n - base))
