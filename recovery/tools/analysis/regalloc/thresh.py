#!/usr/bin/env python3
"""thresh.py <g_family.cpp> [vars=cd] [--oy]: for duel families (names <tag>_cd_k<K>, <tag>_dc_k<K>) print the K at which
the first variable takes the register from the second: 'x' = the threshold is x with c defined first and x+1 with d
defined first (an exact tie at K=x), '(x-1,x)' = c first wins at K=x in both orders (or only the cd order exists:
then a tie at x-1 is not excluded)."""
import sys, re, io, contextlib, collections
import run
src = sys.argv[1]
vs = sys.argv[2] if len(sys.argv) > 2 and not sys.argv[2].startswith('-') else 'cd'
with contextlib.redirect_stdout(io.StringIO()):
    res = run.report(src, run.os.environ.get('FE_PFX_BASE', '2'), '--oy' in sys.argv)
T = collections.OrderedDict()
for fn, h in res.items():
    m = re.fullmatch(r'(.*)_(cd|dc)_k(\d+)', fn)
    if not m:
        continue
    # the second variable wins when it is ever held in a callee-saved register (the first may keep a piece before it)
    T.setdefault(m.group(1), {}).setdefault(m.group(2), {})[int(m.group(3))] = vs[1] if any(x in run.CS for x in h.get(vs[1], [])) else vs[0]
for tag, o in T.items():
    def first_c(d):
        ks = sorted(d)
        bad = [k for k in ks if d[k] not in (vs[0], vs[1])]
        f = next((k for k in ks if d[k] == vs[0]), None)
        return f, bad
    a, bad1 = first_c(o['cd'])
    b, bad2 = first_c(o.get('dc', o['cd']))
    note = ' irregular:%s' % sorted(set(bad1 + bad2))[:6] if bad1 or bad2 else ''
    if a is None or b is None:
        print('%-14s > range%s' % (tag, note))
    elif a == b:
        print('%-14s (%d,%d)%s' % (tag, a - 1, a, note))
    elif b == a + 1:
        print('%-14s %d%s' % (tag, a, note))
    else:
        print('%-14s cd:%s dc:%s%s' % (tag, a, b, note))
