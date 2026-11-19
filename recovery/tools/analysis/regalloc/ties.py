#!/usr/bin/env python3
"""ties.py: register-allocation tie probes, one function per translation unit so the front-end symbol counter
is controlled.  Reads lines from stdin (or a file):

    name | pre | args | spec          (spec language of gen.py; `{N}` anywhere is swept)

`pre` is text placed before the probe function (after probe.h); `PAD` in it expands to N `extern int TiePadK;`
declarations, `FN` to N earlier dummy functions `void TieFnK() {}`.  `--n 0-63` (default 0) is the sweep.
Each cell prints the register letter of every variable in `--vars` (S=esi D=edi B=ebx P=ebp m=memory *=changes).

    python3 ties.py --vars cd --n 0-63 < ties.spec
"""
import sys, os, io, re, contextlib, argparse
from concurrent.futures import ThreadPoolExecutor
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gen, run

L = {'esi': 'S', 'edi': 'D', 'ebx': 'B', 'ebp': 'P'}


def expand(pre, N):
    pre = pre.replace('~', ' ')
    pre = pre.replace('PAD', ''.join('extern int TiePad%d;\n' % k for k in range(N)))
    pre = pre.replace('FN', ''.join('void TieFn%d() {}\n' % k for k in range(N)))
    return pre.replace('{N}', str(N))


def source(pre, args, spec, N):
    spec = spec.replace('{N}', str(N))
    body = spec[4:].replace('~', ' ') if spec.startswith('RAW:') else gen.body(spec)
    args = args.replace('~', ' ')
    sig = 'void probe(%s)' % args if not args.startswith('T::') else 'void T::probe(%s)' % args[3:]
    head = '#include "probe.h"\n'
    if args.startswith('T::'):
        head += 'struct T : Obj { void probe(%s); };\n' % args[3:]
    return head + expand(pre, N) + '\n' + sig + ' { ' + body + ' }\n'


def evaluate(jobs, oy=False, extra=()):
    """jobs: [(key, cpp text)] -> {key: homes dict}"""
    n = int(os.environ.get('FE_JOBS', '5')); base = int(os.environ.get('FE_PFX_BASE', '8'))
    res = {}

    def work(i):
        out = {}
        path = '%s/g_tie%d.cpp' % (run.HERE, i)
        for key, text in jobs[i::n]:
            open(path, 'w').write(text)
            asm = run.compile_asm(path, str(base + i), oy, extra)
            out[key] = next((run.homes(ins) for nm, ins in run.functions(asm) if nm == 'probe'), {})
            out[key]['_asm'] = asm
        return out
    with ThreadPoolExecutor(n) as ex:
        for r in ex.map(work, range(n)):
            res.update(r)
    return res


def cell(h, vs):
    c = ''
    for v in vs:
        x = h.get(v, ['?']) if v != '_' else ['?']
        c += L.get(x[0], 'm') if len(x) == 1 else '*'
    return c


def parse_range(s):
    out = []
    for part in s.split(','):
        a, _, b = part.partition('-')
        out += list(range(int(a), int(b or a) + 1))
    return out


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('spec', nargs='?')
    ap.add_argument('--vars', default='cd')
    ap.add_argument('--n', default='0')
    ap.add_argument('--oy', action='store_true')
    a = ap.parse_args()
    lines = [l for l in (open(a.spec) if a.spec else sys.stdin).read().split('\n') if l.strip() and not l.startswith('#')]
    Ns = parse_range(a.n)
    jobs = []
    rows = []
    for li, l in enumerate(lines):
        name, pre, args, spec = [x.strip() for x in l.split('|', 3)]
        ns = Ns if ('{N}' in l or 'PAD' in pre or 'FN' in pre) else [0]
        rows.append((li, name, ns))
        for N in ns:
            jobs.append(((li, N), source(pre, args, spec, N)))
    res = evaluate(jobs, a.oy)
    for li, name, ns in rows:
        cells = [cell(res[(li, N)], a.vars) for N in ns]
        if len(set(cells)) == 1 and len(ns) > 1:
            print('%-20s all %s' % (name, cells[0]))
        else:
            print('%-20s %s' % (name, ' '.join(cells)))
