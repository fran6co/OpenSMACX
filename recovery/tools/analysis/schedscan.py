#!/usr/bin/env python3
"""schedscan.py <unit> <0xADDR> [--src ALT.cpp] [--pfx N] [--gap N] [--rows]

Scheduling / scratch-register diagnosis for a function whose difference from the image is
(mostly) which of eax/ecx/edx a value gets and in which order independent instructions come.
Read with MATCHING_GUIDE 3.2 "Scratch registers and scheduling" (rules S1..S6).

1. Windows. Differing runs of the fe.py alignment closer than --gap equal rows are merged.
   Each window is classified with eax/ecx/edx (and their 8/16-bit parts) abstracted to R:
     REGS     same instructions in the same order; only the scratch register names differ.
              `rot+k` when one rotation eax->ecx->edx (k steps) maps ours onto the image.
     SCHED    same instructions in another order (after abstraction): scheduling. The moved
              instructions are listed in image order.
     LOADS    same instructions with memory operands exchanged between rows: which value is
              loaded first / becomes the destination (rule S4: reference order of the operands).
     OTHER    different instructions: not a scheduling difference; read it with fe.py.
2. Cursor track (rule S1). VC6 hands out scratch registers round-robin eax -> ecx -> edx in
   code-generation order, continuing through the whole function. Every instruction that starts
   a new value in a scratch register (mov/movsx/movzx/lea r,..., xor r,r) is an allocation; a
   thiscall receiver (`mov ecx, X` right before its call) is fixed and is skipped. Allocations
   are paired along the abstracted alignment and each pair gets the offset
   (ours - image) mod 3. The track prints where the offset changes: the first change is where
   our source allocates one register more (offset 1) or one fewer (offset 2) than the
   original, somewhere in the code generated since the previous allocation in sync.
3. --try: single-site source sweep for the window's cause (MATCHING_GUIDE 3.2 S7..S9). Every
   candidate is one rewrite inside the function, compiled and ranked by fe rows:
     nvarg     one call argument (nvop: one operand inside it, `nv(count) + 14`) passed through an identity inline (`nv(x)`): the argument becomes a
               named value, allocated outside the round-robin cursor (S7). A hit means the original
               read it into a named local or passed it through an inline helper's parameter.
     labelraw / labelget   label_get(k) <-> StringTable.get(Labels.strings_ptr[k]) (S7).
     tableraw / tableget   table_text(x) <-> StringTable.get(x) (S7). These four also get one
               candidate with every site of the function flipped at once.
     unname    a `T v = simple;` local read inline at its uses (S7, S8).
     expand / contract     one-return inline helpers written out or contracted (S1).
     forbound  `p = p + K; if (p > 0) { i = p; do .. --i; while (i != 0) }`-style reassigned
               parameters are not rewritten automatically: see S8.
     nvany     (opt-in) any operand read in the body, conditions and right-hand sides included.
   --try-rules picks a subset (comma list); --top N rows of the ranking.
   --pairs K / --triples K (with --try, when no single site is exact): every pair / triple of the K
   single sites that moved the rows at all, best first (a forward-substituted site leaves the rows
   unchanged and is not worth combining). For functions whose cursor offset changes twice.
Screen only: confirm a fix with fe.py."""
import sys, os, re, argparse, difflib
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
import fe
NV_TEMPLATE = 'template <class T> inline T nv(T v) { return v; }  // schedscan --try probe\n'

SCR = ('eax', 'ecx', 'edx')
PARTS = {'eax': ('eax', 'ax', 'al', 'ah'), 'ecx': ('ecx', 'cx', 'cl', 'ch'), 'edx': ('edx', 'dx', 'dl', 'dh')}
RX = re.compile(r'\b(e?[acd]x|[acd][lh])\b')
ROT = {1: {'eax': 'ecx', 'ecx': 'edx', 'edx': 'eax'}, 2: {'eax': 'edx', 'ecx': 'eax', 'edx': 'ecx'}}


def owner(r):
    for k, v in PARTS.items():
        if r in v:
            return k
    return None


def abstract(ins):
    return RX.sub(lambda m: 'R' + ('8' if m.group(1)[-1] in 'lh' else ''), ins)


