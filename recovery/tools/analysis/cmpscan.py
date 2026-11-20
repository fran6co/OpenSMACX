#!/usr/bin/env python3
"""cmpscan.py [unit ...] [--all] [--json OUT] [--win N] [--weak]
Compare operand order and jcc direction (MATCHING_GUIDE 3.7: VC6 keeps a comparison's operand order and its
jump sense from the source). For every non-exact function (--all: every marker) it walks fe's alignment and
pairs each image `cmp a,b; jcc` that sits in or next to a differing row with an ours `cmp; jcc` within N rows
(default 8), then classifies:
  SWAP     ours `cmp b,a; j<mirror>`        operands written the other way round (`a >= b` vs `b <= a`)
  INV      ours `cmp a,b; j<negated>`       same test, inverted, arms swapped (`if (!c) A else B` vs `if (c) B else A`)
  SWAPINV  ours `cmp b,a; j<neg mirror>`    both
With --weak, register-register compares whose register names differ but whose shape mirrors are reported too
(`RSWAP`, often register allocation noise). Each row prints the image address, our offset and our source line.
Screen only: confirm with fe.py --all before editing."""
import sys, os, re, json, argparse, collections
from concurrent.futures import ThreadPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
sys.path.insert(0, HERE)
import fe  # noqa: E402
import tailscan  # noqa: E402

MARK = re.compile(r'// ORIGINAL: 0x([0-9A-F]{8}) (\S+) 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)')
MIRROR = {'e': 'e', 'ne': 'ne', 'l': 'g', 'g': 'l', 'le': 'ge', 'ge': 'le', 'b': 'a', 'a': 'b', 'be': 'ae',
          'ae': 'be'}
NEG = {'e': 'ne', 'ne': 'e', 'l': 'ge', 'ge': 'l', 'le': 'g', 'g': 'le', 'b': 'ae', 'ae': 'b', 'be': 'a',
       'a': 'be'}
REG = re.compile(r'^(e?[abcd]x|[abcd][lh]|e?[sd]i|e?[sb]p)$')


def pairs(rows, side):
    """[(row index of cmp, row index of jcc, [a, b], cc, differs)]"""
    seq = [(i, r[side]) for i, r in enumerate(rows) if r[side] is not None]
    out = []
    for t in range(len(seq) - 1):
        i, ins = seq[t]
        j, nxt = seq[t + 1]
        mn, _, op = ins[1].partition(' ')
        jm = nxt[1].split(' ')[0]
        if mn != 'cmp' or not jm.startswith('j') or jm == 'jmp' or jm[1:] not in NEG:
            continue
        ops = [x.strip() for x in op.split(',')]
        if len(ops) != 2:
            continue
        out.append((i, j, ops, jm[1:], rows[i][0] != '  ' or rows[j][0] != '  '))
    return out


def shape(o):
    return 'R' if REG.match(o) else o


def classify(a, b, weak):
    (ao, acc), (bo, bcc) = (a[2], a[3]), (b[2], b[3])
    if ao == bo and acc == bcc:
        return None
    if ao[::-1] == bo and ao[0] != ao[1]:
        if bcc == MIRROR[acc] and acc not in ('e', 'ne'):
            return 'SWAP'
        if bcc == NEG[MIRROR[acc]]:
            return 'SWAPINV'
    if ao == bo and bcc == NEG[acc]:
        return 'INV'
    if weak:
        sa, sb = [shape(x) for x in ao], [shape(x) for x in bo]
        if sa[::-1] == sb and sa[0] != sa[1] and bcc == MIRROR[acc] and acc not in ('e', 'ne'):
            return 'SSWAP'   # register vs memory written the other way, registers renamed
        if sa == sb == ['R', 'R'] and ao != bo and bcc == MIRROR[acc] and acc not in ('e', 'ne'):
            return 'RSWAP'
        if sa == sb and 'R' in sa and ao != bo and bcc == NEG[acc]:
            return 'RINV'
    return None


