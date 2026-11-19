#!/usr/bin/env python3
"""tm.py: cross-jump (tail-merge) micro-probes with the game unit flags.
Each generated function has K switch cases (or if/else arms) whose tails are identical
`use(<tailarg>); call();` sequences ending in `break`; the count of `call ?use@@` sites left in the
listing tells how many copies VC6 kept (1 = all merged, K = none).  The knobs vary one property
of the function at a time (filler statements, case count, locals, EH, ...)."""
import sys, os, re, itertools
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + '/inline')
import ib

HDR = '#include "probe.h"\nextern void tl(int);\nstruct D { D(); ~D(); int v; };\n'


def fn(name, k=4, filler=0, pre=0, eh=False, tail='tl(g2); call();', arm='mk(%d);', end='break;',
       loopfill=0, ifs=0, post=''):
    s = 'void %s(int c) {\n' % name
    if eh:
        s += '  D d;\n'
    for i in range(pre):
        s += '  if (g%d == %d) g = %d;\n' % (1 + i % 9 if i % 9 else 2, i, i)
    for i in range(ifs):
        s += '  if (cond()) { use(%d); }\n' % (1000 + i)
    for i in range(loopfill):
        s += '  for (int i%d = 0; i%d < g; i%d++) use(ga[i%d]);\n' % (i, i, i, i)
    s += '  switch (c) {\n'
    for j in range(k):
        s += '  case %d: %s %s %s\n' % (j + 1, arm % (j + 1) if '%d' in arm else arm, tail, end)
    s += '  }\n'
    for i in range(filler):
        s += '  if (g == %d) use(%d);\n' % (i + 100, i + 100)
    s += post + '}\n'
    return s


def copies(asm, name, sym='?use@@YAXH@Z'):
    for f in ib.functions(asm):
        if f[0].startswith(name):
            tail_push = None
            return sum(1 for l in f[2] if l.startswith('call') and sym in l)
    return None


def run(variants, pfx='21'):
    src = HDR + ''.join(v for _, v in variants)
    asm, out = ib.compile_asm(None, pfx, text=src)
    res = []
    for (key, v) in variants:
        name = re.findall(r'void (t_\w+)\(', v)[-1]
        fl = [f for f in ib.functions(asm) if re.search(r'\b' + name + r'\b', f[0])]
        lines = fl[0][2] if fl else []
        res.append((key, sum(1 for l in lines if l.startswith('call') and '?tl@@' in l)))
    return res, asm


if __name__ == '__main__':
    V = []
    for f in (0, 20, 80, 200, 400):
        V.append(('filler%d' % f, fn('t_f%d' % f, filler=f)))
    for k in (2, 4, 8, 16):
        V.append(('cases%d' % k, fn('t_k%d' % k, k=k)))
    V.append(('eh', fn('t_eh', eh=True)))
    for p in (20, 80, 200):
        V.append(('ifs%d' % p, fn('t_i%d' % p, ifs=p)))
    res, asm = run(V)
    for k, n in res:
        print(k, n)
