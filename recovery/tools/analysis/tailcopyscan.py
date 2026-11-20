#!/usr/bin/env python3
"""tailcopyscan.py <unit> <0xADDR[,0xADDR]>|--all [--src ALT.cpp] [--pfx N] [--list]
       tailcopyscan.py --tree [unit ...] [--from FEALL.json] [--json OUT] [--list]
Join-copy (tail duplication) screen, MATCHING_GUIDE 3.7 "Ours merges a tail that the image keeps".

VC6 copies a join block into a jumping arm when the join's code before its final `jmp` takes at
most 15 bytes (par-5 round 41, analysis/probes/tailcopy).  So a copy that one side makes and the
other does not is a SIZE disagreement of the join, not of the arms:

  IMG-COPIES   the image has two identical runs, each ending in `jmp T` (or one falling into T);
               ours has one of them and reaches it by a `jmp` from the other arm.  Ours' join is
               bigger than the image's copy when ours > 15 and the image <= 15: find the source fact
               that shrinks the original's join (a register-held value instead of a memory reload,
               a push of a register, a folded constant, one call argument less).
  OURS-COPIES  the reverse: ours copies, the image joins.  The image's join is the bigger one.

Per disagreement it prints both sides' join code with each instruction's size, the copy's bytes
(before its final jmp) and the joining side's block bytes (from the label its arms jump to up to
its final jmp).  Sizes over 15 on the copying side mean the copy is not a size-rule join copy
(a written-out double, a cross-jump that was refused, or the run includes arm code).
--list prints the instructions.  Screen only; confirm with fe.py and `recover measure`."""
import sys, os, re, json, argparse
from concurrent.futures import ThreadPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.dirname(HERE))
import fe
import tailscan as ts

LIMIT = 15


def copy_clusters(I):
    """[(k, target, [(a, e, kind)])]: identical runs before `jmp T` (kind 'J') or falling into T ('F')"""
    out = []
    for k, cn, copies in ts.dups(I, 1):
        if cn[0] != 'J':
            continue
        # a copied join is one block: cut the run after its last conditional jump or inner label
        cut = 0
        a0, e0 = copies[0]
        for n in range(a0, e0):
            if I[n][1].startswith(('j', 'loop')):
                cut = n - a0 + 1
        if cut >= e0 - a0:
            continue
        k -= cut
        cs = []
        for a, e in copies:
            cs.append((a + cut, e, 'J' if I[e][1] == 'jmp L' else 'F'))
        if sum(1 for c in cs if c[2] == 'J') < 1 or len(cs) < 2:
            continue
        # a run that is only a pending pop or one push is not a join copy worth reporting
        out.append((k, cn[1], cs))
    return out


def nbytes(I, a, e):
    return sum(x[2] for x in I[a:e])


def other_join(I, J, idx_of, amap, a, e):
    """the joining side's block for a present copy (a, e) of the copying side: its aligned rows in J,
    up to J's next unconditional instruction, entered by a `jmp` whose target lies in that straight
    run (the nearest such label at or before the copy's first aligned row).
    -> (j_start_idx, j_end_idx) or None"""
    qs = [idx_of[amap[x[0]]] for x in I[a:e + 1] if x[0] in amap and amap[x[0]] in idx_of]
    if not qs:
        return None
    q = min(qs)
    qe = q
    while qe < len(J) and not ts.is_uncond(J[qe][1]):
        qe += 1
    if qe >= len(J) or J[qe][1] != 'jmp L':
        return None
    jidx = {x[0]: n for n, x in enumerate(J)}
    targets = {jidx[x[3]] for x in J if x[1] == 'jmp L' and x[3] in jidx}
    s = q
    lo_ = q
    while lo_ > 0 and not ts.is_uncond(J[lo_ - 1][1]) and q - lo_ < 40:
        lo_ -= 1
    cand = [t for t in targets if lo_ <= t <= q]
    if cand:
        s = max(cand)
    else:
        cand = [t for t in targets if q < t < qe]
        if not cand:
            return None
        s = min(cand)
    return s, qe


