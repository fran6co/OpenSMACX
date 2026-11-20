#!/usr/bin/env python3
"""orsweep.py <unit.cpp> <addr> [--and] [--write KEY] : value-form sweep of one function's `||` tests (MATCHING_GUIDE 3.7
"Flag tests: threaded or materialised"). Every `if (A || B ...)` / `else if` of the function alone becomes
`if ((A || B ...) != 0)` (the test computed as a value: its arm is laid in place after the last test instead of after
the following code); --and also tries the `&&` conditions. Ranked by sdeck's structural key (EH, named calls, cf
counts, cf hunks, rows) against BASE. --write KEY applies that variant to the source. Lanes FE_PFX_BASE.."""
import sys, os, re
S = os.path.dirname(os.path.abspath(__file__)); T = os.path.dirname(S); R = os.path.dirname(os.path.dirname(T))
sys.path.insert(0, T); sys.path.insert(0, S)
import fe, sdeck


def span(t, addr):
    m = re.search(r'// ORIGINAL: 0x%08X ' % addr, t); s = m.start(); return s, t.index('\n}\n', s)


def close(t, i):
    d = 0
    for j in range(i, len(t)):
        if t[j] == '(':
            d += 1
        elif t[j] == ')':
            d -= 1
            if d == 0:
                return j
    return -1


def toplevel(c, op):
    d = 0
    for j in range(len(c) - 1):
        if c[j] == '(':
            d += 1
        elif c[j] == ')':
            d -= 1
        elif d == 0 and c[j:j + 2] == op:
            return True
    return False


def variants(t, addr, ops):
    s, e = span(t, addr); out = []
    for m in re.finditer(r'\bif \(', t[s:e]):
        a = s + m.end() - 1; b = close(t, a)
        cond = t[a + 1:b]
        if '//' in cond or not any(toplevel(cond, op) for op in ops):
            continue
        out.append(('L%d' % (t.count('\n', 0, a) + 1), t[:a + 1] + '(' + cond + ') != 0' + t[b:]))
    return out


if __name__ == '__main__':
    unit, addr = sys.argv[1], int(sys.argv[2], 16)
    ops = ['||', '&&'] if '--and' in sys.argv else ['||']
    path = R + '/src/' + unit
    t = open(path, encoding='latin-1').read()
    deck = variants(t, addr, ops)
    if '--write' in sys.argv:
        k = sys.argv[sys.argv.index('--write') + 1]
        open(path, 'w', encoding='latin-1').write(dict(deck)[k]); print('wrote', k); sys.exit(0)
    _, lo, hi = fe.ident(t, addr)
    sdeck.run(unit, addr, lo, hi, [('BASE', t)] + deck)
