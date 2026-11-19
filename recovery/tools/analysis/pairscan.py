#!/usr/bin/env python3
"""pairscan.py <unit> <0xADDR> [--src ALT.cpp] [--pfx N] [--sites]
Frame-slot pairing by source variable (MATCHING_GUIDE 3.3/3.7, enemy_move): for every aligned row where the image
and ours access a frame slot with the same instruction shape, the variable our /FAsc listing names at our slot on
that source line, and the image slot it pairs with. A variable whose rows pair with several image slots, or a
slot of ours that serves several image slots, names the sites where the two frames pack differently (a compiler
temporary or block-local the image keeps in another variable's slot). `?a/b` = the line names none or several of
the slot's variables (a temporary or a packed block-local), `T-N` = a slot with no name. --sites prints the
source lines and image addresses of every minority pairing. Screen only; slot identity is not variable identity
when live ranges are disjoint (VC6 packs them)."""
import sys, os, re, collections, argparse
H = os.path.dirname(os.path.abspath(__file__))
T_ = os.path.dirname(H)
R = os.path.dirname(os.path.dirname(T_))
sys.path.insert(0, T_)
sys.path.insert(0, H)
os.chdir(R)
import fe
import regprio

SLOT = re.compile(r'\[ebp ([+-]) (0x[0-9a-f]+|\d+)\]')


def slot(s):
    m = SLOT.search(s)
    if not m:
        return None
    v = int(m.group(2), 0)
    return -v if m.group(1) == '-' else v


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('unit')
    ap.add_argument('addr')
    ap.add_argument('--src')
    ap.add_argument('--pfx', type=int, default=int(os.environ.get('FE_PFX_BASE', '21')))
    ap.add_argument('--sites', action='store_true')
    a = ap.parse_args()
    unit = a.unit if a.unit.startswith('src/') else 'src/' + a.unit
    src = os.path.abspath(a.src) if a.src else R + '/' + unit
    addr = int(a.addr, 16)
    text = open(src, encoding='latin-1').read()
    c, err = fe.compile_unit(unit, src, a.pfx)
    if c is None:
        sys.exit('compile failed: %s' % (err or '')[:300])
    r = fe.evaluate(c, text, addr)
    items, frame = regprio.cod_listing(unit, src, a.pfx, r['sym'])
    names = collections.defaultdict(list)
    for n, o in frame.items():
        names[o].append(n)
    line_of, cur = {}, 0
    for it in items:
        if it[0] == 'line':
            cur = it[1]
        elif it[0] == 'ins':
            line_of[it[1]] = cur
    L = text.split('\n')
    per = collections.defaultdict(collections.Counter)
    sites = collections.defaultdict(list)
    shape = lambda s: SLOT.sub('[S]', s)
    for k, o, c_ in r['rows']:
        if o is None or c_ is None:
            continue
        so, si = slot(c_[1]), slot(o[1])
        if so is None or si is None or shape(o[1]) != shape(c_[1]):
            continue
        ln = line_of.get(c_[0], 0)
        srcl = L[ln - 1] if 0 < ln <= len(L) else ''
        hit = [n for n in names.get(so, []) if re.search(r'\b%s\b' % re.escape(n), srcl)]
        key = hit[0] if len(hit) == 1 else ('?' + '/'.join(names[so]) if names.get(so) else 'T%s' % hex(so))
        per[(key, so)][si] += 1
        sites[(key, so, si)].append((ln, r['lo'] + o[0]))
    print('FE %08X %s diff=%d' % (addr, r['sym'], r['diff']))
    for (key, so), cn in sorted(per.items(), key=lambda x: x[0][1]):
        print('%-34s ours %-6s -> %s' % (key, hex(so), ' '.join('%s:%d' % (hex(s), n) for s, n in cn.most_common(8))))
    if a.sites:
        for (key, so), cn in sorted(per.items(), key=lambda x: x[0][1]):
            top = cn.most_common(1)[0][0]
            for si, n in cn.most_common():
                if si == top:
                    continue
                v = sites[(key, so, si)]
                print('  %s ours %s img %s n=%d: %s' % (key, hex(so), hex(si), n,
                      ' '.join('L%d@%x' % (l, ad) for l, ad in v[:10])))


if __name__ == '__main__':
    main()
