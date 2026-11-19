#!/usr/bin/env python3
"""candset.py <unit> <0xADDR> [--src ALT.cpp] [--pfx N] [--scratch] [--quiet] [--webs]
The register candidates VC6 actually formed, read from the compiled code of both sides (MATCHING_GUIDE 2.7,
"Candidate set").  Where regprio.py counts references in our *source*, this tool takes the candidate set as
ground truth from the instructions: every callee-saved register web (reaching definitions over the CFG, so a
loop variable redefined at the bottom is one web), named by its role, scored by rule 4/4b at instruction level
(a reference = an instruction that reads or writes the register; n_b = distinct candidates referenced in the
block: callee-saved webs, frame slots read at least twice, and with --scratch eax/ecx/edx webs with two uses).

Roles (both sides; the image's slots carry our /FAsc names through the fe alignment):
  this              prologue copy of ecx                    lea:this+K   address temporary of a member (kind a)
  lea:[slot]        address of a local                      lea:[r+r*k]  strength-reduced / scaled address (b)
  [slot]{name}      cached piece of a memory-homed variable (c); `wb` = written back
  ret@line{name}    a kept call result (f)                  #K           a constant (e)
  {name}            a source variable (our listing names the slot or the assignment on its line)
Output:
  1. the web table: side, register, role, range, refs, score, and whether the side's own scores explain
     its register (`ok`: every lower register is held by an interfering web with a higher score);
  2. roles present on one side only (the candidate-set difference) with the kind's source levers;
  3. interfering pairs the two sides order differently, with both sides' scores.
Validation: --quiet prints `consistent A/B` (rank constraints the side's own scores satisfy)."""
import sys, os, re, argparse, collections, struct
H = os.path.dirname(os.path.abspath(__file__))
T = os.path.dirname(H)
sys.path.insert(0, T)
sys.path.insert(0, H)
import fe
import regmap
import regprio
import capstone

CS = ('esi', 'edi', 'ebx', 'ebp')
SCR = ('eax', 'ecx', 'edx')
ALIAS = {'esi': ('esi', 'si'), 'edi': ('edi', 'di'), 'ebx': ('ebx', 'bx', 'bl', 'bh'), 'ebp': ('ebp', 'bp'),
         'eax': ('eax', 'ax', 'al', 'ah'), 'ecx': ('ecx', 'cx', 'cl', 'ch'), 'edx': ('edx', 'dx', 'dl', 'dh')}
WRITES = ('mov', 'lea', 'movsx', 'movzx', 'pop', 'xor', 'or', 'and', 'add', 'sub', 'inc', 'dec', 'neg', 'not', 'shl',
          'shr', 'sar', 'imul', 'sbb', 'adc', 'rol', 'ror', 'setl', 'sete', 'setne', 'setg', 'setge', 'setle', 'xchg')
PURE = ('mov', 'lea', 'movsx', 'movzx', 'pop')  # write without reading the destination
LEVERS = {
    'this': 'this: prologue copy + every member access not routed through an address temporary (d)',
    'lea:this': 'address CSE of a member: forms from two uses of &member or of a struct copy of it; scores def + uses; '
                'removes `this` uses (kind a)',
    'lea:slot': 'address of a local struct/array kept in a register (rule 10: the local itself is not a candidate)',
    'lea:idx': 'strength-reduced or scaled address (&table[i]): replaces the index as a candidate (b, rule 10 cse_addr)',
    'slot': 'cached piece of a memory-homed variable: needs two references, survives calls, reloaded per loop '
            'iteration (c, rule 8)',
    'ret': 'call result kept in a register: a named local or an expression reused (f)',
    'const': 'constant: a candidate from several uses; stores count (e)',
    'var': 'source variable home',
}


def mnem(t):
    return t.split(' ', 1)[0]


def ops(t):
    _, _, r = t.partition(' ')
    return [o.strip() for o in r.split(',')] if r else []


