#!/usr/bin/env python3
"""stmovescan.py <unit> <0xADDR[,..]> [--src ALT.cpp] [--pfx N] | --tree [unit ..] [--from FEALL.json] [--all] [--both] [--json F]

Moved spill-home store screen (MATCHING_GUIDE 3.7 Frame and slots, "A spill-home store the image places right
before the next call ... is an assignment written later in source"; NetWin::draw_session_info 0x00480900).
VC6 does not sink a store to a homed local across statements, so a store to `[ebp-N]` that the image emits
later than ours, past the end of the block where ours stores it, is an assignment that the original wrote
later in source.

Per non-exact function: image and our `mov [ebp - N], v` stores are paired through the fe alignment (the
image slot is mapped to ours by the votes of the aligned rows that name an ebp slot; identity otherwise),
both stores unaligned (a `!!` row), same width, same immediate or both a register value, and no aligned store
to that slot between them.  A pair is flagged when the image's store comes later in alignment order and
our instruction at the image's position lies at least one basic block after our store (blocks split at
jump targets and after jcc/jmp/ret).  `CALL` = the image store is followed by a call within 8 instructions
with no jump in between (the draw_session_info shape: the store sits right before the next call).  Printed:
image address and instruction, our offset and source line of the store, and our source line at the image's
position (where the assignment wants to move), with the blocks crossed.  --both also lists the reverse
direction (image earlier: an assignment written earlier).  Screen only: move the assignment, confirm with fe.py;
the move is behaviour-preserving only when nothing between the two places reads or writes the variable."""
import sys, os, re, json, argparse
from concurrent.futures import ThreadPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
sys.path.insert(0, HERE)
import fe
import tailscan
from rotscan import own_context

ST = re.compile(r'^mov (byte|word|dword) ptr \[ebp - (0x[0-9a-f]+)\], (.+)$')
SLOT = re.compile(r'\[ebp - (0x[0-9a-f]+)\]')


def blocks(ins):
    """instruction list [(off, text, size, tgt)] -> block id per index"""
    leaders = {ins[0][0]} if ins else set()
    for k, x in enumerate(ins):
        t = x[1]
        if x[3] is not None:
            leaders.add(x[3])
        if (t.startswith('j') or t.startswith('ret')) and k + 1 < len(ins):
            leaders.add(ins[k + 1][0])
    b, out = -1, []
    for x in ins:
        if x[0] in leaders:
            b += 1
        out.append(b)
    return out


def slot_map(rows):
    votes = {}
    for k, o, c in rows:
        if o is None or c is None:
            continue
        so, sc = SLOT.findall(o[1]), SLOT.findall(c[1])
        if len(so) != 1 or len(sc) != 1:
            continue
        if SLOT.sub('[S]', o[1]) != SLOT.sub('[S]', c[1]):
            continue
        votes.setdefault(so[0], {}).setdefault(sc[0], 0)
        votes[so[0]][sc[0]] += 1
    return {s: max(v.items(), key=lambda kv: kv[1])[0] for s, v in votes.items()}


def isreg(v):
    return re.fullmatch(r'e?[abcd]x|e?[sd]i|[abcd][lh]|ebp', v) is not None


def scan(unit, addr, text, c, cod, both=False):
    res = fe.evaluate(c, text, addr)
    if 'error' in res:
        return dict(addr=addr, error=res['error'])
    if res['exact']:
        return dict(addr=addr, sym=res['sym'], exact=True, flags=[])
    rows, lo = res['rows'], res['lo']
    lmap = tailscan.lines_of(cod, res['sym'], os.path.basename(unit).lower()) if cod else {}
    I = [r[1] for r in rows if r[1] is not None]
    O = [r[2] for r in rows if r[2] is not None]
    bI, bO = blocks(I), blocks(O)
    iI = {x[0]: k for k, x in enumerate(I)}
    iO = {x[0]: k for k, x in enumerate(O)}
    # our instruction index at (or after) each row
    nxtO = [None] * len(rows)
    last = len(O) - 1 if O else None
    for q in range(len(rows) - 1, -1, -1):
        if rows[q][2] is not None:
            last = iO[rows[q][2][0]]
        nxtO[q] = last
    smap = slot_map(rows)
    st_i, st_o, al = [], [], []   # (row, slot, width, value)
    for q, (k, o, cc) in enumerate(rows):
        if k == '!!' and o is not None and cc is not None and ST.match(o[1]) and ST.match(cc[1]) \
                and ST.match(o[1]).group(1) == ST.match(cc[1]).group(1):
            al.append((q, ST.match(o[1]).group(2)))   # one store in one row, slot or register renamed: aligned
        elif k == '!!':
            if o is not None and ST.match(o[1]) and (cc is None or cc[1] != o[1]):
                m = ST.match(o[1]); st_i.append((q, m.group(2), m.group(1), m.group(3)))
            if cc is not None and ST.match(cc[1]) and (o is None or cc[1] != o[1]):
                m = ST.match(cc[1]); st_o.append((q, m.group(2), m.group(1), m.group(3)))
        elif o is not None and ST.match(o[1]):
            al.append((q, ST.match(o[1]).group(2)))
    flags, used = [], set()
    for qi, si, wi, vi in st_i:
        so = smap.get(si, si)
        best = None
        for qo, sx, wx, vx in st_o:
            if qo in used or sx != so or wx != wi:
                continue
            if not (vx == vi or (isreg(vx) and isreg(vi))):
                continue
            if qo == qi or (qo > qi and not both):
                continue
            a, b = sorted((qi, qo))
            if any(a < q < b and s == si for q, s in al):
                continue
            if best is None or abs(qo - qi) < abs(best[0] - qi):
                best = (qo, sx, wx, vx)
        if best is None:
            continue
        qo = best[0]
        o_store = iO[rows[qo][2][0]]
        o_at = nxtO[qi]
        if o_at is None:
            continue
        crossed = bO[o_at] - bO[o_store]
        if abs(crossed) < 1:
            continue
        used.add(qo)
        k_i = iI[rows[qi][1][0]]
        call = None
        for x in I[k_i + 1:k_i + 9]:
            if x[1].startswith('j') or x[1].startswith('ret'):
                break
            if x[1].startswith('call'):
                call = x[1]
                break
        flags.append(dict(img=lo + rows[qi][1][0], img_ins=rows[qi][1][1], ours=rows[qo][2][0], our_ins=rows[qo][2][1],
                          our_line=lmap.get(rows[qo][2][0], '?'), to_line=lmap.get(O[o_at][0], '?'), to_off=O[o_at][0],
                          crossed=crossed, call=call, later=qi > qo,
                          img_blocks=bI[k_i] - bI[iI[rows[nxt_img(rows, qo)][1][0]]] if nxt_img(rows, qo) is not None else None))
    return dict(addr=addr, unit=unit, sym=res['sym'], diff=res['diff'], exact=False, flags=flags)


