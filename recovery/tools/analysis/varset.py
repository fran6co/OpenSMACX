#!/usr/bin/env python3
"""varset.py <unit> <0xADDR> [--src ALT.cpp] [--pfx N] [--min K] [--sites]
Variable-set reconciliation (MATCHING_GUIDE 3.3 "Slot order follows weighted priority", "One name reused"):
the image's frame slots against our locals.

Ours: every frame operand of the /FAsc listing carries its variable (`_name$[ebp]`, block-locals as
`name$NNN`, compiler temporaries as `$T123`/`tvNN`), so an access is named even where VC6 packs several
variables into one slot. Image: the slot of the aligned row (fe alignment, same instruction shape).

Prints
  IMAGE SLOTS   offset, accesses, stores (imm/callee-saved/scratch), loads, and our variables paired there.
                `mem` = never loaded into a callee-saved register (address-taken, aggregate or low rank).
  OUR LOCALS    name, our slot, our rank against the image rank of its majority slot (slot order = weighted
                reference order, so a large rank difference means the variable has too many or too few
                references in our source), and every image slot its accesses pair with.
  MERGE         several of our variables whose majority image slot is the same one: one original variable
                (a reuse our source split), unless their live ranges are disjoint and VC6 packed them
                (then ours packs them elsewhere: check OUR LOCALS for their own slots).
  SPLIT         one of our variables paired with several image slots: two original variables under one of
                our names (or a temporary/block-local the image packs into another variable's slot).
Screen only: pairing needs the same instruction shape on both sides, so register-resident stretches are blind.
"""
import sys, os, re, collections, argparse
H = os.path.dirname(os.path.abspath(__file__))
T_ = os.path.dirname(H)
R = os.path.dirname(os.path.dirname(T_))
sys.path.insert(0, T_)
sys.path.insert(0, H)
os.chdir(R)
import fe
import regprio

SLOT = re.compile(r'\[ebp ([+-]) (0x[0-9a-f]+|\d+)\]')
SYM = re.compile(r'(\$T\d+|tv\d+|_\w+?\$\d*|\w+\$\d+)\[ebp')
CS = ('ebx', 'esi', 'edi')


def slot(s):
    m = SLOT.search(s)
    if not m:
        return None
    v = int(m.group(2), 0)
    return -v if m.group(1) == '-' else v


def klass(text):
    """store/load class of an instruction touching a frame slot"""
    p = text.split(None, 1)
    mn = p[0]
    ops = [x.strip() for x in (p[1] if len(p) > 1 else '').split(',')]
    if mn == 'lea':
        return 'lea'
    if mn in ('mov', 'movsx', 'movzx') and len(ops) == 2:
        if '[ebp' in ops[0]:
            return 'st_cs' if ops[1] in CS else ('st_imm' if re.match(r'-?(0x)?[0-9a-f]+$', ops[1]) else 'st')
        if '[ebp' in ops[1]:
            return 'ld_cs' if ops[0] in CS else 'ld'
    return 'op'


