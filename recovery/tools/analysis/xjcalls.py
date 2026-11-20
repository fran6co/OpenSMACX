#!/usr/bin/env python3
"""xjcalls.py <unit> <0xADDR[,0xADDR]> [--src ALT.cpp] [--pfx N] [--all-callees]
       xjcalls.py --rungs [RUNGS.json] [--only 2] [--json OUT]
Cross-jump screen for rung-2 call-count differences (MATCHING_GUIDE 3.7 "A written-out copy of a
shared exit").  For every callee whose call count differs between the image and ours
(analysis/callees.py), each call site on each side is printed with
  in   the jumps that land in its straight-line run up to WIN=12 instructions before the call (`j` unconditional,
       `c` conditional, with how many instructions before the call they land: j@-2 = lands 2
       instructions before the call, i.e. after the pushes of a cross-jumped call),
  post jumps that land after the call inside the same run (a partial cross-jump of a later copy),
  ft   whether the run is entered by falling through,
  ->   its continuation (first `jmp` target, or `ret`), named by our source line.
A shared tail in the image shows as fewer sites with several `in` jumps each; written-out copies in
ours show as more sites with the same continuation and no `in` jumps.  The verdict column reads
  IMG-SHARES   image has fewer sites and more jump-ins: replace our copies with a goto to one exit
  OURS-SHARES  the reverse: ours merges what the image keeps (a register/scratch difference before
               the call usually, MATCHING_GUIDE 3.7 "Cross-jumping merges the longest suffix"), or a
               source goto the image does not have
  SITES        the counts differ without a jump-in difference (inline/out-of-line, a missing call)
--rungs: every function whose first differing rung is 2 in build/recovery/rungs/rungs.json
(plus broken claims at rung 2).  Screen only; confirm with `recover measure`."""
import sys, os, re, json, argparse, collections
H = os.path.dirname(os.path.abspath(__file__))
T = os.path.dirname(H)
sys.path.insert(0, T)
sys.path.insert(0, H)
import fe
import callees
import tailscan

REGS = {'eax', 'ebx', 'ecx', 'edx', 'esi', 'edi', 'ebp', 'esp'}
WIN = 12  # jump-ins counted up to this many instructions before the call


def img_name(ins_bytes, op, lo, hi):
    a2s, imp = callees.addr2sym(), callees.iat()
    if ins_bytes[0] == 0xE8:
        t = int(op, 16)
        if lo <= t < hi:
            return None
        return a2s.get(t, 'sub_%x' % t)
    if ins_bytes[0] == 0xFF and ins_bytes[1] == 0x15:
        import struct
        slot = struct.unpack_from('<I', ins_bytes, 2)[0]
        return imp.get(slot, a2s.get(slot, callees._OBJ.get(slot, 'iat_%x' % slot)))
    return '<indirect>'


def sites(I, names):
    """I: [(off, text, size, tgt, masked)], names: {index: callee} -> {callee: [site dict]}"""
    idx = {x[0]: i for i, x in enumerate(I)}
    into = collections.defaultdict(list)  # target index -> [(jumper index, kind)]
    for j, x in enumerate(I):
        if x[3] is not None and x[3] in idx and x[1].startswith('j'):
            into[idx[x[3]]].append((j, 'j' if x[1].startswith('jmp') else 'c'))
    out = collections.defaultdict(list)
    for p, nm in names.items():
        s = p
        while s > 0 and not tailscan.is_uncond(I[s - 1][1]):
            s -= 1
        ins = []
        for q in range(max(s, p - WIN), p + 1):
            for j, k in into.get(q, []):
                ins.append('%s@-%d' % (k, p - q))
        e = p + 1
        post = 0
        while e < len(I) and not tailscan.is_uncond(I[e][1]) and not I[e][1].startswith('j'):
            post += len(into.get(e, []))
            e += 1
        if e < len(I) and I[e][1].startswith('jmp') and I[e][3] is not None:
            post += len(into.get(e, []))
            cont = ('J', I[e][3])
        elif e < len(I) and I[e][1].startswith('ret'):
            cont = ('R', None)
        elif e < len(I):
            cont = ('C', I[e][0])  # a conditional branch ends the run
        else:
            cont = ('E', None)
        ft = s > 0 and not tailscan.is_uncond(I[s - 1][1])
        out[nm].append(dict(off=I[p][0], ins=ins, post=post, cont=cont, start=I[s][0]))
    return out


