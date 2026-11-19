#!/usr/bin/env python3
"""addrhome.py [unit ...] [--addr A[,A]] [--from FEALL.json] [--all] [--min-reloads N] [--json OUT] [-v]
Homed-address screen (MATCHING_GUIDE 2.7 "Homed addresses"): frame slots that hold an ADDRESS or a scaled
array offset, in the image and in our compile, for every function that is not fe-exact.

A slot is listed when a register defined by an address computation is stored into it and the slot is
reloaded.  Kind of the stored value (from the register's definition, followed through copies):

  Pg   `lea r,[i*k + G]`       element address of a global array with scale 1/2/4/8 (one lea)
  Po   `lea r,[off + G + d]`   or `add r, G`: absolute pointer made from a scaled offset
  Pm   `lea r,[base + d]`      address of a member through a pointer (`&this->m`, `&p->m`)
  Pl   `lea r,[ebp - d]`       address of a local (listed with -v only)
  OFF  `imul r,x,K`, `shl`, `lea r,[x + x*k]`: the scaled offset of `Obj[i]` (reads keep the relocation)

Reads through the reloaded value classify the slot again, independently of the definition:
  p = `[r + d]` without relocation (a pointer),  o = `[r + G + d]` unscaled with relocation (an offset
  temporary),  i = `[r*k + G]` (an index),  t = passed in ecx to a call (`this`),  a = pushed (an argument),
  v = other uses.

Each image home is paired with ours through the fe alignment of its store (then by definition text):
  (a) IMG-PTR   image homes a pointer, ours has no such home (re-forms the address, or keeps it in a
                register): candidate for a named pointer local, BUT see the caveat below
  (b) IMG-OFF / OURS-OFF   the offset temporary is homed on one side only: a register-ranking question
  (c) OURS-PTR  ours homes a pointer the image never forms: a named pointer of ours to remove
  =             both sides home it (slot offsets may differ)
  NULLADJ       the sides differ in the number of null-tested base adjustments (`je; lea r,[b+K]; jmp;
                xor r,r`): a `static_cast<Base *>(ptr)` / `Base &b = member` of ours where the image calls
                the base member directly (bare `lea ecx,[b+K]`), or the reverse
  UNFOLD        (registers and slots alike) the image forms `lea r,[b + Kt]` and reaches the member's parts
                as `[r + d]` / `lea x,[r + d]`, where ours has the folded `[b + Kt + d]`: an expanded inline
                member function (its `this` is the member's address: BaseWin::init's `lea ecx,[ebx+0x444]`
                for set_font_redraw), a pointer or reference local, or a homed address CSE

Caveat (custom_planet 0x0058C2A0, 2026-11-19): VC6 homes `&arr[i]` by itself.  With `arr[i]` read before a
long region and written after it, the compiler CSEs the element address (kind Pg, reads `[r]` without a
relocation), spills it and lets `i` die; named pointer locals there gave 84 rows against 11 without them.
Named pointer locals for `&this->m` compile to the same code as none (BaseWin::init 0x0041E8D0).  So Pg and Pm
homes are NOT evidence of a source pointer: the temporary is the compiler's, and it is homed when nothing later
names its base.  Po (an absolute pointer built from the imul offset, reads without relocation) is a member call
on the element or a pointer local (regalloc/c_addrhome.spec idx/W).  Slot rows are noisy (shared slots, linear
register tracking); the UNFOLD and NULLADJ rows are the ones to act on.  Screen only.
Env: FE_PFX_BASE, FE_JOBS."""
import sys, os, re, json, argparse, collections
from concurrent.futures import ThreadPoolExecutor
T = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, T)
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import fe

REGS = ('eax', 'ecx', 'edx', 'ebx', 'esi', 'edi', 'ebp')
SCRATCH = ('eax', 'ecx', 'edx')
MEM = re.compile(r'\[([^\]]+)\]')
NUM = r'(?:0x[0-9a-f]+|\d+)'


