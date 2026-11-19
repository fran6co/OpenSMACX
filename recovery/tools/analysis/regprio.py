#!/usr/bin/env python3
"""regprio.py <unit> <0xADDR> [--src ALT.cpp] [--pfx N] [--const N] [--top K] [--blocks] [--cand a,b] [--webs]
Rule-4 register priorities (MATCHING_GUIDE 2.7) for one function: the model's ranking of our source, the
registers VC6 actually gave our compile, and the registers the image gives the same roles.

Ours (from the /FAsc listing of our compile):
  blocks       a leader at every label, after every jump and ret; edges from jumps, fall-through and switch
               tables; loop depth = number of natural loops (back edge to a dominator) around the block; a block
               named in a switch jump table weighs 1/2.
  references   every textual occurrence of a parameter, local, `this` (prologue copy, members ending in `_`,
               and every line whose code uses this) or field of a local struct (`rect.left`) on a source line,
               and integer literals that occur --const times or more (`#7`, phantom constants; 0 disables).
               Address-taken locals, arrays and whole-struct names are left out (rule 10).  The listing maps
               the line to its instructions and so to blocks; a line spanning several blocks puts the
               reference in the block whose code names the variable (its `_name$` slot or the register our
               compile gave it), else in the line's first block; lines without code borrow the next line's.
  candidates   webs (rule 1): reaching definitions over our CFG join the definitions that reach a common use;
               `x = ...` starts a new definition, `x += ...`/`x++` reads and redefines.  A web needs two
               references.  Parameters, `this` and constants start at their first reference.  Several webs of
               one variable are named `x@<line of first reference>`.
  priority     sum over the web's references of n_b * 2^depth(b) (* 1/2 in a switch case block), n_b = number
               of distinct candidate webs referenced in block b.  Predicted register: served in priority order,
               the first of esi, edi, ebx (ebp under /Oy) not held by an already served web live in a common
               block (liveness over the CFG).
  ours         register tenures of our compile (regmap.py), named by the slot they load from, the prologue
               `mov r, ecx` (this), a write-back store, or the candidate assigned on the line where they start,
               then given to the web of that variable live at that point.
Image: the same tenures on the image side, named through the fe alignment (what our compile has in the aligned
rows, majority vote).  `mem` = only frame slot accesses, `pc:` = cached pieces loaded from the slot, `*` = more
than one register.

Output: the web table (priority, predicted, ours, image), the pairs whose order differs between the image and
ours ("image ranks A above B; ours ranks B above A") with the model's priorities and Δ and, for both webs, the
blocks contributing most (start offset, depth, n_b, refs, contribution, source lines).  A pair where the model
already agrees with the image but our compile does not means the model misses a candidate or a reference our
compile sees: the lines `our tenures outside the webs` and `our frame temporaries` list such candidates
(CSE'd addresses, a base-class `this`, struct fields, globals cached between calls)."""
import sys, os, re, subprocess, argparse, collections
H = os.path.dirname(os.path.abspath(__file__))
T = os.path.dirname(H)
sys.path.insert(0, T)
sys.path.insert(0, H)
import fe
import regmap

KEYWORDS = set('''return else delete new case goto sizeof if while for do switch break continue default typedef
    struct class union enum const static unsigned signed volatile register extern inline operator throw try catch
    public private protected virtual using namespace template typename this true false NULL'''.split())
TYPEWORDS = set('''int char short long unsigned signed void float double bool BOOL DWORD WORD BYTE UINT LPSTR LPCSTR
    size_t uint8_t uint16_t uint32_t int8_t int16_t int32_t HWND HDC RECT POINT'''.split())
REGORD = {'esi': 0, 'edi': 1, 'ebx': 2, 'ebp': 3}
FIELDS = {0: 'left', 4: 'top', 8: 'right', 12: 'bottom'}  # RECT; other structs show as name.+offset


# ------------------------------------------------------------------ listing and CFG
def cod_listing(unit, src, pfx, sym):
    """-> (items, frame): items ('line', n) | ('label', name) | ('ins', off, text) | ('dd', label)"""
    cmd = fe.compile_cmd(unit)
    asm = fe.SCR + '/rp_%d.cod' % os.getpid()
    obj = fe.SCR + '/rp_%d.obj' % os.getpid()
    out = []
    for t in cmd:
        if t.startswith('/Fo'):
            t = '/Fo' + obj
        elif t.startswith('/Fd') or t.startswith('--repro-root') or t == '/W4':
            continue
        elif t == fe.R + '/' + unit:
            t = os.path.abspath(src)
        out.append(t)
    out.insert(-1, '/FAsc')
    out.insert(-1, '/Fa' + asm)
    env = dict(os.environ, VC6_PREFIX='%s/.opensmacx/wine/worker-%s' % (fe.MAIN, pfx))
    subprocess.run(out, cwd=fe.SCR, env=env, capture_output=True, text=True)
    t = open(asm, errors='replace').read().replace('\r', '')
    os.remove(asm)
    os.path.exists(obj) and os.remove(obj)
    i = t.find(sym + ' PROC')
    e = t.find(sym + ' ENDP', i)
    head = t[t.rfind('SEGMENT', 0, i):i]
    frame = {}
    for m in re.finditer(r'^_(\w+?)\$(\d*) = (-?\d+)$', head, re.M):
        frame[m.group(1)] = int(m.group(3))
    items = []
    pend = None
    for l in t[i:e].split('\n'):
        m = re.match(r'; (\d+)\s+:', l)
        if m:
            items.append(('line', int(m.group(1))))
            continue
        m = re.match(r'(\$L\d+):$', l)
        if m:
            items.append(('label', m.group(1)))
            continue
        m = re.match(r'  ([0-9a-f]{5})\t([0-9a-f ]+)(?:\t+ ?(.*))?$', l)
        ins = None
        if m:
            off = int(m.group(1), 16)
            if m.group(3):
                ins = m.group(3)
            else:
                pend = off
                continue
        else:
            m = re.match(r'\t[0-9a-f ]+\t+ ?(\S.*)$', l)
            if m and pend is not None:
                off, ins, pend = pend, m.group(1), None
        if ins:
            ins = re.sub(r'\s+', ' ', ins.split(';')[0]).strip()
            m = re.match(r'DD (\$L\d+)$', ins)
            items.append(('dd', m.group(1)) if m else ('ins', off, ins))
    return items, frame


