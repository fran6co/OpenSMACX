"""deck.py: helpers to build probe files from Python and print per-site decisions.
   F(name, nif)   -> an inline `void name(int k)` with nif one-if statements (branchy cost unit)
   run(funcs)     -> {func: {site: 'E'|'c'}} ; funcs is a list of source strings."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ib
PFX = os.environ.get('FE_PFX_BASE', '21')


def F(name, nif, kw='inline', arg='int k', mark='mk(k);'):
    if nif == 0:
        return '%s void %s(%s) { %s }' % (kw, name, arg, mark)
    return '%s void %s(%s) { %s }' % (kw, name, arg, ' '.join('if (ga[%d] == 1000) %s' % (i, mark) for i in range(nif)))


def run(parts, pfx=PFX, oy=False, extra=()):
    asm, out = ib.compile_asm(None, pfx, oy, extra=extra, text='#include "probe.h"\n' + '\n'.join(parts))
    res = {}
    for name, sym, lines in ib.functions(asm):
        st = {}
        for k, c, x in ib.sites(lines):
            if k < 1000:
                st.setdefault(k, set()).add(x)
        res[name] = ({k: ''.join(sorted(v)) for k, v in st.items()}, ib.tails(lines), lines)
    return res


def show(res, names=None):
    for n, (st, tl, lines) in res.items():
        if names and n not in names:
            continue
        print('%-14s %s' % (n, ' '.join('%s%d' % (st[k], k) for k in sorted(st))))
        if tl:
            print('%-14s tails %s' % ('', tailstr(tl)))


def tailstr(tl):
    """compact: per at() site, the teardown tokens (E = expanded dm marker, cX = call of ~X)"""
    keys = sorted(k for k in tl if k is not None)
    return ' '.join('%d:%s' % (i, '.'.join(x for x in tl[i] if x[0] in 'Ec' and x not in ('call', 'close', 'cFB', 'cGW', 'cond')) or '-') for i in keys)