def reads_writes(t, r):
    """-> (reads r, writes r) for instruction text t"""
    al = ALIAS[r]
    pat = re.compile(r'\b(%s)\b' % '|'.join(al))
    mn = mnem(t)
    os_ = ops(t)
    if not pat.search(t):
        if mn in ('call',) and r in SCR:
            return False, True  # clobbered
        if mn in ('cdq',) and r in ('edx',):
            return False, True
        if mn in ('cdq', 'idiv', 'div') and r in ('eax',):
            return True, True
        if mn in ('idiv', 'div') and r == 'edx':
            return True, True
        if mn.startswith('rep') or mn in ('movsd', 'stosd', 'movsb', 'stosb'):
            return (r in ('ecx', 'edi', 'esi', 'eax')), (r in ('ecx', 'edi', 'esi'))
        return False, False
    if not os_:
        return True, False
    dst = os_[0]
    dreg = pat.fullmatch(dst) is not None
    rd = any(pat.search(o) for o in os_[1:]) or ('[' in dst and pat.search(dst) is not None)
    wr = False
    if dreg:
        if mn in PURE:
            wr = True
            if mn == 'pop':
                return False, True
        elif mn == 'xor' and len(os_) == 2 and os_[1] == dst:
            return False, True
        elif mn in ('cmp', 'test', 'push'):
            rd = True
        elif mn in WRITES or mn.startswith('set'):
            rd, wr = True, True
        else:
            rd = True
    if mn == 'xchg':
        return True, True
    return rd, wr


def image_tables(lo, hi, rows):
    """{our-relative off of a `jmp [r*4 + table]`: [case offs]} on the image side"""
    out = {}
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    for k, o, c in rows:
        if o is None or not (o[1].startswith('jmp dword ptr [') and '*4' in o[1]):
            continue
        b = fe.imgbytes(lo + o[0], lo + o[0] + 7)
        ins = next(md.disasm(b, lo + o[0]), None)
        m = ins and re.search(r'\*4 \+ (0x[0-9a-f]+)\]', ins.op_str)
        if not m:
            continue
        ta = int(m.group(1), 16)
        cases = []
        while True:
            v = struct.unpack('<I', fe.imgbytes(ta + 4 * len(cases), ta + 4 * len(cases) + 4))[0]
            if not (lo <= v < hi) or len(cases) > 400:
                break
            cases.append(v - lo)
        out[o[0]] = cases
    return out