class Func:
    def __init__(self, items):
        self.ins = []          # (off, line, text)
        self.labels = {}       # label -> offset
        self.tables = collections.defaultdict(list)
        line = 0
        pending = []
        table = None
        for it in items:
            if it[0] == 'line':
                line = it[1]
            elif it[0] == 'label':
                pending.append(it[1])
            elif it[0] == 'dd':
                if pending:
                    table = pending[-1]
                    pending = []
                self.tables[table].append(it[1])
            else:
                for p in pending:
                    self.labels[p] = it[1]
                pending = []
                table = None
                self.ins.append((it[1], line, it[2]))
        self.end = self.ins[-1][0] + 1 if self.ins else 0
        leaders = {0}
        for k, (off, ln, tx) in enumerate(self.ins):
            mn = tx.split(' ')[0]
            if (mn.startswith('j') or mn == 'ret') and k + 1 < len(self.ins):
                leaders.add(self.ins[k + 1][0])
        leaders |= {v for v in self.labels.values() if v < self.end}
        self.starts = sorted(leaders)
        caseoffs = {self.labels[c] for tb in self.tables.values() for c in tb if c in self.labels}
        self.blocks = []
        for n, s in enumerate(self.starts):
            e = self.starts[n + 1] if n + 1 < len(self.starts) else self.end
            self.blocks.append(dict(n=n, start=s, end=e, depth=0, case=s in caseoffs, lines=set(), cands=set(),
                                    refs=collections.Counter(), last=None))
        for off, ln, tx in self.ins:
            b = self.blocks[self.block_of(off)]
            b['lines'].add(ln)
            b['last'] = tx
        nb = len(self.blocks)
        self.succ = [set() for _ in range(nb)]
        for b in self.blocks:
            tx = b['last'] or ''
            mn = tx.split(' ')[0]
            m = re.search(r'(\$L\d+)', tx)
            if mn.startswith('j'):
                if 'DWORD PTR' in tx and m and m.group(1) in self.tables:
                    for c in self.tables[m.group(1)]:
                        if c in self.labels:
                            self.succ[b['n']].add(self.block_of(self.labels[c]))
                elif m and m.group(1) in self.labels:
                    self.succ[b['n']].add(self.block_of(self.labels[m.group(1)]))
                if mn != 'jmp' and b['n'] + 1 < nb:
                    self.succ[b['n']].add(b['n'] + 1)
            elif mn != 'ret' and b['n'] + 1 < nb:
                self.succ[b['n']].add(b['n'] + 1)
        self.pred = [set() for _ in range(nb)]
        for u in range(nb):
            for v in self.succ[u]:
                self.pred[v].add(u)
        # dominators and natural loops
        full = set(range(nb))
        dom = [full] * nb
        dom[0] = {0}
        changed = True
        while changed:
            changed = False
            for v in range(1, nb):
                ps = [dom[p] for p in self.pred[v]]
                nd = (set.intersection(*ps) if ps else set()) | {v}
                if nd != dom[v]:
                    dom[v] = nd
                    changed = True
        self.loops = []
        for u in range(nb):
            for h in self.succ[u]:
                if h in dom[u]:
                    body = {h, u}
                    st = [u]
                    while st:
                        x = st.pop()
                        for p in self.pred[x]:
                            if p not in body:
                                body.add(p)
                                st.append(p)
                    self.loops.append((h, body))
        # several back edges to one header form one loop
        merged = collections.defaultdict(set)
        for h, body in self.loops:
            merged[h] |= body
        self.loops = list(merged.items())
        for h, body in self.loops:
            for x in body:
                self.blocks[x]['depth'] += 1

    def block_of(self, off):
        lo, hi = 0, len(self.starts) - 1
        while lo < hi:
            mid = (lo + hi + 1) // 2
            if self.starts[mid] <= off:
                lo = mid
            else:
                hi = mid - 1
        return lo


# ------------------------------------------------------------------ source
def strip_code(s):
    s = re.sub(r'//.*', '', s)
    s = re.sub(r'"(\\.|[^"\\])*"', '""', s)
    s = re.sub(r"'(\\.|[^'\\])*'", "''", s)
    return s


