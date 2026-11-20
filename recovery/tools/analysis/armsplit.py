#!/usr/bin/env python3
"""armsplit.py splitvar|armcopy [unit ...] [--addr 0xADDR] [--src ALT.cpp] [--all] [--json FILE] [--show]
Two layout/web screens from config_game (MATCHING_GUIDE 3.7, par-5 round 54).

splitvar  "a variable held in a register in one region and read from a frame slot in another,
          joined by one copy store, is two source variables (`b = pick`)".  VC6 gives a variable
          one home (a register or a frame slot) for its whole web; it does not split a web.  So a
          store of a callee-saved register into a frame slot that is read elsewhere (`mov [ebp-N],
          ebx` ... `cmp [ebp-N], 0`) is an assignment between two variables.  Per side, the copy
          stores of ebx/esi/edi (and of ebp in a frameless function) into a frame slot that the
          function also reads; a function is listed when the image has more of them than ours
          (SIG: ours keeps one web where the source had two) or fewer (ours copies, the image keeps
          one web).  Spills around a call do not exist in VC6, so the noise is real `x = y`
          statements that the two sides allocate differently.

armcopy   "a merge/reset block that the image lays straight after one arm while other arms jump
          back to it was written in each arm and cross-jumped".  Per side, every block entered both
          by falling in and by a backward `jmp` (a jump from a higher address) that is not a loop
          head of the same shape on the other side; blocks are matched across the sides by their
          register- and slot-normalized first instructions.  Listed when the image has such a
          block and ours' copy of it is reached by forward jumps only (MISSING: ours lays the join
          after the whole chain, write the block in each arm) or falls in from a different
          predecessor (FALLIN: another arm is the fall-in; the copy in the image's fall-in arm
          comes first in source order, or that arm is the one VC6 kept).
Screen only; one fe.py compile per unit.  Confirm by hand and with fe.py.
Run from the worktree root:  FE_PFX_BASE=27 FE_JOBS=5 uv run python recovery/tools/analysis/armsplit.py splitvar"""
import sys, os, re, json, argparse
from collections import Counter
from concurrent.futures import ThreadPoolExecutor
S = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, S); sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import fe
from loopscan import markers
R = fe.R

SLOT = re.compile(r'\[(?:ebp|esp)(?: [-+] (?:0x[0-9a-f]+|\d+))?\]')
REG = re.compile(r'\b(?:e[a-d]x|e[sd]i|ebp|[a-d][lhx]|si|di)\b')


def norm(t):
    t = SLOT.sub('[S]', t)
    return REG.sub('R', t)


# ---------------------------------------------------------------- splitvar
def frameless(ins):
    return not any(t in ('push ebp',) for _, t, _, _ in ins[:3]) or not any(t == 'mov ebp, esp' for _, t, _, _ in ins[:4])


def copies(ins):
    """[(k, slot, reg)]: `mov dword ptr [frame slot], callee-saved reg` where the slot is also read."""
    regs = {'ebx', 'esi', 'edi'}
    if frameless(ins): regs.add('ebp')
    out = []
    reads = Counter()
    for k, (off, t, sz, tgt) in enumerate(ins):
        m = re.match(r'(\w+) (.*)$', t)
        if not m: continue
        mn, ops = m.groups()
        parts = [p.strip() for p in ops.split(',')]
        for i, p in enumerate(parts):
            for s in SLOT.findall(p):
                # destination of a plain mov/lea is not a read
                if i == 0 and mn in ('mov', 'lea'): continue
                reads[s] += 1
    for k, (off, t, sz, tgt) in enumerate(ins):
        m = re.match(r'mov dword ptr (\[ebp - 0x[0-9a-f]+\]|\[esp \+ 0x[0-9a-f]+\]), (\w+)$', t)
        if not m or m.group(2) not in regs: continue
        s = m.group(1)
        if s.startswith('[esp'):
            continue  # esp offsets move with pushes; frame slots only
        if reads[s] == 0: continue
        out.append((k, s, m.group(2)))
    return out


