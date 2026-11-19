#!/usr/bin/env python3
"""padprobe.py <probe.cpp> -f FUNC --cut RX[@K] --upto RX [--max 64]
Recompile the probe with 0..max-1 dummy declarations inserted before the first function body and print the
distinct results of the cut window with the pad counts that give each (symbol-counter sensitivity test)."""
import sys, os, re, argparse, subprocess
from concurrent.futures import ThreadPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0, HERE)
import run
ap = argparse.ArgumentParser(); ap.add_argument('src'); ap.add_argument('-f', required=True); ap.add_argument('--cut'); ap.add_argument('--upto')
ap.add_argument('--max', type=int, default=64); ap.add_argument('--oy', action='store_true')
a = ap.parse_args()
text = open(a.src).read()
m = re.search(r'^\w[^\n;]*\)\s*(const\s*)?\{', text, re.M)
pos = m.start()
pfx = [int(os.environ.get('FE_PFX_BASE', '27')) + k for k in range(int(os.environ.get('FE_JOBS', '5')))]
def one(n):
    k = n % len(pfx)
    p = '%s/pad_%d_%s' % (os.environ.get('FE_SCRATCH', run.R + '/build/recovery/fe') + '/sched', n, os.path.basename(a.src))
    os.makedirs(os.path.dirname(p), exist_ok=True)
    open(p, 'w').write(text[:pos] + ''.join('void zz_pad_%d();\n' % i for i in range(n)) + text[pos:])
    import shutil; shutil.copy(HERE + '/probe.h', os.path.dirname(p) + '/probe.h')
    for name, ins in run.functions(run.compile_asm(p, pfx[k], a.oy)):
        if re.search(a.f, name):
            if a.cut:
                rx, _, kk = a.cut.partition('@'); hits = [i for i, x in enumerate(ins) if re.search(rx, x)]; kk = int(kk or 1)
                ins = ins[hits[kk - 1] + 1:] if len(hits) >= kk else []
            if a.upto:
                h = [i for i, x in enumerate(ins) if re.search(a.upto, x)]
                if h: ins = ins[:h[0] + 1]
            return n, '; '.join(x for x in ins if not x.startswith('$'))
    return n, '?'
import threading
locks = {p: threading.Lock() for p in pfx}
def lone(n):
    with locks[pfx[n % len(pfx)]]:
        return one(n)
res = {}
with ThreadPoolExecutor(len(pfx)) as ex:
    for n, r in ex.map(lone, range(a.max)):
        res.setdefault(r, []).append(n)
for r, ns in res.items():
    print('%2d pads %s\n    %s' % (len(ns), ns if len(ns) < 20 else '%d..%d' % (ns[0], ns[-1]), r))
