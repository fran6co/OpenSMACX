#!/usr/bin/env python3
"""selscan.py [unit ...] [--all] [--json OUT] [--kinds sel,negsbb,...]
Branchless select / bool / abs / min-max idioms present on one side only (MATCHING_GUIDE 3.4 "Ternary setcc
forms", "Bool-to-flag forms differ"). For every non-exact function (--all: every marker) it extracts from the
image and from our compile:
  sel     `setcc; dec; and K; [add B]`  a ternary `c ? K+B : B` whose setcc is the NEGATED source condition
  negsbb  `neg r; sbb r,r; <op>`         `x ? A : B` / `!x` / `x != 0` forms
  cmpsbb  `cmp; sbb r,r; <op>`           unsigned `<` as 0/-1 (or a `x == K ? :` lowered through sub/neg)
  set     a lone `setcc`                 a bool result (no dec/and follows)
  abs     `cdq; xor; sub`                abs() / branchless |x|
  babs    `test r,r; jge; neg r`         a branch abs
  mm      `cmp a,b; jcc L; mov a,b` with L the next row  a windows.h min/max or `if (a < b) a = b;`
  neg     a lone `neg r` (not neg/sbb)  `-x` kept as a value: `Facility[-p]` folds the negation into a sub,
                                         a named `const int f = -p` keeps it (MapWin::right_menu)
and prints the signature multisets that differ, each with the image addresses / our offsets. A `sel` whose
(K, B) agrees but whose condition differs is a comparison-direction difference (`cond`); the same select with
the arms swapped reads (K', B') = (-K, B+K). Screen only: confirm by reading the rows (fe.py --all)."""
import sys, os, re, json, argparse, collections
from concurrent.futures import ThreadPoolExecutor
T = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, T)
import fe  # noqa: E402

MARK = re.compile(r'// ORIGINAL: 0x([0-9A-F]{8}) (\S+) 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)')
FAM = {}
for big, rest in (('eax', 'ax al ah'), ('ebx', 'bx bl bh'), ('ecx', 'cx cl ch'), ('edx', 'dx dl dh'),
                  ('esi', 'si'), ('edi', 'di'), ('ebp', 'bp')):
    for r in [big] + rest.split():
        FAM[r] = big
NEG = {'e': 'ne', 'ne': 'e', 'l': 'ge', 'ge': 'l', 'le': 'g', 'g': 'le', 'b': 'ae', 'ae': 'b', 'be': 'a',
       'a': 'be', 's': 'ns', 'ns': 's'}


def fam(r):
    return FAM.get(r.strip(), r.strip())


def k(s):
    if s.strip() in FAM:
        return 'R'
    try:
        v = int(s, 0)
    except ValueError:
        return s
    v &= 0xFFFFFFFF
    return v - (1 << 32) if v & 0x80000000 else v


def norm_k(v, reg):
    """byte-register constants: sign-extend from 8 bits so `and al,0xe7` reads -25."""
    if isinstance(v, int) and reg in ('al', 'bl', 'cl', 'dl', 'ah', 'bh', 'ch', 'dh') and 0 <= v < 256:
        return v - 256 if v & 0x80 else v
    return v


def parse(t):
    mn, _, op = t.partition(' ')
    return mn, [x.strip() for x in op.split(',')] if op else []


def mentions(P, j, reg):
    return any(fam(x) == fam(reg) or re.search(r'\b%s\b' % fam(reg), x) for x in P[j][1])


def next_on(P, j, reg, win=4):
    """index of the next row from j that mentions reg's family (scheduled code interleaves unrelated rows),
    stopping at a jump/call/label-like boundary."""
    for t in range(j, min(len(P), j + win)):
        if P[t][0].startswith('j') or P[t][0] in ('call', 'ret'):
            return None
        if mentions(P, t, reg):
            return t
    return None


def op_on(P, j, reg):
    """(next row index, (mnemonic, second operand)) of the next row from j that writes reg's family."""
    t = next_on(P, j, reg)
    if t is not None and P[t][1] and fam(P[t][1][0]) == fam(reg) and P[t][0] not in ('mov', 'push', 'cmp', 'test'):
        o = P[t][1][1] if len(P[t][1]) > 1 else ''
        return t, (P[t][0], 'R' if o in FAM else o)
    return None, ()


