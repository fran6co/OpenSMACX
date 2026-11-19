#!/usr/bin/env python3
"""imgmerge.py <unit> <0xADDR> [--plan] [--out FILE | --write] [--min K] [--only SLOT,..] [--skip NAME,..] [--sites SLOT,..]
Image-slot variable merge (MATCHING_GUIDE 3.3 "One reused function-scope variable per image home").

A decompiler-style body declares a block-local per role; the original often had a handful of function-scope
scratch variables (i, x, y, val, flag ...) reused by every block.  VC6 gives one home per NAME, so the image
shows such a variable as one frame slot that many of our block-locals pair with (varset.py MERGE), ranked by
the sum of their references.  This tool acts on that evidence:

  1. pairs every frame access of ours with the image slot of the aligned row (fe alignment, same instruction
     shape) and names it from the /FAsc operand; a block-local instance `name$NNN` is mapped back to its
     declaration through the source lines of its accesses;
  2. per image slot S (locals only, not address-taken): the OWNER is our function-scope int/BOOL whose
     majority slot is S; with no such variable the first block-local paired with S is hoisted to function
     scope (its declaration becomes an assignment);
  3. every other block-local whose majority image slot is S (>= K paired accesses, >= 60% of its pairs) is
     renamed into the owner when (a) the owner is not named in the block-local's scope and no goto enters the
     scope, and (b) the kill probe `owner = K;` at the declaration compiles away (reusesweep.py's proof that
     the owner is dead there).
Renames are applied one at a time on the current text (each proved on the text it is applied to).  The result
is fingerprinted (frame, slot-map difference, rows) against the base.  --plan prints the slot table only;
--sites prints the image-address timeline of the given slots (every image access, our aligned instruction,
variable and source line); --out writes the merged file elsewhere, --write in place.
Function-scope/function-scope pairs in one image slot are listed, not merged (their ranges need a manual read).
Screen and rewrite tool: review the diff; an alignment error can pair a variable with a neighbour's slot.
Env: FE_PFX_BASE (first lane prefix), FE_JOBS."""
import sys, os, re, argparse, collections, threading, queue
H = os.path.dirname(os.path.abspath(__file__))
T_ = os.path.dirname(H)
R = os.path.dirname(os.path.dirname(T_))
sys.path.insert(0, T_)
sys.path.insert(0, H)
os.chdir(R)
import fe
import regprio
import varset
import reusesweep as rs

PB = int(os.environ.get('FE_PFX_BASE', '21'))

_locate = rs.armwo.locate


def locate(t, addr):
    """armwo.locate takes the first body after the marker; a marker followed by inline helpers (enemy_strategy's
    base_owner_of) needs the body of the function the marker's symbol names"""
    bs, be, x = _locate(t, addr)
    m = re.search(r'// ORIGINAL: 0x%s \?{1,2}(\w+)@(?:(\w+)@@)?' % addr[2:].upper().zfill(8), t)
    if not m:
        return bs, be, x
    name = m.group(1)
    head = t[t.rfind('\n', 0, t.rfind(')', 0, bs)) - 400:bs]
    if re.search(r'\b%s\s*\(' % re.escape(name), head):
        return bs, be, x
    mk = rs.mask(t)
    for mm in re.finditer(r'\b%s\s*\(' % re.escape(name), mk[m.end():]):
        p = m.end() + mm.end() - 1
        d = 0
        while p < len(mk):
            if mk[p] == '(':
                d += 1
            elif mk[p] == ')':
                d -= 1
                if d == 0:
                    break
            p += 1
        q = p + 1
        while q < len(mk) and mk[q] in ' \n\tconst':
            q += 1
        if q < len(mk) and mk[q] == '{':
            d, e = 0, q
            while e < len(mk):
                if mk[e] == '{':
                    d += 1
                elif mk[e] == '}':
                    d -= 1
                    if d == 0:
                        break
                e += 1
            return q, e + 1, x
    return bs, be, x


