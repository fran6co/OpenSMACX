#!/usr/bin/env python3
"""ctrscan.py <unit> <0xADDR[,..]> [--src ALT.cpp] [--pfx N] [-v] | --tree [unit ..] [--from FEALL.json] [--all] [--json F]

Loop-counter and hoisted-global screen (MATCHING_GUIDE 2.7 G1-G3). For every natural loop of both sides
(candset.Code CFG; the image's loops are paired with ours through the fe alignment of their headers):
  counter   the register stepped by a constant inside the loop (`inc/dec/add/sub r,K`, also `lea r,[r+K]`) and
            tested in it, classified `S:own` (scratch register, its own `xor`/`mov K` before the loop),
            `C:own` (callee-saved, own initialisation), `C:zero` (callee-saved, the initial `xor` also feeds
            zero stores before the loop: the G2 zero web), `S:zero`, or `M` (stepped in memory);
  hoisted   registers loaded from an absolute global (`mov r,[<r>]`, any width) outside the loop and read
            inside it with no redefinition there (G3), and the per-use reads `[<r>]` inside the loop;
  scratch   (call-free loops) the scratch registers live across the back edge (held over the whole loop) and
            what defines each one, i.e. the values that keep a scratch register busy (G1).
A loop is reported when the counter class differs (S against C, own against zero) or the hoisted-register count
differs. Tree mode prints one block per such function, ranked by fe rows. Screen only: confirm with fe.py."""
import sys, os, re, json, argparse, collections
from concurrent.futures import ThreadPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
sys.path.insert(0, HERE)
import fe
import tailscan
import candset as C
from rotscan import own_context

STEP = re.compile(r'(inc|dec) (e\w\w)$|(add|sub) (e\w\w), (0x[0-9a-f]+|\d+)$|lea (e\w\w), \[(e\w\w) [-+] (0x[0-9a-f]+|\d+)\]$')
GLOB = re.compile(r'(mov|movsx|movzx) (e\w\w), (?:byte |word |dword )?ptr \[<r>\]$')
REGS = C.CS[:3] + C.SCR + ('ebp',)
STORE = re.compile(r'(mov|add|sub|or|and|xor|inc|dec|neg|not|shl|shr|sar) (byte|word|dword) ptr \[')
GREADS = os.environ.get('CTR_GREADS') == '1'


def loops_of(code):
    """-> [(header block, body block set)] like candset.Code, kept"""
    nb = len(code.starts)
    n = len(code.ins)
    succ = [set() for _ in range(nb)]
    for b in range(nb if n else 0):
        e = (code.starts[b + 1] if b + 1 < nb else n) - 1
        for j in code.isucc[e]:
            succ[b].add(code.bof[j])
    pred = [set() for _ in range(nb)]
    for u in range(nb):
        for v in succ[u]:
            pred[v].add(u)
    full = set(range(nb))
    dom = [full] * nb
    if nb:
        dom[0] = {0}
    ch = True
    while ch:
        ch = False
        for v in range(1, nb):
            ps = [dom[p] for p in pred[v]]
            nd = (set.intersection(*ps) if ps else set()) | {v}
            if nd != dom[v]:
                dom[v] = nd
                ch = True
    loops = collections.defaultdict(set)
    latches = collections.defaultdict(set)
    for u in range(nb):
        for h in succ[u]:
            if h in dom[u]:
                body = {h, u}
                st = [u] if u != h else []
                while st:
                    x = st.pop()
                    for p in pred[x]:
                        if p not in body:
                            body.add(p)
                            st.append(p)
                loops[h] |= body
                latches[h].add(u)
    return [(h, b, latches[h]) for h, b in sorted(loops.items())]


def binsts(code, blocks):
    n = len(code.ins)
    out = []
    for b in sorted(blocks):
        e = code.starts[b + 1] if b + 1 < len(code.starts) else n
        out += range(code.starts[b], e)
    return out


