#!/usr/bin/env python3
"""ivscan.py <unit> <0xADDR[,..]> [--src ALT.cpp] [--pfx N] | --from FEALL.json [--json F] [--min-rows N]
Pointer / byte-offset induction census of both sides (MATCHING_GUIDE 2.4, 2.7 "Strength reduction is a
candidate-set question"; setup_game 0x005B3920).  An induction step is, inside a natural loop:
  add|sub R, K   lea R, [R +- K]   inc|dec R used as a base   add|sub dword ptr [slot], K
where R (any register but esp) is used as an address base or index `[R ...]`/`[... + R*s]` somewhere in a
loop containing the step, or compared against a relocated address (`cmp R, <r>`).  A slot step counts when the
slot is loaded into a register that is then used as an address within the next few instructions.
Each side's steps form a multiset keyed by (sign, stride, form: reg|slot); the difference is printed:
  IMG-ONLY  the image steps a pointer ours does not (a pointer the original wrote beside an index read in an
            inner loop, or a pointer the original declared: write `T *p = &A[k]; ... k++, p++`);
  OUR-ONLY  ours steps a pointer the image does not (an invented pointer local, or VC6 strength-reduced our
            index because the original read it inside an inner loop / indexed per use).
Sites carry image addresses and our /FAsc source lines.  Tree mode runs every non-exact function of an
feall.py --out JSON on FE_JOBS lanes from FE_PFX_BASE.  Screen only: confirm with fe.py / recover measure."""
import sys, os, re, argparse, collections, json
from concurrent.futures import ThreadPoolExecutor
H = os.path.dirname(os.path.abspath(__file__))
T = os.path.dirname(H)
sys.path.insert(0, T)
sys.path.insert(0, H)
import fe
import tailscan
from candset import Code, image_tables

LATCH_ONLY = os.environ.get('IV_ANYBLOCK') is None  # steps in latch blocks only (pointer IVs live in the increment)
REGS = ('eax', 'ecx', 'edx', 'ebx', 'esi', 'edi', 'ebp')
NUM = r'(0x[0-9a-f]+|\d+)'


def cod_tables(cod, sym):
    """our switch tables from the /FAsc listing: {offset of the jmp: [case offsets]}"""
    i = cod.find(sym + '\tPROC')
    if i < 0:
        i = cod.find(sym + ' PROC')
    e = cod.find(sym + '\tENDP', i)
    if e < 0:
        e = cod.find(sym + ' ENDP', i)
    if i < 0:
        return {}
    labels, tables, pend, cur, jmps = {}, collections.defaultdict(list), [], None, {}
    pendoff = None
    for l in cod[i:e].split('\n'):
        l = l.rstrip('\r')
        m = re.match(r'(\$L\d+):$', l)
        if m:
            pend.append(m.group(1))
            continue
        m = re.search(r'\bDD\s+(\$L\d+)', l)
        if m:
            if pend:
                cur = pend[-1]
                pend = []
            tables[cur].append(m.group(1))
            continue
        m = re.match(r'  ([0-9a-f]{5})\t([0-9a-f ]+)(?:\t+ ?(.*))?$', l)
        txt = None
        if m:
            off = int(m.group(1), 16)
            if m.group(3):
                txt = m.group(3)
            else:
                pendoff = off
                continue
        else:
            m = re.match(r'\t[0-9a-f ]+\t+ ?(\S.*)$', l)
            if m and pendoff is not None:
                off, txt, pendoff = pendoff, m.group(1), None
        if txt is not None:
            for p in pend:
                labels[p] = off
            pend = []
            cur = None
            mm = re.search(r'jmp\s+DWORD PTR (\$L\d+)\[', txt)
            if mm:
                jmps[off] = mm.group(1)
    return {off: [labels[x] for x in tables.get(t, ()) if x in labels] for off, t in jmps.items()}


def trim_tables(tb):
    """image_tables reads contiguous jump tables as one: cut a table where another table's entries follow"""
    out = {}
    for k, v in tb.items():
        cut = len(v)
        for k2, v2 in tb.items():
            if k2 != k and v2 and len(v2) < len(v) and v[len(v) - len(v2):] == v2:
                cut = min(cut, len(v) - len(v2))
        out[k] = v[:cut]
    return out


def uses_as_addr(t, r):
    return bool(re.search(r'\[[^\]]*\b%s\b[^\]]*\]' % r, t))


