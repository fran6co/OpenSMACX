#!/usr/bin/env python3
"""constfold.py [unit ...] [--all] [--exact] [--json OUT]
Unfolded constant arithmetic (MATCHING_GUIDE 3.4 "Runtime constant arithmetic"): every place one side
computes constant (+) constant at run time, `mov r,K1` followed by `add/sub/shl/sar/shr/imul/and/or/xor r,K2`
or `lea r,[r+K2]` with no other write to r, no label and no jump in between (a call ends the window for
eax/ecx/edx only). VC6 /Og folds literal arithmetic, so such a pair means the first operand was not a
literal in the source at folding time.

For each function (non-exact by default; --all adds exact ones, --exact lists only exact ones, which is
where the spelling that produces the shape can be read off), prints the pairs present on the image side
and on ours, and marks each image pair `ours-folded` when our side has no pair with the same K1 op K2
but does use the folded value as an immediate. Screen only."""
import sys, os, re, json, argparse, collections
from concurrent.futures import ThreadPoolExecutor
T = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, T)
sys.path.insert(0, T + '/analysis')
import fe  # noqa: E402

MARK = re.compile(r'// ORIGINAL: 0x([0-9A-F]{8}) (\S+) 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)')
R32 = ('eax', 'ebx', 'ecx', 'edx', 'esi', 'edi', 'ebp')
SUBS = {'eax': ('eax', 'ax', 'al', 'ah'), 'ebx': ('ebx', 'bx', 'bl', 'bh'), 'ecx': ('ecx', 'cx', 'cl', 'ch'),
        'edx': ('edx', 'dx', 'dl', 'dh'), 'esi': ('esi', 'si'), 'edi': ('edi', 'di'), 'ebp': ('ebp', 'bp')}
OPS = {'add', 'sub', 'shl', 'sar', 'shr', 'imul', 'and', 'or', 'xor'}
NUM = r'(-?(?:0x[0-9a-f]+|\d+))'


def ival(s):
    return int(s, 0)


def writes(text, reg):
    mn, _, op = text.partition(' ')
    ops = [x.strip() for x in op.split(',')] if op else []
    names = SUBS[reg]
    if mn in ('cmp', 'test', 'push') or mn.startswith('j'):
        return False
    if mn in ('cdq',) and reg == 'edx':
        return True
    if mn in ('mul', 'div', 'idiv') and reg in ('eax', 'edx'):
        return True
    if mn == 'imul' and len(ops) == 1 and reg in ('eax', 'edx'):
        return True
    if mn.startswith(('rep', 'movs', 'stos', 'lods', 'scas', 'cmps')) and reg in ('ecx', 'esi', 'edi', 'eax'):
        return True
    if mn == 'xchg' and any(o in names for o in ops):
        return True
    return bool(ops) and ops[0] in names


def pairs(stream):
    """[(k_def, k_op, reg, K1, op, K2)] in one listing: arithmetic whose inputs are all known constants
    inside one straight-line stretch (constants from `mov r,K`, `xor r,r`, `or r,-1`)."""
    labels = {x[3] for x in stream if x[3] is not None}
    out = []
    known = {}
    for j, (o2, t2, _, _) in enumerate(stream):
        mn = t2.split(' ')[0]
        if o2 in labels:
            known = {}
        hit = None
        m2 = re.fullmatch(r'(\w+) (e[a-z]{2}), (?:%s|(e[a-z]{2}))' % NUM, t2)
        if m2 and m2.group(1) in OPS and m2.group(2) in known and m2.group(2) in R32:
            k2 = ival(m2.group(3)) if m2.group(3) else (known[m2.group(4)][1] if m2.group(4) in known else None)
            if k2 is not None and not (m2.group(1) in ('xor', 'sub') and m2.group(2) == m2.group(4)):
                hit = (m2.group(2), m2.group(1), k2)
        m2 = re.fullmatch(r'lea (e[a-z]{2}), \[(e[a-z]{2}) ([+-]) %s\]' % NUM, t2)
        if m2 and m2.group(2) in known:
            hit = (m2.group(1), 'add', ival(m2.group(4)) * (1 if m2.group(3) == '+' else -1), m2.group(2))
        if hit:
            reg = hit[0]
            src = hit[3] if len(hit) > 3 else reg
            out.append((known[src][0], j, reg, known[src][1], hit[1], hit[2]))
            known[reg] = (j, fold(known[src][1], hit[1], hit[2]))
            continue
        m = re.fullmatch(r'mov (e[a-z]{2}), ' + NUM, t2)
        if m and m.group(1) in R32:
            known[m.group(1)] = (j, ival(m.group(2)) & 0xFFFFFFFF)
            continue
        m = re.fullmatch(r'xor (e[a-z]{2}), (e[a-z]{2})', t2)
        if m and m.group(1) == m.group(2):
            known[m.group(1)] = (j, 0)
            continue
        m = re.fullmatch(r'mov (e[a-z]{2}), (e[a-z]{2})', t2)
        if m and m.group(2) in known:
            known[m.group(1)] = known[m.group(2)]
            continue
        if mn.startswith('j') or mn.startswith('ret'):
            known = {}
            continue
        if mn == 'call':
            for r_ in ('eax', 'ecx', 'edx'):
                known.pop(r_, None)
            continue
        for r_ in list(known):
            if writes(t2, r_):
                del known[r_]
    return out