def rename(ins, perm):
    def sub(m):
        r = m.group(1)
        o = owner(r)
        return PARTS[perm[o]][PARTS[o].index(r)]
    return RX.sub(sub, ins)


def alloc_reg(seq, i):
    """scratch register newly defined by seq[i], or None"""
    ins = seq[i]
    m = re.match(r'(mov|movsx|movzx|lea) (e[acd]x|[acd][lh]), (.*)', ins)
    if m:
        dst = owner(m.group(2))
        if m.group(1) == 'mov' and m.group(2)[-1] in 'lh':
            return None  # byte load into a zeroed register: the xor allocated it
        if dst in [owner(r) for r in RX.findall(m.group(3))]:
            return None  # mov eax, [eax+4]: reuses the register
        if dst == 'ecx' and m.group(1) == 'mov':
            for j in range(i + 1, min(len(seq), i + 4)):
                if seq[j].startswith('call'):
                    return None  # thiscall receiver
                if 'ecx' in seq[j] or 'cl' in RX.findall(seq[j]):
                    break
        return dst
    m = re.match(r'xor (e[acd]x), (e[acd]x)$', ins)
    if m and m.group(1) == m.group(2):
        return m.group(1)
    return None


def merge(rs, gap):
    out = []
    for s, e in rs:
        if out and s - out[-1][1] <= gap:
            out[-1] = (out[-1][0], e)
        else:
            out.append((s, e))
    return out


