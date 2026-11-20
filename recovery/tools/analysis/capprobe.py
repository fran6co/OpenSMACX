#!/usr/bin/env python3
"""capprobe.py <unit> <0xADDR> '<call expression>' [--src ALT.cpp] [--decl 'TEXT']

Per-callee auto-inline cap screen (MATCHING_GUIDE 2.1 / 2.8, "The per-callee cap counts IL nodes"):
appends `void __capprobe(...) { <expr>; __capprobe_g = 1; }` to a copy of the unit (or ALT.cpp), compiles
it, and reports whether the callee marked at ADDR is still called from that one-statement caller
(CALLED = over the cap, EXPANDED = under it), plus the callee's own fe result in the same compile
(it must stay exact out of line). `--decl` adds text before the probe (for example a receiver:
`--decl 'extern Sprite *__cp_s;'` with expression `__cp_s->close()`).
"""
import sys, os, argparse
H = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(H))
import fe  # noqa: E402


def probe(unit, addr, expr, src=None, decl='', pfx=None):
    src = src or (fe.R + '/' + unit)
    t = open(src, encoding='latin-1').read()
    sym = fe.ident(t, addr)[0]
    p = t + '\nint __capprobe_g;\n' + decl + '\nvoid __capprobe() { ' + expr + '; __capprobe_g = 1; }\n'
    out = os.path.join(fe.SCR, 'capprobe_%s_%d.cpp' % (os.path.basename(unit).split('.')[0], os.getpid()))
    open(out, 'w', encoding='latin-1').write(p)
    c, err = fe.compile_unit(unit, out, pfx or fe.PFX)
    if c is None:
        return dict(error=err)
    s = c.function('?__capprobe@@YAXXZ')
    sec = c.sections[s.section - 1]
    names = []
    for r in sec.relocations:
        try:
            names.append(c.symbols[r[1]].name)
        except Exception:
            names.append(str(r[1:]))
    r = fe.evaluate(c, p, addr)
    return dict(sym=sym, called=sym in names, refs=names, exact=r.get('exact'), rows=r.get('diff'))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('unit')
    ap.add_argument('addr')
    ap.add_argument('expr')
    ap.add_argument('--src')
    ap.add_argument('--decl', default='')
    a = ap.parse_args()
    unit = a.unit if a.unit.startswith('src/') else 'src/' + a.unit
    d = probe(unit, int(a.addr, 16), a.expr, a.src and os.path.abspath(a.src), a.decl)
    if 'error' in d:
        print('COMPILE FAIL', d['error'])
        return 3
    print('%s %s callee_exact=%s rows=%s refs=%s' % ('CALLED' if d['called'] else 'EXPANDED', d['sym'], d['exact'],
                                                   d['rows'], ' '.join(sorted(set(d['refs'])))[:300]))
    return 0


if __name__ == '__main__':
    sys.exit(main())
