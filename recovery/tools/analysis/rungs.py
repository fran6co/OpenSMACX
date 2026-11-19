#!/usr/bin/env python3
"""rungs.py [unit ...] [--addr A[,A]] [--out DIR] [--jobs N] [--pfx-base N] [--top K] [--md]
Rung-ladder census (AGENTS.md "Structure first, no ratchet"): for every function that is not fe-exact, the FIRST
rung on which our compile differs from the image, in the project's comparison order:

  1 EH      FuncInfo: unwind state count, parent chain, funclet (action) slots, try blocks   (ehcensus.img_fi/our_fi)
  2 calls   named call multiset: per callee image/our counts (a destructor or header inline that one side
            calls and the other expands shows as a count difference)                         (callees.diff)
  3 frame   local frame size, ebp/esp frame kind, EH registration                             (framescan.frame)
  4 layout  basic-block count, return-tail count, block-order signature (each block's
            terminator kind and target block, in layout order), prologue-save class a/b       (prosave.shape/classes)
  5 slots   the frame-slot map: offsets used, access widths, address-taken, reference counts  (framescan.slots)
  6 regs    callee-saved set / push order, or differing rows that vanish (or shrink) when
            register names are masked
  7 bytes   same registers: operand order, instruction selection, schedule

One compile per unit (FE_JOBS lanes, prefixes FE_PFX_BASE..), about 2 minutes tree-wide.  It composes the existing
screens' functions; use the per-rung tool on one function for the detail (ehsites.py, callees.py / inlinemap.py,
framescan.py --addr, prosave.py / tailscan.py, varset.py / pairscan.py, regmap.py).

Output (default build/recovery/rungs/, an ignored path): rungs.json (one record per function with every rung's
image/our values and `rung` = the first that differs) and rungs.txt (counts per rung; the table sorted by rung
then rows; the same for the count-sensitive canaries and the broken claims = BYTE_EXACT markers that are not
fe-exact).  `--md` prints the MATCHING_GUIDE summary (counts and the top K functions per rung).
Rung 4 is split into causes (`layout.sub`, the `4x` column): a = return-tail count differs; b = basic-block count
differs with equal tails (`b:sw` jump-table count, `b:jcc` conditional-branch count = a test one side splits or
duplicates, `b:jmp` unconditional jumps = a merged / duplicated tail, `b:mix` both, `b:tgt` same branches but
a jump lands inside a block = a tail cross-jumped at another depth); c = equal counts, the order
signature differs (which arm falls through); p = prologue-save class only; `+d` = the loop set differs (headers,
back edges, `jmp`-closed back edges = unrotated `for (;;)`, loops entered by a jump to their test).  `first` is
the first block where the two block sequences (terminator kind and call count per block) stop aligning: image
address, our offset and our source line (/FAsc, one more compile for units with a rung-4 function).
Image callees without a marker are named `sub_<addr>` and never pair with ours; when the only call difference
is such names against the same number of our-only names the function is classed by the next rung and flagged `c?`.
Screen only: fe masks relocations; confirm with `recover measure`."""
import sys, os, re, json, argparse, collections, subprocess
H = os.path.dirname(os.path.abspath(__file__))
T = os.path.dirname(H)
sys.path.insert(0, T)
sys.path.insert(0, H)
os.chdir(os.path.dirname(os.path.dirname(T)))  # ehcensus imports fe through the working directory
import fe
import callees, framescan, prosave, ehcensus

NAMES = {1: 'EH', 2: 'calls', 3: 'frame', 4: 'layout', 5: 'slots', 6: 'regs', 7: 'bytes'}
# AGENTS.md "Count-sensitive canaries"
CANARIES = {0x004A6330, 0x00524870, 0x004E2000, 0x0053EB50, 0x004CEDE0, 0x00527290, 0x004C3090, 0x0042B630,
            0x0055BC80, 0x00435400, 0x0048D840, 0x005C6DB0, 0x00453A70, 0x004CC360, 0x004A0260, 0x00500B60}