class Code:
    """instructions [(off, text, target)] + switch tables -> blocks, CFG, loop depth, webs"""

    def __init__(self, ins, tables):
        self.ins = ins
        self.idx = {off: i for i, (off, t, g) in enumerate(ins)}
        n = len(ins)
        lead = {0}
        caseset = set()
        for i, (off, t, g) in enumerate(ins):
            mn = mnem(t)
            if mn.startswith('j') or mn == 'ret':
                if i + 1 < n:
                    lead.add(i + 1)
                if g is not None and g in self.idx:
                    lead.add(self.idx[g])
            for c in tables.get(off, ()):
                if c in self.idx:
                    lead.add(self.idx[c])
                    caseset.add(self.idx[c])
        self.starts = sorted(x for x in lead if x < n) or [0]
        self.bof = [0] * n
        for b, s in enumerate(self.starts):
            e = self.starts[b + 1] if b + 1 < len(self.starts) else n
            for i in range(s, e):
                self.bof[i] = b
        nb = len(self.starts)
        self.case = [self.starts[b] in caseset for b in range(nb)]
        # instruction-level successors
        self.isucc = [[] for _ in range(n)]
        for i, (off, t, g) in enumerate(ins):
            mn = mnem(t)
            if mn == 'ret':
                continue
            if mn.startswith('j'):
                if g is not None and g in self.idx:
                    self.isucc[i].append(self.idx[g])
                for c in tables.get(off, ()):
                    if c in self.idx:
                        self.isucc[i].append(self.idx[c])
                if mn != 'jmp' and i + 1 < n:
                    self.isucc[i].append(i + 1)
            elif i + 1 < n:
                self.isucc[i].append(i + 1)
        self.ipred = [[] for _ in range(n)]
        for i in range(n):
            for j in self.isucc[i]:
                self.ipred[j].append(i)
        succ = [set() for _ in range(nb)]
        for b in range(nb if n else 0):
            e = (self.starts[b + 1] if b + 1 < nb else n) - 1
            for j in self.isucc[e]:
                succ[b].add(self.bof[j])
        pred = [set() for _ in range(nb)]
        for u in range(nb):
            for v in succ[u]:
                pred[v].add(u)
        full = set(range(nb))
        dom = [full] * nb
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
        loops = collections.defaultdict(set)
        for u in range(nb):
            for h in succ[u]:
                if h in dom[u]:
                    body = {h, u}
                    st = [u] if u != h else []
                    while st:
                        x = st.pop()
                        for p in pred[x]:
                            if p not in body:
                                body.add(p)
                                st.append(p)
                    loops[h] |= body
        self.depth = [0] * nb
        for h, body in loops.items():
            for x in body:
                self.depth[x] += 1
        self.nloops = len(loops)
        # prologue/epilogue saves are not references
        self.skip = set()
        for i, (off, t, g) in enumerate(ins[:12]):
            if re.fullmatch(r'push (esi|edi|ebx|ebp)', t):
                self.skip.add(i)
        for i, (off, t, g) in enumerate(ins):
            if mnem(t) == 'ret':
                j = i - 1
                while j >= 0 and re.fullmatch(r'(pop (esi|edi|ebx|ebp)|mov esp, ebp|leave|add esp, \w+|mov ecx, dword ptr \[ebp - 0xc\]|mov dword ptr fs:\[0\], ecx)', ins[j][1]):
                    self.skip.add(j)
                    j -= 1
        for i, (off, t, g) in enumerate(ins[:6]):
            if t == 'mov ebp, esp':
                self.skip.add(i)
                self.frame_ebp = True

    def webs(self, r):
        """-> list of webs for register r: dict(reg, defs, uses, refs (instr idx), live (instr idx set))"""
        n = len(self.ins)
        rw = []
        for i, (off, t, g) in enumerate(self.ins):
            rw.append((False, False) if i in self.skip else reads_writes(t, r))
        defs = [i for i in range(n) if rw[i][1]]
        did = {d: k for k, d in enumerate(defs)}
        # reaching definitions (bitsets as python ints)
        IN = [0] * n
        OUT = [0] * n
        work = list(range(n))
        inq = [True] * n
        while work:
            i = work.pop()
            inq[i] = False
            x = 0
            for p in self.ipred[i]:
                x |= OUT[p]
            IN[i] = x
            o = (1 << did[i]) if rw[i][1] else x
            if o != OUT[i]:
                OUT[i] = o
                for s in self.isucc[i]:
                    if not inq[s]:
                        inq[s] = True
                        work.append(s)
        parent = list(range(len(defs)))

        def find(a):
            while parent[a] != a:
                parent[a] = parent[parent[a]]
                a = parent[a]
            return a
        usesof = collections.defaultdict(list)
        for i in range(n):
            if rw[i][0]:
                ds = [k for k in range(len(defs)) if IN[i] >> k & 1]
                if rw[i][1]:
                    ds.append(did[i])
                for k in ds[1:]:
                    parent[find(k)] = find(ds[0])
                if ds:
                    usesof[i] = ds
        # liveness: backward, per instruction
        LIN = [False] * n
        ch = True
        while ch:
            ch = False
            for i in range(n - 1, -1, -1):
                lo = any(LIN[s] for s in self.isucc[i])
                li = rw[i][0] or (lo and not rw[i][1])
                if li != LIN[i]:
                    LIN[i] = li
                    ch = True
        groups = collections.defaultdict(lambda: dict(reg=r, defs=[], uses=[], refs=set(), live=set()))
        for k, d in enumerate(defs):
            groups[find(k)]['defs'].append(d)
            groups[find(k)]['refs'].add(d)
        for i, ds in usesof.items():
            g = groups[find(ds[0])]
            g['uses'].append(i)
            g['refs'].add(i)
        # live points: instruction i where r is live-in and reaching defs belong to the web
        for i in range(n):
            if LIN[i] and IN[i]:
                ks = [k for k in range(len(defs)) if IN[i] >> k & 1]
                groups[find(ks[0])]['live'].add(i)
        out = []
        for g in groups.values():
            if not g['uses']:
                continue
            g['live'] |= g['refs']
            out.append(g)
        return out