def func_source(text, addr):
    """-> (line number of the definition's first line, its lines)"""
    lines = text.split('\n')
    m = None
    for n, l in enumerate(lines):
        if l.startswith('// ORIGINAL: 0x%08X ' % addr):
            m = n
            break
    if m is None:
        raise SystemExit('no marker')
    n = m
    while lines[n].startswith('//') or not lines[n].strip():
        n += 1
    first = n
    depth = 0
    seen = False
    while n < len(lines):
        c = strip_code(lines[n])
        depth += c.count('{') - c.count('}')
        seen = seen or '{' in c
        if seen and depth <= 0:
            break
        n += 1
    return first + 1, lines[first:n + 1]


def declared_names(code, sig):
    """-> (names, params, excluded, structs)"""
    names, params = [], []
    m = re.search(r'\(([^()]*(?:\([^()]*\)[^()]*)*)\)\s*(?:const\s*)?\{', sig, re.S)
    if m:
        for p in m.group(1).split(','):
            w = re.findall(r'[A-Za-z_]\w*', p.split('=')[0])
            if len(w) >= 2 and w[-1] not in TYPEWORDS:
                params.append(w[-1])
    names += params
    decl = re.compile(r'(?:^|[;{}(]|\belse\b)\s*((?:const\s+|unsigned\s+|signed\s+|struct\s+|volatile\s+)*[A-Za-z_]\w*(?:::\w+)*(?:<[^;<>]*>)?)((?:\s*[*&]\s*|\s+)(?:const\s+)?)([A-Za-z_]\w*)\s*(?=[=;,\[)]|\(\s*[-\w])', re.M)
    for m in decl.finditer(code):
        ty, nm = m.group(1).split()[-1], m.group(3)
        if ty in KEYWORDS or nm in KEYWORDS:
            continue
        if not (ty in TYPEWORDS or re.match(r'[A-Z]', ty) or ty.endswith('_t') or '::' in m.group(1) or '*' in m.group(2)):
            continue
        if nm not in names:
            names.append(nm)
        rest = code[m.end():]
        k, lvl = 0, 0
        while k < len(rest) and not (rest[k] == ';' and lvl == 0):
            if rest[k] in '([{':
                lvl += 1
            elif rest[k] in ')]}':
                lvl -= 1
                if lvl < 0:
                    break
            elif rest[k] == ',' and lvl == 0:
                mm = re.match(r',\s*[*&]?\s*([A-Za-z_]\w*)\s*(?=[=;,\[])', rest[k:])
                if mm and mm.group(1) not in names:
                    names.append(mm.group(1))
            k += 1
    excl = set(re.findall(r'(?<![&\w])&\s*([A-Za-z_]\w*)\b(?!\s*[.\[]|\s*->)', code))
    for m in re.finditer(r'\b([A-Za-z_]\w*)[\s*]+([A-Za-z_]\w*)\s*\[[^\]]*\]\s*[;=]', code):
        if m.group(1) not in KEYWORDS:
            excl.add(m.group(2))
    structs = {nm for nm in names if re.search(r'\b%s\s*\.\s*[A-Za-z_]' % re.escape(nm), code)}
    return names, params, excl, structs


ASSIGN = re.compile(r'\s*(=(?!=)|[-+*/|&^%]=|<<=|>>=|\+\+|--)')


def refs_on_line(code, names, structs, member, consts):
    """-> [(key, kind, column)] kind: 'u' use, 'd' definition, 'ud' read and redefined"""
    out = []
    for m in re.finditer(r'(->|\.|::)?\s*\b([A-Za-z_]\w*)\b(\s*\.\s*([A-Za-z_]\w*))?', code):
        if m.group(1):
            continue
        nm = m.group(2)
        key = None
        if nm in structs:
            if m.group(4):
                key = nm + '.' + m.group(4)
        elif nm in names:
            key = nm
        elif member and (nm == 'this' or (nm.endswith('_') and len(nm) > 1)):
            out.append(('this', 'u', m.start()))
            continue
        if not key:
            continue
        pre = code[:m.start(2)]
        if re.search(r'(?:^|[;{(,]\s*)(?:const\s+|struct\s+|unsigned\s+)*([A-Za-z_][\w:]*)(?:\s*[*&]+\s*|\s+)$', pre) and \
                re.search(r'([A-Za-z_][\w:]*)(?:\s*[*&]+\s*|\s+)$', pre).group(1) not in KEYWORDS and \
                re.match(r'\s*[;,)\[]', code[m.end():]):
            continue  # bare declaration
        a = ASSIGN.match(code, m.end())
        b = re.search(r'(\+\+|--)\s*$', pre)
        if a and a.group(1) == '=':
            kind = 'd'
        elif a or b:
            kind = 'ud'
        else:
            kind = 'u'
        out.append((key, kind, m.start()))
    if consts:
        for m in re.finditer(r'(?<![\w.\[])(-?(?:0x[0-9a-fA-F]+|\d+))[uUlL]*\b(?!\s*\])', code):
            v = m.group(1)
            v = int(v, 16) if v.lstrip('-').startswith('0x') else int(v, 10)
            out.append(('#%d' % v, 'u', m.start()))
    return out


# ------------------------------------------------------------------ webs
class UF:
    def __init__(self):
        self.p = {}

    def find(self, x):
        self.p.setdefault(x, x)
        while self.p[x] != x:
            self.p[x] = self.p[self.p[x]]
            x = self.p[x]
        return x

    def union(self, a, b):
        self.p[self.find(a)] = self.find(b)