def analyse(code, lmap=None):
    """-> list of loop dicts"""
    webs = {r: code.webs(r) for r in REGS}
    res = []
    for h, body, latch in loops_of(code):
        I = set(binsts(code, body))
        callfree = not any(C.mnem(code.ins[i][1]) == 'call' for i in I)
        d = dict(head=code.ins[code.starts[h]][0], n=len(I), callfree=callfree, depth=code.depth[h])
        # counter
        ctr = None
        for i in sorted(I):
            t = code.ins[i][1]
            m = STEP.match(t)
            if not m:
                if re.match(r'(add|sub|inc|dec) dword ptr \[e[bs]p [-+] [^\]]+\](, (0x[0-9a-f]+|\d+))?$', t):
                    ctr = ctr or dict(cls='M', reg=t.split(' ', 1)[1].split(',')[0], at=code.ins[i][0])
                continue
            r = m.group(2) or m.group(4) or m.group(6)
            if m.group(6) and m.group(7) != r:
                continue
            if r == 'esp' or (r == 'ebp' and getattr(code, 'frame_ebp', False)):
                continue
            tested = any(re.match(r'(cmp|test) %s\b|cmp \S+, %s$' % (r, r), code.ins[j][1]) for j in I) or \
                (i + 1 < len(code.ins) and code.ins[i + 1][1].startswith('j') and code.ins[i + 1][1] not in ('jmp L',))
            if not tested:
                continue
            w = next((w for w in webs[r] if i in w['refs']), None)
            if w is None:
                continue
            outdefs = [x for x in w['defs'] if x not in I]
            own = True
            for x in outdefs:
                # the initial value also feeds stores/compares outside the loop before it: a shared (zero) web
                if any(u not in I and u != x and re.match(r'(mov \S+ ptr \[[^\]]+\], %s|cmp \S+ ptr \[[^\]]+\], %s|push %s)$' % (r, r, r),
                                                          code.ins[u][1]) for u in w['uses']):
                    own = False
            init = code.ins[min(outdefs)][1] if outdefs else ''
            k = 'zero' if (not own and init.startswith('xor')) else ('shared' if not own else 'own')
            cand = dict(cls=('S' if r in C.SCR else 'C') + ':' + k, reg=r, at=code.ins[i][0], init=init)
            if ctr is None or ctr['cls'] == 'M':
                ctr = cand
        d['ctr'] = ctr
        # hoisted globals and per-use global reads
        hoist = []
        for r in REGS:
            for w in webs[r]:
                if w['defs'] and all(x not in I for x in w['defs']) and any(u in I for u in w['uses']):
                    if any(GLOB.match(code.ins[x][1]) for x in w['defs']):
                        hoist.append((r, code.ins[min(w['defs'])][0]))
        d['hoist'] = hoist
        d['greads'] = sum(1 for i in I if '[<r>]' in code.ins[i][1] and not code.ins[i][1].startswith('call'))
        # in-loop stores to memory other than the frame (global or through a pointer): what may alias a hoist
        d['stores'] = sum(1 for i in I if STORE.match(code.ins[i][1]) and not re.match(r'\w+ \w+ ptr \[e[bs]p', code.ins[i][1]))
        # scratch registers held across the back edge (live into the header from a latch) in a call-free loop
        held = []
        if callfree:
            hi = code.starts[h]
            for r in C.SCR:
                for w in webs[r]:
                    if hi in w['live'] and any(code.bof[x] in latch for x in w['live']):
                        d0 = min(w['defs'])
                        held.append((r, code.ins[d0][0], code.ins[d0][1], lmap.get(code.ins[d0][0]) if lmap else None))
        d['held'] = held
        res.append(d)
    return res