REG = re.compile(r'\b(e?[abcd]x|[abcd][lh]|e?[sd]i|e?bp|e?sp)\b')


def maskreg(s):
    def f(m):
        r = m.group(1)
        if r in ('esp', 'sp'):
            return r
        return 'R32' if len(r) == 3 else ('R8' if r[1] in 'lh' else 'R16')
    return REG.sub(f, s)


def blocks(L):
    """fe.dis rows -> (block count, signature = [(terminator kind, target block index)])"""
    lead = {0}
    for k, r in enumerate(L):
        mn = r[1].split(' ', 1)[0]
        if r[3] is not None and mn != 'call':
            lead.add(r[3])
        if (mn.startswith('j') or mn.startswith('ret')) and k + 1 < len(L):
            lead.add(L[k + 1][0])
    offs = sorted(o for o in lead if any(True for _ in [0]))
    present = {r[0] for r in L}
    offs = [o for o in offs if o in present]
    idx = {o: i for i, o in enumerate(offs)}
    sig = []
    cur = None
    for k, r in enumerate(L):
        if r[0] in idx:
            if cur is not None:
                sig.append(('fall', -1))
            cur = idx[r[0]]
        mn = r[1].split(' ', 1)[0]
        last = k + 1 == len(L) or L[k + 1][0] in idx
        if last:
            if mn.startswith('ret'):
                sig.append(('ret', -1))
            elif mn == 'jmp':
                sig.append(('jmp', idx.get(r[3], -2) if r[3] is not None else -3))  # -3 = switch / tail jump
            elif mn.startswith('j'):
                sig.append(('jcc', idx.get(r[3], -2)))
            else:
                sig.append(('fall', -1))
            cur = None
    return len(offs), sig


def blockrows(L):
    """fe.dis rows -> [dict(off, kind, tgt, calls, jt)] per basic block, in layout order"""
    lead = {0}
    for k, r in enumerate(L):
        mn = r[1].split(' ', 1)[0]
        if r[3] is not None and mn != 'call':
            lead.add(r[3])
        if (mn.startswith('j') or mn.startswith('ret')) and k + 1 < len(L):
            lead.add(L[k + 1][0])
    present = {r[0] for r in L}
    lead = {o for o in lead if o in present}
    out = []
    cur = None
    for k, r in enumerate(L):
        if r[0] in lead:
            cur = dict(off=r[0], kind='fall', tgt=None, calls=0, jt=False, n=0)
            out.append(cur)
        mn = r[1].split(' ', 1)[0]
        cur['n'] += 1
        if mn == 'call':
            cur['calls'] += 1
        last = k + 1 == len(L) or L[k + 1][0] in lead
        if last:
            if mn.startswith('ret'):
                cur['kind'] = 'ret'
            elif mn == 'jmp':
                cur['kind'] = 'jmp'
                cur['tgt'] = r[3]
                cur['jt'] = r[3] is None and '*4' in r[1]
            elif mn.startswith('j'):
                cur['kind'] = 'jcc'
                cur['tgt'] = r[3]
    return out