def build_webs(F, refs, entrydef):
    """refs: list of dicts(key, kind, block, seq).  -> {web id: dict(key, refs=[idx], live=set(blocks))}"""
    byvar = collections.defaultdict(list)
    for i, r in enumerate(refs):
        byvar[r['key']].append(i)
    webs = {}
    nb = len(F.blocks)
    for key, idxs in byvar.items():
        uf = UF()
        ev = collections.defaultdict(list)
        for i in idxs:
            ev[refs[i]['block']].append(i)
        for b in ev:
            ev[b].sort(key=lambda i: refs[i]['seq'])
        defs_in = {}
        gen = {}
        for b, lst in ev.items():
            d = [i for i in lst if refs[i]['kind'] in ('d', 'ud')]
            if d:
                gen[b] = {d[-1]}
        IN = [set() for _ in range(nb)]
        OUT = [set() for _ in range(nb)]
        IN[0] = {'E'} if key in entrydef else set()
        changed = True
        while changed:
            changed = False
            for b in range(nb):
                inn = set().union(*[OUT[p] for p in F.pred[b]]) if F.pred[b] else set()
                if b == 0 and key in entrydef:
                    inn |= {'E'}
                out = gen.get(b, inn)
                if inn != IN[b] or out != OUT[b]:
                    IN[b], OUT[b] = inn, out
                    changed = True
        orphans = []
        for b, lst in ev.items():
            cur = set(IN[b])
            for i in lst:
                uf.find(i)
                k = refs[i]['kind']
                if k in ('u', 'ud'):
                    for d in cur:
                        uf.union(i, d)
                    if not cur:
                        orphans.append(i)
                if k in ('d', 'ud'):
                    cur = {i}
        # a use no definition reaches (listing line attribution put the definition after it, or code motion):
        # join the nearest earlier definition in source order
        alldefs = sorted((refs[i]['line'], i) for i in idxs if refs[i]['kind'] in ('d', 'ud'))
        for i in orphans:
            prev = [d for ln, d in alldefs if ln <= refs[i]['line'] and d != i]
            if prev:
                uf.union(i, prev[-1])
            elif alldefs:
                uf.union(i, alldefs[0][1])
        groups = collections.defaultdict(list)
        for i in idxs:
            groups[uf.find(i)].append(i)
        for g, members in groups.items():
            webs[(key, g)] = dict(key=key, refs=sorted(members, key=lambda i: (F.blocks[refs[i]['block']]['start'], refs[i]['seq'])))
    for wid, w in webs.items():
        if w['key'].startswith('#'):
            # a constant is rematerialised: live in its blocks and the loops around them only
            live = {refs[i]['block'] for i in w['refs']}
            for h, body in F.loops:
                if live & body:
                    live |= body
            w['live'] = live
        else:
            w['live'] = liveness(F, refs, w['refs'])
    return webs


