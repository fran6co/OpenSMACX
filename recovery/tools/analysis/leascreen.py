#!/usr/bin/env python3
"""leascreen.py FEALL.json [--max N] : non-exact functions whose ONLY differences are commutative operand order
(MATCHING_GUIDE 3.4 counter class): `lea r,[a+b+K]` against `[b+a+K]`, or a block of differing rows that holds the
same instructions in another order (load/statement order). Prints one line per function: kinds and rows.
One compile per unit (lane FE_PFX_BASE).  Screen only."""
import sys, os, re, json, collections, argparse
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import fe
ap = argparse.ArgumentParser(); ap.add_argument('feall'); ap.add_argument('--max', type=int, default=16)
a = ap.parse_args()
d = [x for x in json.load(open(a.feall)) if not x['exact'] and x['diff'] <= a.max]
byunit = collections.defaultdict(list)
for x in d: byunit[x['unit']].append(int(x['addr'], 16))
LEA = re.compile(r'(lea \w+, (?:dword ptr )?\[)(\w+) \+ (\w+)(.*)$')
def canon(t):
    m = LEA.match(t)
    return m.group(1) + '+'.join(sorted([m.group(2), m.group(3)])) + m.group(4) if m else t
for unit, addrs in sorted(byunit.items()):
    c, err = fe.compile_unit(unit, fe.R + '/' + unit, os.environ.get('FE_PFX_BASE', '21'))
    if c is None: continue
    text = open(fe.R + '/' + unit, encoding='latin-1').read()
    for addr in addrs:
        r = fe.evaluate(c, text, addr)
        if 'error' in r or r['exact']: continue
        rows = r['rows']
        if any(k == '!>' for k, o, c_ in rows): continue
        blocks, cur = [], None
        for k, o, c_ in rows:
            if k == '!!':
                if cur is None: cur = ([], []); blocks.append(cur)
                if o: cur[0].append(o[1])
                if c_: cur[1].append(c_[1])
            else: cur = None
        kinds = []
        for oi, ci in blocks:
            if collections.Counter(map(canon, oi)) != collections.Counter(map(canon, ci)): kinds = None; break
            kinds.append('lea' if [canon(t) for t in oi] == [canon(t) for t in ci] else 'order%d' % len(oi))
        if kinds is not None and blocks:
            print('%08X %-4d %-40s %s  %s' % (addr, r['diff'], r['sym'][:40], ','.join(kinds), unit))