def parse_mem(inner):
    """-> dict(base, index, scale, reloc, disp, regs)"""
    d = dict(base=None, index=None, scale=1, reloc='<r>' in inner, disp=0)
    s = inner.replace('- ', '+ -')
    for part in [p.strip() for p in s.split('+')]:
        if not part or part == '<r>':
            continue
        m = re.fullmatch(r'(\w+)\*(\d)', part)
        if m:
            d['index'], d['scale'] = m.group(1), int(m.group(2))
        elif part in REGS or part == 'esp':
            if d['base'] is None:
                d['base'] = part
            else:
                d['index'] = part
        else:
            try:
                d['disp'] = int(part.replace(' ', ''), 0)
            except ValueError:
                pass
    return d


class Side:
    """Linear register tracking over one listing: homes[slot] = dict(defs=[(off, kind, text)], loads, uses)"""

    def __init__(self, ins):
        self.ins = ins
        self.labels = {x[3] for x in ins if x[3] is not None}
        self.ebp = len(ins) > 1 and ins[0][1] == 'push ebp' and ins[1][1] == 'mov ebp, esp'
        self.homes = collections.defaultdict(lambda: dict(defs=[], loads=[], uses=collections.Counter(), stores=0))
        self.forms = collections.Counter()
        self.ecx_fresh = set()
        self.run()

    def slot_of(self, mem, depth):
        if self.ebp:
            if mem['base'] == 'ebp' and mem['index'] is None and not mem['reloc'] and mem['disp'] < 0:
                return mem['disp']
            return None
        if mem['base'] == 'esp' and mem['index'] is None and not mem['reloc']:
            return mem['disp'] - depth
        return None

    def run(self):
        st = {}  # reg -> (kind, def off, def text) or ('slot', S)
        depth = 0
        frame_base = 'ebp' if self.ebp else 'esp'
        for off, tx, sz, tgt in self.ins:
            if off in self.labels:
                for r in SCRATCH:
                    st.pop(r, None)
            mn, _, ops = tx.partition(' ')
            opl = [o.strip() for o in ops.split(',')] if ops else []
            mems = [(k, parse_mem(m.group(1))) for k, o in enumerate(opl) for m in [MEM.search(o)] if m]
            # uses of slot-loaded registers inside memory operands
            for k, mem in mems:
                if mn == 'lea':
                    continue
                for role in ('base', 'index'):
                    r = mem[role]
                    if r in st and st[r][0] == 'slot':
                        h = self.homes[st[r][1]]
                        if mem['reloc']:
                            h['uses']['i' if (role == 'index' and mem['scale'] > 1) else 'o'] += 1
                        else:
                            h['uses']['p' if role == 'base' else 'v'] += 1
            dst = opl[0] if opl else None
            src = opl[1] if len(opl) > 1 else None
            if dst == 'ecx' and mn in ('mov', 'lea'):
                self.ecx_fresh = {'ecx'}   # ecx written since the last call: a `this` argument
            elif mn == 'call':
                pass
            if mn == 'lea' and dst in REGS:
                mem = mems[0][1]
                for role in ('base', 'index'):
                    r = mem[role]
                    if r in st and st[r][0] == 'slot':
                        self.homes[st[r][1]]['uses']['o' if mem['reloc'] else ('p' if mem['index'] is None else 'v')] += 1
                bk = st.get(mem['base'], (None,))[0]
                if mem['reloc']:
                    if mem['index'] is not None and mem['base'] is None:
                        kind = 'Pg'
                    elif mem['base'] is not None and mem['index'] is None and bk in ('OFF', 'slot'):
                        kind = 'Po'
                    else:
                        kind = 'Pg'
                    self.forms['lea<r>'] += 1
                elif mem['base'] == frame_base and mem['index'] is None:
                    kind = 'Pl'
                elif mem['base'] in ('ebp', 'esp'):
                    kind = 'Pl'
                elif mem['index'] is not None and mem['disp'] == 0:
                    kind = 'OFF' if (mem['base'] in (None, mem['index']) or bk == 'OFF') else 'other'
                elif mem['base'] is not None and mem['index'] is None:
                    kind = 'OFF' if bk == 'OFF' and mem['disp'] == 0 else 'Pm'
                else:
                    kind = 'other'
                st[dst] = (kind, off, tx)
            elif mn == 'imul' and dst in REGS and len(opl) == 3:
                st[dst] = ('OFF', off, tx)
                self.forms['imul ' + opl[2]] += 1
            elif mn == 'shl' and dst in REGS:
                if st.get(dst, (None,))[0] in ('OFF',):
                    st[dst] = ('OFF', off, st[dst][2] + '; ' + tx)
                else:
                    st.pop(dst, None)
            elif mn == 'add' and dst in REGS and src == '<r>':
                was = st.get(dst, (None,))
                if was[0] == 'slot':
                    self.homes[was[1]]['uses']['o'] += 1
                st[dst] = ('Po', off, (was[2] + '; ' if was[0] == 'OFF' else '') + tx)
            elif mn == 'mov' and dst in REGS and src in REGS:
                if src in st:
                    st[dst] = st[src]
                else:
                    st.pop(dst, None)
            elif mn == 'mov' and dst in REGS and mems and mems[0][0] == 1:
                s = self.slot_of(mems[0][1], depth) if 'dword' in opl[1] or 'ptr' not in opl[1] else None
                if s is not None:
                    st[dst] = ('slot', s)
                    self.homes[s]['loads'].append(off)
                else:
                    st.pop(dst, None)
            elif mn == 'mov' and mems and mems[0][0] == 0:
                s = self.slot_of(mems[0][1], depth)
                if s is not None:
                    h = self.homes[s]
                    h['stores'] += 1
                    if src in st and st[src][0] != 'slot':
                        h['defs'].append((off, st[src][0], st[src][2]))
                    elif src in st and st[src][0] == 'slot' and st[src][1] != s:
                        h['defs'].append((off, 'copy', 'slot %+#x' % st[src][1]))
                    else:
                        h['defs'].append((off, 'val', tx))
            elif mn == 'call':
                if st.get('ecx', (None,))[0] == 'slot' and 'ecx' in self.ecx_fresh:
                    self.homes[st['ecx'][1]]['uses']['t'] += 1
                for r in SCRATCH:
                    st.pop(r, None)
                self.ecx_fresh = set()
            elif mn == 'push':
                if dst in st and st[dst][0] == 'slot':
                    self.homes[st[dst][1]]['uses']['a'] += 1
            elif mn in ('cmp', 'test') or mn.startswith('j') or mn.startswith('f'):
                pass
            elif mn in ('cdq', 'idiv', 'div'):
                st.pop('eax', None)
                st.pop('edx', None)
            elif mn == 'pop' and dst in REGS:
                st.pop(dst, None)
            elif mn == 'xchg':
                st.pop(opl[0], None)
                st.pop(opl[1], None)
            elif mn == 'rep':
                for r in ('ecx', 'esi', 'edi'):
                    st.pop(r, None)
            elif dst in REGS:
                # any other write to a register: an OFF stays an OFF under add/sub of a register
                if not (mn in ('add', 'sub') and st.get(dst, (None,))[0] == 'OFF' and src in REGS):
                    st.pop(dst, None)
            elif dst and len(dst) <= 2:  # al, bx ...
                for r in REGS:
                    if r[1] == dst[0]:
                        st.pop(r, None)
            if not self.ebp:
                if mn == 'push':
                    depth += 4
                elif mn == 'pop':
                    depth -= 4
                elif mn in ('add', 'sub') and dst == 'esp':
                    try:
                        depth += (-1 if mn == 'add' else 1) * int(src, 0)
                    except (ValueError, TypeError):
                        pass
                elif mn.startswith('ret'):
                    depth = 0

    def addr_homes(self, min_reloads, verbose):
        out = []
        for s, h in sorted(self.homes.items()):
            kinds = [d for d in h['defs'] if d[1] in ('Pg', 'Po', 'Pm', 'OFF') or (verbose and d[1] == 'Pl')]
            if not kinds or len(h['loads']) < min_reloads:
                continue
            k = collections.Counter(d[1] for d in kinds).most_common(1)[0][0]
            u = h['uses']
            # a slot is an address home when most of its stores are address definitions, or (a slot shared by
            # live-range packing) when reloads are used as an address
            if len(kinds) * 2 < len(h['defs']) and not (u['o'] if k == 'OFF' else u['p'] + u['t']):
                continue
            # `lea r,[x + 3]` is arithmetic unless something reads through it or it is passed on
            if k == 'Pm' and not (u['p'] + u['t'] + u['a']) :
                continue
            out.append(dict(slot=s, kind=k, defs=kinds, loads=h['loads'], uses=dict(h['uses'])))
        return out


