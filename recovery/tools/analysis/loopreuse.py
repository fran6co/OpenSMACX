#!/usr/bin/env python3
"""loopreuse.py <unit> <0xADDR> [--write] [--iter N] [--show KEY] | --from FEALL.json [--min 1] [--skip A,B] [--write] [--json F]

Dedicated loop-counter screen (MATCHING_GUIDE 3.7 Frame and slots, load_vehicles 0x00454270): the
reusesweep.py machinery (compiler-proved deadness, frame/slot/candset/rows ranking) applied to the loops of a
function-scope local instead of a block-local's scope.  For every depth-1 int/BOOL local L, every `for`
statement whose init clause assigns L (`for (L = ..`, `for (x = 2, L = 0; ..`) is a segment; L is renamed
to each other depth-1 local T of the same type (or int parameter) that
  (1) is not named in the segment,
  (2) is dead at the segment's start (kill probe `T = 0x2468ACF;` before the `for` compiles away; with (1)
      it is dead over the whole segment), and
      (--textual skips the probe: VC6 keeps T live through a zero-trip loop path, as load_vehicles'
      variant, read after the chassis loop; then T's next reference after the segment must be a write), and
  (3) L's next reference after the segment is not a read (L's exit value is not used).
Variants: each segment alone and all of L's qualifying segments into one T together.  The declaration of L
is left in place (an unreferenced local generates nothing); drop it by hand when every use moved.
The original counted load_vehicles' column loops on `variant` (its exit value 8 is stored to variant's home)
and the border scan on `y`.  Env: FE_PFX_BASE, FE_JOBS."""
import sys, os, re, json, argparse
H = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(H))
sys.path.insert(0, H)
import reusesweep as rs

TYPES = {}  # name -> declared type of the last analyse()


def stmt_end(m, p, be):
    """end (exclusive) of the statement starting at p (a `for`)"""
    q = m.find('(', p)
    d = 0
    while q < be:
        if m[q] == '(':
            d += 1
        elif m[q] == ')':
            d -= 1
            if d == 0:
                break
        q += 1
    q += 1
    while q < be and m[q].isspace():
        q += 1
    if m[q] == '{':
        d = 0
        while q < be:
            if m[q] == '{':
                d += 1
            elif m[q] == '}':
                d -= 1
                if d == 0:
                    return q + 1
            q += 1
        return be
    if re.match(r'(for|while)\b', m[q:q + 6]):
        return stmt_end(m, q, be)
    pd = bd = 0
    while q < be:
        c = m[q]
        if c in '([':
            pd += 1
        elif c in ')]':
            pd -= 1
        elif c == '{':
            bd += 1
        elif c == '}':
            bd -= 1
            if bd < 0:
                return q
        elif c == ';' and pd == 0 and bd == 0:
            return q + 1
        q += 1
    return be


def read_next(m, e, be, T):
    """reusesweep's screen, with a for-init clause `for (.., T = e, ..` counted as a write"""
    mm = rs.ident_re(T).search(m, e, be)
    if not mm:
        return False
    a = mm.start()
    pre = m[max(m.rfind(';', 0, a), m.rfind('{', 0, a), m.rfind('}', 0, a)) + 1:a]
    fm = re.fullmatch(r'\s*for\s*\(([^;]*,)?\s*', pre, re.S)
    if fm:
        rest = m[mm.end():m.find(';', mm.end())]
        part = rs.split_top(rest)[0]
        am = re.match(r'\s*=(?!=)(.*)$', part, re.S)
        return not am or bool(rs.ident_re(T).search(am.group(1)))
    return rs.READ_NEXT(m, e, be, T)


def for_header_init(m, p):
    q = m.find('(', p)
    e = m.find(';', q)
    return m[q + 1:e]


def analyse(text, addr):
    bs, be, m, fvars, locs = rs.ANALYSE(text, addr)
    TYPES.clear()
    p = m.rfind(')', 0, bs)
    q = m.rfind('(', 0, p)
    for prm in rs.split_top(m[q + 1:p]):
        mm = re.fullmatch(r'\s*(?:const\s+)?(int|BOOL)\s+(\w+)\s*', prm)
        if mm:
            TYPES[mm.group(2)] = mm.group(1)
    for at, names in rs.FDECL:
        ty = re.match(r'\w+', m[at:]).group(0)
        for nm in names:
            TYPES.setdefault(nm, ty)
    segs = []
    for nm, at in fvars.items():
        if at < 0:
            continue
        for fm in re.finditer(r'\bfor\s*\(', m[bs:be]):
            p = bs + fm.start()
            init = for_header_init(m, p)
            if not any(re.fullmatch(r'\s*%s\s*=(?!=).*' % re.escape(nm), part, re.S) for part in rs.split_top(init)):
                continue
            e = stmt_end(m, p, be)
            if rs.read_next(m, e, be, nm):
                continue
            segs.append(dict(name=nm, at=p, s=p, e=e, isfor=False, line=text.count('\n', 0, p) + 1,
                             kwlen=0, init=True, end=p))
    return bs, be, m, fvars, segs


def pairs(text, addr):
    ps, m = rs.PAIRS(text, addr)
    ps = [(L, T) for L, T in ps if TYPES.get(T) == TYPES.get(L['name']) and T in TYPES]
    by = {}
    for L, T in ps:
        by.setdefault((L['name'], T), []).append(L)
    for (nm, T), Ls in by.items():
        if len(Ls) > 1:  # every qualifying segment of L into T at once
            Ls = sorted(Ls, key=lambda x: x['s'])
            C = dict(Ls[0], segs=Ls, line=Ls[0]['line'], name=nm)
            C['line'] = 'ALL%d' % Ls[0]['line']
            ps.append((C, T))
    return ps, m


def kill_variant(text, group):
    flat = []
    for L, T in group:
        flat += [(S, T) for S in L.get('segs', [L])]
    seen, out = set(), []
    for S, T in flat:
        if (S['s'], T) not in seen:
            seen.add((S['s'], T))
            out.append((S, T))
    return rs.KILL_VARIANT(text, out)


def rename_variant(text, m, L, T):
    for S in sorted(L.get('segs', [L]), key=lambda x: -x['s']):
        text = rs.RENAME(text, m, S, T)
    return text


def main():
    rs.READ_NEXT, rs.read_next = rs.read_next, read_next
    rs.ANALYSE, rs.PAIRS, rs.KILL_VARIANT, rs.RENAME = rs.analyse, rs.pairs, rs.kill_variant, rs.rename_variant
    rs.analyse, rs.pairs, rs.kill_variant, rs.rename_variant = analyse, pairs, kill_variant, rename_variant
    if '--textual' in sys.argv:
        # no kill probe: VC6 keeps T live through a zero-trip path (load_vehicles' variant is read after the
        # chassis loop), which the original's reuse ignored; the textual screens (1) and (3) remain
        sys.argv.remove('--textual')
        rs.volatile_dead = lambda unit, addr, text, ps, log: ({(id(L), T) for L, T in ps}, {T for L, T in ps})
        rs.dead_pairs = lambda unit, addr, text, keyed, bsig, log: list(keyed)
    rs.SCR = rs.R + '/build/recovery/fe/loopreuse'
    sys.argv[0] = 'loopreuse.py'
    rs.main()


if __name__ == '__main__':
    main()