def loopset(B):
    """blocks -> (loop headers, back edges, back edges closed by `jmp`, loops entered by a jump to their test).
    A back edge is a branch to a block that dominates it, so a tail cross-jumped backwards is not a loop."""
    n = len(B)
    idx = {b['off']: i for i, b in enumerate(B)}
    succ = [[] for _ in B]
    for i, b in enumerate(B):
        if b['kind'] in ('fall', 'jcc') and i + 1 < n:
            succ[i].append(i + 1)
        if b['tgt'] is not None and b['tgt'] in idx:
            succ[i].append(idx[b['tgt']])
    has = set(j for v in succ for j in v)
    orphans = [i for i in range(1, n) if i not in has]  # jump-table targets
    for i, b in enumerate(B):
        if b['jt']:
            succ[i] += orphans
    pred = [[] for _ in B]
    for i, v in enumerate(succ):
        for j in v:
            pred[j].append(i)
    full = (1 << n) - 1
    dom = [full] * n
    dom[0] = 1
    ch = True
    while ch:
        ch = False
        for i in range(1, n):
            d = full
            for p in pred[i]:
                d &= dom[p]
            if not pred[i]:
                d = 0
            d |= 1 << i
            if d != dom[i]:
                dom[i] = d
                ch = True
    fwd = [(i, idx[b['tgt']]) for i, b in enumerate(B) if b['kind'] == 'jmp' and b['tgt'] in idx and idx[b['tgt']] > i + 1]
    # a loop entered by a jump to its test: the body's first block does not dominate the latch
    mid = lambda j, i: any(a < j < t <= i and a >= j - 2 for a, t in fwd)
    back = [(i, j) for i, v in enumerate(succ) for j in v
            if j <= i and not B[i]['jt'] and (dom[i] >> j & 1 or mid(j, i))]
    heads = {}
    for i, h in back:
        heads[h] = max(heads.get(h, h), i)
    jb = sum(1 for i, h in back if B[i]['kind'] == 'jmp')
    ent = 0
    for h, latch in heads.items():
        if mid(h, latch):
            ent += 1
    return [len(heads), len(back), jb, ent]


def layout_split(LI, LO, tails):
    """-> dict(sub, first=[image block offset, our block offset, opcode], counts)"""
    import difflib
    BI, BO = blockrows(LI), blockrows(LO)
    li, lo_ = loopset(BI), loopset(BO)
    cnt = lambda B: [sum(1 for b in B if b['kind'] == 'jcc'), sum(1 for b in B if b['kind'] == 'jmp' and not b['jt']),
                     sum(1 for b in B if b['jt'])]
    ci, co = cnt(BI), cnt(BO)
    KI = [(b['kind'], b['calls']) for b in BI]
    KO = [(b['kind'], b['calls']) for b in BO]
    first = None
    for t, i1, i2, j1, j2 in difflib.SequenceMatcher(None, KI, KO, autojunk=False).get_opcodes():
        if t != 'equal':
            first = [BI[min(i1, len(BI) - 1)]['off'], BO[min(j1, len(BO) - 1)]['off'],
                     {'insert': 'ours+%d' % (j2 - j1), 'delete': 'img+%d' % (i2 - i1),
                      'replace': 'img%d/ours%d' % (i2 - i1, j2 - j1)}[t]]
            break
    if first is None:  # same kind sequence: the first block whose branch goes to a different block
        ii = {b['off']: i for i, b in enumerate(BI)}
        oo = {b['off']: i for i, b in enumerate(BO)}
        for a, b in zip(BI, BO):
            if ii.get(a['tgt']) != oo.get(b['tgt']):
                first = [a['off'], b['off'], 'target']
                break
    if tails[0] != tails[1]:
        sub = 'a'
    elif len(BI) != len(BO):
        sub = 'b:' + ('sw' if ci[2] != co[2] else 'mix' if ci[0] != co[0] and ci[1] != co[1] else
                      'jcc' if ci[0] != co[0] else 'jmp' if ci[1] != co[1] else 'tgt')
    elif KI != KO or first is not None:
        sub = 'c'
    else:
        sub = 'p'
    if li != lo_:
        sub += '+d'
    return dict(sub=sub, first=first, loops=[li, lo_], jcc=[ci[0], co[0]], jmp=[ci[1], co[1]], sw=[ci[2], co[2]])


def slotkey(m):
    return sorted((k, ''.join(str(w) for w in sorted(v[0])), bool(v[1]), v[2]) for k, v in m.items())


def ehkey(f):
    return None if not isinstance(f, dict) else (f['ms'], tuple(f['par']), tuple(f['act']), f['nt'])