def idioms(stream):
    """[(index, kind, sig)] of one listing; stream = [(off, text, size, tgt)]"""
    out = []
    n = len(stream)
    P = [parse(x[1]) for x in stream]
    used = set()
    for i in range(n):
        mn, ops = P[i]
        if mn.startswith('set') and len(ops) == 1:
            cc = mn[3:]
            r = fam(ops[0])
            # look for dec/and/add on the same family within 4 rows
            j = i + 1
            seq = []
            while j < n and j <= i + 5:
                m2, o2 = P[j]
                if o2 and fam(o2[0]) == r:
                    seq.append((j, m2, o2))
                    if m2 not in ('dec', 'and', 'add', 'mov', 'movzx', 'neg', 'inc', 'sub', 'lea', 'or'):
                        break
                elif m2 in ('mov', 'movzx') and len(o2) == 2 and fam(o2[1]) == r:
                    r = fam(o2[0])  # moved to another register
                    seq.append((j, m2, o2))
                elif m2.startswith('j') or m2 == 'call' or m2.startswith('ret'):
                    break
                j += 1
            ms = [s[1] for s in seq if s[1] not in ('mov', 'movzx')]
            if ms[:2] == ['dec', 'and']:
                a = [s for s in seq if s[1] == 'and'][0]
                K = norm_k(k(a[2][1]), a[2][0])
                B = 0
                rest = ms[2:]
                if rest and rest[0] in ('add', 'sub', 'or'):
                    b = [s for s in seq if s[1] == rest[0]][0]
                    B = norm_k(k(b[2][1]), b[2][0]) if len(b[2]) > 1 else b[2][1]
                    if rest[0] == 'sub' and isinstance(B, int):
                        B = -B
                    if rest[0] == 'or':
                        B = ('or', B)
                out.append((i, 'sel', (cc, K, B)))
            elif ms[:1] == ['dec'] and not ms[1:2]:
                out.append((i, 'sel', (cc, 'dec', 0)))
            elif ms[:1] in (['inc'], ['neg'], ['sub'], ['add'], ['lea']) and ms[0] != 'and':
                out.append((i, 'set', (cc, ms[0])))
            else:
                out.append((i, 'set', (cc,)))
        elif mn == 'neg' and len(ops) == 1:
            j = next_on(P, i + 1, ops[0], 8)
            m2, o2 = P[j] if j is not None else ('', [])
            if m2 == 'sbb' and len(o2) == 2 and o2[0] == o2[1] and fam(o2[0]) == fam(ops[0]):
                t1, tail = op_on(P, j + 1, ops[0])
                tail2 = op_on(P, t1 + 1, ops[0])[1] if tail else ()
                out.append((i, 'negsbb', tail + tail2))
                used.add(j)
            elif not (i > 0 and P[i - 1][0] == 'sbb'):
                out.append((i, 'neg', (m2,) if m2 in ('lea', 'add', 'sub', 'imul', 'shl', 'mov') else ()))
        elif mn == 'sbb' and len(ops) == 2 and ops[0] == ops[1] and i not in used and i > 0 and P[i - 1][0] == 'cmp':
            tail = op_on(P, i + 1, ops[0])[1]
            out.append((i, 'cmpsbb', tail))
        elif mn == 'cdq' and i + 2 < n and P[i + 1][0] == 'xor' and P[i + 2][0] == 'sub' \
                and P[i + 1][1][1:] == ['edx'] and P[i + 2][1][1:] == ['edx']:
            out.append((i, 'abs', ()))
        elif mn == 'test' and len(ops) == 2 and ops[0] == ops[1] and i + 2 < n and P[i + 1][0] in ('jge', 'jns') \
                and P[i + 2][0] == 'neg' and P[i + 2][1] == [ops[0]]:
            out.append((i, 'babs', ()))
        elif mn == 'cmp' and len(ops) == 2 and i + 2 < n and P[i + 1][0].startswith('j') \
                and P[i + 1][0] not in ('jmp', 'je', 'jne') and P[i + 2][0] == 'mov' \
                and stream[i + 1][3] is not None and i + 3 < n and stream[i + 1][3] == stream[i + 3][0] \
                and len(P[i + 2][1]) == 2 and P[i + 2][1][0] == ops[0] and P[i + 2][1][1] == ops[1]:
            out.append((i, 'mm', (P[i + 1][0],)))
    return out


def key(kind, sig):
    """comparison key without the condition (the condition is compared separately)."""
    if kind == 'sel':
        return (kind,) + tuple(sig[1:])
    if kind == 'set':
        return (kind,) + tuple(sig[1:])
    return (kind,) + tuple(sig)