rs.armwo.locate = locate
SCR = R + '/build/recovery/fe/imgmerge'


LOCK = threading.Lock()
JOBS = int(os.environ.get('FE_JOBS', '5'))


def compile_text(unit, text, addr, tag='m', pfx=None):
    pfx = PB if pfx is None else pfx
    os.makedirs(SCR, exist_ok=True)
    path = '%s/%s%d.cpp' % (SCR, tag, pfx)
    open(path, 'w', encoding='latin-1').write(text)
    c, err = fe.compile_unit(unit, path, pfx)
    if c is None:
        return None, None, err
    with LOCK:  # capstone's decoder is shared
        r = fe.evaluate(c, text, addr)
    return r, path, None


def pairing(unit, text, addr):
    """-> r, acc [(our var listing name, our slot, image slot or None, source line, image addr or None, img text, our text)],
    img {slot: Counter}"""
    r, path, err = compile_text(unit, text, addr)
    if r is None:
        sys.exit('compile failed: %s' % (err or '')[:400])
    items, _ = regprio.cod_listing(unit, path, PB, r['sym'])
    name_at, line_of, cur = {}, {}, 0
    for it in items:
        if it[0] == 'line':
            cur = it[1]
        elif it[0] == 'ins':
            line_of[it[1]] = cur
            m = varset.SYM.search(it[2])
            if m:
                name_at[it[1]] = varset.varname(m.group(1))
    img = collections.defaultdict(collections.Counter)
    acc = []
    timeline = collections.defaultdict(list)
    last = 0
    shape = lambda s: varset.SLOT.sub('[S]', s)
    for k, o, c_ in r['rows']:
        if c_ is not None and c_[0] in line_of:
            last = line_of[c_[0]]
        si = varset.slot(o[1]) if o is not None else None
        if si is not None:
            img[si][varset.klass(o[1])] += 1
            img[si]['n'] += 1
        so = varset.slot(c_[1]) if c_ is not None else None
        nm = name_at.get(c_[0], 'T%s' % hex(so)) if so is not None else None
        if so is not None:
            ok = si is not None and shape(o[1]) == shape(c_[1])
            acc.append((nm, so, si if ok else None, line_of.get(c_[0], 0)))
        if si is not None:
            if c_ is not None:
                timeline[si].append('%x %-40s | L%-5d %-40s %s' % (r['lo'] + o[0], o[1][:40], line_of.get(c_[0], 0),
                                                                    c_[1][:40], nm or ''))
            else:
                timeline[si].append('%x %-40s | ~L%d' % (r['lo'] + o[0], o[1][:40], last))
    return r, acc, img, timeline


def instances(text, addr, acc):
    """-> (analysis, {listing name: ('f', name) | ('b', loc) | None}, pair {listing name: Counter(image slot)},
    n {listing name: accesses})"""
    bs, be, m, fvars, locs = rs.analyse(text, addr)
    lines = collections.defaultdict(set)
    pair = collections.defaultdict(collections.Counter)
    n = collections.Counter()
    for nm, so, si, ln in acc:
        n[nm] += 1
        if ln:
            lines[nm].add(ln)
        if si is not None:
            pair[nm][si] += 1
    for L in locs:
        L['l0'] = text.count('\n', 0, L['s']) + 1
        L['l1'] = text.count('\n', 0, L['e']) + 1
    kind = {}
    for nm in n:
        if nm.startswith('T') or nm.startswith('$') or nm.startswith('tv'):
            kind[nm] = None
            continue
        base = nm.split('#')[0]
        if '#' not in nm:
            kind[nm] = ('f', base) if base in fvars else None
            continue
        cand = [L for L in locs if L['name'] == base and all(L['l0'] <= x <= L['l1'] for x in lines[nm])]
        if not cand or not lines[nm]:
            kind[nm] = None
            continue
        kind[nm] = ('b', min(cand, key=lambda L: L['e'] - L['s']))
    return (bs, be, m, fvars, locs), kind, pair, n


