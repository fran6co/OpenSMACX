#!/usr/bin/env python3
"""scan.py FEALL.json [unit ...]: per non-exact function, count `global * value` multiplies by operand
order on each side: Gm = `imul r, [<global>]` (value copied/loaded first, global as the memory operand),
Gr = `mov r, [<global>]` followed by `imul r, <reg or stack slot>` (global loaded first).  Functions
whose counts differ are printed; per mulorder/inl.py a Gm the image has and ours lacks usually means the
original passed the field expression (not a named local) to an inline such as map_loc.
Run from the repo root with FE_PFX_BASE set."""
import sys, os, re, json, collections
sys.path.insert(0, os.getcwd() + '/recovery/tools')
import fe
pfx = os.environ.get('FE_PFX_BASE', '21')
J = json.load(open(sys.argv[1])); units = set(sys.argv[2:])


def count(seq):
    gm = gr = 0
    for i, t in enumerate(seq):
        if re.match(r'imul (\w+), dword ptr \[<r>\]$', t):
            gm += 1
        m = re.match(r'mov (\w+), dword ptr \[<r>\]$', t)
        if m:
            for u in seq[i + 1:i + 4]:
                if re.match(r'imul %s, (e\w\w|dword ptr \[e[bs]p [-+] 0x\w+\])$' % m.group(1), u):
                    gr += 1; break
    return gm, gr


by = collections.defaultdict(list)
for e in J:
    if not e.get('exact') and (not units or e['unit'] in units):
        by[e['unit']].append(int(e['addr'], 16))
for u, addrs in sorted(by.items()):
    text = open(fe.R + '/' + u, encoding='latin-1').read()
    c, err = fe.compile_unit(u, fe.R + '/' + u, pfx)
    if c is None:
        continue
    for a in addrs:
        r = fe.evaluate(c, text, a)
        if 'rows' not in r:
            continue
        img = [o[1] for k, o, c_ in r['rows'] if o is not None]
        our = [c_[1] for k, o, c_ in r['rows'] if c_ is not None]
        ci, co = count(img), count(our)
        if ci != co:
            print('%s %08X %-40s img Gm=%d Gr=%d  ours Gm=%d Gr=%d  diff=%d' % (u, a, r['sym'][:40], ci[0], ci[1], co[0], co[1], r['diff']))