def slot_name(op):
    m = re.search(r'\[ebp ([-+]) (0x[0-9a-f]+|\d+)\]', op)
    return (('-' if m.group(1) == '-' else '') + m.group(2)) if m else None


def role_of(code, w, names, lst, this_reg, srclines):
    """name the web by what its first definition loads"""
    d0 = min(w['defs'])
    off, t, g = code.ins[d0]
    os_ = ops(t)
    src = os_[1] if len(os_) > 1 else ''
    mn = mnem(t)
    kind = 'var'
    if mn == 'mov' and src == 'ecx' and d0 < 14 and w['reg'] == this_reg:
        return 'this', 'this'
    def isconst(t_):
        o_ = ops(t_)
        return (mnem(t_) == 'xor' and len(o_) == 2 and o_[0] == o_[1]) or (mnem(t_) == 'or' and o_[-1] in ('0xffffffff', '-1')) \
            or (mnem(t_) == 'mov' and re.fullmatch(r'-?(0x[0-9a-f]+|\d+)', o_[-1]) is not None)
    if isconst(t) and not all(isconst(code.ins[d][1]) for d in w['defs']):
        v = 0 if mn == 'xor' else (-1 if mn == 'or' else int(src, 0))
        return 'init#%d' % v + ('@%d' % lst[off][0] if lst and off in lst else ''), 'var'
    if mn == 'xor' and src == os_[0]:
        return '#0', 'const'
    if mn == 'or' and src in ('0xffffffff', '-1'):
        return '#-1', 'const'
    if mn == 'mov' and re.fullmatch(r'-?(0x[0-9a-f]+|\d+)', src):
        return '#' + str(int(src, 0)), 'const'
    if mn == 'lea':
        s = slot_name(src)
        if s:
            return 'lea:[%s]{%s}' % (s, names.get(s, '?')), 'lea:slot'
        m = re.fullmatch(r'\[(e\w\w) \+ (0x[0-9a-f]+|\d+)\]', src)
        if m and m.group(1) == this_reg:
            return 'lea:this+0x%x' % int(m.group(2), 0), 'lea:this'
        return 'lea:' + re.sub(r'\b(esi|edi|ebx|ebp)\b', 'R', re.sub(r'\s', '', src)), 'lea:idx'
    s = slot_name(src) if mn in ('mov', 'movsx', 'movzx') else None
    if s:
        return '[%s]{%s}' % (s, names.get(s, '?')), 'slot'
    # a store of the register to a slot names the variable it holds
    for i in sorted(w['refs']):
        tt = code.ins[i][1]
        m = re.match(r'mov (?:dword ptr )?\[ebp ([-+]) (0x[0-9a-f]+|\d+)\], %s$' % w['reg'], tt)
        if m:
            s = ('-' if m.group(1) == '-' else '') + m.group(2)
            return 'wb[%s]{%s}' % (s, names.get(s, '?')), 'slot'
    if mn == 'mov' and src == 'eax' and d0 > 0 and mnem(code.ins[d0 - 1][1]) == 'call':
        ln = lst.get(off, (None,))[0] if lst else None
        return ('ret@%s' % ln) if ln else 'ret', 'ret'
    if lst and off in lst:
        ln = lst[off][0]
        return '%s@%d' % (re.sub(r'\s+', '', src)[:16], ln), 'var'
    if lst is None:
        return re.sub(r'\s+', '', src)[:16], 'var'
    return re.sub(r'\s+', '', src)[:20] or mn, 'var'


OPT = dict(penalty=True, slotmin=2, scratch=False, constimm=0, constw=100, constdef=1)


