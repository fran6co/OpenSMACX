#!/usr/bin/env python3
"""widthscan.py [unit ...] [--all] [--json OUT] [--min N]
Expression width differences (MATCHING_GUIDE 3.4 "Narrow arithmetic"): inside each differing block of fe's
alignment, pair the register arithmetic of the two sides by mnemonic (add sub and or xor shl shr sar inc dec
neg not cmp test imul, and mov between registers or from an immediate) and report the mnemonics one side
runs on a byte/word register (`shr al,3`, `add dl,4`, `cmp cl,0x7f`) where the other runs the same
mnemonic on a full register (`shr edx,3`), or the reverse. Memory operands are left to accdiff.py.

A narrow row in ours against a full row in the image means our expression's type is narrower somewhere
(a char/short local, parameter, return type or cast that VC6 propagates into the arithmetic); the
reverse means the image's expression was narrow. Rows are per function: `n:img/ours` counts of narrow
and full occurrences of each mnemonic in the differing blocks. Screen only."""
import sys, os, re, json, argparse, collections
from concurrent.futures import ThreadPoolExecutor
T = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, T)
import fe  # noqa: E402

MARK = re.compile(r'// ORIGINAL: 0x([0-9A-F]{8}) (\S+) 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)')
NARROW = {'al', 'ah', 'bl', 'bh', 'cl', 'ch', 'dl', 'dh', 'ax', 'bx', 'cx', 'dx', 'si', 'di', 'bp'}
FULL = {'eax', 'ebx', 'ecx', 'edx', 'esi', 'edi', 'ebp'}
ARITH = {'add', 'sub', 'and', 'or', 'xor', 'shl', 'shr', 'sar', 'inc', 'dec', 'neg', 'not', 'cmp', 'test',
         'imul', 'mov'}


def cls(text):
    mn, _, op = text.partition(' ')
    if mn not in ARITH:
        return None
    ops = [x.strip() for x in op.split(',')]
    if not ops or not ops[0]:
        return None
    d = ops[0]
    if 'ptr' in op:
        return None
    if mn == 'mov':
        # register-to-register or immediate moves only
        if len(ops) != 2 or 'ptr' in ops[1]:
            return None
    if mn in ('shl', 'shr', 'sar') and len(ops) == 2 and ops[1] == 'cl' and d in NARROW | FULL:
        pass
    if d in NARROW:
        return (mn, 'n')
    if d in FULL:
        return (mn, 'f')
    return None


def scan_unit(unit, pfx, allm):
    text = open(fe.R + '/' + unit, encoding='latin-1').read()
    c, err = fe.compile_unit(unit, fe.R + '/' + unit, pfx)
    if c is None:
        return [('ERR', unit, err)]
    out = []
    for m in MARK.finditer(text):
        addr = int(m.group(1), 16)
        r = fe.evaluate(c, text, addr)
        if 'error' in r or r['exact']:
            continue
        rows = r['rows']
        k = 0
        sites = []
        while k < len(rows):
            if rows[k][0] != '!!':
                k += 1
                continue
            k2 = k
            while k2 < len(rows) and rows[k2][0] == '!!':
                k2 += 1
            co = collections.Counter()
            cc = collections.Counter()
            fo = {}
            fc = {}
            for x in rows[k:k2]:
                if x[1] is not None:
                    q = cls(x[1][1])
                    if q:
                        co[q] += 1
                        fo.setdefault(q, x[1])
                if x[2] is not None:
                    q = cls(x[2][1])
                    if q:
                        cc[q] += 1
                        fc.setdefault(q, x[2])
            for mn in sorted({q[0] for q in co} | {q[0] for q in cc}):
                on, of_ = co[(mn, 'n')], co[(mn, 'f')]
                cn, cf = cc[(mn, 'n')], cc[(mn, 'f')]
                if mn == 'mov':
                    # narrow moves accompany narrow arithmetic; count them only alongside another row
                    pass
                # a narrow op on one side matched by a full op of the same mnemonic on the other
                if on > cn and cf > of_:
                    sites.append(dict(mn=mn, dir='img-narrow', img=fo[(mn, 'n')][1], ours=fc[(mn, 'f')][1],
                                      at='%08X' % (r['lo'] + fo[(mn, 'n')][0]), n=min(on - cn, cf - of_)))
                elif cn > on and of_ > cf:
                    sites.append(dict(mn=mn, dir='ours-narrow', img=fo[(mn, 'f')][1], ours=fc[(mn, 'n')][1],
                                      at='%08X' % (r['lo'] + fo[(mn, 'f')][0]), n=min(cn - on, of_ - cf)))
            k = k2
        sites = [s for s in sites if s['mn'] != 'mov' or any(t['mn'] != 'mov' for t in sites)]
        if sites:
            out.append(dict(unit=unit, addr='%08X' % addr, fn=r['sym'], diff=r['diff'], sites=sites))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='*')
    ap.add_argument('--all', action='store_true')
    ap.add_argument('--json')
    ap.add_argument('--min', type=int, default=1)
    ap.add_argument('--jobs', type=int, default=int(os.environ.get('FE_JOBS', '5')))
    ap.add_argument('--pfx-base', type=int, default=int(os.environ.get('FE_PFX_BASE', '21')))
    a = ap.parse_args()
    cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
    units = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/')]
    units = [u for u in units if '/vendor/' not in u and u.endswith('.cpp')]
    if a.units:
        units = [u for u in units if u in a.units or os.path.basename(u) in a.units]
    fe.compile_cmd(units[0])

    def lane(k):
        r = []
        for i in range(k, len(units), a.jobs):
            r += scan_unit(units[i], a.pfx_base + k, a.all)
        return r
    res = []
    with ThreadPoolExecutor(a.jobs) as ex:
        for o in ex.map(lane, range(a.jobs)):
            res += o
    for e in res:
        if isinstance(e, tuple):
            print('ERROR', e[1], (e[2] or '')[:200])
    res = [x for x in res if isinstance(x, dict)]
    if a.json:
        json.dump(res, open(a.json, 'w'), indent=0)
    tot = collections.Counter()
    for x in sorted(res, key=lambda x: -sum(s['n'] for s in x['sites'] if s['mn'] != 'mov')):
        n = sum(s['n'] for s in x['sites'] if s['mn'] != 'mov')
        if n < a.min:
            continue
        print('%s %-40s %s diff=%d narrow-rows=%d' % (x['addr'], x['fn'][:40], x['unit'], x['diff'], n))
        for s in x['sites']:
            tot[s['dir']] += s['n']
            print('    %-11s %s %-5s img %-32s ours %s' % (s['dir'], s['at'], s['mn'], s['img'][:32], s['ours'][:40]))
    print('functions=%d img-narrow=%d ours-narrow=%d' % (len(res), tot['img-narrow'], tot['ours-narrow']))


if __name__ == '__main__':
    main()