def majority(pc, mn):
    if not pc:
        return None
    s, k = pc.most_common(1)[0]
    tot = sum(pc.values())
    return s if (k >= mn and k * 5 >= tot * 3) else None


def plan(text, addr, acc, img, mn):
    an, kind, pair, n = instances(text, addr, acc)
    by = collections.defaultdict(list)
    for nm, kd in kind.items():
        if kd is None:
            continue
        s = majority(pair[nm], mn)
        if s is None or s >= 0 or img[s]['lea']:
            continue
        by[s].append((nm, kd, pair[nm][s]))
    return an, by, pair, n


def fingerprint(r):
    fd, sd, fo = rs.frame_slots(r)
    return fd, sd, r['diff'], fo


def hoist_name(m, bs, be, L, slot):
    """the block-local's own name when nothing else in the function uses it, else name_<slot>"""
    nm = L['name']
    outside = m[bs:L['s']] + m[L['e']:be]
    if not rs.ident_re(nm).search(outside):
        return nm
    return '%s_%x' % (nm, -slot)


def insert_decl(text, m, bs, name, typ='int'):
    p = m.find('{', bs)
    return text[:p + 1] + '\n    %s %s;' % (typ, name) + text[p + 1:]


def decl_type(text, L):
    return text[L['at']:L['at'] + L['kwlen']].strip()


def stmt_start(m, pos, bs):
    """offset where the statement containing pos starts: after the last ; { } before pos that is outside every
    parenthesis (a for-clause semicolon is inside one)"""
    d, last = 0, bs + 1
    for i in range(bs, pos):
        c = m[i]
        if c == '(':
            d += 1
        elif c == ')':
            d -= 1
        elif c in ';{}' and d == 0:
            last = i + 1
    return last


def pure_defs(m, bs, be, name):
    """offsets of `name = ...` (plain assignment; a compound assignment reads first)"""
    return [bs + mm.start() for mm in re.finditer(
        r'(?<![\w.$>:])%s(?!\w)\s*=(?!=)' % re.escape(name), m[bs:be])]