def ehgroups(par):
    """unwind parents -> states per root state (one root per teardown stage of a return site)"""
    g = []
    for v in par:
        if v == -1 or not g:
            g.append(1)
        else:
            g[-1] += 1
    return g


def ehfmt(f):
    if f is None:
        return '-'
    if not isinstance(f, dict):
        return str(f)
    return 'ms=%d fn=%d try=%d' % (f['ms'], sum(f['act']), f['nt'])  # fn < ms: states without an action (phantom teardown)


def one(c, text, unit, addr):
    idn = fe.ident(text, addr)
    if not idn:
        return None
    sym, lo, hi = idn
    r = fe.evaluate(c, text, addr)
    if 'error' in r:
        return dict(unit=unit, addr='%08X' % addr, sym=sym, error=r['error'])
    if r['exact']:
        return dict(unit=unit, addr='%08X' % addr, sym=sym, exact=True)
    s = c.function(sym)
    sec = c.sections[s.section - 1]
    rec = dict(unit=unit, addr='%08X' % addr, sym=sym, exact=False, diff=r['diff'], img=r['img'], ours=r['ours'])
    diffs = []
    # 1 EH
    ei, eo = ehcensus.img_fi(lo), ehcensus.our_fi(c, sym)
    rec['eh'] = [ei, eo]
    if ehkey(ei) != ehkey(eo):
        diffs.append(1)
    # 2 calls
    d = callees.diff(c, text, addr)
    cd = {}
    if d:
        ci, co = d
        for k in sorted(set(ci) | set(co)):
            if ci[k] != co[k]:
                cd[k] = [ci[k], co[k]]
    rec['calls'] = cd
    sub_i = sum(v[0] - v[1] for k, v in cd.items() if k.startswith('sub_'))
    rest = {k: v for k, v in cd.items() if not k.startswith('sub_')}
    unnamed = bool(cd) and sub_i > 0 and all(v[1] > v[0] for v in rest.values()) and \
        sum(v[1] - v[0] for v in rest.values()) == sub_i
    rec['calls_unnamed'] = unnamed
    if cd and not unnamed:
        diffs.append(2)
    # 3 frame
    I = framescan.insns(fe.imgbytes(lo, hi), lo)
    O = framescan.insns(bytes(sec.data), 0)
    fi, fo = framescan.frame(I), framescan.frame(O)
    rec['frame'] = [list(fi[:3]), list(fo[:3])]
    if fi[:3] != fo[:3]:
        diffs.append(3)
    # 4 layout
    LI = fe.imgdis(lo, hi)
    LO = fe.dis(sec.data, 0, sec.relocations, selfsec=s.section, syms=c.symbols)
    bi, sgi = blocks(LI)
    bo, sgo = blocks(LO)
    si, so = prosave.shape(LI), prosave.shape(LO)
    cls = prosave.classes(si, so)
    rec['layout'] = dict(blocks=[bi, bo], tails=[len(si['tails']), len(so['tails'])], order=sgi == sgo,
                         cls=cls, save=[prosave.fmt(si), prosave.fmt(so)])
    rec['layout'].update(layout_split(LI, LO, rec['layout']['tails']))
    rec['lo'] = lo
    if bi != bo or len(si['tails']) != len(so['tails']) or sgi != sgo or 'a' in cls or 'b' in cls:
        diffs.append(4)
    # 5 slots
    mi = slotkey(framescan.slots(I, fi[0], fi[2], fi[3]))
    mo = slotkey(framescan.slots(O, fo[0], fo[2], fo[3]))
    oi, oo = {x[0] for x in mi}, {x[0] for x in mo}
    rec['slots'] = dict(n=[len(mi), len(mo)], only_img=sorted(oi - oo)[:12], only_ours=sorted(oo - oi)[:12],
                        refs=sum(1 for x in mi if x not in mo) + sum(1 for x in mo if x not in mi))
    if mi != mo:
        diffs.append(5)
    # 6 registers
    A = [maskreg(x[1]) for x in LI]
    B = [maskreg(x[1]) for x in LO]
    if A == B:
        md_ = 0
    else:
        md_ = sum(max(i2 - i1, j2 - j1) for t, i1, i2, j1, j2 in fe.align(A, B) if t != 'equal')
    saved = [si['up'] + si['de'], so['up'] + so['de']]
    rec['regs'] = dict(masked=md_, saved=saved)
    if saved[0] != saved[1] or md_ < r['diff'] - r.get('jt', 0) or md_ == 0:
        diffs.append(6)
    diffs.append(7)
    rec['rungs'] = diffs
    rec['rung'] = diffs[0]
    return rec