def scan(unit, addr, text, c, cod):
    r = fe.evaluate(c, text, addr)
    if 'error' in r:
        return dict(addr=addr, error=r['error'])
    rows = r['rows']
    lmap = tailscan.lines_of(cod, r['sym'], os.path.basename(unit).lower()) if cod else {}
    iins = [(x[0], x[1], x[3]) for k, x, cc in rows if x is not None]
    oins = [(x[0], x[1], x[3]) for k, o, x in rows if x is not None]
    if not iins or not oins:
        return dict(addr=addr, error='empty')
    icode = C.Code(iins, C.image_tables(r['lo'], r['lo'] + max(o[0] for o in iins) + 16, rows))
    ocode = C.Code(oins, {})
    IL, OL = analyse(icode), analyse(ocode, lmap)
    # image offset -> our offset through aligned rows
    i2o = {}
    last = None
    for k, o, x in rows:
        if o is not None and x is not None:
            last = x[0]
        if o is not None:
            i2o[o[0]] = x[0] if x is not None else last
    found = []
    used = set()
    for L in IL:
        oo = i2o.get(L['head'])
        cand = [M for M in OL if id(M) not in used]
        if oo is None or not cand:
            found.append((L, None))
            continue
        M = min(cand, key=lambda M: (abs(M['head'] - oo), abs(M['n'] - L['n'])))
        if abs(M['head'] - oo) > max(24, L['n'] * 3):
            found.append((L, None))
            continue
        used.add(id(M))
        found.append((L, M))
    flagged = []
    for L, M in found:
        if M is None:
            continue
        ci, co = (L['ctr'] or {}).get('cls'), (M['ctr'] or {}).get('cls')
        why = []
        if ci and co and ci != co:
            why.append('counter %s/%s' % (ci, co))
        if len(L['hoist']) != len(M['hoist']):
            why.append('hoist %d/%d' % (len(L['hoist']), len(M['hoist'])))
        if GREADS and L['greads'] != M['greads'] and abs(L['n'] - M['n']) <= max(3, L['n'] // 8):
            why.append('greads %d/%d stores %d/%d' % (L['greads'], M['greads'], L['stores'], M['stores']))
        if why:
            flagged.append(dict(why=why, img=L, ours=M))
    return dict(addr=addr, unit=unit, sym=r['sym'], diff=r['diff'], exact=r['exact'], lo=r['lo'], loops=len(IL),
                oloops=len(OL), flagged=flagged)


def fmt_side(tag, L, lo, ours):
    c = L['ctr']
    cs = ('%s %s@%x init[%s]' % (c['cls'], c['reg'], (c['at'] if ours else lo + c['at']), c.get('init', ''))) if c else '-'
    s = '    %s head %x n=%d %s ctr %s | hoist %s greads %d' % (
        tag, L['head'] if ours else lo + L['head'], L['n'], 'callfree' if L['callfree'] else 'calls', cs,
        ','.join('%s@%x' % (r, a if ours else lo + a) for r, a in L['hoist']) or '-', L['greads'])
    if L['callfree'] and L['held']:
        s += '\n      held over the loop: ' + '; '.join('%s<-%s%s' % (r, t, ' %s' % ln if ln else '') for r, a, t, ln in L['held'])
    return s


def report(x):
    if 'error' in x:
        return ['ERR %08X %s' % (x['addr'], x['error'])]
    if not x['flagged']:
        return []
    out = ['FN %s %08X %s rows=%d loops %d/%d' % (x['unit'][4:], x['addr'], x['sym'][:60], x['diff'], x['loops'], x['oloops'])]
    for f in x['flagged']:
        out.append('  LOOP %s' % ', '.join(f['why']))
        out.append(fmt_side('img ', f['img'], x['lo'], False))
        out.append(fmt_side('ours', f['ours'], x['lo'], True))
    return out


def run_unit(unit, addrs, src, pfx):
    text = open(src, encoding='latin-1').read()
    c, cod = tailscan.compile_listing(unit, src, pfx)
    if c is None:
        return [dict(addr=a, error='COMPILE FAIL') for a in addrs]
    out = []
    for a in addrs:
        try:
            out.append(scan(unit, a, text, c, cod))
        except Exception as e:
            out.append(dict(addr=a, error=repr(e)))
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('unit', nargs='?')
    ap.add_argument('addr', nargs='?')
    ap.add_argument('--src')
    ap.add_argument('--pfx', default=os.environ.get('FE_PFX_BASE', '8'))
    ap.add_argument('--tree', nargs='*')
    ap.add_argument('--from', dest='frm')
    ap.add_argument('--all', action='store_true', help='include BYTE_EXACT markers (validation: they should not flag)')
    ap.add_argument('--json')
    a = ap.parse_args()
    if a.tree is not None or a.frm:
        work = {}
        if a.frm:
            for r in json.load(open(a.frm)):
                if not r.get('exact'):
                    work.setdefault(r['unit'] if r['unit'].startswith('src/') else 'src/' + r['unit'], []).append(int(str(r['addr']), 16))
        else:
            cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
            units = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/')]
            if a.tree:
                units = [u for u in units if u in a.tree or os.path.basename(u) in a.tree or u[4:] in a.tree]
            for u in units:
                try:
                    t = open(fe.R + '/' + u, encoding='latin-1').read()
                except OSError:
                    continue
                ad = [x for x in tailscan.markers(t, a.all) if own_context(t, x, u)]
                if ad:
                    work[u] = ad
        work = sorted(work.items())
        jobs = int(os.environ.get('FE_JOBS', '5'))
        fe.compile_cmd(work[0][0])
        base = int(a.pfx)

        def lane(k):
            r = []
            for i in range(k, len(work), jobs):
                u, ad = work[i]
                r += run_unit(u, ad, fe.R + '/' + u, str(base + k))
            return r
        allr = []
        with ThreadPoolExecutor(jobs) as ex:
            for o in ex.map(lane, range(jobs)):
                allr += o
        if not a.all:
            allr = [x for x in allr if 'error' in x or not x['exact']]
        allr.sort(key=lambda x: -x.get('diff', 0))
        for x in allr:
            for l in report(x):
                print(l)
        fl = [x for x in allr if x.get('flagged')]
        print('functions %d, with a flagged loop %d (exact among them %d), loops flagged %d' % (
            len(allr), len(fl), sum(1 for x in fl if x['exact']), sum(len(x['flagged']) for x in fl)))
        if a.json:
            json.dump(allr, open(a.json, 'w'), indent=0, default=str)
        return
    unit = a.unit if a.unit.startswith('src/') else 'src/' + a.unit
    src = os.path.abspath(a.src) if a.src else fe.R + '/' + unit
    for r in run_unit(unit, [int(x, 16) for x in a.addr.split(',')], src, a.pfx):
        ls = report(r)
        print('\n'.join(ls) if ls else 'FN %08X no flagged loop' % r['addr'])


if __name__ == '__main__':
    main()