def score_webs(code, webs, slots_n, scratch_webs, penalty=None):
    penalty = OPT['penalty'] if penalty is None else penalty
    nb = len(code.starts)
    cands = collections.defaultdict(set)  # block -> candidate ids referenced
    for k, w in enumerate(webs + scratch_webs):
        for i in w['refs']:
            cands[code.bof[i]].add(('w', k))
    for b, ss in slots_n.items():
        for s in ss:
            cands[b].add(('s', s))
    nbk = {b: len(c) for b, c in cands.items()}
    for w in webs:
        sc = 0.0
        refs = w['refs']
        if w.get('kind') == 'const' and not OPT['constdef']:
            refs = refs - set(w['defs'])
        for i in refs:
            b = code.bof[i]
            sc += nbk.get(b, 1) * 2 ** code.depth[b] * (0.5 if code.case[b] else 1)
        if penalty:
            refb = {code.bof[i] for i in w['refs']}
            liveb = {code.bof[i] for i in w['live']}
            for b in liveb - refb:
                if nbk.get(b):
                    sc -= 0.5 * nbk[b] * 2 ** code.depth[b]
        if w.get('kind') == 'const':
            sc = sc * OPT['constw'] / 100.0
        w['score'] = sc
        w['first'] = min(w['refs'])
        w['firstb'] = code.bof[w['first']]


def interferes(a, b):
    return bool(a['live'] & b['live'])


def side_analysis(code, regs, names, lst, srclines, scratch=False):
    this_reg = None
    for off, t, g in code.ins[:14]:
        m = re.fullmatch(r'mov (esi|edi|ebx|ebp), ecx', t)
        if m:
            this_reg = m.group(1)
    webs = []
    for r in regs:
        webs += code.webs(r)
    sw = []
    if scratch:
        for r in SCR:
            sw += [w for w in code.webs(r) if len(w['uses']) >= 2]
    slotc = collections.Counter()
    slotb = collections.defaultdict(set)
    for i, (off, t, g) in enumerate(code.ins):
        if i in code.skip:
            continue
        for o in ops(t):
            s = slot_name(o)
            if s and not t.startswith('lea'):
                slotc[s] += 1
                slotb[code.bof[i]].add(s)
    slots_n = {b: {s for s in ss if slotc[s] >= OPT['slotmin']} for b, ss in slotb.items()}
    for w in webs:
        w['role'], w['kind'] = role_of(code, w, names, lst, this_reg, srclines)
    if OPT['constimm']:
        # a constant candidate is referenced by every use of its value, including those VC6 kept as immediates
        for w in webs:
            if w['kind'] != 'const':
                continue
            v = int(w['role'][1:])
            pats = [r'(?<![\w+-])%s$' % re.escape(str(v)), r'(?<![\w+-])%s$' % re.escape(hex(v & 0xffffffff))]
            for i in w['live']:
                t = code.ins[i][1]
                if i in w['refs'] or i in code.skip:
                    continue
                os_ = ops(t)
                if mnem(t) in ('push', 'mov', 'cmp', 'add', 'sub', 'and', 'or') and os_ and any(re.search(p_, os_[-1]) for p_ in pats):
                    w['refs'].add(i)
                elif v == 0 and mnem(t) == 'test' and len(os_) == 2 and os_[0] == os_[1]:
                    w['refs'].add(i)
    score_webs(code, webs, slots_n, sw)
    for w in webs:
        w['byte'] = w['reg'] == 'ebx' and any(re.search(r'\b(bl|bh)\b', code.ins[i][1]) for i in w['refs'])
        # registers an instruction inside the web's range clobbers implicitly (rep movs/stos: esi, edi)
        w['excl'] = set()
        for i in w['live']:
            if i in w['refs']:
                continue
            t = code.ins[i][1]
            if t.startswith('rep ') or mnem(t) in ('movsd', 'movsb', 'stosd', 'stosb'):
                w['excl'] |= {'esi', 'edi'}
        if w['byte']:
            w['excl'] |= {'esi', 'edi'}
    return webs


def better(a, b):
    """does the model rank a above b (4c: exact tie -> later first block, then earlier first reference)"""
    if a['score'] != b['score']:
        return a['score'] > b['score']
    if a['firstb'] != b['firstb']:
        return a['firstb'] > b['firstb']
    return a['first'] < b['first']


def pairs(webs, regs):
    """interfering pairs in different registers that the model orders as the registers are ordered"""
    ok = tot = 0
    for x in webs:
        for y in webs:
            if regs.index(x['reg']) < regs.index(y['reg']) and interferes(x, y) and x['reg'] not in y['excl']:
                tot += 1
                ok += better(x, y)
    return ok, tot