def fold(k1, op, k2):
    m = 0xFFFFFFFF
    if op in ('shl', 'sar', 'shr'):
        k2 &= 31
    if k1 & 0x80000000:
        k1 -= 1 << 32
    v = {'add': lambda: k1 + k2, 'sub': lambda: k1 - k2, 'shl': lambda: k1 << k2, 'sar': lambda: k1 >> k2,
         'shr': lambda: (k1 & m) >> k2, 'imul': lambda: k1 * k2, 'and': lambda: k1 & k2, 'or': lambda: k1 | k2,
         'xor': lambda: k1 ^ k2}[op]()
    return v & m


def scan_unit(unit, pfx, mode):
    text = open(fe.R + '/' + unit, encoding='latin-1').read()
    c, err = fe.compile_unit(unit, fe.R + '/' + unit, pfx)
    if c is None:
        return [('ERR', unit, err)]
    out = []
    for m in MARK.finditer(text):
        exact_claim = 'BYTE_EXACT' in m.group(3)
        addr = int(m.group(1), 16)
        r = fe.evaluate(c, text, addr)
        if 'error' in r:
            continue
        if mode == 'nonexact' and r['exact']:
            continue
        if mode == 'exact' and not r['exact']:
            continue
        rows = r['rows']
        O = [x[1] for x in rows if x[1] is not None]
        C = [x[2] for x in rows if x[2] is not None]
        po, pc = pairs(O), pairs(C)
        if not po and not pc:
            continue
        cimm = set()
        for x in C:
            for n in re.findall(r'(?<![\w\[+-])' + NUM + r'(?![\w\]])', x[1]):
                try:
                    cimm.add(ival(n) & 0xFFFFFFFF)
                except ValueError:
                    pass
        csig = collections.Counter((k1, op, k2) for _, _, _, k1, op, k2 in pc)
        osig = collections.Counter((k1, op, k2) for _, _, _, k1, op, k2 in po)
        img = []
        for i, j, reg, k1, op, k2 in po:
            f = fold(k1, op, k2)
            st = 'both' if csig[(k1, op, k2)] else ('ours-folded' if f in cimm else 'ours-other')
            img.append(dict(at='%08X' % (r['lo'] + O[i][0]), reg=reg, k1=k1, op=op, k2=k2, val=f, st=st,
                            txt='%s | %s' % (O[i][1], O[j][1])))
        ours = [dict(at='+%X' % C[i][0], reg=reg, k1=k1, op=op, k2=k2, val=fold(k1, op, k2),
                     st='both' if osig[(k1, op, k2)] else 'ours-only', txt='%s | %s' % (C[i][1], C[j][1]))
                for i, j, reg, k1, op, k2 in pc]
        out.append(dict(unit=unit, addr='%08X' % addr, fn=r['sym'], exact=r['exact'], claim=exact_claim,
                        diff=r['diff'], img=img, ours=ours))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='*')
    ap.add_argument('--all', action='store_true')
    ap.add_argument('--exact', action='store_true')
    ap.add_argument('--json')
    ap.add_argument('--jobs', type=int, default=int(os.environ.get('FE_JOBS', '5')))
    ap.add_argument('--pfx-base', type=int, default=int(os.environ.get('FE_PFX_BASE', '21')))
    a = ap.parse_args()
    mode = 'exact' if a.exact else 'all' if a.all else 'nonexact'
    cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
    units = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/')]
    units = [u for u in units if '/vendor/' not in u and u.endswith('.cpp')]
    if a.units:
        units = [u for u in units if u in a.units or os.path.basename(u) in a.units]
    fe.compile_cmd(units[0])

    def lane(k):
        r = []
        for i in range(k, len(units), a.jobs):
            r += scan_unit(units[i], a.pfx_base + k, mode)
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
    for x in sorted(res, key=lambda x: (x['exact'], x['unit'], x['addr'])):
        io = [p for p in x['img'] if p['st'] != 'both']
        oo = [p for p in x['ours'] if p['st'] != 'both']
        tot['img_pairs'] += len(x['img'])
        tot['img_unmatched'] += len(io)
        tot['ours_only'] += len(oo)
        if mode == 'nonexact' and not io and not oo:
            continue
        print('%s %s %-40s %s diff=%d' % ('EXACT' if x['exact'] else 'NONEX', x['addr'], x['fn'][:40],
                                          x['unit'], x['diff']))
        for p in (x['img'] if mode != 'nonexact' else io):
            print('   img  %s %-4s 0x%X %s 0x%X = 0x%X  %-11s %s' % (p['at'], p['reg'], p['k1'], p['op'], p['k2'],
                                                                     p['val'], p['st'], p['txt']))
        for p in oo:
            print('   ours %s %-4s 0x%X %s 0x%X = 0x%X  %-11s %s' % (p['at'], p['reg'], p['k1'], p['op'], p['k2'],
                                                                     p['val'], p['st'], p['txt']))
    print('functions=%d image pairs=%d (unmatched by ours %d) ours-only pairs=%d' %
          (len(res), tot['img_pairs'], tot['img_unmatched'], tot['ours_only']))


if __name__ == '__main__':
    main()
