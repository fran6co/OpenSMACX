#!/usr/bin/env python3
"""duel.py SPEC [FILTER]: per probe family, the smallest N at which candidate c outranks d
(MATCHING_GUIDE 2.7 "Tail-merged references count"; m_*.spec).  One probe function per translation unit.

Spec: probes separated by lines `== name`, each the C++ text placed after probe.h.  `{N}` becomes N copies
of `use_c(c);`, `{NT}` N copies of `m();` (for `this`).  A line `// duel X Y` names the two candidates
(default c d; `this` = the prologue copy of ecx).  A local's register is read from the pushes before its
use_X calls.  The threshold moves by the extra family text's weight in units of one reference of c."""
import sys, os, re
from concurrent.futures import ThreadPoolExecutor
RA = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, RA)
import run, cand
ORDER = ['esi', 'edi', 'ebx', 'ebp']
os.environ.setdefault('FE_PFX_BASE', '8')
NJ = 5
import threading
lock = threading.Lock()
_b = int(os.environ.get('FE_PFX_BASE', '8'))
free = list(range(_b, _b + NJ))

def comp(text, cname='c', dname='d'):
    with lock:
        p = free.pop()
    try:
        path = '%s/g_duel%d.cpp' % (RA, p - _b)
        open(path, 'w').write('#include "probe.h"\n' + text + '\n')
        asm = run.compile_asm(path, str(p))
        fn = [ins for nm, ins in run.functions(asm) if re.search(r'\bprobe\b', nm)]
        ins = fn[0]
    finally:
        with lock:
            free.append(p)
    rl = cand.roles(ins)
    def reg(name):
        if name == 'this':
            rs = [r for r, v in rl.items() if 'this' in v]
            return rs[0] if rs else 'mem'
        from collections import Counter
        cnt = Counter()
        for k, l in enumerate(ins):
            if re.match(r'call\s+\?use_%s@' % name, l) and k and ins[k-1].startswith('push'):
                cnt[ins[k-1].split()[1]] += 1
        regs = [r for r, _ in cnt.most_common() if r in ORDER]
        return regs[0] if regs else 'mem'
    return reg(cname), reg(dname), ins

def rank(r):
    return ORDER.index(r) if r in ORDER else 9

def threshold(body, cname, dname, lo=0, hi=40):
    # smallest N with rank(c) < rank(d)
    def win(n):
        t = body.replace('{N}', ' '.join(['use_c(c);'] * n)).replace('{NT}', ' '.join(['m();'] * n))
        a, b, _ = comp(t, cname, dname)
        return rank(a) < rank(b)
    if not win(hi):
        return '>%d' % hi
    while lo < hi:
        mid = (lo + hi) // 2
        if win(mid):
            hi = mid
        else:
            lo = mid + 1
    return lo

if __name__ == '__main__':
    text = open(sys.argv[1]).read()
    fams = []
    for blk in text.split('\n== ')[1:] if text.startswith('== ') is False else text[3:].split('\n== '):
        name, body = blk.split('\n', 1)
        fams.append((name.strip(), body))
    only = sys.argv[2] if len(sys.argv) > 2 else None
    fams = [f for f in fams if not only or only in f[0]]
    def job(f):
        name, body = f
        m = re.search(r'//\s*duel\s+(\w+)\s+(\w+)', body)
        cn, dn = (m.group(1), m.group(2)) if m else ('c', 'd')
        return name, threshold(body, cn, dn)
    with ThreadPoolExecutor(NJ) as ex:
        for name, t in ex.map(job, fams):
            print('%-28s T=%s' % (name, t), flush=True)