def consistency(webs, regs):
    """rank constraints: a web in register k needs, for each lower register j, an interfering web in j that the
    model ranks above it"""
    ok = tot = 0
    bad = []
    for w in webs:
        k = regs.index(w['reg'])
        good = True
        for j in range(k):
            if regs[j] in w['excl']:
                continue  # a byte value, or a value live across rep movs, cannot live there
            tot += 1
            hold = [x for x in webs if x['reg'] == regs[j] and interferes(x, w)]
            if any(better(x, w) for x in hold):
                ok += 1
            else:
                good = False
                bad.append((w, regs[j], hold))
        w['ok'] = good
    return ok, tot, bad


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('unit')
    ap.add_argument('addr')
    ap.add_argument('--src')
    ap.add_argument('--pfx', default=os.environ.get('FE_PFX_BASE', '2'))
    ap.add_argument('--scratch', action='store_true', help='count eax/ecx/edx webs with two uses in n_b')
    ap.add_argument('--quiet', action='store_true')
    ap.add_argument('--min', type=int, default=2, help='hide webs with fewer references')
    ap.add_argument('--opt', default='', help='model options k=v,... (penalty=0, slotmin=N)')
    a = ap.parse_args()
    for kv in filter(None, a.opt.split(',')):
        k, v = kv.split('=')
        OPT[k] = type(OPT[k])(int(v)) if not isinstance(OPT[k], str) else v
    unit = a.unit if a.unit.startswith('src/') else 'src/' + a.unit
    addr = int(a.addr, 16)
    src = os.path.abspath(a.src) if a.src else fe.R + '/' + unit
    text = open(src, encoding='latin-1').read()
    c, err = fe.compile_unit(unit, src, a.pfx)
    if c is None:
        raise SystemExit('COMPILE FAIL\n' + str(err))
    r = fe.evaluate(c, text, addr)
    if 'error' in r:
        raise SystemExit(r['error'])
    items, frame = regprio.cod_listing(unit, src, a.pfx, r['sym'])
    report(r, text, items, addr, a.quiet, a.min, a.scratch)


