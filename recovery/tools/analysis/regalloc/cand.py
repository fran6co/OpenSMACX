#!/usr/bin/env python3
"""cand.py: candidate-set probes (MATCHING_GUIDE 2.7 'Candidate set').  One probe function per translation unit.

    python3 cand.py FILE.spec [--oy] [--asm NAME] [--grep REGEX]

A spec file holds probes separated by lines `== name`; each probe is the C++ text placed after probe.h
(it must define a function named `probe` or a member `probe`).  Lines `%% K=a,b,c` before a probe sweep
`{K}` over the values (one probe per value, named name/K).  For every probe the tool prints the callee-saved
registers the prologue saves, the frame size and, per register, the sequence of roles it takes
(`this` = prologue copy of ecx, `lea:this+K` = an address temporary, `slot:x` = a cached piece of local x,
`def:x` = the result of def_x(), `#K` = constant, `call` = a kept call result, `?` = other)."""
import sys, os, re, argparse
from concurrent.futures import ThreadPoolExecutor
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import run

CS = ('esi', 'edi', 'ebx', 'ebp')


def parse(text):
    probes = []
    sweep = None
    cur = None
    for l in text.split('\n'):
        if l.startswith('== '):
            if cur:
                probes.append(cur)
            cur = [l[3:].strip(), [], sweep]
            sweep = None
        elif l.startswith('%% '):
            k, v = l[3:].split('=', 1)
            sweep = (k.strip(), [x.strip() for x in v.split(',')])
        elif cur is not None:
            cur[1].append(l)
    if cur:
        probes.append(cur)
    out = []
    for name, lines, sw in probes:
        body = '\n'.join(lines)
        if sw:
            for v in sw[1]:
                out.append(('%s/%s' % (name, v), body.replace('{%s}' % sw[0], v)))
        else:
            out.append((name, body))
    return out


def roles(ins):
    """per callee-saved register: list of roles in order"""
    ecx_this = True
    res = {r: [] for r in CS}
    for k, l in enumerate(ins):
        m = re.match(r'(mov|lea|xor|movsx|movzx|pop)\s+(e[sd]i|ebx|ebp), ?(.*)$', l.replace('\t', ' ').strip())
        if not m:
            continue
        op, r, src = m.groups()
        if op == 'pop':
            continue
        if r == 'ebp' and src == 'esp':
            continue
        if op == 'xor' and src == r:
            role = '#0'
        elif op == 'lea':
            role = 'lea:' + re.sub(r'DWORD PTR |\s', '', src)
        elif src == 'ecx' and k < 12:
            role = 'this'
        elif src == 'eax' and k > 0 and ins[k - 1].startswith('call'):
            mm = re.search(r'\?(def|get)_?(\w*)@', ins[k - 1])
            role = 'def:' + mm.group(2) if mm and mm.group(1) == 'def' else 'call'
        elif re.fullmatch(r'-?\d+', src):
            role = '#' + src
        else:
            s = re.search(r'_(\w+)\$\d*\[', src)
            role = ('slot:' + s.group(1)) if s else ('=' + re.sub(r'DWORD PTR |\s', '', src))
        if not res[r] or res[r][-1] != role:
            res[r].append(role)
    return {r: v for r, v in res.items() if v}


def evaluate(jobs, oy=False):
    n = int(os.environ.get('FE_JOBS', '5'))
    base = int(os.environ.get('FE_PFX_BASE', '8'))
    res = {}

    def work(i):
        out = {}
        path = '%s/g_cand%d.cpp' % (run.HERE, i)
        for key, text in jobs[i::n]:
            open(path, 'w').write('#include "probe.h"\n' + text + '\n')
            asm = run.compile_asm(path, str(base + i), oy)
            fn = [(nm, ins) for nm, ins in run.functions(asm) if re.search(r'\bprobe\b', nm)]
            out[key] = fn[0][1] if fn else []
        return out
    with ThreadPoolExecutor(n) as ex:
        for r in ex.map(work, range(n)):
            res.update(r)
    return res


def summary(ins):
    saved = []
    for i in ins[:12]:
        if i.startswith("push") and i.split()[1] in CS[:3] and i.split()[1] not in saved:
            saved.append(i.split()[1])
    fr = next((i.split(',')[-1].strip() for i in ins[:8] if re.match(r'sub\s+esp', i)), '0')
    rl = roles(ins)
    return 'save=%s frame=%s  %s' % (','.join(saved), fr, '  '.join('%s:%s' % (r, '>'.join(v)) for r, v in rl.items()))


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('spec')
    ap.add_argument('--oy', action='store_true')
    ap.add_argument('--asm', default=None, help='print the listing of probes whose name contains this')
    ap.add_argument('--grep', default=None, help='print the instructions matching this regex per probe')
    a = ap.parse_args()
    jobs = parse(open(a.spec).read())
    res = evaluate(jobs, a.oy)
    for key, text in jobs:
        ins = res[key]
        print('%-22s %s' % (key, summary(ins) if ins else 'NO FUNCTION'))
        if a.grep:
            for i in ins:
                if re.search(a.grep, i):
                    print('        ' + i.replace('\t', ' '))
        if a.asm and a.asm in key:
            print('\n'.join('      ' + i.replace('\t', ' ') for i in ins))