def worker(unit, pfx):
    text = open(fe.R + '/' + unit, encoding='latin-1').read()
    c, err = fe.compile_unit(unit, fe.R + '/' + unit, pfx)
    out = []
    if c is None:
        return [dict(unit=unit, error=err)]
    ctx = {}
    for m in re.finditer(r'// ORIGINAL: 0x([0-9A-F]{8}) \S+ 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)\n((?://(?! ORIGINAL)[^\n]*\n)*)', text):
        addr = int(m.group(1), 16)
        if WANT and addr not in WANT:
            continue
        cc_ = c
        mc = re.search(r'// compile_context (\S+)', m.group(3))
        if mc and mc.group(1) != unit:
            if mc.group(1) not in ctx:
                ctx[mc.group(1)] = fe.compile_unit(mc.group(1), fe.R + '/' + mc.group(1), pfx)[0]
            cc_ = ctx[mc.group(1)] or c
        try:
            rec = one(cc_, text, unit, addr)
        except Exception as e:  # a marker without a symbol in this compile (asm, fragments)
            rec = dict(unit=unit, addr='%08X' % addr, sym=m.group(0).split()[3], error=repr(e)[:120])
        if rec is None:
            continue
        rec['claimed'] = 'BYTE_EXACT' in m.group(2)
        rec['canary'] = addr in CANARIES
        out.append(rec)
    need = [r for r in out if r.get('rung') == 4 and r['layout'].get('first')]
    if need:
        import tailscan
        cod = tailscan.compile_listing(unit, fe.R + '/' + unit, pfx)[1] or ''
        for r in need:
            lm = tailscan.lines_of(cod, r['sym'], os.path.basename(unit).lower())
            ks = [k for k in sorted(lm) if k >= r['layout']['first'][1]]
            r['layout']['line'] = lm[ks[0]] if ks else '?'
    return out