def varname(sym):
    s = sym
    if s.startswith('_'):
        s = s[1:]
    m = re.match(r'(\w+?)\$(\d*)$', s)
    if m:
        return m.group(1) + ('#' + m.group(2)[-3:] if m.group(2) else '')
    return s


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('unit')
    ap.add_argument('addr')
    ap.add_argument('--src')
    ap.add_argument('--pfx', type=int, default=int(os.environ.get('FE_PFX_BASE', '21')))
    ap.add_argument('--min', type=int, default=3)
    ap.add_argument('--sites', action='store_true')
    a = ap.parse_args()
    unit = a.unit if a.unit.startswith('src/') else 'src/' + a.unit
    src = os.path.abspath(a.src) if a.src else R + '/' + unit
    addr = int(a.addr, 16)
    text = open(src, encoding='latin-1').read()
    c, err = fe.compile_unit(unit, src, a.pfx)
    if c is None:
        sys.exit('compile failed: %s' % (err or '')[:300])
    r = fe.evaluate(c, text, addr)
    items, frame = regprio.cod_listing(unit, src, a.pfx, r['sym'])
    # our accesses by offset: variable name from the listing operand
    name_at, line_of, cur = {}, {}, 0
    for it in items:
        if it[0] == 'line':
            cur = it[1]
        elif it[0] == 'ins':
            line_of[it[1]] = cur
            m = SYM.search(it[2])
            if m:
                name_at[it[1]] = varname(m.group(1))
    img = collections.defaultdict(collections.Counter)      # image slot -> class counts
    ours = collections.defaultdict(collections.Counter)     # our var -> our slot counts
    pair = collections.defaultdict(collections.Counter)     # our var -> image slot
    rev = collections.defaultdict(collections.Counter)      # image slot -> our var
    sites = collections.defaultdict(list)
    shape = lambda s: SLOT.sub('[S]', s)
    for k, o, c_ in r['rows']:
        if o is not None:
            si = slot(o[1])
            if si is not None:
                img[si][klass(o[1])] += 1
                img[si]['n'] += 1
        if c_ is not None:
            so = slot(c_[1])
            if so is not None:
                nm = name_at.get(c_[0], 'T%s' % hex(so))
                ours[nm][so] += 1
                if o is not None:
                    si = slot(o[1])
                    if si is not None and shape(o[1]) == shape(c_[1]):
                        pair[nm][si] += 1
                        rev[si][nm] += 1
                        sites[(nm, si)].append((line_of.get(c_[0], 0), r['lo'] + o[0]))
    islots = sorted((s for s in img if s < 0), reverse=True)
    irank = {s: i for i, s in enumerate(islots)}
    oslots = sorted({s for v in ours.values() for s in v if s < 0}, reverse=True)
    orank = {s: i for i, s in enumerate(oslots)}
    print('FE %08X %s diff=%d   image slots %d, our slots %d, our named frame variables %d' % (
        addr, r['sym'], r['diff'], len(islots), len(oslots), len(ours)))
    print('\nIMAGE SLOTS (rank offset accesses | stores imm/cs/scratch loads cs/scratch | our variables)')
    for s in islots + sorted(s for s in img if s >= 0):
        cn = img[s]
        mem = ' mem' if not cn['ld_cs'] and not cn['st_cs'] else ''
        print('%3d %7s n=%-4d st %d/%d/%d ld %d/%d%s%s | %s' % (
            irank.get(s, -1), hex(s), cn['n'], cn['st_imm'], cn['st_cs'], cn['st'], cn['ld_cs'], cn['ld'],
            ' &' if cn['lea'] else '', mem,
            ' '.join('%s:%d' % (n, k) for n, k in rev[s].most_common(8))))
    print('\nOUR LOCALS (name, our slot(s), our rank -> image rank of its majority slot, image slots)')
    major = {}
    for nm in sorted(ours, key=lambda n: -max(ours[n], key=lambda s: ours[n][s])):
        so = ours[nm].most_common(1)[0][0]
        pc = pair[nm]
        if pc:
            major[nm] = pc.most_common(1)[0][0]
        d = ''
        if nm in major and so in orank and major[nm] in irank:
            d = '%3d ->%3d (%+d)' % (orank[so], irank[major[nm]], irank[major[nm]] - orank[so])
        print('%-18s %-7s n=%-4d %-14s | %s' % (nm, hex(so), sum(ours[nm].values()), d,
              ' '.join('%s:%d' % (hex(s), k) for s, k in pc.most_common(8)) or '(no aligned frame access)'))
    print('\nMERGE candidates (image slot <- our variables whose majority slot it is)')
    by = collections.defaultdict(list)
    for nm, s in major.items():
        if pair[nm][s] >= a.min:
            by[s].append(nm)
    out = []
    for s, ns in by.items():
        if len(ns) > 1:
            out.append((-sum(pair[n][s] for n in ns), s, ns))
    for _, s, ns in sorted(out):
        print('  %7s <- %s' % (hex(s), ', '.join('%s(%s,%d)' % (n, hex(ours[n].most_common(1)[0][0]), pair[n][s])
                                                 for n in sorted(ns, key=lambda n: -pair[n][s]))))
    print('\nSPLIT candidates (our variable -> several image slots)')
    out = []
    for nm, pc in pair.items():
        tot = sum(pc.values())
        big = [(s, k) for s, k in pc.most_common() if k >= a.min and k * 5 >= tot]
        if len(big) > 1:
            out.append((-big[1][1], nm, big))
    for _, nm, big in sorted(out):
        print('  %-18s -> %s' % (nm, ' '.join('%s:%d' % (hex(s), k) for s, k in big)))
        if a.sites:
            for s, k in big:
                print('      %s: %s' % (hex(s), ' '.join('L%d@%x' % x for x in sites[(nm, s)][:16])))


if __name__ == '__main__':
    main()