def merge_fvar(unit, addr, cur, B, T, pfx):
    """function-scope B renamed into function-scope T when their live ranges are disjoint: T is dead at every
    plain definition of B and B at every plain definition of T (kill probes). -> (text or None, reason)"""
    bs, be, m, fvars, locs = rs.analyse(cur, addr)
    if fvars.get(B, -1) < 0 or T not in fvars:
        return None, 'not a function-scope local'
    body = m[bs:be]
    for v in (B, T):
        if re.search(r'(?<![&\w])&\s*%s(?!\w)' % re.escape(v), body):
            return None, '%s has its address taken' % v
        if any(L['name'] == v for L in locs):
            return None, '%s is also a block-local name' % v
    decl = next((x for x in rs.FDECL if B in x[1]), None)
    if decl is None or len(decl[1]) != 1:
        return None, 'declaration of %s is not a single declarator' % B
    probes = collections.defaultdict(list)
    for dpos in pure_defs(m, bs, be, B):
        probes[stmt_start(m, dpos, bs)].append(T)
    for dpos in pure_defs(m, bs, be, T):
        probes[stmt_start(m, dpos, bs)].append(B)
    if not probes:
        return None, 'no definitions'
    kt = cur
    for pos in sorted(probes, reverse=True):
        kt = kt[:pos] + ''.join(' %s = %s;' % (v, rs.KILL) for v in sorted(set(probes[pos]))) + kt[pos:]
    rb, _, _ = compile_text(unit, cur, addr, 'b', pfx)
    rk, _, err = compile_text(unit, kt, addr, 'k', pfx)
    if rk is None:
        return None, 'probe variant does not compile'
    if rs.ours_sig(rk) != rs.ours_sig(rb):
        # name the definitions where the other variable is live: one probe position at a time
        bsig, live = rs.ours_sig(rb), []
        for pos in sorted(probes):
            for v in sorted(set(probes[pos])):
                k1 = cur[:pos] + ' %s = %s;' % (v, rs.KILL) + cur[pos:]
                r1, _, _ = compile_text(unit, k1, addr, 'k', pfx)
                if r1 is None or rs.ours_sig(r1) != bsig:
                    live.append('%s live at L%d' % (v, cur.count('\n', 0, pos) + 1 + (1 if cur[pos:pos + 1] == '\n' else 0)))
            if len(live) >= 8:
                break
        return None, 'live ranges overlap: ' + ', '.join(live)
    # rename: every B in the body becomes T; B's declaration loses its keyword or disappears
    at = decl[0]
    end = m.find(';', at)
    dtext = cur[at:end]
    init = '=' in m[at:end]
    seg_a, seg_b = cur[bs:at], cur[end + 1:be]
    msa, msb = m[bs:at], m[end + 1:be]
    def ren(seg, mseg):
        out, last = [], 0
        for mm in rs.ident_re(B).finditer(mseg):
            out.append(seg[last:mm.start()])
            out.append(T)
            last = mm.end()
        out.append(seg[last:])
        return ''.join(out)
    if init:
        mid = ren(dtext, m[at:end])
        mid = re.sub(r'^(?:int|BOOL)\s+', '', mid) + ';'
    else:
        mid = ''
        ls = cur.rfind('\n', 0, at) + 1
        le = cur.find('\n', end)
        if not cur[ls:at].strip() and not m[end + 1:le].strip():
            # the declaration is the whole line: drop the line (keep a trailing comment out too)
            new = cur[:bs] + ren(cur[bs:ls], m[bs:ls]) + ren(cur[le + 1:be], m[le + 1:be]) + cur[be:]
            r2, _, err = compile_text(unit, new, addr, 'h', pfx)
            return (new, 'ok') if r2 is not None else (None, 'merge does not compile: %s' % (err or '')[:120])
    new = cur[:bs] + ren(seg_a, msa) + mid + ren(seg_b, msb) + cur[be:]
    r2, _, err = compile_text(unit, new, addr, 'h', pfx)
    return (new, 'ok') if r2 is not None else (None, 'merge does not compile: %s' % (err or '')[:120])


def find_loc(locs, name, ordinal):
    same = sorted((y for y in locs if y['name'] == name), key=lambda y: y['at'])
    return same[ordinal] if ordinal < len(same) else None


