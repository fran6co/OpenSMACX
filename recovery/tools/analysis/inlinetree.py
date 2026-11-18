#!/usr/bin/env python3
"""inlinetree.py [--all] [--out F.json] [--sig KEY=a,b ...] [unit ...]

Tree-wide inlinemap: compile each unit once (FE_PFX_BASE.., FE_JOBS lanes), run inlinemap.analyse on
every non-BYTE_EXACT function (--all: every function) and aggregate the decisions per inline callee:
functions, sites, image E/c against ours E/c, and how many functions expand it more / less than the
image. Writes the per-function rows to --out. Read with recovery/MATCHING_GUIDE.md 2.8.
"""
import sys, os, re, json, argparse, collections
from concurrent.futures import ThreadPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.dirname(HERE))
import threading  # noqa: E402
import fe  # noqa: E402
import inlinemap as im  # noqa: E402

R = fe.R
ap = argparse.ArgumentParser()
ap.add_argument('units', nargs='*')
ap.add_argument('--all', action='store_true')
ap.add_argument('--out', default=os.path.join(fe.SCR, 'inlinetree.json'))
ap.add_argument('--sig', action='append', default=[])
ap.add_argument('--callee', action='append', default=[])
ap.add_argument('--top', type=int, default=60)
a = ap.parse_args()
cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
units = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/')]
if a.units:
    units = [u for u in units if u in a.units or os.path.basename(u) in a.units]
work = []
for u in units:
    try:
        t = open(R + '/' + u, encoding='latin-1').read()
    except OSError:
        continue
    addrs = [int(m.group(1), 16) for m in re.finditer(r'// ORIGINAL: 0x([0-9A-F]{8}) \S+ 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)', t)
             if a.all or 'BYTE_EXACT' not in m.group(2)]
    if addrs:
        work.append((u, t, addrs))
LOCK = threading.Lock()  # inlinemap keeps module-level signature caches
jobs = int(os.environ.get('FE_JOBS', '5'))
base = int(os.environ.get('FE_PFX_BASE', '21'))


def lane(k):
    out = []
    for i in range(k, len(work), jobs):
        u, t, addrs = work[i]
        c, err = fe.compile_unit(u, R + '/' + u, str(base + k))
        if c is None:
            out.append(dict(unit=u, addr='-', sym='COMPILE FAIL %s' % (err or '')[-300:], callee='?', image='', ours='', level=''))
            continue
        for addr in addrs:
            try:
                sym, lo, hi = fe.ident(t, addr)
                with LOCK:
                    img, ours, rows, per, cands = im.analyse(c, sym, lo, hi, a.callee, a.sig)
            except Exception as e:
                out.append(dict(unit=u, addr='%08X' % addr, sym='ERROR %r' % e, callee='?', image='', ours='', level=''))
                continue
            for n, sites in per.items():
                ri, ro, lv = im.strings(sites)
                out.append(dict(unit=u, addr='%08X' % addr, sym=sym, callee=n, image=ri, ours=ro, level=lv))
    return out


fe.compile_cmd(work[0][0] if work else '')  # fill fe's command cache before the lanes share it
res = []
with ThreadPoolExecutor(jobs) as ex:
    for o in ex.map(lane, range(jobs)):
        res += o
json.dump(res, open(a.out, 'w'), indent=0)
agg = collections.defaultdict(lambda: collections.Counter())
for r in res:
    g = agg[r['callee']]
    g['fn'] += 1
    g['sites'] += len(r['image'])
    g['iE'] += r['image'].count('E')
    g['ic'] += r['image'].count('c')
    g['oE'] += r['ours'].count('E')
    g['oc'] += r['ours'].count('c')
    dE = r['ours'].count('E') - r['image'].count('E')
    if r['image'] != r['ours']:
        g['differ'] += 1
        g['ours_more_E' if dE > 0 else 'ours_less_E' if dE < 0 else 'same_count'] += 1
print('%-45s %4s %5s %9s %9s %6s %5s %5s %5s' % ('callee', 'fn', 'sites', 'img E/c', 'ours E/c', 'differ', 'oE>', 'oE<', 'oE='))
for n, g in sorted(agg.items(), key=lambda kv: -kv[1]['differ'])[:a.top]:
    print('%-45s %4d %5d %4d/%-4d %4d/%-4d %6d %5d %5d %5d' % (n[:45], g['fn'], g['sites'], g['iE'], g['ic'], g['oE'], g['oc'],
                                                         g['differ'], g['ours_more_E'], g['ours_less_E'], g['same_count']))
