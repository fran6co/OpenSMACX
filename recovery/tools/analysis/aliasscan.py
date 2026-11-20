#!/usr/bin/env python3
"""aliasscan.py <unit> <0xADDR[,..]> [--src ALT.cpp] | --from FEALL.json [--json F]
"Scalar that was an array element" screen (MATCHING_GUIDE 3.3, colortables_init 0x00423570).
VC6 never enregisters an address-taken local or an array element: inside a loop it re-reads such a value
from its frame slot every iteration, and multiplies by it with `imul r, [slot]`, where a plain scalar is
cached in a register or strength-reduced.  For every natural loop of both sides this counts, per the
innermost loop of each instruction:
  imul   `imul r, dword ptr [ebp +- N]` (a multiply by a frame value)
  inv    reads of an ebp slot that no instruction of the same loop writes or address-takes (loop-invariant
         frame reloads), as (distinct slots, reads)
and prints the functions where the image has more of either than ours, with the image sites.
IMG > OURS: the image's value lives in memory - an array element, an address-taken local (passed by
pointer/reference to a call or an inline), or one lost to register pressure; read the site against the source.
Screen only: confirm with fe.py / recover measure."""
import sys, os, re, argparse, collections, json
from concurrent.futures import ThreadPoolExecutor
H = os.path.dirname(os.path.abspath(__file__))
T = os.path.dirname(H)
sys.path.insert(0, T)
sys.path.insert(0, H)
import fe
import tailscan
from candset import image_tables
from ivscan import LCode, trim_tables, cod_tables

SLOT = re.compile(r'\[ebp ([+-]) (0x[0-9a-f]+|\d+)\]')
WRITES = ('mov', 'add', 'sub', 'inc', 'dec', 'and', 'or', 'xor', 'shl', 'sar', 'shr', 'neg', 'not', 'imul',
          'lea', 'pop', 'fstp', 'fst', 'fistp', 'fist', 'adc', 'sbb', 'movsx', 'movzx', 'sete', 'setne', 'setl',
          'setg', 'setle', 'setge', 'setb', 'seta', 'setbe', 'setae')


def slot_of(op):
    m = SLOT.search(op)
    return None if not m else (-1 if m.group(1) == '-' else 1) * int(m.group(2), 0)


def census(code, ins):
    n = len(ins)
    imul, inv = [], []
    cache = {}
    for i, (off, t, g) in enumerate(ins):
        b = code.bof[i]
        if code.depth[b] == 0:
            continue
        body = code.loop_blocks_of(b)
        if not body:
            continue
        key = frozenset(body)
        if key not in cache:
            written = set()
            for j in range(n):
                if code.bof[j] not in body:
                    continue
                tj = ins[j][1]
                mn, _, ops = tj.partition(' ')
                first = ops.split(',')[0]
                s = slot_of(first)
                if s is not None and (mn in WRITES or mn.startswith('set') or mn.startswith('f') and 'st' in mn):
                    written.add(s)
                if mn == 'lea':
                    s2 = slot_of(ops.partition(',')[2])
                    if s2 is not None:
                        written.add(s2)  # address taken in the loop: may be written
            cache[key] = written
        written = cache[key]
        mn, _, ops = t.partition(' ')
        if mn == 'lea':
            continue
        m = re.fullmatch(r'imul (e[abcd]x|esi|edi), dword ptr \[ebp ([+-]) (0x[0-9a-f]+|\d+)\]', t)
        if m:
            imul.append(i)
        parts = ops.split(',')
        srcs = parts[1:] if (mn in WRITES and mn not in ('imul',) or mn.startswith('set')) else parts
        if mn == 'imul' and len(parts) == 2:
            srcs = parts[1:]
        for p in srcs:
            s = slot_of(p)
            if s is not None and s not in written and s < 0:
                inv.append((i, s))
    return imul, inv