def report(r, text, items, addr, quiet=False, minrefs=2, scratch=False, sections=('webs', 'only', 'order', 'unexplained')):
    """print the candidate-set comparison for one evaluated function (fe.evaluate result, source text, /FAsc items)"""
    rows = r['rows']
    srclines = text.split('\n')
    F = regprio.Func(items)
    lst = {off: (ln, tx) for off, ln, tx in F.ins}
    # our names of frame slots
    ournames = {}
    for k, o, cc in rows:
        if cc is None or cc[0] not in lst:
            continue
        s = regmap.slot_of(cc[1])
        m = re.search(r'_(\w+)\$\d*\[ebp', lst[cc[0]][1])
        if s and m:
            ournames.setdefault(s, m.group(1))
    imgnames = regmap.vote_slots(rows, ournames)
    oy = not any(x[2] and x[2][1] == 'mov ebp, esp' for x in rows[:3])
    regs = ['esi', 'edi', 'ebx'] + (['ebp'] if oy else [])
    # our tables from the listing
    otab = {}
    for off, ln, tx in F.ins:
        m = re.search(r'(\$L\d+)\[', tx)
        if tx.startswith('jmp') and m and m.group(1) in F.tables:
            otab[off] = [F.labels[x] for x in F.tables[m.group(1)] if x in F.labels]
    oins = [(x[0], x[1], x[3]) for k, o, x in rows if x is not None]
    iins = [(x[0], x[1], x[3]) for k, x, cc in rows if x is not None]
    ocode = Code(oins, otab)
    icode = Code(iins, image_tables(r['lo'], r['lo'] + max(o[0] for o in iins) + 16, rows))
    ow = side_analysis(ocode, regs, ournames, lst, srclines, scratch)
    iw = side_analysis(icode, regs, imgnames, None, srclines, scratch)
    # image webs get our line-based role names through the alignment where the role is a computed value
    o2i = {}
    for k, o, cc in rows:
        if o is not None and cc is not None:
            o2i[cc[0]] = o[0]
    i2o = {v: k for k, v in o2i.items()}
    for w in iw:
        if w['kind'] in ('var', 'ret'):
            d0 = icode.ins[min(w['defs'])][0]
            oo = i2o.get(d0)
            if oo is not None and oo in lst:
                w['role'] = re.sub(r'@[0-9a-f]+$', '@%d' % lst[oo][0], w['role'])
    oc = consistency(ow, regs)
    ic = consistency(iw, regs)
    print('FE %08X %s exact=%s diff=%d  regs=%s  blocks %d/%d loops %d/%d  webs %d/%d (image/ours)' % (
        addr, r['sym'][:50], r['exact'], r['diff'], ','.join(regs), len(icode.starts), len(ocode.starts),
        icode.nloops, ocode.nloops, len(iw), len(ow)))
    print('consistent image %d/%d ours %d/%d' % (ic[0], ic[1], oc[0], oc[1]))
    print('pairs image %d/%d ours %d/%d' % (pairs(iw, regs) + pairs(ow, regs)))
    if quiet:
        return
    def fmt(side, code, w):
        o0 = code.ins[min(w['live'])][0]
        o1 = code.ins[max(w['live'])][0]
        return '%-3s %-4s %-34s %4x-%-4x refs %-3d score %7.1f %s' % (side, w['reg'], w['role'][:34], o0, o1, len(w['refs']),
                                                                      w['score'], 'ok' if w['ok'] else 'NOT EXPLAINED')
    if 'webs' in sections:
        print('--- webs (image, then ours), by register and start')
    for side, code, ws in (('IMG', icode, iw), ('OUR', ocode, ow)):
        for w in sorted(ws, key=lambda w: (regs.index(w['reg']), min(w['live']))):
            if len(w['refs']) >= minrefs and 'webs' in sections:
                print(fmt(side, code, w))
    # candidate-set difference by role (registers ignored)
    def key(w):
        return re.sub(r'@\w+$', '', w['role'])
    ir = collections.defaultdict(list)
    orr = collections.defaultdict(list)
    for w in iw:
        ir[key(w)].append(w)
    for w in ow:
        orr[key(w)].append(w)
    print('--- candidates on one side only, or formed a different number of times (role: image n refs / ours n refs)')
    for k in sorted(set(ir) | set(orr)):
        a_, b_ = ir.get(k, []), orr.get(k, [])
        ra = sum(len(w['refs']) for w in a_)
        rb = sum(len(w['refs']) for w in b_)
        if len(a_) != len(b_) or abs(ra - rb) > 1:
            if max(ra, rb) < minrefs:
                continue
            kind = (a_ or b_)[0]['kind']
            print('  %-36s image %d webs %3d refs %-14s ours %d webs %3d refs %-14s | %s' % (
                k[:36], len(a_), ra, ','.join(sorted({w['reg'] for w in a_})), len(b_), rb, ','.join(sorted({w['reg'] for w in b_})),
                LEVERS.get(kind, '')[:90]))
    print('--- register order the sides disagree on (pairs of interfering webs, matched by role)')
    seen = set()
    for x in iw:
        for y in iw:
            if x is y or regs.index(x['reg']) >= regs.index(y['reg']) or not interferes(x, y):
                continue
            ox = [w for w in orr.get(key(x), [])]
            oy_ = [w for w in orr.get(key(y), [])]
            for p in ox:
                for q in oy_:
                    if interferes(p, q) and regs.index(p['reg']) > regs.index(q['reg']):
                        kk = (key(x), key(y))
                        if kk in seen:
                            continue
                        seen.add(kk)
                        print('  image %s (%s %.1f) above %s (%s %.1f); ours %s (%s %.1f) above (%s %.1f)' % (
                            key(x), x['reg'], x['score'], key(y), y['reg'], y['score'], key(y), q['reg'], q['score'], p['reg'], p['score']))
    for tag, bad in (('image', ic[2]), ('ours', oc[2])):
        if bad and 'unexplained' in sections:
            print('--- %s webs its own instruction-level scores do not explain' % tag)
            for w, rj, hold in bad[:12]:
                print('  %-34s %s %.1f: %s held by %s' % (w['role'][:34], w['reg'], w['score'], rj,
                                                       ', '.join('%s %.1f' % (h['role'][:24], h['score']) for h in hold) or 'nothing interfering'))


if __name__ == '__main__':
    main()