def compare(O, C):
    io, co = idioms(O), idioms(C)
    ik = collections.Counter(key(kd, s) for _, kd, s in io)
    ck = collections.Counter(key(kd, s) for _, kd, s in co)
    diffs = []
    for kk in sorted(set(ik) | set(ck), key=str):
        if ik[kk] != ck[kk]:
            diffs.append(dict(key=kk, img=ik[kk], ours=ck[kk],
                              img_at=[(O[i][0], s) for i, kd, s in io if key(kd, s) == kk],
                              ours_at=[(C[i][0], s) for i, kd, s in co if key(kd, s) == kk]))
    # same select shape, different condition multiset
    conds = []
    for kk in set(ik) & set(ck):
        if kk[0] not in ('sel', 'set'):
            continue
        a = collections.Counter(s[0] for _, kd, s in io if key(kd, s) == kk)
        b = collections.Counter(s[0] for _, kd, s in co if key(kd, s) == kk)
        if a != b:
            conds.append(dict(key=kk, img=dict(a), ours=dict(b),
                              img_at=[(O[i][0], s) for i, kd, s in io if key(kd, s) == kk],
                              ours_at=[(C[i][0], s) for i, kd, s in co if key(kd, s) == kk]))
    return diffs, conds, len(io), len(co)


def scan_unit(unit, pfx, mode):
    text = open(fe.R + '/' + unit, encoding='latin-1').read()
    c, err = fe.compile_unit(unit, fe.R + '/' + unit, pfx)
    if c is None:
        return [('ERR', unit, err)]
    out = []
    for m in MARK.finditer(text):
        addr = int(m.group(1), 16)
        r = fe.evaluate(c, text, addr)
        if 'error' in r or (mode == 'nonexact' and r['exact']):
            continue
        rows = r['rows']
        O = [x[1] for x in rows if x[1] is not None]
        C = [x[2] for x in rows if x[2] is not None]
        diffs, conds, ni, nc = compare(O, C)
        if not diffs and not conds:
            continue
        out.append(dict(unit=unit, addr='%08X' % addr, lo=r['lo'], fn=r['sym'], exact=r['exact'], diff=r['diff'],
                        diffs=diffs, conds=conds, img_n=ni, ours_n=nc))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='*')
    ap.add_argument('--all', action='store_true')
    ap.add_argument('--json')
    ap.add_argument('--kinds', default='')
    ap.add_argument('--jobs', type=int, default=int(os.environ.get('FE_JOBS', '5')))
    ap.add_argument('--pfx-base', type=int, default=int(os.environ.get('FE_PFX_BASE', '21')))
    a = ap.parse_args()
    mode = 'all' if a.all else 'nonexact'
    kinds = set(a.kinds.split(',')) if a.kinds else None
    cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
    units = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/')]
    units = [u for u in units if '/vendor/' not in u and u.endswith('.cpp')]
    if a.units:
        units = [u for u in units if u in a.units or os.path.basename(u) in a.units]
    fe.compile_cmd(units[0])

    def lane(j):
        r = []
        for i in range(j, len(units), a.jobs):
            r += scan_unit(units[i], a.pfx_base + j, mode)
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
        json.dump(res, open(a.json, 'w'), indent=0, default=str)
    tot = collections.Counter()
    for x in sorted(res, key=lambda x: (x['unit'], x['addr'])):
        ds = [d for d in x['diffs'] if not kinds or d['key'][0] in kinds]
        cs = [d for d in x['conds'] if not kinds or d['key'][0] in kinds]
        if not ds and not cs:
            continue
        print('%s %s %-40s %s diff=%d' % ('EXACT' if x['exact'] else 'NONEX', x['addr'], x['fn'][:40], x['unit'],
                                          x['diff']))
        for d in ds:
            tot[d['key'][0]] += 1
            print('   %-28s img %d ours %d  img@%s  ours@%s' % (d['key'], d['img'], d['ours'],
                  ','.join('%X%s' % (x['lo'] + o, '' if s == d['key'][1:] else ':' + str(s[0]) if s else '')
                           for o, s in d['img_at']),
                  ','.join('+%X' % o for o, s in d['ours_at'])))
        for d in cs:
            tot['cond'] += 1
            print('   COND %-23s img %s ours %s  img@%s ours@%s' % (d['key'], d['img'], d['ours'],
                  ','.join('%X' % (x['lo'] + o) for o, s in d['img_at']),
                  ','.join('+%X' % o for o, s in d['ours_at'])))
    print('functions=%d %s' % (len(res), dict(tot)))


if __name__ == '__main__':
    main()
