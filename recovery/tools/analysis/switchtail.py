#!/usr/bin/env python3
"""switchtail.py <unit> <addr[,addr]> [--src ALT.cpp] | --from FEALL.json [--units u,..] [--json F]
Shared switch-tail screen (MATCHING_GUIDE 3.7, "A switch arm that falls into a shared tail ... is a
flag-tested tail after the switch").

For every jump-table dispatch (casescan.dispatches) on both sides it splits the code after the table's
lowest target into case regions (each region starts at a case target and runs to the next one), then
finds the joins: addresses that two or more case regions reach by an unconditional `jmp`. For each join
it names the HOST, the region whose code falls through into the join (or `-` when the join follows a
jmp/ret, i.e. a free-standing block), with the case values of that region, and matches image joins to
ours through the fe alignment. A line is printed when the two sides disagree:

  HOST    same join, the image falls into it from another case than ours (the image's host case
          values first). `img-first`: the image's host is the earliest participating case and ours a
          later one: the on_key_click shape (ours wrote `goto label` with the label after one case; the
          original set a flag / `break` in each case and tested the tail after the switch).
  NJOIN   a join with N jumpers on one side has another count (or no counterpart) on the other: a
          tail one side duplicated or merged.
Screen only; read each hit with fe.py --all / jseq.py before editing."""
import sys, os, json, argparse, bisect
S = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, S)
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import fe, casescan, struct


def joins(ins, d, end):
    """-> {J: dict(jumpers=[region idx], host=region idx or None)}; regions are the table's distinct targets"""
    starts = sorted(set(d['targets']) | ({d['dflt']} if d['dflt'] is not None else set()))
    vals = {}
    for i, t in enumerate(d['targets']):
        vals.setdefault(t, []).append(i + d['bias'])
    lo0 = starts[0]
    reg = lambda a: bisect.bisect_right(starts, a) - 1
    J = {}
    pos = {x[0]: k for k, x in enumerate(ins)}
    # the last region ends at its first jmp/ret (the code after it is not a case of this table)
    k = pos.get(starts[-1])
    if k is not None:
        while k < len(ins) and ins[k][1] != 'jmp' and not ins[k][1].startswith('ret'): k += 1
        end = min(end, ins[k][0] + 1 if k < len(ins) else end)
    for k, (a, mn, op, sz) in enumerate(ins):
        if a < lo0 or a >= end: continue
        if mn == 'jmp':
            try: t = int(op, 0)
            except ValueError: continue
            if t in starts and t != d['dflt']: continue
            J.setdefault(t, set()).add(reg(a))
    out = {}
    for t, rs in J.items():
        if len(rs) < 2: continue
        k = pos.get(t)
        host = None
        if k is not None and k > 0:
            pa, pm, po, ps = ins[k - 1]
            if pm != 'jmp' and not pm.startswith('ret') and pa >= lo0:
                host = reg(pa)
        out[t] = dict(jumpers=sorted(rs), host=host)
    names = [vals.get(s, ['dflt'] if s == d['dflt'] else ['?']) for s in starts]
    return out, starts, names