def detail(r):
    k = r['rung']
    if k == 1:
        s = 'img %s | ours %s' % (ehfmt(r['eh'][0]), ehfmt(r['eh'][1]))
        if all(isinstance(f, dict) for f in r['eh']):
            gi, go = ehgroups(r['eh'][0]['par']), ehgroups(r['eh'][1]['par'])
            if len(gi) == len(go):  # same teardown sites: per-site depth (states under one root), ours against the image
                s += '; sites %d: ours deeper %d, shallower %d' % (len(gi), sum(1 for a, b in zip(gi, go) if b > a),
                                                                   sum(1 for a, b in zip(gi, go) if b < a))
            else:
                s += '; root states %d/%d' % (len(gi), len(go))
        return s
    if k == 2:
        it = sorted(r['calls'].items(), key=lambda kv: -abs(kv[1][0] - kv[1][1]))
        return ' '.join('%s %d/%d' % (n[:38], a, b) for n, (a, b) in it[:4]) + (' +%d' % (len(it) - 4) if len(it) > 4 else '')
    if k == 3:
        (a, ea, ba), (b, eb, bb) = r['frame']
        return 'img 0x%x%s%s | ours 0x%x%s%s' % (a, ' ebp' if ba else ' esp', ' eh' if ea else '', b, ' ebp' if bb else ' esp', ' eh' if eb else '')
    if k == 4:
        l = r['layout']
        s = '4%-7s blocks %d/%d tails %d/%d jcc %d/%d jmp %d/%d sw %d/%d loops %s/%s order %s%s' % (
            l.get('sub', '?'), l['blocks'][0], l['blocks'][1], l['tails'][0], l['tails'][1], l['jcc'][0], l['jcc'][1],
            l['jmp'][0], l['jmp'][1], l['sw'][0], l['sw'][1], '.'.join(map(str, l['loops'][0])), '.'.join(map(str, l['loops'][1])),
            '=' if l['order'] else 'differs', (' save-class ' + l['cls']) if l['cls'] else '')
        if l.get('first'):
            s += ' first img 0x%X ours +0x%x %s (%s)' % (r['lo'] + l['first'][0], l['first'][1], l.get('line', '?'), l['first'][2])
        return s
    if k == 5:
        s = r['slots']
        return 'slots %d/%d img-only %s ours-only %s changed %d' % (s['n'][0], s['n'][1], ','.join(hex(x) for x in s['only_img'][:5]) or '-',
                                                                   ','.join(hex(x) for x in s['only_ours'][:5]) or '-', s['refs'])
    if k == 6:
        g = r['regs']
        return 'saved %s | %s; rows with registers masked %d' % (','.join(g['saved'][0]) or '-', ','.join(g['saved'][1]) or '-', g['masked'])
    return 'registers equal'


def line(r):
    return '%d %-6s %s %-26s %-40s rows=%4d %s%s' % (r['rung'], NAMES[r['rung']], r['addr'], r['unit'][4:30], r['sym'][:40], r['diff'],
                                                    detail(r), ' c?' if r.get('calls_unnamed') else '')


def subtable(rs):
    """rung-4 causes -> markdown rows"""
    g = collections.defaultdict(list)
    for r in rs:
        if r['rung'] == 4:
            g[r['layout']['sub']].append(r)
    out = ['| cause | functions | rows | largest |', '|---|---|---|---|']
    for k in sorted(g):
        v = sorted(g[k], key=lambda r: -r['diff'])
        out.append('| %s | %d | %d | %s |' % (k, len(v), sum(r['diff'] for r in v), ', '.join(
            '%s 0x%s (%d)' % (re.sub(r'^\?+(\w+)@(?:(\w+)@)?.*', lambda m: (m.group(2) + '::' if m.group(2) else '') + m.group(1), r['sym'])[:30],
                              r['addr'], r['diff']) for r in v[:4])))
    return out


def table(rs, title, out):
    rs = sorted(rs, key=lambda r: (r['rung'], -r['diff']))
    cnt = collections.Counter(r['rung'] for r in rs)
    out.append('== %s: %d functions, %d rows' % (title, len(rs), sum(r['diff'] for r in rs)))
    out.append('   ' + '  '.join('%d %s %d (%d rows)' % (k, NAMES[k], cnt[k], sum(r['diff'] for r in rs if r['rung'] == k)) for k in sorted(NAMES)))
    for r in rs:
        out.append(line(r))
    out.append('')


WANT = set()