def classify(img, ours):
    ai, ao = [abstract(x) for x in img], [abstract(x) for x in ours]
    if ai == ao:
        for k, rot in ROT.items():
            if [rename(x, rot) for x in ours] == img:
                return 'REGS', 'rot+%d' % k
        return 'REGS', ''
    if sorted(ai) == sorted(ao):
        return 'SCHED', ''
    mem = re.compile(r'(?:byte|word|dword) ptr \[[^\]]+\]')
    if [mem.sub('M', x) for x in ai] == [mem.sub('M', x) for x in ao] and sorted(
            m for x in img for m in mem.findall(x)) == sorted(m for x in ours for m in mem.findall(x)):
        return 'LOADS', ''
    return 'OTHER', ''


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('unit')
    ap.add_argument('addr')
    ap.add_argument('--src')
    ap.add_argument('--pfx', default=os.environ.get('FE_PFX_BASE', '27'))
    ap.add_argument('--gap', type=int, default=4, help='merge differing runs separated by at most this many equal rows')
    ap.add_argument('--rows', action='store_true', help='print the rows of every window')
    ap.add_argument('--try', dest='try_', action='store_true', help='single-site source sweep (nvarg, labelraw, labelget, unname, expand)')
    ap.add_argument('--try-rules', default='nvarg,labelraw,labelget,tableraw,tableget,unname,expand,contract')
    ap.add_argument('--top', type=int, default=12)
    ap.add_argument('--pairs', type=int, default=0, metavar='K', help='after --try: every pair of the K single sites that moved the rows most favourably')
    ap.add_argument('--triples', type=int, default=0, metavar='K', help='after --try: every triple of the best K moving single sites')
    a = ap.parse_args()
    unit = a.unit if a.unit.startswith('src/') else 'src/' + a.unit
    path = os.path.abspath(a.src) if a.src else fe.R + '/' + unit
    text = open(path, encoding='latin-1').read()
    c, err = fe.compile_unit(unit, path, a.pfx)
    if c is None:
        raise SystemExit('COMPILE FAIL\n' + err)
    addr = int(a.addr, 16)
    res = fe.evaluate(c, text, addr)
    if 'error' in res:
        raise SystemExit(res['error'])
    rows, lo = res['rows'], res['lo']
    print('FE %08X %s diff=%d img=%d ours=%d' % (addr, res['sym'][:50], res['diff'], res['img'], res['ours']))

    # 1. windows
    rs, i = [], 0
    differs = lambda r: r[0] == '!!'  # '!>' rows (same text, other jump target) are layout, not scheduling
    while i < len(rows):
        if differs(rows[i]):
            j = i
            while j < len(rows) and differs(rows[j]):
                j += 1
            rs.append((i, j))
            i = j
        else:
            i += 1
    kinds = {}
    for s, e in merge(rs, a.gap):
        img = [r[1][1] for r in rows[s:e] if r[1]]
        ours = [r[2][1] for r in rows[s:e] if r[2]]
        kind, det = classify(img, ours)
        kinds[kind] = kinds.get(kind, 0) + 1
        at = next((lo + r[1][0] for r in rows[s:e] if r[1]), None)
        print('%-5s %-6s %s  %d img / %d ours' % (kind, det, ('%x' % at) if at else '?', len(img), len(ours)))
        if kind == 'SCHED':
            ai, ao = [abstract(x) for x in img], [abstract(x) for x in ours]
            sm = difflib.SequenceMatcher(None, ao, ai, autojunk=False)
            for op, a1, a2, b1, b2 in sm.get_opcodes():
                if op in ('insert', 'replace'):
                    for k in range(b1, b2):
                        print('        image places `%s` %s' % (img[k], 'earlier' if ai[k] in ao[a2:] else 'later' if ai[k] in ao[:a1] else 'here'))
        if a.rows or kind in ('OTHER', 'LOADS'):
            for r in rows[s:e]:
                o, q = r[1], r[2]
                print('        %s %-6s %-40s | %s' % (r[0], ('%x' % (lo + o[0])) if o else '', o[1] if o else '', q[1] if q else ''))
    print('windows: %s' % ', '.join('%s %d' % kv for kv in sorted(kinds.items())))

    # 2. cursor track over the whole function, along an alignment of the abstracted listings
    I = [r for r in res['rows'] if r[1]]
    img_seq = [x[1][1] for x in sorted(I, key=lambda r: r[1][0])]
    img_off = [x[1][0] for x in sorted(I, key=lambda r: r[1][0])]
    O = [r for r in res['rows'] if r[2]]
    our_seq = [x[2][1] for x in sorted(O, key=lambda r: r[2][0])]
    sm = difflib.SequenceMatcher(None, [abstract(x) for x in img_seq], [abstract(x) for x in our_seq], autojunk=False)
    pairs = []
    for op, a1, a2, b1, b2 in sm.get_opcodes():
        if op == 'equal':
            for k in range(a2 - a1):
                ri, ro = alloc_reg(img_seq, a1 + k), alloc_reg(our_seq, b1 + k)
                if ri and ro:
                    pairs.append((img_off[a1 + k], (SCR.index(ro) - SCR.index(ri)) % 3, img_seq[a1 + k], our_seq[b1 + k]))
    print('\ncursor track (S1): %d paired allocations' % len(pairs))
    last, last_at = None, None
    changes = 0
    for at, d, si, so in pairs:
        if d != last:
            changes += 1
            if changes <= 40:
                print('  %x offset %d%s   image `%s`  ours `%s`' % (lo + at, d, '' if last is None else
                      ' (was %d since %x)' % (last, lo + last_at), si, so))
            last = d
        last_at = at
    nz = [p for p in pairs if p[1]]
    if not nz:
        print('  every paired allocation is in sync: the differences are order (S3/S4) or fixed registers.')
    else:
        print('  %d of %d paired allocations are out of sync; offset 1 = ours one allocation ahead, 2 = one behind.'
              % (len(nz), len(pairs)))
    if a.try_:
        try_sweep(unit, addr, text, res['diff'], a.try_rules.split(','), a.top, a.pairs, a.triples)


KEYWORDS = {'if', 'while', 'for', 'switch', 'return', 'sizeof', 'static_cast', 'reinterpret_cast', 'const_cast',
            'catch', 'delete', 'new', 'defined', 'nv', 'operator'}


