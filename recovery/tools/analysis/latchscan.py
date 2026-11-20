#!/usr/bin/env python3
"""latchscan.py [unit ...] [--addr 0xADDR] [--all] [--json FILE] [--show]
Preheader/latch clear screen (MATCHING_GUIDE 3.7 "A loop-local zeroed in the preheader AND in the latch").

For every loop (back edge) of the image and of our compile, a zeroed destination (`xor r,r`,
`mov r/[slot],0`, `and [slot],0`) is classified by where the clears sit:
  PL   cleared in the preheader (the straight-line run before the loop head) and in the latch
       (the straight-line run ending at the back edge), not in the first block of the body;
  TOP  cleared in the first block after the head (body top, or an inner for-init) and not in the latch.
Registers differ between the two compiles, so a function is listed when the image's PL/TOP counts
differ from ours: image PL > ours PL with ours TOP > image TOP is the rule's signature
("clear before the loop, re-clear at the end of each iteration" in the image, body-top clear in ours).
Screen only; one fe.py compile per unit.
Run from the worktree root:  FE_PFX_BASE=27 FE_JOBS=5 uv run python recovery/tools/analysis/latchscan.py"""
import sys, os, re, json, argparse
from concurrent.futures import ThreadPoolExecutor
S = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, S); sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import fe
from loopscan import markers
R = fe.R
W = 10  # instructions scanned per window


def zdest(t):
    m = re.match(r'xor (\w+), (\w+)$', t)
    if m and m.group(1) == m.group(2): return m.group(1)
    m = re.match(r'(?:mov|and) (?:dword ptr |word ptr |byte ptr )?(\[e[bs]p[^\]]*\]|e[a-ds][xip]|e[sd]i), 0$', t)
    if m: return m.group(1)
    return None


def is_branch(t): return t.startswith('j') or t.startswith('ret') or t.startswith('call')


def targets(ins): return {x[3] for x in ins if x[3] is not None}


LOW = {'eax': ('al', 'ax'), 'ebx': ('bl', 'bx'), 'ecx': ('cl', 'cx'), 'edx': ('dl', 'dx'), 'esi': ('si',), 'edi': ('di',)}


def zd(ins, k):
    """zeroed destination of ins[k], ignoring the `xor r,r; mov rl,...` zero-extension idiom."""
    d = zdest(ins[k][1])
    if d in LOW:
        for j in range(k + 1, min(k + 4, len(ins))):
            if re.match(r'mov (%s), ' % '|'.join(LOW[d]), ins[j][1]): return None
    return d


def back_run(ins, i, tg, guard_past=None):
    """zero dests in the straight-line run ending just before index i (exclusive).
    guard_past: offset; a conditional jump whose target lies beyond it (the rotated loop's
    entry guard) does not end the run."""
    z = set(); k = i - 1; n = 0
    while k >= 0 and n < W:
        t = ins[k][1]
        if t.startswith('ret') or t.startswith('jmp'): break
        if t.startswith('j'):
            if guard_past is None or ins[k][3] is None or ins[k][3] <= guard_past: break
            k -= 1; n += 1; continue
        d = zd(ins, k)
        if d: z.add(d)
        if ins[k][0] in tg and guard_past is None: break  # block start
        k -= 1; n += 1
    return z


def fwd_run(ins, i, tg):
    z = set(); k = i; n = 0
    while k < len(ins) and n < W:
        t = ins[k][1]
        if k > i and ins[k][0] in tg: break
        d = zd(ins, k)
        if d: z.add(d)
        if t.startswith('j') or t.startswith('ret'): break
        k += 1; n += 1
    return z


def classify(ins):
    by = {x[0]: i for i, x in enumerate(ins)}; tg = targets(ins); out = []
    for i, (off, t, sz, tgt) in enumerate(ins):
        if tgt is None or tgt > off or not t.startswith('j'): continue
        h = by.get(tgt)
        if h is None: continue
        latch = back_run(ins, i, tg)
        pre = back_run(ins, h, tg, guard_past=off)
        # top-tested loop entered by jmp: the body top is after the head's test
        top = fwd_run(ins, h, tg)
        if h + 1 < len(ins):
            for k in range(h, min(h + 6, len(ins))):
                if ins[k][1].startswith('j'):
                    top |= fwd_run(ins, k + 1, tg); break
        pl = sorted(d for d in latch & pre if d not in top)
        tp = sorted(d for d in top if d not in latch)
        out.append(dict(head=tgt, tail=off, pl=pl, top=tp))
    return out