def main():
    global WANT
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='*')
    ap.add_argument('--addr')
    ap.add_argument('--out', default=fe.R + '/build/recovery/rungs')
    ap.add_argument('--jobs', type=int, default=int(os.environ.get('FE_JOBS', '5')))
    ap.add_argument('--pfx-base', type=int, default=int(os.environ.get('FE_PFX_BASE', '21')))
    ap.add_argument('--top', type=int, default=8)
    ap.add_argument('--md', action='store_true')
    ap.add_argument('--worker')
    ap.add_argument('--pfx')
    a = ap.parse_args()
    if a.addr:
        WANT = {int(x, 16) for x in a.addr.split(',')}
    if a.worker:
        json.dump(worker(a.worker, a.pfx), sys.stdout)
        return
    cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
    units = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/')]
    units = [u for u in units if os.path.exists(fe.R + '/' + u)]
    if a.units:
        units = [u for u in units if u in a.units or os.path.basename(u) in a.units]
    if WANT:
        keep = []
        for u in units:
            t = open(fe.R + '/' + u, encoding='latin-1').read()
            if any('// ORIGINAL: 0x%08X ' % x in t for x in WANT):
                keep.append(u)
        units = keep
    units.sort(key=lambda u: -os.path.getsize(fe.R + '/' + u))
    import queue
    from concurrent.futures import ThreadPoolExecutor
    Q = queue.Queue()
    for i in range(a.jobs):
        Q.put(i)

    def run(u):
        L = Q.get()
        try:
            cmd = [sys.executable, os.path.abspath(__file__), '--worker', u, '--pfx', str(a.pfx_base + L)]
            if a.addr:
                cmd += ['--addr', a.addr]
            p = subprocess.run(cmd, capture_output=True, text=True)
            try:
                return json.loads(p.stdout)
            except Exception:
                return [dict(unit=u, error=(p.stderr or p.stdout)[-300:])]
        finally:
            Q.put(L)
    allr = []
    with ThreadPoolExecutor(a.jobs) as ex:
        for o in ex.map(run, units):
            allr += o
    os.makedirs(a.out, exist_ok=True)
    json.dump(allr, open(a.out + '/rungs.json', 'w'))
    bad = [r for r in allr if r.get('exact') is False]
    out = ['rungs.py: %d markers, %d fe-exact, %d not exact, %d unmeasured' % (
        len(allr), sum(1 for r in allr if r.get('exact')), len(bad), sum(1 for r in allr if 'error' in r))]
    out.append('')
    table([r for r in bad if not r['claimed']], 'non-exact functions (no BYTE_EXACT claim)', out)
    table([r for r in bad if r['canary']], 'count-sensitive canaries (AGENTS.md list) that are not fe-exact', out)
    table([r for r in bad if r['claimed'] and not r['canary']], 'broken claims (BYTE_EXACT marker, not fe-exact; includes the markers fe cannot measure in their own unit)', out)
    for r in allr:
        if 'error' in r:
            out.append('UNMEASURED %s %s %s %s' % (r.get('addr', ''), r['unit'], r.get('sym', ''), r['error'][:100]))
    open(a.out + '/rungs.txt', 'w').write('\n'.join(out) + '\n')
    if a.md:
        for title, rs in (('Non-exact functions', [r for r in bad if not r['claimed']]),
                          ('Count-sensitive canaries', [r for r in bad if r['canary']]),
                          ('Broken claims', [r for r in bad if r['claimed'] and not r['canary']])):
            cnt = collections.Counter(r['rung'] for r in rs)
            print('%s (%d functions, %d rows):\n' % (title, len(rs), sum(r['diff'] for r in rs)))
            print('| rung | functions | rows | largest |')
            print('|---|---|---|---|')
            for k in sorted(NAMES):
                g = sorted((r for r in rs if r['rung'] == k), key=lambda r: -r['diff'])
                print('| %d %s | %d | %d | %s |' % (k, NAMES[k], cnt[k], sum(r['diff'] for r in g),
                                                   ', '.join('%s 0x%s (%d)' % (re.sub(r'^\?+(\w+)@(?:(\w+)@)?.*', lambda m: (m.group(2) + '::' if m.group(2) else '') + m.group(1), r['sym'])[:34], r['addr'], r['diff']) for r in g[:a.top])))
            if title == 'Non-exact functions':
                print('Rung 4 by cause:\n')
                print('\n'.join(subtable(rs)))
                print()
    else:
        print('\n'.join(l for l in out if l.startswith(('rungs.py', '==', '   '))))
        print('written', a.out + '/rungs.txt', a.out + '/rungs.json')


if __name__ == '__main__':
    main()