def scan(unit, addr, text, c, cod, listing=False):
    r = fe.evaluate(c, text, addr)
    if 'error' in r:
        return ['TC %08X %s' % (addr, r['error'])], []
    sym = r['sym']
    _, lo, hi = fe.ident(text, addr)
    s = c.function(sym)
    sec = c.sections[s.section - 1]
    Cs = fe.dis(sec.data, 0, sec.relocations, selfsec=s.section, syms=c.symbols)
    Os = fe.imgdis(lo, hi)
    C = ts.named(sec.data, 0, sec.relocations, c.symbols, Cs)
    O = ts.image_named(lo, hi, Os)
    src = ts.lines_of(cod, sym, os.path.basename(unit).lower())
    i2o, o2i = {}, {}
    for k_, o, c_ in r['rows']:
        if k_ != '!!' and o and c_:
            i2o[o[0]] = c_[0]
            o2i[c_[0]] = o[0]
    cidx = {x[0]: n for n, x in enumerate(C)}
    oidx = {x[0]: n for n, x in enumerate(O)}

    def iline(off):
        best = None
        for io in sorted(i2o):
            if io <= off:
                best = io
            else:
                break
        return ('~' + src.get(i2o[best], '?')) if best is not None else '~?'

    def lines(I, a, e, ours):
        ls = []
        for x in I[a:e + 1]:
            l = src.get(x[0], '?') if ours else iline(x[0])
            if l not in ls:
                ls.append(l)
        return ','.join(ls[:4]) + (',..' if len(ls) > 4 else '')

    out, rows = [], []
    for side, I, J, amap, jdx, ours in (('img', O, C, i2o, cidx, False), ('ours', C, O, o2i, oidx, True)):
        org = 0 if ours else lo
        for k, tgt, cs in copy_clusters(I):
            fr = [sum(1 for x in I[a:e] if x[0] in amap) / max(1, e - a) for a, e, _ in cs]
            miss = [c_ for c_, f in zip(cs, fr) if f < 0.5]
            if not miss or len(miss) == len(cs):
                continue
            cb = nbytes(I, cs[0][0], cs[0][1])
            # the joining side's block, through the first missing copy
            jb = None
            for (a, e, _), f in zip(cs, fr):
                if f >= 0.5:
                    jb = other_join(I, J, jdx, amap, a, e)
                    if jb:
                        break
            if jb is None:
                continue
            js, je = jb
            jbytes = nbytes(J, js, je)
            jorg = lo if ours else 0
            tag = 'OURS-COPIES' if ours else 'IMG-COPIES '
            # the copy must be the joining block's tail (masked text), or the pair is not one join
            a0, e0 = cs[0][0], cs[0][1]
            m = 0
            while m < e0 - a0 and je - 1 - m >= js and I[e0 - 1 - m][4] == J[je - 1 - m][4]:
                m += 1
            flag = ''
            if ours and cb <= LIMIT < jbytes:
                flag = 'SIZE (image join %d > %d, ours copy %d)' % (jbytes, LIMIT, cb)
            elif not ours and cb <= LIMIT < jbytes:
                flag = 'SIZE (ours join %d > %d, image copy %d)' % (jbytes, LIMIT, cb)
            elif cb > LIMIT:
                flag = 'copy over limit (%d): not a size-rule copy' % cb
            if flag.startswith('SIZE') and m < e0 - a0:
                flag = 'tail differs (%d of %d match): not one join' % (m, e0 - a0)
            elif flag.startswith('SIZE') and jbytes > 3 * LIMIT:
                flag = 'join %dB: a straight run, not a copyable join' % jbytes
            out.append('  %s %s copies x%d k=%d copy=%dB other join=%dB @%x [%s] -> %s' % (
                tag, side, len(cs), k, cb, jbytes, jorg + J[js][0],
                lines(J, js, je, not ours), flag))
            for (a, e, kd), f in zip(cs, fr):
                out.append('        %s @%x %s %s' % ('MISSING' if f < 0.5 else 'present', org + I[a][0], kd,
                                                     lines(I, a, e, ours)))
            if listing:
                out.append('        copy (%s):' % side)
                for x in I[cs[0][0]:cs[0][1] + 1]:
                    out.append('          %2d  %s' % (x[2], x[1]))
                out.append('        join (%s):' % ('img' if ours else 'ours'))
                for x in J[js:je + 1]:
                    out.append('          %2d  %s' % (x[2], x[1]))
            rows.append(dict(side=side, k=k, copy=cb, join=jbytes, flag=flag))
    head = 'TC %08X %-34s diff=%d copies=%d size=%d' % (addr, sym[:34], r['diff'], len(rows),
                                                        sum(1 for x in rows if x['flag'].startswith('SIZE')))
    return [head] + out, rows