def rule_nvarg(t, mt, pos, end):
    """wrap one call argument in nv(...)"""
    import rules
    for m in re.finditer(r'(?<![\w])([A-Za-z_]\w*)\s*\(', mt[pos:end]):
        name = m.group(1)
        if name in KEYWORDS:
            continue
        o = pos + m.end() - 1
        # skip declarations and definitions: `T name(` at statement start followed by a type list is rare
        # inside a body; casts like (int)(x) have no identifier before the paren.
        c = rules.match(mt, o, '(', ')')
        if c < 0 or c > end:
            continue
        args, d, last = [], 0, o + 1
        for k in range(o + 1, c):
            ch = mt[k]
            if ch in '([{':
                d += 1
            elif ch in ')]}':
                d -= 1
            elif ch == ',' and d == 0:
                args.append((last, k))
                last = k + 1
        args.append((last, c))
        for a0, a1 in args:
            raw = t[a0:a1]
            a = raw.strip()
            if not a or re.fullmatch(r"-?(0x[0-9A-Fa-f]+|\d+)[uUlL]*|'.*'|\".*\"|this|NULL|true|false", a, re.S):
                continue
            if re.fullmatch(r'[A-Z_][A-Z0-9_]*', a):
                continue  # enum constant / macro
            lead = len(raw) - len(raw.lstrip())
            s0 = a0 + lead
            yield (s0, s0 + len(a), 'nv(' + a + ')', 'nvarg')
            # operands inside a compound argument (`count + 14`: nv(count) + 14)
            for om in OPERAND.finditer(mt, s0, s0 + len(a)):
                o0, o1 = om.span()
                if (o0, o1) == (s0, s0 + len(a)) or om.group(0) in KEYWORDS or mt[o1:o1 + 1] == '(' \
                        or re.fullmatch(r'[A-Z_][A-Z0-9_]*|this', om.group(0)) or mt[o0 - 1:o0] in '.>&*':
                    continue
                if mt[o1:].lstrip().startswith('('):
                    continue
                yield (o0, o1, 'nv(' + t[o0:o1] + ')', 'nvop')


def rule_nvany(t, mt, pos, end):
    """wrap any operand read in the body (conditions, assignments' right-hand sides, subscripts)"""
    body = mt.find('{', pos)
    for om in OPERAND.finditer(mt, body, end):
        o0, o1 = om.span()
        w = om.group(0)
        if w in KEYWORDS or re.fullmatch(r'[A-Z_][A-Z0-9_]*|this|int|char|unsigned|signed|short|long|const|void|static|else|do|break|continue|goto|case|default|struct|class', w):
            continue
        if mt[o0 - 1:o0] in '.>&*' or mt[o1:].lstrip()[:1] in ('(', ':') or re.match(r'\s*(=[^=]|\+=|-=|\*=|/=|\|=|&=|\^=|<<=|>>=|\+\+|--)', mt[o1:o1 + 4]):
            continue
        if re.search(r'(\+\+|--)\s*$', mt[o0 - 3:o0]) or re.match(r'[ \t]*(?:\w+[ \t]*\**[ \t]*)$', mt[mt.rfind('\n', 0, o0) + 1:o0]) and re.match(r'\s*(=|;|,|\[)', mt[o1:o1 + 3]):
            continue  # a declaration's name
        yield (o0, o1, 'nv(' + t[o0:o1] + ')', 'nvany')


def rule_tableraw(t, mt, pos, end):
    import rules
    for m in re.finditer(r'\btable_text\(', mt[pos:end]):
        o = pos + m.end() - 1
        c = rules.match(mt, o, '(', ')')
        if 0 < c <= end:
            yield (pos + m.start(), c + 1, 'StringTable.get(' + t[o + 1:c] + ')', 'tableraw')


def rule_tableget(t, mt, pos, end):
    import rules
    for m in re.finditer(r'\bStringTable\.get\(', mt[pos:end]):
        o = pos + m.end() - 1
        c = rules.match(mt, o, '(', ')')
        if 0 < c <= end:
            yield (pos + m.start(), c + 1, 'table_text(' + t[o + 1:c] + ')', 'tableget')


ALL_RULES = ('labelraw', 'labelget', 'tableraw', 'tableget')


OPERAND = re.compile(r'(?<![\w.>])[A-Za-z_]\w*(?:(?:\.|->)[A-Za-z_]\w*|\[[^\[\]]*\])*')