def apply_slot(unit, addr, cur, s, owner, members, pfx, gone=None):
    """the merges of one image slot, one rename at a time, each proved on the text it is applied to.
    A rename removes one same-named declaration, so the ordinals of later members of that name shift."""
    done, refused = [], []
    gone = collections.defaultdict(set) if gone is None else gone   # name -> base ordinals already renamed away
    sig = None
    for bname, ordn, bline in members:
        if ordn is None:   # a function-scope variable
            if owner is None:
                refused.append((s, bname, 0, 'no owner'))
                continue
            new, why = merge_fvar(unit, addr, cur, bname, owner, pfx)
            if new is None:
                refused.append((s, bname, 0, why))
            else:
                cur, sig = new, None
                done.append((s, '%s (function scope)' % bname, '-> %s' % owner))
            continue
        bs, be, m, fvars, locs = rs.analyse(cur, addr)
        L = find_loc(locs, bname, ordn - sum(1 for g in gone[bname] if g < ordn))
        if L is None:
            refused.append((s, bname, bline, 'declaration not found'))
            continue
        if owner is None:
            T = hoist_name(m, bs, be, L, s)
            typ = decl_type(cur, L)
            new = rs.rename_variant(cur, m, L, T)
            bs2, be2, m2, _, _ = rs.analyse(new, addr)
            new = insert_decl(new, m2, bs2, T, typ)
            r2, _, err = compile_text(unit, new, addr, 'h', pfx)
            if r2 is None:
                refused.append((s, bname, bline, 'hoist does not compile: %s' % (err or '')[:120]))
                continue
            cur, owner, sig = new, T, rs.ours_sig(r2)
            gone[bname].add(ordn)
            done.append((s, '%s@%d' % (bname, bline), 'hoisted as %s' % T))
            continue
        T = owner
        if rs.gotos_into(m, bs, be, L['s'], L['e']):
            refused.append((s, bname, bline, 'goto into scope'))
            continue
        # a block-local that shadows the owner (same name) only loses its declaration
        if T != bname and rs.ident_re(T).search(m[L['s']:L['e']]):
            refused.append((s, bname, bline, '%s is named in the scope' % T))
            continue
        if sig is None:
            rb, _, _ = compile_text(unit, cur, addr, 'b', pfx)
            sig = rs.ours_sig(rb)
        rk, _, err = compile_text(unit, rs.kill_variant(cur, [(L, T)]), addr, 'k', pfx)
        if rk is None or rs.ours_sig(rk) != sig:
            refused.append((s, bname, bline, '%s is live at the declaration (kill probe kept)' % T))
            continue
        new = rs.rename_variant(cur, m, L, T)
        r2, _, err = compile_text(unit, new, addr, 'h', pfx)
        if r2 is None:
            refused.append((s, bname, bline, 'rename does not compile: %s' % (err or '')[:120]))
            continue
        cur, sig = new, rs.ours_sig(r2)
        gone[bname].add(ordn)
        done.append((s, '%s@%d' % (bname, bline), '-> %s' % T))
    return cur, done, refused


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('unit')
    ap.add_argument('addr')
    ap.add_argument('--plan', action='store_true')
    ap.add_argument('--out')
    ap.add_argument('--write', action='store_true')
    ap.add_argument('--min', type=int, default=2)
    ap.add_argument('--only', default='')
    ap.add_argument('--skip', default='')
    ap.add_argument('--sites', default='')
    ap.add_argument('--no-hoist', action='store_true')
    ap.add_argument('--all', action='store_true')
    ap.add_argument('--fscope', action='store_true', help='also merge function-scope variables that share an image slot')
    a = ap.parse_args()
    unit = a.unit if a.unit.startswith('src/') else 'src/' + a.unit
    addr = int(a.addr, 16)
    src = R + '/' + unit
    text = open(src, encoding='latin-1').read()
    only = set(int(x, 16) for x in a.only.split(',') if x)
    skip = set(x for x in a.skip.split(',') if x)
    r, acc, img, timeline = pairing(unit, text, addr)
    base_fp = fingerprint(r)
    if a.sites:
        want = None if a.sites == 'all' else set(int(x, 16) for x in a.sites.split(','))
        for s in sorted(timeline, reverse=True):
            if want is None or s in want:
                print('== image slot %s (%d)' % (hex(s), len(timeline[s])))
                for l in timeline[s]:
                    print('  ' + l.replace('dword ptr ', ''))
        return
    an, by, pair, n = plan(text, addr, acc, img, a.min)
    print('BASE frame diff %d, slot-map diff %d, rows %d (our frame %#x)' % base_fp)
    print('\nIMAGE SLOT <- our variables (f = function scope, b = block-local @line; paired accesses)')
    for s in sorted(by, reverse=True):
        mem = by[s]
        if len(mem) < 2 and not a.plan:
            continue
        print('  %7s n=%-3d <- %s' % (hex(s), img[s]['n'], ', '.join(
            '%s:%s%s(%d)' % (kd[0], nm, '' if kd[0] == 'f' else '@%d' % kd[1]['line'], k)
            for nm, kd, k in sorted(mem, key=lambda x: (x[1][0] != 'f', x[1][1]['line'] if x[1][0] == 'b' else 0)))))
    if a.plan:
        return
    todo = []
    for s in sorted(by, reverse=True):
        if only and s not in only:
            continue
        mem = by[s]
        fs = [x for x in mem if x[1][0] == 'f']
        bl = sorted((x for x in mem if x[1][0] == 'b' and x[1][1]['name'] not in skip), key=lambda x: x[1][1]['line'])
        if len(fs) > 1 and not a.fscope:
            print('  NOTE %s hosts several function-scope variables: %s (merged only with --fscope)' % (
                hex(s), ', '.join(x[1][1] for x in fs)))
        owner = max(fs, key=lambda x: x[2])[1][1] if fs else None
        if (not bl and not (a.fscope and len(fs) > 1)) or (owner is None and (len(bl) < 2 or a.no_hoist)):
            continue
        # declarations are found again by (name, ordinal among the same-named block-locals): lines move
        locs = an[4]
        members = []
        for x in bl:
            L = x[1][1]
            same = sorted(y['at'] for y in locs if y['name'] == L['name'])
            members.append((L['name'], same.index(L['at']), L['line']))
        if a.fscope:
            members += [(x[1][1], None, 0) for x in fs if x[1][1] != owner and x[1][1] not in skip]
        todo.append((s, owner, members))
    done, refused = [], []
    cur = text
    gone = collections.defaultdict(set)
    if a.all or only:
        for s, owner, members in todo:
            cur, d, rf = apply_slot(unit, addr, cur, s, owner, members, PB, gone)
            done += d
            refused += rf
    else:
        # each slot alone, in lanes; then the improving slots cumulatively, best first
        res = {}
        qu = queue.Queue()
        for item in todo:
            qu.put(item)
        def lane(k):
            while True:
                try:
                    s, owner, members = qu.get_nowait()
                except queue.Empty:
                    return
                tx, d, rf = apply_slot(unit, addr, text, s, owner, members, PB + k)
                fp = None
                if d:
                    r2, _, _ = compile_text(unit, tx, addr, 'f', PB + k)
                    with LOCK:
                        fp = fingerprint(r2) if r2 is not None else None
                res[s] = (d, rf, fp)
        th = [threading.Thread(target=lane, args=(k,)) for k in range(JOBS)]
        [x.start() for x in th]
        [x.join() for x in th]
        print('\nEACH SLOT ALONE (frame diff, slot-map diff, rows)   base %s' % (base_fp[:3],))
        good = []
        for s, owner, members in todo:
            d, rf, fp = res[s]
            print('  %7s applied %d refused %d -> %s' % (hex(s), len(d), len(rf), fp[:3] if fp else '-'))
            for _, b, ln, why in rf:
                print('            refused %s@%d: %s' % (b, ln, why))
            if fp and fp[:2] < base_fp[:2]:
                good.append((fp[:3], s, owner, members))
        best = base_fp
        for _, s, owner, members in sorted(good):
            g2 = collections.defaultdict(set, {k: set(v) for k, v in gone.items()})
            tx, d, rf = apply_slot(unit, addr, cur, s, owner, members, PB, g2)
            if not d:
                continue
            r2, _, _ = compile_text(unit, tx, addr, 'f')
            fp = fingerprint(r2) if r2 is not None else None
            if fp and fp[:2] <= best[:2]:
                cur, best, gone = tx, fp, g2
                done += d
                refused += rf
                print('  keep %s -> %s' % (hex(s), fp[:3]))
            else:
                print('  drop %s (cumulative %s)' % (hex(s), fp[:3] if fp else '-'))
    print('\nAPPLIED')
    for s, b, what in done:
        print('  %7s %-28s %s' % (hex(s), b, what))
    print('REFUSED')
    for s, b, ln, why in refused:
        print('  %7s %s@%d: %s' % (hex(s), b, ln, why))
    if not done:
        return
    r2, _, _ = compile_text(unit, cur, addr, 'f')
    fp = fingerprint(r2)
    print('\nRESULT frame diff %d, slot-map diff %d, rows %d (our frame %#x)   base %s' % (fp + (base_fp[:3],)))
    if a.out:
        open(a.out, 'w', encoding='latin-1').write(cur)
        print('written to', a.out)
    elif a.write:
        open(src, 'w', encoding='latin-1').write(cur)
        print('written to', src)


if __name__ == '__main__':
    main()