def run_unit(unit, addrs, src, pfx, listing):
    text = open(src, encoding='latin-1').read()
    c, cod = ts.compile_listing(unit, src, pfx)
    if c is None:
        return [(['COMPILE FAIL ' + unit], [])]
    res = []
    for a in addrs:
        try:
            res.append(scan(unit, a, text, c, cod, listing))
        except Exception as e:
            res.append((['TC %08X error %s' % (a, e)], []))
    return res


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('unit', nargs='?')
    ap.add_argument('addr', nargs='?')
    ap.add_argument('--all', action='store_true')
    ap.add_argument('--tree', nargs='*')
    ap.add_argument('--from', dest='frm')
    ap.add_argument('--src')
    ap.add_argument('--pfx', default=os.environ.get('FE_PFX_BASE', '21'))
    ap.add_argument('--list', action='store_true')
    ap.add_argument('--json')
    a = ap.parse_args()
    if a.tree is not None:
        work = {}
        if a.frm:
            for x in json.load(open(a.frm)):
                if not x.get('exact'):
                    work.setdefault(x['unit'], []).append(int(x['addr'], 16))
        else:
            cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
            for e in cc:
                if not e['file'].startswith(fe.MAIN + '/src/'):
                    continue
                u = e['file'][len(fe.MAIN) + 1:]
                try:
                    t = open(fe.R + '/' + u, encoding='latin-1').read()
                except OSError:
                    continue
                ad = ts.markers(t)
                if ad:
                    work[u] = ad
        if a.tree:
            work = {u: v for u, v in work.items() if u in a.tree or os.path.basename(u) in a.tree}
        work = sorted(work.items())
        jobs = int(os.environ.get('FE_JOBS', '5'))
        fe.compile_cmd(work[0][0])
        base = int(a.pfx)

        def lane(k):
            r = []
            for i in range(k, len(work), jobs):
                u, ad = work[i]
                for lines, rows in run_unit(u, ad, fe.R + '/' + u, str(base + k), a.list):
                    if rows:
                        r.append(dict(unit=u, lines=lines, rows=rows))
            return r
        allr = []
        with ThreadPoolExecutor(jobs) as ex:
            for o in ex.map(lane, range(jobs)):
                allr += o
        allr.sort(key=lambda x: -sum(1 for y in x['rows'] if y['flag'].startswith('SIZE')))
        for x in allr:
            print(x['unit'][4:], '\n'.join(x['lines']))
        print('functions with a copy disagreement', len(allr), 'size-rule rows',
              sum(1 for x in allr for y in x['rows'] if y['flag'].startswith('SIZE')))
        if a.json:
            json.dump(allr, open(a.json, 'w'), indent=0)
        return
    unit = a.unit if a.unit.startswith('src/') else 'src/' + a.unit
    src = os.path.abspath(a.src) if a.src else fe.R + '/' + unit
    text = open(src, encoding='latin-1').read()
    addrs = ts.markers(text) if a.all else [int(x, 16) for x in a.addr.split(',')]
    for lines, _ in run_unit(unit, addrs, src, a.pfx, a.list):
        print('\n'.join(lines))


if __name__ == '__main__':
    main()