def try_sweep(unit, addr, text, base_diff, rule_names, top, pairs=0, triples=0):
    import rules
    import perm
    mt = rules.mask(text)
    marks = [(int(m.group(1), 16), m.start()) for m in rules.MARK.finditer(text)]
    starts = [p for a, p in marks]
    me = [p for a, p in marks if a == addr]
    if not me:
        print('no marker for %08X' % addr)
        return
    lo = me[0]
    hi = min([p for p in starts if p > lo] + [len(text)])
    # the identity inline goes after the last top-level #include (a marker can sit inside a comment block)
    inc = [m.end() for m in re.finditer(r'(?m)^#include[^\n]*\n', text[:lo])]
    inject = inc[-1] if inc else 0
    deck = []
    site_of = {}
    for rn in rule_names:
        sites = []
        if rn == 'nvarg':
            cands = rule_nvarg(text, mt, lo, hi)
        elif rn == 'nvany':
            cands = rule_nvany(text, mt, lo, hi)
        elif rn == 'tableraw':
            cands = rule_tableraw(text, mt, lo, hi)
        elif rn == 'tableget':
            cands = rule_tableget(text, mt, lo, hi)
        else:
            cands = rules.RULES[rn](text, mt, lo)
        for cand in cands:
            s0, e0, rep, label = cand[:4]
            if s0 < lo or e0 > hi:
                if s0 >= hi:
                    break
                continue
            v = text[:s0] + rep + text[e0:]
            if rn in ('nvarg', 'nvany') and 'inline T nv(T v)' not in text:
                v = v[:inject] + NV_TEMPLATE + v[inject:]
            line = text.count('\n', 0, s0) + 1
            sites.append((s0, e0, rep))
            deck.append(('%s L%d `%s` -> `%s`' % (label, line, ' '.join(text[s0:e0].split())[:50], ' '.join(rep.split())[:60]), v))
            site_of[deck[-1][0]] = (s0, e0, rep, rn in ('nvarg', 'nvany'))
        if rn in ALL_RULES and len(sites) > 1:
            # every site of the rule at once (read_rules: three labels had to flip together)
            v, last = [], 0
            for s0, e0, rep in sorted(sites):
                if s0 < last:
                    continue
                v.append(text[last:s0])
                v.append(rep)
                last = e0
            v.append(text[last:])
            deck.append(('%s ALL %d sites' % (rn, len(sites)), ''.join(v)))
    print('\n--try: %d single-site candidates (%s)' % (len(deck), ','.join(rule_names)))
    if not deck:
        return
    res = perm.run_deck(unit, '0x%08X' % addr, deck, jobs=int(os.environ.get('FE_JOBS', '5')))
    better = [r for r in res if r[1] < base_diff]
    print('  %d of %d improve on %d rows' % (len(better), len(res), base_diff))
    for w, d, ex, key in res[:top]:
        print('  %4d%s %s' % (d, ' EXACT' if ex else '', key))
    if not (pairs or triples) or any(r[2] for r in res):
        return
    # Pairs and triples (two named/anonymous disagreements: the cursor offset changes twice). A site that
    # VC6 forward-substitutes leaves the rows where they were, so the sites worth combining are the ones
    # that moved the rows at all, best first.
    import itertools
    moving, seen = [], set()
    for w, d, ex, key in res:
        if d == base_diff or d >= 9999 or key not in site_of or site_of[key][:2] in seen:
            continue
        seen.add(site_of[key][:2])  # nvarg and nvany can name one span
        moving.append(key)

    def combo(keys):
        ss = sorted(site_of[k] for k in keys)
        if any(a[1] > b[0] for a, b in zip(ss, ss[1:])):
            return None
        v, last = [], 0
        for s0, e0, rep, _ in ss:
            v += [text[last:s0], rep]
            last = e0
        v = ''.join(v + [text[last:]])
        if any(s[3] for s in ss) and 'inline T nv(T v)' not in text:
            v = v[:inject] + NV_TEMPLATE + v[inject:]
        return v
    deck2 = []
    for n, k in ((2, pairs), (3, triples)):
        for keys in itertools.combinations(moving[:k], n):
            v = combo(keys)
            if v is not None:
                deck2.append((' + '.join(keys), v))
    print('\n--pairs/--triples: %d moving single sites, %d combinations' % (len(moving), len(deck2)))
    if not deck2:
        return
    res2 = perm.run_deck(unit, '0x%08X' % addr, deck2, jobs=int(os.environ.get('FE_JOBS', '5')))
    best1 = min(r[1] for r in res)
    print('  %d of %d improve on the best single site (%d rows; base %d)' % (len([r for r in res2 if r[1] < min(best1, base_diff)]), len(res2), best1, base_diff))
    for w, d, ex, key in res2[:top]:
        print('  %4d%s %s' % (d, ' EXACT' if ex else '', key))


if __name__ == '__main__':
    main()