def steps(code, ins):
    """-> list of (key, instr index, text); key = (sign, stride, form)"""
    out = []
    n = len(ins)
    loops_of = {}  # block -> set of loop bodies containing it (approximated by depth>0 + same-loop scan)
    for i, (off, t, g) in enumerate(ins):
        b = code.bof[i]
        if code.depth[b] == 0:
            continue
        key = None
        m = re.fullmatch(r'(add|sub) (e[abcd]x|esi|edi|ebp), %s' % NUM, t)
        if m:
            k = int(m.group(3), 0)
            key = ('+' if m.group(1) == 'add' else '-', k, 'reg', m.group(2))
        m = m or None
        if key is None:
            m = re.fullmatch(r'lea (e[abcd]x|esi|edi|ebp), \[(e[abcd]x|esi|edi|ebp) ([+-]) %s\]' % NUM, t)
            if m and m.group(1) == m.group(2):
                key = (m.group(3), int(m.group(4), 0), 'reg', m.group(1))
        if key is None:
            m = re.fullmatch(r'(inc|dec) (e[abcd]x|esi|edi|ebp)', t)
            if m:
                key = ('+' if m.group(1) == 'inc' else '-', 1, 'reg', m.group(2))
        if key is None:
            m = re.fullmatch(r'(add|sub) dword ptr \[(ebp|esp) ([+-]) %s\], %s' % (NUM, NUM), t)
            if m and int(m.group(5), 0) >= 2:
                slot = '[%s %s %s]' % (m.group(2), m.group(3), m.group(4))
                # loaded somewhere into a register used as an address within 4 instructions
                ok = False
                for j, (o2, t2, g2) in enumerate(ins):
                    mm = re.fullmatch(r'mov (e[abcd]x|esi|edi|ebx|ebp), dword ptr \[%s %s %s\]' % (
                        m.group(2), re.escape(m.group(3)), m.group(4)), t2)
                    if mm and any(uses_as_addr(ins[q][1], mm.group(1)) for q in range(j + 1, min(n, j + 5))):
                        ok = True
                        break
                if ok and (not LATCH_ONLY or code.near_latch(b)):
                    out.append((('+' if m.group(1) == 'add' else '-', int(m.group(5), 0), 'slot'), i, t))
                continue
        if key is None:
            continue
        sign, k, form, r = key
        if k == 0 or r == 'ebp' and code.frame_ebp_flag:
            continue
        if LATCH_ONLY and not code.near_latch(b):
            continue
        # r used as an address, or compared with a relocated address, in a block of the same loop nest
        lo_b = b
        # collect blocks of the innermost loop containing b: blocks with depth >= depth[b] reachable both ways is
        # costly; use the loop bodies Code computed if exposed, else the whole function's loop blocks
        cand = [j for j in range(n) if code.depth[code.bof[j]] > 0 and code.bof[j] in code.loop_blocks_of(b)]
        used = any(uses_as_addr(ins[j][1], r) for j in cand if j != i) or any(
            re.fullmatch(r'cmp %s, <r>' % r, ins[j][1]) for j in cand)
        if not used:
            # stepped in a scratch register and written back to its frame home: the home is loaded into a register
            # that is used as an address in the loop
            for j in cand:
                mm = re.fullmatch(r'mov dword ptr (\[e[bs]p [+-] 0x[0-9a-f]+\]), %s' % r, ins[j][1])
                if not mm:
                    continue
                for q in cand:
                    m2 = re.fullmatch(r'mov (e[abcd]x|esi|edi|ebx|ebp), dword ptr %s' % re.escape(mm.group(1)), ins[q][1])
                    if m2 and any(uses_as_addr(ins[z][1], m2.group(1)) for z in range(q + 1, min(n, q + 5))):
                        used = True
                        break
                if used:
                    break
        if not used:
            continue
        if k == 1 and not any(re.search(r'byte ptr \[[^\]]*\b%s\b' % r, ins[j][1]) for j in cand):
            continue  # an inc/dec counter, not a byte pointer
        out.append(((sign, k, 'reg'), i, t))
    return out