def scan(c, text, addr):
    idn = fe.ident(text, addr)
    if not idn: return dict(error='no marker')
    sym, lo, hi = idn
    s = c.function(sym)
    sec = c.sections[s.section - 1]; data = sec.data
    rel = {r[0]: r for r in sec.relocations}
    res = fe.evaluate(c, text, addr)
    if 'error' in res: return res
    omap = {}
    for k, o, cc in res['rows']:
        if o is not None and cc is not None: omap[o[0]] = cc[0]
    ib = fe.imgbytes(lo, hi + 0x2000)
    iins = [(a - lo, m, o, z) for a, m, o, z in casescan.raw_insns(fe.imgbytes(lo, hi), lo)]
    # image operands carry absolute addresses: rebase jump operands
    def rb(x):
        a, m, o, z = x
        if m[0] == 'j' and '[' not in o:
            try: return (a, m, hex(int(o, 0) - lo), z)
            except ValueError: pass
        return x
    iins = [rb(x) for x in iins]
    def i_rd32(off):
        return struct.unpack_from('<I', ib, off)[0] - lo if 0 <= off <= len(ib) - 4 else None
    def i_rd8(off): return ib[off] if 0 <= off < len(ib) else 0
    def i_start(a, sz, op):
        b = fe.imgbytes(lo + a, lo + a + sz)
        return struct.unpack_from('<I', b, sz - 4)[0] - lo
    icode = {x[0] for x in iins}
    idis = casescan.dispatches(iins, i_rd32, i_rd8, i_start, lambda t: t in icode)
    def o_sym(off):
        r = rel.get(off)
        if not r: return None
        sy = c.symbols.get(r[1])
        if sy is None or sy.section != s.section: return None
        return sy.value + struct.unpack_from('<I', data, off)[0]
    oend = casescan.res_end(res)
    oins = [x for x in casescan.raw_insns(data, 0) if x[0] < oend]
    ocode = {x[0] for x in oins}
    odis = casescan.dispatches(oins, lambda o: o_sym(o) if 0 <= o <= len(data) - 4 else None,
                               lambda o: data[o] if 0 <= o < len(data) else 0,
                               lambda a, sz, op: o_sym(a + sz - 4), lambda t: t in ocode)
    out = dict(sym=sym, addr='%08X' % addr, diff=res['diff'], finds=[])
    used = set()
    for d in idis:
        e = None
        if d['at'] in omap: e = next((x for x in odis if x['at'] == omap[d['at']]), None)
        if e is None: e = next((x for x in odis if id(x) not in used and x['n'] == d['n']), None)
        if e is None: continue
        used.add(id(e))
        ji, si, ni = joins(iins, d, hi - lo)
        jo, so, no = joins(oins, e, oend)
        tag = 'table %x' % (lo + d['at'])
        seen = set()
        def cv(names, i): return 'L' if i is None else ','.join(map(str, names[i][:4])) + ('..' if len(names[i]) > 4 else '')
        for t, x in sorted(ji.items()):
            ot = omap.get(t)
            y = jo.get(ot)
            if y is None:
                # unaligned join: pair it with our free join whose jumpers carry the most shared case values
                iv = {v for r in x['jumpers'] + [x['host']] if r is not None for v in ni[r]}
                best = None
                for t2, y2 in jo.items():
                    if t2 in seen: continue
                    ov = {v for r in y2['jumpers'] + [y2['host']] if r is not None for v in no[r]}
                    sc = len(iv & ov) / max(1, len(iv | ov))
                    if sc >= 0.5 and (best is None or sc > best[0]): best = (sc, t2)
                if best: ot = best[1]; y = jo[ot]
            if y is None:
                out['finds'].append(('NJOIN', '%s join img %x (%d jumpers, host %s) has no our join (aligned %s)' % (
                    tag, lo + t, len(x['jumpers']), cv(ni, x['host']), '%x' % ot if ot is not None else '-')))
                continue
            seen.add(ot)
            # compare host by case values
            hi_ = cv(ni, x['host']) if x['host'] is not None else '-'
            ho_ = cv(no, y['host']) if y['host'] is not None else '-'
            if hi_ != ho_:
                part = sorted(set(x['jumpers']) | ({x['host']} if x['host'] is not None else set()))
                first = x['host'] is not None and x['host'] == part[0]
                opart = sorted(set(y['jumpers']) | ({y['host']} if y['host'] is not None else set()))
                ofirst = y['host'] is not None and y['host'] == opart[0]
                out['finds'].append(('HOST', '%s join img %x / ours %x: image host case %s, ours %s%s; jumpers img %d ours %d' % (
                    tag, lo + t, ot, hi_, ho_, ' img-first' if first and not ofirst else '', len(x['jumpers']), len(y['jumpers']))))
            elif len(x['jumpers']) != len(y['jumpers']):
                out['finds'].append(('NJOIN', '%s join img %x / ours %x: jumpers img %d ours %d (host %s)' % (
                    tag, lo + t, ot, len(x['jumpers']), len(y['jumpers']), hi_)))
        for t, y in sorted(jo.items()):
            if t not in seen:
                out['finds'].append(('NJOIN', '%s our join %x (%d jumpers, host %s) has no image join' % (
                    tag, t, len(y['jumpers']), cv(no, y['host']))))
    return out


def main():
    ap = argparse.ArgumentParser(); ap.add_argument('unit', nargs='?'); ap.add_argument('addr', nargs='?')
    ap.add_argument('--src'); ap.add_argument('--from', dest='frm'); ap.add_argument('--json'); ap.add_argument('--units', default='')
    a = ap.parse_args()
    work = {}
    if a.frm:
        for r in json.load(open(a.frm)):
            if r.get('exact') or r['addr'] == '?': continue
            if a.units and os.path.basename(r['unit']) not in a.units.split(','): continue
            work.setdefault(r['unit'], []).append(int(r['addr'], 16))
    else:
        work[casescan.unitpath(a.unit)] = [int(x, 16) for x in a.addr.split(',')]
    allres = []
    for u, addrs in sorted(work.items()):
        src = os.path.abspath(a.src) if a.src else fe.R + '/' + u
        text = open(src, encoding='latin-1').read()
        c, err = fe.compile_unit(u, src, fe.PFX)
        if c is None: print('COMPILE FAIL', u, err); continue
        for ad in addrs:
            try: r = scan(c, text, ad)
            except Exception as ex: r = dict(error=repr(ex)[:120])
            r['unit'] = u; allres.append(r)
            if 'error' in r:
                if not a.frm: print('ERR %08X %s' % (ad, r['error']))
                continue
            if not r['finds']: continue
            print('%08X %-40s %s diff=%d' % (ad, r['sym'][:40], os.path.basename(u), r['diff']))
            for k, msg in r['finds']: print('   %-5s %s' % (k, msg))
    if a.json: json.dump(allres, open(a.json, 'w'), indent=0)


if __name__ == '__main__': main()