def norm(tx):
    return re.sub(r'\b(e[abcd]x|e[sd]i)\b', 'R', tx)


def analyse(rows, lo, min_reloads, verbose):
    I = Side([o for k, o, c in rows if o is not None])
    O = Side([c for k, o, c in rows if c is not None])
    i2o = {o[0]: c for k, o, c in rows if o is not None and c is not None}
    o2i = {c[0]: o for k, o, c in rows if o is not None and c is not None}
    hi = I.addr_homes(min_reloads, verbose)
    ho = O.addr_homes(min_reloads, verbose)
    ostore = {d[0]: h for h in ho for d in h['defs']}
    used = set()
    res = []
    for h in hi:
        match = None
        for d in h['defs']:
            c = i2o.get(d[0])
            if c is not None and c[0] in ostore and id(ostore[c[0]]) not in used:
                match = ostore[c[0]]
                break
        if match is None:
            want = {norm(d[2]) for d in h['defs']}
            for g in ho:
                if id(g) not in used and g['kind'] == h['kind'] and want & {norm(d[2]) for d in g['defs']}:
                    match = g
                    break
        if match is not None:
            used.add(id(match))
        res.append((h, match))
    rest = [g for g in ho if id(g) not in used]
    return I, O, res, rest


def unfold(rows):
    """Member accesses the image makes through a pointer to the member object (`lea r,[b + Kt]`, then
    `[r + d]`) where ours folds the offset into the owner (`[b + Kt + d]`).  -> [(Kt, [(d, n_img, n_ours)])]
    The image side of an expanded inline member function (its `this` is the member's address), of a
    pointer/reference local, or of a homed address CSE."""
    def disps(side):
        mem = collections.Counter()
        lea = collections.Counter()
        for k, o, c in rows:
            x = o if side == 0 else c
            if x is None:
                continue
            mn, _, ops = x[1].partition(' ')
            for m in MEM.finditer(ops):
                d = parse_mem(m.group(1))
                if d['reloc'] or d['base'] in (None, 'ebp', 'esp') or d['index'] is not None:
                    continue
                (lea if mn == 'lea' else mem)[d['disp']] += 1
                if mn == 'lea':
                    mem[d['disp']] += 0
            m = re.fullmatch(r'add (e..), (%s)' % NUM, x[1])
            if m and m.group(1) in REGS:
                lea[int(m.group(2), 0)] += 1
        return mem, lea
    mi, li = disps(0)
    mo, lo = disps(1)
    alli = mi + li
    allo = mo + lo
    extra_o = {k: allo[k] - alli[k] for k in allo if allo[k] > alli[k] and k >= 0x40}
    extra_i = {k: alli[k] - allo[k] for k in alli if alli[k] > allo[k]}
    out = []
    for kt in sorted(li):
        if kt < 0x40 or li[kt] < 1:
            continue
        hits = [(k - kt, extra_i[k - kt], n) for k, n in sorted(extra_o.items()) if 0 <= k - kt < 0x1000 and (k - kt) in extra_i]
        # the pointer itself: ours forms it more often (re-forms) or as often
        if len(hits) >= 2 or (hits and hits[0][0] >= 0x40):
            out.append((kt, li[kt], lo[kt], hits))
    return out