def scan_fn(res, lmap, win, weak):
    rows = res['rows']
    I, O = pairs(rows, 1), pairs(rows, 2)
    used = set()
    flags = []
    for a in I:
        if not a[4]:
            continue
        best = None
        for k, b in enumerate(O):
            if k in used or not b[4] or abs(b[0] - a[0]) > win:
                continue
            c = classify(a, b, weak)
            if c and (best is None or abs(b[0] - a[0]) < best[0]):
                best = (abs(b[0] - a[0]), k, c)
        if best:
            used.add(best[1])
            b = O[best[1]]
            io, oo = rows[a[0]][1], rows[b[0]][2]
            flags.append(dict(kind=best[2], img=res['lo'] + io[0], ours=oo[0], line=lmap.get(oo[0], '?'),
                              img_txt='cmp %s; j%s' % (','.join(a[2]), a[3]),
                              ours_txt='cmp %s; j%s' % (','.join(b[2]), b[3])))
    return flags


def scan_unit(unit, pfx, mode, win, weak, only=None):
    text = open(fe.R + '/' + unit, encoding='latin-1').read()
    c, cod = tailscan.compile_listing(unit, fe.R + '/' + unit, pfx)
    if c is None:
        return [('ERR', unit, 'compile failed')]
    out = []
    for m in MARK.finditer(text):
        addr = int(m.group(1), 16)
        if only and addr not in only:
            continue
        r = fe.evaluate(c, text, addr)
        if 'error' in r or (mode == 'nonexact' and r['exact']):
            continue
        lmap = tailscan.lines_of(cod, r['sym'], os.path.basename(unit).lower()) if cod else {}
        fl = scan_fn(r, lmap, win, weak)
        if fl:
            out.append(dict(unit=unit, addr='%08X' % addr, fn=r['sym'], exact=r['exact'], diff=r['diff'], flags=fl))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='*')
    ap.add_argument('--all', action='store_true')
    ap.add_argument('--json')
    ap.add_argument('--win', type=int, default=8)
    ap.add_argument('--weak', action='store_true')
    ap.add_argument('--addr', default='')
    ap.add_argument('--jobs', type=int, default=int(os.environ.get('FE_JOBS', '5')))
    ap.add_argument('--pfx-base', type=int, default=int(os.environ.get('FE_PFX_BASE', '21')))
    a = ap.parse_args()
    mode = 'all' if a.all else 'nonexact'
    only = set(int(x, 16) for x in a.addr.split(',') if x) or None
    cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
    units = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/')]
    units = [u for u in units if '/vendor/' not in u and u.endswith('.cpp')]
    if a.units:
        units = [u for u in units if u in a.units or os.path.basename(u) in a.units]
    fe.compile_cmd(units[0])

    def lane(j):
        r = []
        for i in range(j, len(units), a.jobs):
            r += scan_unit(units[i], a.pfx_base + j, mode, a.win, a.weak, only)
        return r
    res = []
    with ThreadPoolExecutor(a.jobs) as ex:
        for o in ex.map(lane, range(a.jobs)):
            res += o
    for e in res:
        if isinstance(e, tuple):
            print('ERROR', e[1], e[2])
    res = [x for x in res if isinstance(x, dict)]
    if a.json:
        json.dump(res, open(a.json, 'w'), indent=0)
    tot = collections.Counter()
    for x in sorted(res, key=lambda x: (x['unit'], x['addr'])):
        print('%s %s %-40s %s diff=%d' % ('EXACT' if x['exact'] else 'NONEX', x['addr'], x['fn'][:40], x['unit'],
                                          x['diff']))
        for f in x['flags']:
            tot[f['kind']] += 1
            print('   %-8s img %08X %-28s ours +%04X %-28s %s' % (f['kind'], f['img'], f['img_txt'], f['ours'],
                                                                f['ours_txt'], f['line']))
    print('functions=%d %s' % (len(res), dict(tot)))


if __name__ == '__main__':
    main()