def main():
    ap = argparse.ArgumentParser(); ap.add_argument('units', nargs='*'); ap.add_argument('--addr'); ap.add_argument('--src'); ap.add_argument('--all', action='store_true')
    ap.add_argument('--json'); ap.add_argument('--show', action='store_true'); ap.add_argument('--jobs', type=int, default=int(os.environ.get('FE_JOBS', '5')))
    a = ap.parse_args(); base = int(os.environ.get('FE_PFX_BASE', '21'))
    cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
    units = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/')]
    if a.units: units = [u for u in units if u in a.units or os.path.basename(u) in a.units or u[4:] in a.units]
    work = []
    for u in units:
        p = os.path.abspath(a.src) if a.src else R + '/' + u
        try: t = open(p, encoding='latin-1').read()
        except OSError: continue
        ad = [int(a.addr, 16)] if a.addr else markers(t, a.all)
        ad = [x for x in ad if re.search(r'// ORIGINAL: 0x%08X' % x, t)]
        if ad: work.append((u, p, t, ad))
    def lane(k): return [(work[i], fe.compile_unit(work[i][0], work[i][1], base + k)) for i in range(k, len(work), a.jobs)]
    comp = []
    with ThreadPoolExecutor(a.jobs) as ex:
        for o in ex.map(lane, range(a.jobs)): comp += o
    found = []
    for (u, p, t, ad), (c, err) in comp:
        if c is None: print('COMPILE FAIL', u, (err or '')[:200], file=sys.stderr); continue
        for addr in ad:
            r = fe.evaluate(c, t, addr)
            if 'error' in r or (r['exact'] and not a.addr and not a.all): continue
            O = [x[1] for x in r['rows'] if x[1] is not None]; C = [x[2] for x in r['rows'] if x[2] is not None]
            li, lo_ = classify(O), classify(C)
            ipl = sum(len(x['pl']) for x in li); opl = sum(len(x['pl']) for x in lo_)
            itp = sum(len(x['top']) for x in li); otp = sum(len(x['top']) for x in lo_)
            if ipl == opl and itp == otp and not a.addr: continue
            sig = ipl > opl
            found.append(dict(unit=u, addr=addr, sym=r['sym'], diff=r['diff'], img=r['img'], sig=sig,
                              ipl=ipl, opl=opl, itp=itp, otp=otp,
                              img_loops=[dict(head='%x' % (r['lo'] + x['head']), tail='%x' % (r['lo'] + x['tail']), pl=x['pl'], top=x['top']) for x in li if x['pl'] or x['top']],
                              our_loops=[dict(head='+%x' % x['head'], tail='+%x' % x['tail'], pl=x['pl'], top=x['top']) for x in lo_ if x['pl'] or x['top']]))
    found.sort(key=lambda f: (not f['sig'], -(f['ipl'] - f['opl']), -f['diff']))
    for f in found:
        print('%08X %-22s %-40s diff=%d/%d PL img %d ours %d | TOP img %d ours %d%s' % (f['addr'], f['unit'][4:], f['sym'][:40], f['diff'], f['img'], f['ipl'], f['opl'], f['itp'], f['otp'], ' SIG' if f['sig'] else ''))
        if a.show:
            for x in f['img_loops']: print('    img  %s..%s PL=%s TOP=%s' % (x['head'], x['tail'], x['pl'], x['top']))
            for x in f['our_loops']: print('    ours %s..%s PL=%s TOP=%s' % (x['head'], x['tail'], x['pl'], x['top']))
    print('functions', len(found), 'sig', sum(f['sig'] for f in found))
    if a.json: json.dump(found, open(a.json, 'w'), indent=0)


if __name__ == '__main__': main()