def splitvar(r, O, C):
    """copy stores of one side whose aligned row on the other side is not a copy store."""
    ci, co = copies(O), copies(C)
    si = {O[k][0] for k, _, _ in ci}; so = {C[k][0] for k, _, _ in co}
    ui, uo = [], []
    st = re.compile(r'mov dword ptr \[e[bs]p[^\]]*\], e(?:[sd]i|bx|bp)$')
    for tag, o, c in r['rows']:
        io = o is not None and o[0] in si; ic = c is not None and c[0] in so
        # the other side stores a callee-saved register into a slot too (another slot, or one it
        # does not read back): a renumbered copy, not a missing one
        if io and c is not None and st.match(c[1]): continue
        if ic and o is not None and st.match(o[1]): continue
        if io and not ic: ui.append('%x %s   | ours %s' % (r['lo'] + o[0], o[1], c[1] if c else '-'))
        if ic and not io: uo.append('+%x %s   | img %s' % (c[0], c[1], o[1] if o else '-'))
    if not ui and not uo: return None
    return dict(sig=len(ui) > len(uo), img_n=len(ui), our_n=len(uo), img=ui, ours=uo)


# ---------------------------------------------------------------- armcopy
def blocks(ins):
    by = {x[0]: i for i, x in enumerate(ins)}
    lead = {0}
    for i, (off, t, sz, tgt) in enumerate(ins):
        if tgt is not None and tgt in by: lead.add(by[tgt])
        if (t.startswith('j') or t.startswith('ret')) and i + 1 < len(ins): lead.add(i + 1)
    lead = sorted(lead)
    bl = []
    for n, a in enumerate(lead):
        e = lead[n + 1] if n + 1 < len(lead) else len(ins)
        bl.append((a, e))
    preds = {a: [] for a, _ in bl}
    for n, (a, e) in enumerate(bl):
        last = ins[e - 1][1]
        if n + 1 < len(bl) and not (last.startswith('jmp') or last.startswith('ret')):
            preds[bl[n + 1][0]].append(('F', e - 1))
        for k in range(a, e):
            tgt = ins[k][3]
            if tgt is not None and tgt in by and ins[k][1].startswith('j'):
                b = by[tgt]
                if b in preds: preds[b].append(('B' if k > b else 'J', k))
    # A backward entry whose jumping block is reachable from the target by forward edges only
    # closes a loop ('L'); otherwise it is a cross-jump into an earlier copy ('B').
    bi = {a: n for n, (a, _) in enumerate(bl)}
    blk = {}
    for n, (a, e) in enumerate(bl):
        for k in range(a, e): blk[k] = n
    succ = [[] for _ in bl]
    for n, (a, e) in enumerate(bl):
        last = ins[e - 1]
        if n + 1 < len(bl) and not (last[1].startswith('jmp') or last[1].startswith('ret')): succ[n].append(n + 1)
        if last[3] is not None and last[3] in by and last[1].startswith('j') and by[last[3]] in bi:
            m = bi[by[last[3]]]
            if m > n: succ[n].append(m)
    def fwd_reach(n):
        seen = {n}; st = [n]
        while st:
            x = st.pop()
            for y in succ[x]:
                if y not in seen: seen.add(y); st.append(y)
        return seen
    for a in preds:
        if any(kind == 'B' for kind, _ in preds[a]):
            fr = fwd_reach(bi[a])
            preds[a] = [('L' if kind == 'B' and blk[k] in fr else kind, k) for kind, k in preds[a]]
    return bl, preds


def bsig(ins, a, e, n=4):
    body = [norm(x[1]) for x in ins[a:e] if not x[1].startswith('j')]
    # an epilogue (a merged `return K`) is a return merge, not an arm copy
    if any(x.startswith('ret') for x in body[:6]) or (body and body[0].startswith(('pop ', 'mov esp'))): return None
    if len(body) < 2 and not any(x.startswith('call') for x in body): return None
    return tuple(body[:n])


def tail(ins, k, n=3):
    """normalized last n non-jump instructions ending at index k (the fall-in predecessor)."""
    out = []
    while k >= 0 and len(out) < n:
        if not ins[k][1].startswith('j'): out.append(norm(ins[k][1]))
        k -= 1
    return tuple(reversed(out))


def joins(ins):
    """{sig: [dict(idx, fall, back, fwd, ftail)]} for every block with a signature."""
    bl, preds = blocks(ins)
    out = {}
    for a, e in bl:
        s = bsig(ins, a, e)
        if s is None: continue
        p = preds[a]
        f = [k for kind, k in p if kind == 'F']
        out.setdefault(s, []).append(dict(idx=a, fall=bool(f), back=sum(1 for kind, _ in p if kind == 'B'),
                                          fwd=sum(1 for kind, _ in p if kind in 'JL'), ftail=tail(ins, f[0]) if f else None))
    return out