def liveness(F, refs, mem):
    """blocks where the web of these references is live: backward from every use whose block has no earlier
    definition of the web, stopping at definitions; a web without a definition starts at its first reference
    (loops around it excepted)."""
    mem = set(mem)
    refblocks = {refs[i]['block'] for i in mem}
    defblocks = {refs[i]['block'] for i in mem if refs[i]['kind'] in ('d', 'ud')}
    lo = min(F.blocks[b]['start'] for b in refblocks)
    live = set(refblocks)
    st = []
    for b in refblocks:
        lst = sorted((i for i in mem if refs[i]['block'] == b), key=lambda i: refs[i]['seq'])
        if refs[lst[0]]['kind'] in ('u', 'ud'):
            st.append(b)
    seen = set()
    while st:
        b = st.pop()
        for p in F.pred[b]:
            if p in seen:
                continue
            seen.add(p)
            if F.blocks[p]['start'] < lo and p not in defblocks and not any(p in body and b in body for h, body in F.loops):
                continue
            live.add(p)
            if p not in defblocks:
                st.append(p)
    return live


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('unit')
    ap.add_argument('addr')
    ap.add_argument('--src')
    ap.add_argument('--pfx', default=os.environ.get('FE_PFX_BASE', '2'))
    ap.add_argument('--const', type=int, default=3, help='a literal with this many occurrences is a candidate (0: none)')
    ap.add_argument('--top', type=int, default=4, help='contributing blocks shown per web')
    ap.add_argument('--blocks', action='store_true', help='print every block with its candidate webs')
    ap.add_argument('--cand', default='', help='also print the contributing blocks of these webs (comma list)')
    ap.add_argument('--min', type=float, default=0, help='hide webs below this priority in the table')
    ap.add_argument('--pairs', type=int, default=25, help='disagreeing pairs shown')
    ap.add_argument('--quiet', action='store_true', help='summary lines only (validation runs)')
    ap.add_argument('--model', default='new', choices=('new', 'old'),
                    help='new: rule 4 minus the live-through penalty, rule-4c tie order (2026-11-19); old: rule 4 only')
    a = ap.parse_args()
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
    items, frame = cod_listing(unit, src, a.pfx, r['sym'])
    F = Func(items)
    first, flines = func_source(text, addr)
    last = first + len(flines) - 1
    stripped = [strip_code(l) for l in flines]
    body = '\n'.join(stripped)
    names, params, excl, structs = declared_names(body[body.find('{') + 1:], body)
    names_set = set(names) - excl
    structs -= excl
    oy = not any(x[2] and x[2][1] == 'mov ebp, esp' for x in r['rows'][:3])
    regs = ['esi', 'edi', 'ebx'] + (['ebp'] if oy else [])
    member = r['sym'].startswith('?') and re.search(r'@@[QAIUEM]', r['sym']) is not None
    linetxt = {first + i: stripped[i] for i in range(len(flines))}
    lineof = {off: ln for off, ln, tx in F.ins}
    textof = {off: tx for off, ln, tx in F.ins}
    offs = [off for off, ln, tx in F.ins]

    # ---- our tenures named by variable
    our = regmap.Side(r['rows'], 2, 0)
    ten = our.tenures(regs)

    def assigned(ln):
        out = []
        for k, kind, col in refs_on_line(linetxt.get(ln, ''), names_set, structs, False, 0):
            if kind in ('d', 'ud'):
                out.append(k)
        return out

    for t in ten:
        nm = None
        tx = textof.get(t['start'], '')
        m = re.search(r'_(\w+?)\$\d*\[ebp(?:\+(\d+))?\]', tx.split(',', 1)[1] if ',' in tx else '')
        if t['role'] == 'this':
            nm = 'this'
        elif m:
            nm = m.group(1)
            if nm in structs:
                nm = '%s.%s' % (nm, FIELDS.get(int(m.group(2) or 0), '+%s' % (m.group(2) or 0)))
            t['piece'] = True
        else:
            for off in offs:
                if t['start'] < off <= t['end']:
                    mm = re.match(r'mov DWORD PTR _(\w+?)\$\d*\[ebp\], %s$' % t['reg'], textof[off])
                    if mm:
                        nm = mm.group(1)
                        break
            if nm is None:
                asg = assigned(lineof.get(t['start'], 0))
                if len(set(asg)) == 1:
                    nm = asg[0]
                elif re.fullmatch(r'-?\d+|0x[0-9a-f]+', t['role']):
                    nm = '#%d' % int(t['role'].replace('0xffffffff', '-1'), 0)
        t['name'] = nm or 'expr@%d' % lineof.get(t['start'], 0)
    regname = collections.defaultdict(dict)  # off -> reg -> tenure
    for t in ten:
        for off in offs:
            if t['start'] <= off <= t['end']:
                regname[off][t['reg']] = t

    # ---- references
    linecode = collections.defaultdict(list)
    for off, ln, tx in F.ins:
        linecode[ln].append(off)
    raw = []  # (key, kind, line, col)
    for i, s in enumerate(stripped):
        if i == 0:
            continue
        for k, kind, col in refs_on_line(s, names_set, structs, member, a.const):
            raw.append((k, kind, first + i, col))
    if member:
        raw.append(('this', 'd', -1, 0))
        thisreg = our.this
        srcthis = {ln for k, kind, ln, col in raw if k == 'this'}
        for ln, oo in linecode.items():
            if ln in srcthis or not (first < ln <= last):
                continue
            if any(re.search(r'_this\$', textof[o]) or (thisreg and regname[o].get(thisreg, {}).get('name') == 'this'
                   and regmap.uses_reg(textof[o], thisreg) and not textof[o].startswith('push ' + thisreg)) for o in oo):
                raw.append(('this', 'u', ln, 0))
    cnt = collections.Counter(k for k, kind, ln, col in raw)
    consts = {k for k in cnt if k.startswith('#') and cnt[k] >= a.const}
    raw = [x for x in raw if not x[0].startswith('#') or x[0] in consts]
    codelines = sorted(ln for ln in linecode if first <= ln <= last)

    def blocks_of_line(ln):
        if ln == -1:
            return [0]
        if ln not in linecode:
            nxt = [x for x in codelines if x > ln]
            prv = [x for x in codelines if x < ln]
            ln = nxt[0] if nxt else (prv[-1] if prv else None)
            if ln is None:
                return [0]
        return sorted({F.block_of(o) for o in linecode[ln]})

    def evidence(k, b, ln):
        base = k.split('.')[0]
        for off in linecode.get(ln, ()):
            if F.block_of(off) != b:
                continue
            tx = textof[off]
            if k.startswith('#'):
                v = int(k[1:])
                if re.search(r'(?<![\w$])(%d|0x%x)\b' % (v, v & 0xffffffff), tx) or (v == 0 and re.match(r'xor (\w+), \1$', tx)):
                    return True
                continue
            if re.search(r'_%s\$' % re.escape(base), tx):
                return True
            for rg, t in regname[off].items():
                if t['name'] == k and regmap.uses_reg(tx, rg):
                    return True
        return False

    refs = []
    for k, kind, ln, col in raw:
        bs = blocks_of_line(ln)
        pick = bs[0]
        if len(bs) > 1:
            ev = [b for b in bs if evidence(k, b, ln)]
            if ev:
                pick = ev[0] if kind == 'u' else ev[-1]
        seq = ln * 1000 + (999 if kind == 'd' and ln != -1 else col)
        refs.append(dict(key=k, kind=kind, block=pick, line=ln, seq=seq if ln != -1 else -1))
    entrydef = set(params) | {'this'} | consts
    webs = build_webs(F, refs, entrydef)
    # compiler temporaries our compile shows: registers loaded with something that is no source variable
    # (an address CSE such as `lea edi, [esi+0x444]` = a base-class `this`, a cached global, a common
    # subexpression) and unnamed frame temporaries (`-4+[ebp]`).  Each instruction using one is a reference.
    pseudo = collections.defaultdict(list)

    def lea_label(off, reg):
        for o2 in reversed([o for o in offs if o <= off][-8:]):
            m = re.match(r'lea %s, DWORD PTR \[(\w+)\+(\d+)\]$' % reg, textof[o2])
            if m:
                bt = regname[o2].get(m.group(1))
                if bt is not None and bt['name'] == 'this':
                    return 'this+0x%x' % int(m.group(2))
                return None
        return None
    tmplabel = {}
    for off in offs:
        m = re.match(r'mov DWORD PTR (-\d+)\+\[ebp\], (\w+)$', textof[off])
        if m and m.group(1) not in tmplabel:
            tmplabel[m.group(1)] = lea_label(off, m.group(2)) or 'tmp[%s]' % m.group(1)
    for off in offs:
        m = re.search(r'DWORD PTR (-\d+)\+\[ebp\]', textof[off])
        if m:
            lab = tmplabel.get(m.group(1), 'tmp[%s]' % m.group(1))
            kind = 'd' if textof[off].startswith('mov DWORD PTR ' + m.group(1)) else 'u'
            refs.append(dict(key='T:' + lab, kind=kind, block=F.block_of(off), line=lineof[off], seq=off))
            pseudo[lab].append(len(refs) - 1)
    for t in ten:
        if not t['name'].startswith('expr@') or t['uses'] < 2:
            continue
        tx = textof.get(t['start'], '')
        lab = lea_label(t['start'], t['reg'])
        m = re.search(r'DWORD PTR (-\d+)\+\[ebp\]', tx)
        if m:
            lab = tmplabel.get(m.group(1), 'tmp[%s]' % m.group(1))
        if lab is None:
            lab = '%s:%s' % (t['name'], t['reg'])
        t['pseudo'] = 'T:' + lab
        for off in offs:
            if t['start'] <= off <= t['end'] and (off == t['start'] or regmap.uses_reg(textof[off], t['reg'])):
                if off == t['start'] and m:
                    continue  # the slot read is already a reference
                refs.append(dict(key='T:' + lab, kind='d' if off == t['start'] and not m else 'u', block=F.block_of(off), line=lineof[off], seq=off))
                pseudo[lab].append(len(refs) - 1)
    for lab, idxs in pseudo.items():
        webs[('T:' + lab, 0)] = dict(key='T:' + lab, refs=idxs, live=liveness(F, refs, idxs))
    nweb = collections.Counter(w['key'] for w in webs.values())
    for wid, w in webs.items():
        ln = min(refs[i]['line'] for i in w['refs'])
        w['name'] = w['key'] if nweb[w['key']] == 1 else '%s@%d' % (w['key'], ln)
    cands = {wid for wid, w in webs.items() if len(w['refs']) >= 2}
    webof = {}
    for wid in cands:
        for i in webs[wid]['refs']:
            webof[i] = wid
    for i, rf in enumerate(refs):
        if i in webof:
            F.blocks[rf['block']]['cands'].add(webof[i])
            F.blocks[rf['block']]['refs'][webof[i]] += 1
    prio = collections.Counter()
    contrib = collections.defaultdict(collections.Counter)
    for i, rf in enumerate(refs):
        if i not in webof:
            continue
        B = F.blocks[rf['block']]
        wgt = len(B['cands']) * 2 ** B['depth'] * (0.5 if B['case'] else 1)
        prio[webof[i]] += wgt
        contrib[webof[i]][rf['block']] += wgt
    holes = collections.defaultdict(list)
    if a.model != 'old':
        # live-through penalty (guide 2.7 rule 4b): half the weight of every block the web is live through
        # without a reference while other candidates are referenced there
        for w in cands:
            own = {refs[i]['block'] for i in webs[w]['refs']}
            for b in webs[w]['live'] - own:
                B = F.blocks[b]
                if B['cands']:
                    pen = 0.5 * len(B['cands']) * 2 ** B['depth'] * (0.5 if B['case'] else 1)
                    prio[w] -= pen
                    contrib[w][b] -= pen
                    holes[w].append((b, pen))
    first_ref = lambda w: min(webs[w]['refs'], key=lambda i: (F.blocks[refs[i]['block']]['start'], refs[i]['seq']))
    if a.model == 'old':
        order = sorted(cands, key=lambda w: (-prio[w], F.blocks[refs[webs[w]['refs'][0]]['block']]['start']))
    else:
        # exact ties (rule 4c): the web starting in the later block first, then the earlier first reference
        order = sorted(cands, key=lambda w: (-prio[w], -F.blocks[refs[first_ref(w)]['block']]['start'],
                                             refs[first_ref(w)]['seq']))
    pred = {}
    for w in order:
        taken = {pred[o] for o in pred if pred[o] != 'mem' and webs[o]['live'] & webs[w]['live']}
        free = [g for g in regs if g not in taken]
        pred[w] = free[0] if free else 'mem'

    # ---- which web is a variable's at a block
    byvar = collections.defaultdict(list)
    for wid in webs:
        byvar[webs[wid]['key']].append(wid)

    def web_at(var, b):
        ws = byvar.get(var, [])
        hit = [w for w in ws if any(refs[i]['block'] == b for i in webs[w]['refs'])]
        if not hit:
            hit = [w for w in ws if b in webs[w]['live']]
        if not hit and len(ws) == 1:
            hit = ws
        return hit[0] if hit else None
    for t in ten:
        if t.get('pseudo'):
            t['web'] = (t['pseudo'], 0)
            continue
        t['web'] = web_at(t['name'], F.block_of(t['start'])) if t['name'] else None
        if t['web'] is None and t['name'] and nweb.get(t['name']):
            for off in offs:
                if t['start'] <= off <= t['end'] and regmap.uses_reg(textof[off], t['reg']):
                    t['web'] = web_at(t['name'], F.block_of(off))
                    if t['web']:
                        break
        if t['web'] is None and byvar.get(t['name']):
            # fall back to the web whose source lines are nearest the tenure's line
            ln = lineof.get(t['start'], 0)
            t['web'] = min(byvar[t['name']], key=lambda w: min(abs(refs[i]['line'] - ln) for i in webs[w]['refs']))
    for off in regname:
        for rg in regname[off]:
            pass

    def summarize(tens):
        res = collections.defaultdict(collections.Counter)
        pieces = collections.Counter()
        for t in tens:
            w = t.get('web')
            if w is None:
                continue
            res[w][t['reg']] += t['uses']
            pieces[w] += bool(t.get('piece'))
        out = {}
        for w, cc in res.items():
            g = cc.most_common(1)[0][0]
            out[w] = ('pc:' if pieces[w] else '') + g + ('*' if len(cc) > 1 else '')
        return out
    ours = summarize(ten)
    # ---- image tenures named through the alignment
    img = regmap.Side(r['rows'], 1, r['lo'])
    iten = img.tenures(regs)
    for t in iten:
        vote = collections.Counter()
        for kk, o, cc in r['rows']:
            if o is None or cc is None:
                continue
            ia = r['lo'] + o[0]
            if not (t['start'] <= ia <= t['end']) or not regmap.uses_reg(o[1], t['reg']):
                continue
            b = F.block_of(cc[0])
            for x in regmap.counterpart(o[1], cc[1], t['reg']):
                if x in regmap.CS:
                    ot = regname[cc[0]].get(x)
                    if ot is not None and ot.get('web') is not None:
                        vote[ot['web']] += 1
                    elif ot is not None:
                        vote[ot['name']] += 1
                elif x.startswith('['):
                    m = re.search(r'_(\w+?)\$', textof.get(cc[0], ''))
                    if m:
                        w = web_at(m.group(1), b)
                        vote[w if w is not None else m.group(1)] += 1
        t['web'] = None
        if t['role'] == 'this':
            t['web'] = web_at('this', 0)
        elif vote:
            t['web'] = vote.most_common(1)[0][0]
        t['piece'] = bool(t['slot'])
    imgs = summarize([t for t in iten if t['web'] in webs])
    for w in cands:
        if w not in ours and webs[w]['key'].split('.')[0] in frame:
            ours[w] = 'mem'
        if w not in imgs:
            imgs[w] = 'mem' if any(webs[w]['key'].split('.')[0] in (t['web'] or '') for t in iten if isinstance(t['web'], str)) else '-'
    nm = lambda w: webs[w]['name'] if w in webs else str(w)

    # ---- report
    print('FE %08X %s exact=%s diff=%d  blocks=%d loops=%d  regs=%s  webs=%d' % (
        addr, r['sym'][:50], r['exact'], r['diff'], len(F.blocks), len(F.loops), ','.join(regs), len(cands)))
    if not a.quiet:
        print('%-24s %9s %4s %-5s %-9s %-9s %s' % ('web', 'prio', 'refs', 'pred', 'ours', 'image', 'lines'))
    agree = n = 0
    for w in order:
        o = ours.get(w, '-')
        oreg = o.replace('pc:', '').rstrip('*')
        if oreg in regs and not webs[w]['key'].startswith(('#', 'T:')):
            n += 1
            agree += oreg == pred[w]
        if prio[w] < a.min or a.quiet:
            continue
        flag = '' if pred[w] == oreg or (pred[w] == 'mem' and oreg in ('mem', '-')) else '  <- model'
        lns = [refs[i]['line'] for i in webs[w]['refs'] if refs[i]['line'] > 0]
        print('%-24s %9.1f %4d %-5s %-9s %-9s %s-%s%s' % (nm(w)[:24], prio[w], len(webs[w]['refs']), pred[w], o, imgs.get(w, '?'),
                                                    min(lns) if lns else '-', max(lns) if lns else '-', flag))
    pa = pt = 0
    single = {w: ours[w] for w in order if ours.get(w, '-') in regs}
    for i, x in enumerate(order):
        for y in order[i + 1:]:
            if x in single and y in single and single[x] != single[y] and webs[x]['live'] & webs[y]['live'] \
                    and prio[x] != prio[y]:
                pt += 1
                pa += REGORD[single[x]] < REGORD[single[y]]
    print('model predicts %d of %d enregistered source-variable webs of our compile; orders %d of %d interfering register pairs' % (agree, n, pa, pt))
    if a.quiet:
        return
    other = collections.Counter()
    for t in ten:
        if t.get('web') not in cands and t['uses'] >= 2:
            other[(t['reg'], t['name'])] += t['uses']
    if other:
        print('our tenures outside the webs: ' + ', '.join('%s %s r%d' % (g, x, u) for (g, x), u in other.most_common(10)))
    tmp = collections.defaultdict(list)
    for off, ln, tx in F.ins:
        m = re.search(r'DWORD PTR (-\d+)\+\[ebp\]', tx)
        if m:
            tmp[m.group(1)].append(ln)
    if tmp:
        print('our frame temporaries: ' + ', '.join('[%s] x%d L%s' % (k, len(v), ','.join(str(x) for x in sorted(set(v))[:5])) for k, v in tmp.items()))
    io = collections.Counter()
    for t in iten:
        if t['web'] not in cands and t['uses'] >= 2:
            io[(t['reg'], str(t['web']) if t['web'] is not None else ('[%s]' % t['slot'] if t['slot'] else t['role']))] += t['uses']
    if io:
        print('image tenures outside the webs: ' + ', '.join('%s %s r%d' % (g, x, u) for (g, x), u in io.most_common(10)))

    def rank(s):
        return REGORD.get(s.replace('pc:', '').rstrip('*'), 9)

    def show(w):
        print('    %s prio %.1f:' % (nm(w), prio[w]), end='')
        for b, wgt in contrib[w].most_common(a.top):
            B = F.blocks[b]
            lines = sorted({refs[i]['line'] for i in webs[w]['refs'] if refs[i]['block'] == b})
            print('  [%x d%d n%d%s x%d =%.0f L%s]' % (B['start'], B['depth'], len(B['cands']), ' case' if B['case'] else '', B['refs'][w], wgt,
                                                   ','.join(str(x) for x in lines)), end='')
        print()
    def tie_advice(hi_, lo_):
        """the image wants hi_ above lo_: say which tie-break or near-tie lever (guide 2.7 'Tie-breaking') does it"""
        d = prio[hi_] - prio[lo_]
        fh, fl = first_ref(hi_), first_ref(lo_)
        bh, bl = refs[fh]['block'], refs[fl]['block']
        sh, sl = F.blocks[bh]['start'], F.blocks[bl]['start']
        tie_hi = (-sh, refs[fh]['seq']) < (-sl, refs[fl]['seq'])
        pen = lambda w: sum(p for b, p in holes[w])
        if d == 0:
            print('    TIE (delta 0): tie order (later start block, then earlier first reference) puts %s first: %s'
                  % (nm(hi_) if tie_hi else nm(lo_), 'the image agrees, so ours is not an exact tie (check the candidate set)'
                     if tie_hi else 'to give the image order, start %s in a later block than %s (define it after the branch '
                     'that precedes %s) or, in one block, reference it first' % (nm(hi_), nm(lo_), nm(lo_))))
        elif abs(d) <= 2 * max(1, min(len(F.blocks[refs[i]['block']]['cands']) for i in webs[lo_]['refs'] + webs[hi_]['refs'])):
            print('    NEAR TIE (delta %+.1f): one reference moves it.' % d, end='')
            if pen(hi_) or pen(lo_):
                print(' live-through penalty %s %.1f [%s], %s %.1f [%s]: ending a range before those blocks (reread its'
                      ' source instead of keeping the variable) or referencing it there removes the penalty.' % (
                          nm(hi_), pen(hi_), ' '.join('%x' % F.blocks[b]['start'] for b, p in holes[hi_][:6]),
                          nm(lo_), pen(lo_), ' '.join('%x' % F.blocks[b]['start'] for b, p in holes[lo_][:6])), end='')
            print(' Tie order would put %s first.' % (nm(hi_) if tie_hi else nm(lo_)))

    print('--- disagreements (image order vs ours, interfering webs)')
    shown = set()
    npairs = 0
    for i, x in enumerate(order):
        for y in order[i + 1:]:
            if not (webs[x]['live'] & webs[y]['live']):
                continue
            ix, iy = rank(imgs.get(x, '-')), rank(imgs.get(y, '-'))
            ox, oy_ = rank(ours.get(x, '-')), rank(ours.get(y, '-'))
            if ix == iy or ox == oy_ or imgs.get(x) == '-' or imgs.get(y) == '-':
                continue
            if (ix < iy) != (ox < oy_):
                npairs += 1
                if npairs > a.pairs:
                    continue
                hi_, lo_ = (x, y) if ix < iy else (y, x)
                print('image ranks %s (%s) above %s (%s); ours %s (%s) above %s (%s); model %.1f vs %.1f (delta %+.1f)%s' % (
                    nm(hi_), imgs.get(hi_), nm(lo_), imgs.get(lo_), nm(lo_), ours.get(lo_, '-'), nm(hi_), ours.get(hi_, '-'),
                    prio[hi_], prio[lo_], prio[hi_] - prio[lo_],
                    '  [model agrees with image]' if prio[hi_] > prio[lo_] else ''))
                for k in (hi_, lo_):
                    if k not in shown:
                        show(k)
                        shown.add(k)
                if a.model != 'old':
                    tie_advice(hi_, lo_)
    if npairs > a.pairs:
        print('(%d more pairs; --pairs N)' % (npairs - a.pairs))
    for k in a.cand.split(','):
        for w in order:
            if k and nm(w) == k and w not in shown:
                show(w)
    if a.blocks:
        print('--- blocks')
        for B in F.blocks:
            if B['cands']:
                print('  %x-%x d%d%s n%d L%s: %s' % (B['start'], B['end'], B['depth'], ' case' if B['case'] else '', len(B['cands']),
                                                  ','.join(str(x) for x in sorted(l for l in B['lines'] if first <= l <= last)[:6]),
                                                  ' '.join('%s:%d' % (nm(w), n_) for w, n_ in sorted(B['refs'].items(), key=lambda kv: nm(kv[0])))))


if __name__ == '__main__':
    main()