def scan(unit, addr, text, c, cod):
    r = fe.evaluate(c, text, addr)
    if 'error' in r:
        return dict(addr=addr, unit=unit, error=r['error'])
    rows = r['rows']
    lo = r['lo']
    oins = [(x[0], x[1], x[3]) for k, o, x in rows if x is not None]
    iins = [(x[0], x[1], x[3]) for k, x, cc in rows if x is not None]
    if not oins or not iins:
        return dict(addr=addr, unit=unit, error='empty')
    icode = LCode(iins, trim_tables(image_tables(lo, lo + max(o[0] for o in iins) + 16, rows)), True)
    ocode = LCode(oins, cod_tables(cod, r['sym']) if cod else {}, True)
    lmap = tailscan.lines_of(cod, r['sym'], os.path.basename(unit).lower()) if cod else {}
    # aligned pairs: the image reads a loop-invariant ebp slot where ours, same mnemonic, has a register
    inv_set = set(i for i, _ in census(icode, iins)[1])
    pairs = []
    ii = oi = 0
    for k, x, y in rows:
        if x is not None and y is not None and k != '  ' and ii in inv_set:
            mx, _, ox = iins[ii][1].partition(' ')
            my, _, oy = oins[oi][1].partition(' ')
            px, py = [q.strip() for q in ox.split(',')], [q.strip() for q in oy.split(',')]
            if mx == my and len(px) == len(py) and any(
                    SLOT.search(a) and re.fullmatch(r'e[abcd]x|esi|edi|ebx|[abcd][lh]', b) for a, b in zip(px, py)):
                pairs.append('%08X %-34s | %x %s %s' % (lo + iins[ii][0], iins[ii][1], oins[oi][0],
                                                      lmap.get(oins[oi][0], '?'), oins[oi][1]))
        if x is not None:
            ii += 1
        if y is not None:
            oi += 1
    im_i, inv_i = census(icode, iins)
    im_o, inv_o = census(ocode, oins)
    return dict(addr=addr, unit=unit, sym=r['sym'], diff=r['diff'], pairs=pairs,
                imul=(len(im_i), len(im_o)),
                inv=(len(set(s for _, s in inv_i)), len(inv_i), len(set(s for _, s in inv_o)), len(inv_o)),
                img_sites=['%08X %s' % (lo + iins[i][0], iins[i][1]) for i in im_i] +
                          ['%08X %s' % (lo + iins[i][0], iins[i][1]) for i, s in inv_i][:12],
                our_sites=['%x %s %s' % (oins[i][0], lmap.get(oins[i][0], '?'), oins[i][1]) for i in im_o] +
                          ['%x %s %s' % (oins[i][0], lmap.get(oins[i][0], '?'), oins[i][1]) for i, s in inv_o][:12])


def run_unit(unit, addrs, src, pfx):
    try:
        text = open(src, encoding='latin-1').read()
    except OSError:
        return [dict(addr=a, unit=unit, error='NO SOURCE') for a in addrs]
    c, cod = tailscan.compile_listing(unit, src, pfx)
    if c is None:
        return [dict(addr=a, unit=unit, error='COMPILE FAIL') for a in addrs]
    out = []
    for a in addrs:
        try:
            out.append(scan(unit, a, text, c, cod))
        except Exception as e:
            out.append(dict(addr=a, unit=unit, error=repr(e)))
    return out


def flagged(x):
    return 'error' not in x and bool(x['pairs'])


def show(x, sites=True):
    if 'error' in x:
        return ['ERR %08X %s %s' % (x['addr'], x['unit'], x['error'])]
    l = ['%08X %-40s %-26s diff %4d  imul img/ours %d/%d  inv slots,reads img %d,%d ours %d,%d  mem/reg pairs %d' % (
        (x['addr'], x['sym'][:40], x['unit'][4:], x['diff']) + x['imul'] + x['inv'] + (len(x['pairs']),))]
    l += ['    PAIR ' + q for q in x['pairs']]
    if sites:
        l += ['    IMG ' + s for s in x['img_sites']]
        l += ['    OUR ' + s for s in x['our_sites']]
    return l


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('unit', nargs='?')
    ap.add_argument('addr', nargs='?')
    ap.add_argument('--src')
    ap.add_argument('--pfx', default=os.environ.get('FE_PFX_BASE', '2'))
    ap.add_argument('--from', dest='frm')
    ap.add_argument('--all', action='store_true', help='with --from: include exact functions (validation)')
    ap.add_argument('--json')
    a = ap.parse_args()
    if a.frm:
        work = {}
        for r in json.load(open(a.frm)):
            if a.all or not r.get('exact'):
                u = r['unit'] if r['unit'].startswith('src/') else 'src/' + r['unit']
                work.setdefault(u, []).append(int(str(r['addr']), 16))
        work = sorted(work.items())
        jobs = int(os.environ.get('FE_JOBS', '5'))
        fe.compile_cmd(work[0][0])
        base = int(a.pfx)

        def lane(k):
            res = []
            for i in range(k, len(work), jobs):
                u, ad = work[i]
                res += run_unit(u, ad, fe.R + '/' + u, str(base + k))
            return res
        allr = []
        with ThreadPoolExecutor(jobs) as ex:
            for o in ex.map(lane, range(jobs)):
                allr += o
        fl = [x for x in allr if flagged(x)]
        fl.sort(key=lambda x: (-len(x['pairs']), x['diff']))
        for x in fl:
            print('\n'.join(show(x, sites=False)))
        print('functions %d, flagged %d, errors %d' % (len(allr), len(fl), sum('error' in x for x in allr)))
        if a.json:
            json.dump(allr, open(a.json, 'w'), indent=0)
        return
    unit = a.unit if a.unit.startswith('src/') else 'src/' + a.unit
    src = os.path.abspath(a.src) if a.src else fe.R + '/' + unit
    for x in run_unit(unit, [int(v, 16) for v in a.addr.split(',')], src, a.pfx):
        print('\n'.join(show(x)))


if __name__ == '__main__':
    main()