class LCode(Code):
    """Code + per-block loop bodies"""

    def __init__(self, ins, tables, frame_ebp):
        super().__init__(ins, tables)
        self.frame_ebp_flag = frame_ebp
        n = len(ins)
        nb = len(self.starts)
        succ = [set() for _ in range(nb)]
        for b in range(nb if n else 0):
            e = (self.starts[b + 1] if b + 1 < nb else n) - 1
            for j in self.isucc[e]:
                succ[b].add(self.bof[j])
        pred = [set() for _ in range(nb)]
        for u in range(nb):
            for v in succ[u]:
                pred[v].add(u)
        # recompute loop bodies (Code only keeps depth)
        full = set(range(nb))
        dom = [full] * nb
        if nb:
            dom[0] = {0}
        ch = True
        while ch:
            ch = False
            for v in range(1, nb):
                ps = [dom[p] for p in pred[v]]
                nd = (set.intersection(*ps) if ps else set()) | {v}
                if nd != dom[v]:
                    dom[v] = nd
                    ch = True
        self.bsucc = succ
        self.bodies = []
        self.latch = set()
        for u in range(nb):
            for h in succ[u]:
                if h in dom[u]:
                    self.latch.add(u)
                    body = {h, u}
                    st = [u] if u != h else []
                    while st:
                        x = st.pop()
                        for p in pred[x]:
                            if p not in body:
                                body.add(p)
                                st.append(p)
                    self.bodies.append(body)

    def near_latch(self, b):
        """b is a latch, or falls straight into one (the for-increment block)"""
        seen = set()
        while b not in seen:
            seen.add(b)
            if b in self.latch:
                return True
            if len(self.bsucc[b]) != 1:
                return False
            b = next(iter(self.bsucc[b]))
        return False

    def loop_blocks_of(self, b):
        """blocks of the innermost loop containing b"""
        bs = [x for x in self.bodies if b in x]
        return min(bs, key=len) if bs else set()


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
    ebp_i = any(t == 'mov ebp, esp' for _, t, _ in iins[:6])
    ebp_o = any(t == 'mov ebp, esp' for _, t, _ in oins[:6])
    icode = LCode(iins, trim_tables(image_tables(lo, lo + max(o[0] for o in iins) + 16, rows)), ebp_i)
    ocode = LCode(oins, cod_tables(cod, r['sym']) if cod else {}, ebp_o)
    lmap = tailscan.lines_of(cod, r['sym'], os.path.basename(unit).lower()) if cod else {}
    si, so = steps(icode, iins), steps(ocode, oins)
    ci = collections.Counter(k for k, _, _ in si)
    co = collections.Counter(k for k, _, _ in so)
    img_only = {k: v - co.get(k, 0) for k, v in ci.items() if v > co.get(k, 0)}
    our_only = {k: v - ci.get(k, 0) for k, v in co.items() if v > ci.get(k, 0)}
    fmtk = lambda k: '%s0x%x%s' % (k[0], k[1], '' if k[2] == 'reg' else '@slot')
    return dict(addr=addr, unit=unit, sym=r['sym'], diff=r['diff'], exact=r['exact'],
                img_only={fmtk(k): v for k, v in img_only.items()}, our_only={fmtk(k): v for k, v in our_only.items()},
                img_sites=['%08X %s' % (lo + iins[i][0], t) for k, i, t in si if k in img_only],
                our_sites=['%x %s %s' % (oins[i][0], lmap.get(oins[i][0], '?'), t) for k, i, t in so if k in our_only],
                img_all=len(si), our_all=len(so))


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


def show(x):
    if 'error' in x:
        return ['ERR %08X %s %s' % (x['addr'], x['unit'], x['error'])]
    if not x['img_only'] and not x['our_only']:
        return []
    l = ['%08X %-40s %-28s diff %4d  img-only %s  our-only %s' % (
        x['addr'], x['sym'][:40], x['unit'][4:], x['diff'],
        ','.join('%s:%d' % kv for kv in sorted(x['img_only'].items())) or '-',
        ','.join('%s:%d' % kv for kv in sorted(x['our_only'].items())) or '-')]
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
    ap.add_argument('--tree', nargs='*', help='every non-BYTE_EXACT marker of the tree (or of the named units)')
    ap.add_argument('--all', action='store_true', help='with --from/--tree: include exact functions (validation)')
    ap.add_argument('--json')
    a = ap.parse_args()
    if a.frm or a.tree is not None:
        work = {}
        if a.tree is not None:
            from rotscan import own_context
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
        for r in (json.load(open(a.frm)) if a.frm else []):
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
        allr.sort(key=lambda x: ('error' in x, not x.get('img_only'), -x.get('diff', 0)))
        for x in allr:
            for l in show(x):
                print(l)
        ok = [x for x in allr if 'error' not in x]
        print('functions %d: image-only steps %d, ours-only steps %d, errors %d' % (
            len(allr), sum(1 for x in ok if x['img_only']), sum(1 for x in ok if x['our_only']),
            len(allr) - len(ok)))
        if a.json:
            json.dump(allr, open(a.json, 'w'), indent=0)
        return
    unit = a.unit if a.unit.startswith('src/') else 'src/' + a.unit
    src = os.path.abspath(a.src) if a.src else fe.R + '/' + unit
    for x in run_unit(unit, [int(v, 16) for v in a.addr.split(',')], src, a.pfx):
        ls = show(x)
        print('\n'.join(ls) if ls else '%08X no step difference (img %d, ours %d steps)' % (
            x['addr'], x.get('img_all', 0), x.get('our_all', 0)))


if __name__ == '__main__':
    main()