def nxt_img(rows, q):
    for p in range(q, len(rows)):
        if rows[p][1] is not None:
            return p
    return None


def report(r):
    if 'error' in r:
        return ['SM %08X error %s' % (r['addr'], r['error'])]
    L = []
    for f in r['flags']:
        L.append('SM %08X %-32s diff=%-4d img %x `%s`  ours %x %-9s -> %-9s %s blocks %+d (image %s)%s' % (
            r['addr'], r['sym'][:32], r['diff'], f['img'], f['img_ins'], f['ours'], f['our_line'], f['to_line'],
            'LATER' if f['later'] else 'EARLIER', f['crossed'], f['img_blocks'],
            ('  CALL ' + f['call']) if f['call'] else ''))
    return L


def run_unit(unit, addrs, src, pfx, both):
    text = open(src, encoding='latin-1').read()
    c, cod = tailscan.compile_listing(unit, src, pfx)
    if c is None:
        return [dict(addr=a, error='COMPILE FAIL') for a in addrs]
    out = []
    for a in addrs:
        try:
            out.append(scan(unit, a, text, c, cod, both))
        except Exception as e:  # one bad marker must not stop a tree pass
            out.append(dict(addr=a, error=repr(e)))
    for r in out:
        r['unit'] = unit
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('unit', nargs='?')
    ap.add_argument('addr', nargs='?')
    ap.add_argument('--src')
    ap.add_argument('--pfx', default=os.environ.get('FE_PFX_BASE', '8'))
    ap.add_argument('--tree', nargs='*')
    ap.add_argument('--from', dest='frm')
    ap.add_argument('--all', action='store_true', help='include BYTE_EXACT markers (validation)')
    ap.add_argument('--both', action='store_true', help='also list stores the image emits earlier')
    ap.add_argument('--json')
    a = ap.parse_args()
    if a.tree is not None or a.frm:
        work = {}
        if a.frm:
            for r in json.load(open(a.frm)):
                if not r.get('exact'):
                    work.setdefault(r['unit'] if r['unit'].startswith('src/') else 'src/' + r['unit'], []).append(int(str(r['addr']), 16))
        else:
            cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
            units = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/')]
            if a.tree:
                units = [u for u in units if u in a.tree or os.path.basename(u) in a.tree or u[4:] in a.tree]
            for u in units:
                try:
                    t = open(fe.R + '/' + u, encoding='latin-1').read()
                except OSError:
                    continue
                ad = [x for x in tailscan.markers(t, a.all) if own_context(t, x, u)]
                if ad:
                    work[u] = ad
        work = sorted(work.items())
        jobs = int(os.environ.get('FE_JOBS', '5'))
        fe.compile_cmd(work[0][0])
        base = int(a.pfx)

        def lane(k):
            r = []
            for i in range(k, len(work), jobs):
                u, ad = work[i]
                r += run_unit(u, ad, fe.R + '/' + u, str(base + k), a.both)
            return r
        allr = []
        with ThreadPoolExecutor(jobs) as ex:
            for o in ex.map(lane, range(jobs)):
                allr += o
    else:
        unit = a.unit if a.unit.startswith('src/') else 'src/' + a.unit
        allr = run_unit(unit, [int(x, 16) for x in a.addr.split(',')], a.src or fe.R + '/' + unit, a.pfx, a.both)
    allr.sort(key=lambda r: (r.get('diff', 0), r['addr']))
    nf = 0
    for r in allr:
        for l in report(r):
            print(l)
        nf += bool(r.get('flags'))
    print('functions with flagged stores: %d of %d' % (nf, len(allr)))
    if a.json:
        json.dump(allr, open(a.json, 'w'), indent=0)


if __name__ == '__main__':
    main()