def armcopy(r, O, C):
    ji, jo = joins(O), joins(C)
    hits = []
    for s, lst in ji.items():
        cand = [x for x in lst if x['fall'] and x['back']]
        if not cand: continue
        ol = jo.get(s, [])
        # loop heads: same signature with a backward entry in ours too and the same fall-in tail
        for x in cand:
            same = [y for y in ol if y['back'] and y['fall'] and y['ftail'] == x['ftail']]
            if same: continue
            backs = [y for y in ol if y['back']]
            if not ol:
                continue  # no block of that shape in ours: not comparable
            kind = 'FALLIN' if backs else 'MISSING'
            hits.append(dict(kind=kind, at='%x' % (r['lo'] + O[x['idx']][0]), sig=list(s), back=x['back'], fwd=x['fwd'],
                             img_fall=list(x['ftail'] or ()),
                             ours=[dict(at='+%x' % C[y['idx']][0], fall=list(y['ftail'] or ()), back=y['back'], fwd=y['fwd']) for y in ol]))
    if not hits: return None
    return dict(sig=any(h['kind'] == 'MISSING' for h in hits), hits=hits)


SCREENS = dict(splitvar=splitvar, armcopy=armcopy)


def main():
    ap = argparse.ArgumentParser(); ap.add_argument('screen', choices=sorted(SCREENS)); ap.add_argument('units', nargs='*')
    ap.add_argument('--addr'); ap.add_argument('--src'); ap.add_argument('--all', action='store_true')
    ap.add_argument('--json'); ap.add_argument('--show', action='store_true'); ap.add_argument('--jobs', type=int, default=int(os.environ.get('FE_JOBS', '5')))
    a = ap.parse_args(); base = int(os.environ.get('FE_PFX_BASE', '21')); fn = SCREENS[a.screen]
    cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
    units = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/')]
    if a.units: units = [u for u in units if u in a.units or os.path.basename(u) in a.units or u[4:] in a.units]
    work = []
    for u in units:
        p = os.path.abspath(a.src) if a.src else R + '/' + u
        try: t = open(p, encoding='latin-1').read()
        except OSError: continue
        ad = [int(a.addr, 16)] if a.addr else markers(t, True)
        ad = [x for x in ad if re.search(r'// ORIGINAL: 0x%08X' % x, t)]
        if ad: work.append((u, p, t, ad))
    if work: fe.compile_cmd(work[0][0])  # load compile_commands once, before the lanes race on it
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
            h = fn(r, O, C)
            if h is None: continue
            h.update(unit=u, addr=addr, sym=r['sym'], diff=r['diff'], img_len=r['img'])
            found.append(h)
    found.sort(key=lambda f: (not f['sig'], -f['diff']))
    for f in found:
        if a.screen == 'splitvar':
            print('%08X %-20s %-40s diff=%d/%d copies img %d ours %d%s' % (f['addr'], f['unit'][4:], f['sym'][:40], f['diff'], f['img_len'], f['img_n'], f['our_n'], ' SIG' if f['sig'] else ''))
            if a.show:
                for x in f['img']: print('    img  ' + x)
                for x in f['ours']: print('    ours ' + x)
        else:
            print('%08X %-20s %-40s diff=%d/%d %s' % (f['addr'], f['unit'][4:], f['sym'][:40], f['diff'], f['img_len'], ' '.join('%s@%s' % (h['kind'], h['at']) for h in f['hits'])))
            if a.show:
                for h in f['hits']:
                    print('    %s img %s back=%d fwd=%d fall-in %s' % (h['kind'], h['at'], h['back'], h['fwd'], ' ; '.join(h['img_fall'])))
                    print('        block %s' % ' ; '.join(h['sig']))
                    for y in h['ours']: print('        ours %s back=%d fwd=%d fall-in %s' % (y['at'], y['back'], y['fwd'], ' ; '.join(y['fall']) or '-'))
    print('functions', len(found), 'sig', sum(f['sig'] for f in found))
    if a.json: json.dump(found, open(a.json, 'w'), indent=0)


if __name__ == '__main__': main()