def nulladj(rows):
    """Null-tested base adjustments (`je; lea r,[b + K]; jmp; xor r,r`: a pointer or reference CAST to a
    base at a non-zero offset) per side -> (image count, our count, our first sites)"""
    def scan(side):
        L = [(o if side == 0 else c) for k, o, c in rows]
        L = [x for x in L if x is not None]
        hits = []
        for i in range(len(L) - 2):
            m = re.fullmatch(r'lea (e..), \[e.. \+ (%s)\]' % NUM, L[i][1])
            if m and L[i + 1][1].startswith('jmp') and L[i + 2][1] == 'xor %s, %s' % (m.group(1), m.group(1)):
                hits.append((L[i][0], m.group(2)))
        return hits
    return scan(0), scan(1)


def describe(h, org):
    u = ' '.join('%s%d' % (k, v) for k, v in sorted(h['uses'].items())) or '-'
    d = h['defs'][0]
    return '%-3s %+#6x def %x `%s` x%d reloads %d (%s) reads %s' % (
        h['kind'], h['slot'], org + d[0], d[2][-46:], len(h['defs']), len(h['loads']),
        ','.join('%x' % (org + x) for x in h['loads'][:4]), u)


def scan_unit(unit, addrs, pfx, a):
    src = fe.R + '/' + unit
    text = open(src, encoding='latin-1').read()
    c, err = fe.compile_unit(unit, src, pfx)
    if c is None:
        return [('ERR', unit, err)]
    out = []
    ctxc = {}
    for addr in addrs:
        cc_ = c
        m = re.search(r'// ORIGINAL: 0x%08X [^\n]*\n((?://(?! ORIGINAL)[^\n]*\n)*)' % addr, text)
        mc = m and re.search(r'// compile_context (\S+)', m.group(1))
        if mc and mc.group(1) != unit:
            if mc.group(1) not in ctxc:
                ctxc[mc.group(1)] = fe.compile_unit(mc.group(1), fe.R + '/' + mc.group(1), pfx)[0]
            if ctxc[mc.group(1)] is not None:
                cc_ = ctxc[mc.group(1)]
        r = fe.evaluate(cc_, text, addr)
        if 'error' in r or (r['exact'] and not a.all):
            continue
        I, O, res, rest = analyse(r['rows'], r['lo'], a.min_reloads, a.verbose)
        lines = []
        cls = collections.Counter()
        for h, g in res:
            if g is None:
                tag = 'IMG-OFF' if h['kind'] == 'OFF' else 'IMG-PTR'
                lines.append('  %-8s img  %s' % (tag, describe(h, r['lo'])))
            elif g['kind'] != h['kind']:
                tag = 'KIND'
                lines.append('  %-8s img  %s\n  %-8s ours %s' % (tag, describe(h, r['lo']), '', describe(g, 0)))
            else:
                tag = '='
                if a.verbose:
                    lines.append('  %-8s img  %s\n  %-8s ours %s' % (tag, describe(h, r['lo']), '', describe(g, 0)))
            cls[tag] += 1
        for g in rest:
            tag = 'OURS-OFF' if g['kind'] == 'OFF' else 'OURS-PTR'
            cls[tag] += 1
            lines.append('  %-8s ours %s' % (tag, describe(g, 0)))
        forms = {k: (I.forms[k], O.forms[k]) for k in set(I.forms) | set(O.forms) if I.forms[k] != O.forms[k]}
        ni_, no_ = nulladj(r['rows'])
        if len(ni_) != len(no_):
            cls['NULLADJ'] += 1
            lines.append('  NULLADJ  null-tested base casts: img %d (%s) ours %d (%s)' % (
                len(ni_), ' '.join('%x:+%s' % (r['lo'] + a_, k_) for a_, k_ in ni_[:6]), len(no_), ' '.join('%x:+%s' % x_ for x_ in no_[:6])))
        for kt, ni, no, hits in unfold(r['rows']):
            cls['UNFOLD'] += 1
            lines.append('  UNFOLD   img  `lea r,[b + %#x]` x%d (ours x%d); ours folds %s' % (
                kt, ni, no, ' '.join('%#x(+%#x) x%d' % (kt + d, d, n) for d, a_, n in hits)))
        out.append(dict(unit=unit, addr='%08X' % addr, sym=r['sym'], diff=r['diff'], cls=dict(cls), lines=lines, forms=forms))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='*')
    ap.add_argument('--addr', default='')
    ap.add_argument('--from', dest='src_json')
    ap.add_argument('--all', action='store_true')
    ap.add_argument('--min-reloads', type=int, default=1)
    ap.add_argument('--jobs', type=int, default=int(os.environ.get('FE_JOBS', '5')))
    ap.add_argument('--pfx-base', type=int, default=int(os.environ.get('FE_PFX_BASE', '21')))
    ap.add_argument('--json')
    ap.add_argument('-v', dest='verbose', action='store_true')
    a = ap.parse_args()
    cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
    units = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/')]
    if a.units:
        units = [u for u in units if u in a.units or os.path.basename(u) in a.units]
    only = {int(x, 16) for x in a.addr.split(',') if x}
    inexact = None
    if a.src_json:
        inexact = {int(x['addr'], 16) for x in json.load(open(a.src_json)) if not x.get('exact') and x['addr'] != '?'}
    work = []
    rx = re.compile(r'// ORIGINAL: 0x([0-9A-F]{8}) \S+ 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)')
    for u in units:
        try:
            t = open(fe.R + '/' + u, encoding='latin-1').read()
        except OSError:
            continue
        addrs = []
        for m in rx.finditer(t):
            ad = int(m.group(1), 16)
            ok = (ad in only) if only else (ad in inexact) if inexact is not None else (a.all or 'BYTE_EXACT' not in m.group(2))
            if ok:
                addrs.append(ad)
        if addrs:
            work.append((u, addrs))
    if only:
        a.all = True
    fe.compile_cmd(work[0][0] if work else 'src/x.cpp')

    def lane(k):
        r = []
        for i in range(k, len(work), a.jobs):
            r += scan_unit(work[i][0], work[i][1], a.pfx_base + k, a)
        return r
    res = []
    with ThreadPoolExecutor(a.jobs) as ex:
        for o in ex.map(lane, range(a.jobs)):
            res += o
    errs = [x for x in res if isinstance(x, tuple)]
    res = [x for x in res if isinstance(x, dict)]
    tot = collections.Counter()
    res.sort(key=lambda x: (-(x['cls'].get('IMG-PTR', 0) + x['cls'].get('OURS-PTR', 0) + x['cls'].get('UNFOLD', 0) + x['cls'].get('NULLADJ', 0)), x['diff']))
    for x in res:
        for k, v in x['cls'].items():
            tot[k] += v
        if not x['lines'] and not a.verbose:
            continue
        fm = ' '.join('%s:%d/%d' % (k, v[0], v[1]) for k, v in sorted(x['forms'].items()))
        print('FUNC %s %-22s %-36s rows=%d %s%s' % (x['addr'], x['unit'][4:], x['sym'][:36], x['diff'],
              ' '.join('%s:%d' % kv for kv in sorted(x['cls'].items())), ('  forms img/ours ' + fm) if fm else ''))
        for l in x['lines']:
            print(l)
    for e in errs:
        print('ERROR', e[1], (e[2] or '')[:200])
    print('TOTAL', ' '.join('%s:%d' % kv for kv in sorted(tot.items())), 'functions:', len(res))
    if a.json:
        json.dump(res, open(a.json, 'w'), indent=0)


if __name__ == '__main__':
    main()