def analyse(unit, addr, text, c, cod, all_callees=False):
    r = fe.evaluate(c, text, addr)
    if 'error' in r:
        return ['XJ %08X %s' % (addr, r['error'])], None
    sym = r['sym']
    _, lo, hi = fe.ident(text, addr)
    s = c.function(sym)
    sec = c.sections[s.section - 1]
    Cs = fe.dis(sec.data, 0, sec.relocations, selfsec=s.section, syms=c.symbols)
    Os = fe.imgdis(lo, hi)
    C = tailscan.named(sec.data, 0, sec.relocations, c.symbols, Cs)
    O = tailscan.image_named(lo, hi, Os)
    src = tailscan.lines_of(cod, sym, os.path.basename(unit).lower())
    i2o = {}
    for k_, o, c_ in r['rows']:
        if k_ != '!!' and o and c_:
            i2o[o[0]] = c_[0]
    ali = sorted(i2o)

    def iline(off):
        best = None
        for io in ali:
            if io <= off:
                best = io
            else:
                break
        return ('~' + src.get(i2o[best], '?')) if best is not None else '~?'
    # names per call instruction
    ibytes = {i.address - lo: i for i in fe.md.disasm(fe.imgbytes(lo, hi), lo)}
    inames = {}
    for n, x in enumerate(O):
        if x[1].startswith('call'):
            ins = ibytes[x[0]]
            nm = img_name(ins.bytes, ins.op_str, lo, hi)
            if nm:
                inames[n] = nm
    onames = {}
    for n, x in enumerate(C):
        if x[1].startswith('call'):
            t = x[1][5:].strip()
            if t == 'L':
                continue
            if t.startswith('dword ptr') or not re.match(r'[?_A-Za-z]', t) or t.startswith('e'):
                m = re.fullmatch(r'dword ptr \[([?_A-Za-z][^\s\]+]*)\]', t)
                # `call [sym]`: an import or a data object called through (callees.py names both)
                nm = callees.norm_ours(m.group(1)) if m and m.group(1) not in REGS else '<indirect>'
            else:
                nm = callees.shared_name(callees.norm_ours(t))
            if nm == sym:
                continue
            onames[n] = nm
    ci = collections.Counter(inames.values())
    co = collections.Counter(onames.values())
    co = callees.fold_twins(ci, co)
    diff_names = sorted(k for k in set(ci) | set(co) if ci[k] != co[k])
    if all_callees:
        diff_names = sorted(set(ci) | set(co))
    Si, So = sites(O, inames), sites(C, onames)
    out = ['XJ %08X %s rows=%d' % (addr, sym[:60], r['diff'])]
    verdicts = []

    def cont_s(cn, ours):
        if cn[0] == 'R':
            return 'ret'
        if cn[0] == 'E':
            return 'end'
        if cn[0] == 'C':
            return 'jcc'
        return '->%s' % (src.get(cn[1], '?') if ours else iline(cn[1]))
    for nm in diff_names:
        si, so = Si.get(nm, []), So.get(nm, [])
        ji = sum(len([k for k in x['ins'] if k[0] == 'j']) + x['post'] for x in si)
        jo = sum(len([k for k in x['ins'] if k[0] == 'j']) + x['post'] for x in so)
        if len(si) < len(so) and ji > jo:
            v = 'IMG-SHARES'
        elif len(si) > len(so) and jo > ji:
            v = 'OURS-SHARES'
        else:
            v = 'SITES'
        verdicts.append(v)
        out.append('  %-11s %-44s img %d (jmp-ins %d)  ours %d (jmp-ins %d)' % (v, nm[:44], len(si), ji, len(so), jo))
        li = collections.Counter(iline(x['off'])[1:] for x in si)
        lo_ = collections.Counter(src.get(x['off'], '?') for x in so)
        dl = ['%s %d/%d' % (k, li[k], lo_[k]) for k in sorted(set(li) | set(lo_)) if li[k] != lo_[k]]
        if dl:
            out.append('      lines img/ours: ' + ' '.join(dl))
        for side, L, ours in (('img ', si, False), ('ours', so, True)):
            for x in L:
                where = ('%5x %-10s' % (x['off'], src.get(x['off'], '?'))) if ours else (
                    '%x %-10s' % (lo + x['off'], iline(x['off'])))
                out.append('      %s %s in[%s] post=%d %s' % (side, where, ' '.join(x['ins']), x['post'],
                                                           cont_s(x['cont'], ours)))
    return out, dict(sym=sym, rows=r['diff'], verdicts=verdicts)


def run_unit(unit, addrs, src, pfx, all_callees):
    text = open(src, encoding='latin-1').read()
    c, cod = tailscan.compile_listing(unit, src, pfx)
    if c is None:
        return [(['XJ COMPILE FAIL ' + unit], None)]
    res = []
    for a in addrs:
        try:
            res.append(analyse(unit, a, text, c, cod, all_callees))
        except Exception as e:
            res.append((['XJ %08X error %r' % (a, e)], None))
    return res


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('unit', nargs='?')
    ap.add_argument('addr', nargs='?')
    ap.add_argument('--src')
    ap.add_argument('--pfx', default=os.environ.get('FE_PFX_BASE', '21'))
    ap.add_argument('--all-callees', action='store_true')
    ap.add_argument('--rungs', nargs='?', const=fe.R + '/build/recovery/rungs/rungs.json')
    ap.add_argument('--only', default='2')
    ap.add_argument('--json')
    a = ap.parse_args()
    if a.rungs:
        from concurrent.futures import ThreadPoolExecutor
        rows = json.load(open(a.rungs))
        want = {int(x) for x in a.only.split(',')}
        by = collections.defaultdict(list)
        for x in rows:
            if x.get('exact') or x.get('rung') not in want:
                continue
            by[x['unit']].append(int(x['addr'], 16))
        work = sorted(by.items())
        jobs = int(os.environ.get('FE_JOBS', '5'))
        base = int(a.pfx)
        fe.compile_cmd(work[0][0])

        def lane(k):
            res = []
            for i in range(k, len(work), jobs):
                u, ad = work[i]
                res += run_unit(u, ad, fe.R + '/' + u, str(base + k), a.all_callees)
            return res
        allr = []
        with ThreadPoolExecutor(jobs) as ex:
            for o in ex.map(lane, range(jobs)):
                allr += o
        allr.sort(key=lambda r: -(r[1] or {}).get('rows', 0))
        for lines, _ in allr:
            print('\n'.join(lines))
        if a.json:
            json.dump([dict(lines=l, **(c or {})) for l, c in allr], open(a.json, 'w'), indent=0)
        return 0
    unit = a.unit if a.unit.startswith('src/') else 'src/' + a.unit
    src = os.path.abspath(a.src) if a.src else fe.R + '/' + unit
    for lines, _ in run_unit(unit, [int(x, 16) for x in a.addr.split(',')], src, a.pfx, a.all_callees):
        print('\n'.join(lines))
    return 0


if __name__ == '__main__':
    sys.exit(main())
