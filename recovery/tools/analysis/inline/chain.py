#!/usr/bin/env python3
"""chain.py: nested inline chains A -> L1 -> L2 -> L3 (MATCHING_GUIDE 2.8 rule 5, 2026-11-20 par-4).

   chain.run(levels, body, extra=()) compiles one caller `body` (list of statements) against inline levels
   `levels` = [(name, marker, nifs, children, kw), ...] where each level's body is `marker(k);` then `nifs`
   one-line ifs then a call `child(k);` per child.  Returns, per site constant k, a string of per-level
   decisions in the listing order: for each level name, 'E' if its marker is pushed with k, 'c' if the
   level itself is called with k, '.' if neither (its parent was called)."""
import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ib
PFX = os.environ.get('FE_PFX_BASE', '21')


def level(name, marker, nifs, children=(), kw='inline', base=0, pre=()):
    ifs = ' '.join('if (ga[%d] == 1000) use(%d);' % (base + i, base + i) for i in range(nifs))
    kids = ' '.join('%s(k);' % c for c in children)
    return '%s void %s(int k) { %s(k); %s %s %s }' % (kw, name, marker, ' '.join(pre), ifs, kids)


def compile_(parts, pfx=PFX, extra=()):
    hdr = '#include "probe.h"\nextern void mk3(int); extern void mk4(int); extern void mk5(int); extern void mk6(int);\n'
    asm, out = ib.compile_asm(None, pfx, False, extra=extra, text=hdr + '\n'.join(parts))
    return {name: lines for name, sym, lines in ib.functions(asm)}


def decisions(lines, names, markers):
    """-> {k: {level_name: 'E'|'c'}} from push K; call X pairs."""
    res = {}
    for i, l in enumerate(lines):
        m = re.match(r'call\s+(\S+)', l)
        if not m:
            continue
        sym = m.group(1)
        nm = ib.demangle_name(sym)
        k = ib.last_push(lines, i)
        if k is None:
            continue
        if nm in markers:
            res.setdefault(k, {})[markers[nm]] = 'E'
        elif nm in names:
            res.setdefault(k, {})[nm] = 'c'
    return res


def fmt(res, order, ks):
    rows = []
    for lv in order:
        rows.append('%-6s %s' % (lv, ''.join(res.get(k, {}).get(lv, '.') for k in ks)))
    return '\n'.join(rows)
