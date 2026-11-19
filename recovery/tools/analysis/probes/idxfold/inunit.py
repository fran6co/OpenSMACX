#!/usr/bin/env python3
"""inunit.py: the idxfold question inside the real units.  Each variant replaces one source
spelling of `Facility[FacilitySPStart + id]` in base_editor_proj (basewin.cpp) or
process_message (netdaemon.cpp) and reports whether our compile keeps +0xd2 in the
`lea r,[r+r*2+K]` (INDEX, the image) or folds 0xd20 into the displacement (FOLDED), plus fe rows.
Usage: uv run python inunit.py [variant-file.py]   (FE_PFX_BASE, default 14; 5 lanes)"""
import sys, os, re, subprocess
from concurrent.futures import ThreadPoolExecutor
R = os.path.abspath(os.path.dirname(os.path.abspath(__file__)) + '/../../../../..')
T = R + '/recovery/tools'
BASE = int(os.environ.get('FE_PFX_BASE', '14'))

def run(unit, addr, variants):
    src = open(R + '/' + unit, encoding='latin-1').read()
    deck = []
    for key, pairs in variants:
        t = src
        for a, b in pairs:
            assert t.count(a) == 1, (key, a, t.count(a))
            t = t.replace(a, b)
        deck.append((key, t))
    d = R + '/build/recovery/fe/idxfold'; os.makedirs(d, exist_ok=True)
    def one(k):
        key, text = deck[k]; lane = BASE + k % 5
        p = '%s/v%d_%d.cpp' % (d, lane, k); open(p, 'w', encoding='latin-1').write(text)
        env = dict(os.environ, FE_PFX_BASE=str(lane))
        dis = subprocess.run(['uv', 'run', 'python', T + '/objdis.py', unit, p, addr], cwd=R, env=env, capture_output=True, text=True).stdout
        fe = subprocess.run(['uv', 'run', 'python', T + '/fe.py', unit, addr, '--src', p, '--pfx', str(lane), '--quiet'], cwd=R, capture_output=True, text=True).stdout
        m = re.search(r'diff=(\d+)', fe)
        kept = len(re.findall(r'\*2 \+ 0xd2\]', dis))
        fold = len(re.findall(r'0xd20\]|0xd20$', dis))
        return key, kept, m.group(1) if m else '?', dis.count('\n')
    # one lane at a time per prefix
    import threading
    locks = [threading.Lock() for _ in range(5)]
    def locked(k):
        with locks[k % 5]: return one(k)
    with ThreadPoolExecutor(5) as ex:
        for key, kept, rows, n in ex.map(locked, range(len(deck))):
            print('%-14s index_kept=%d rows=%s len=%d' % (key, kept, rows, n))

if __name__ == '__main__':
    g = {}; exec(open(sys.argv[1]).read(), g)
    for unit, addr, variants in g['DECKS']:
        print('==', unit, addr); run(unit, addr, variants)
