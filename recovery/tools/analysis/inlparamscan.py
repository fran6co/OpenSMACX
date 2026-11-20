#!/usr/bin/env python3
"""inlparamscan.py <unit> <0xADDR[,..]> [--src ALT.cpp] | --from FEALL.json [--json F] [--min N] [--sites]
Inline-parameter home screen (MATCHING_GUIDE 3.3 "A never-reloaded store of a value whose reads all come
from a register is an inline function's parameter", BaseWin::draw_expenses 0x00412AA0).
VC6 homes an expanded inline's parameters: it stores each argument to a frame slot at the binding even
when every read in the body is served from the register that computed it.  A named local or a value
computed in place is copy-propagated and leaves no store.  Per function, on both sides:
  dead  a store `mov [ebp+-N], reg|imm` that no path reads before the slot is overwritten or the function
        returns (flow-sensitive liveness over the frame bytes; address-taken aggregates are read by calls,
        the EH state/registration slots are skipped)
  used  `dead` whose stored register is read again before it is redefined (value served from the register:
        the draw_expenses sign); `imm` an immediate store
  tern  a slot stored on two converging paths and read once right after the join (`mov [m],K; jcc; mov
        [m],eax` ... `mov r,[m]`: a ?:/min/max result bound to an inline parameter)
Image events are paired with ours through fe's row alignment (an event matches an event of the same kind
within +-WIN rows).  IMG-only: the original passed that value through an inline's parameter where ours
computes it in place (the line shown is our nearest source line).  OUR-only: ours expands an inline (the
header line shown) the image does not, or binds a value the image keeps in a register.  Screen only:
read each site with fe.py --all; confirm with recover measure."""
import sys, os, re, argparse, json
from concurrent.futures import ThreadPoolExecutor
H = os.path.dirname(os.path.abspath(__file__))
T = os.path.dirname(H)
sys.path.insert(0, T)
sys.path.insert(0, H)
import fe
import tailscan

WIN = int(os.environ.get('IP_WIN', '4'))
AGG = int(os.environ.get('IP_AGG', '0x10'), 0)  # extent of an address-taken frame aggregate read by calls
MEM = re.compile(r'(?:(byte|word|dword|qword) ptr )?\[ebp(?: ([+-]) (0x[0-9a-f]+|\d+))?\]')
WID = {'byte': 1, 'word': 2, 'dword': 4, 'qword': 8, None: 4}
REGS = {'eax': ('eax', 'ax', 'al', 'ah'), 'ebx': ('ebx', 'bx', 'bl', 'bh'), 'ecx': ('ecx', 'cx', 'cl', 'ch'),
        'edx': ('edx', 'dx', 'dl', 'dh'), 'esi': ('esi', 'si'), 'edi': ('edi', 'di')}
SUB = {s: r for r, ss in REGS.items() for s in ss}
NOWRITE = ('cmp', 'test', 'push')
PURE = ('mov', 'movsx', 'movzx', 'lea', 'setl', 'setg', 'sete', 'setne', 'setle', 'setge', 'seta', 'setb',
        'setae', 'setbe', 'pop')


def acc(op):
    m = MEM.search(op)
    if not m:
        return None
    v = int(m.group(3) or '0', 0)
    d = -v if m.group(2) == '-' else v
    w = WID[m.group(1)]
    return d, w


def parse(ins):
    """ins [(off, text, target)] -> per instruction (mnemonic, reads{bytes}, writes{bytes} full, lea base)"""
    out = []
    for off, t, tg in ins:
        mn, _, ops = t.partition(' ')
        parts = [p.strip() for p in ops.split(', ')] if ops else []
        rd, wr, lea = set(), set(), None
        if mn == 'lea':
            a = acc(parts[1]) if len(parts) > 1 else None
            if a:
                lea = a[0]
        else:
            for k, p in enumerate(parts):
                a = acc(p)
                if not a:
                    continue
                rng = set(range(a[0], a[0] + a[1]))
                if k == 0 and mn in PURE:
                    wr |= rng
                elif k == 0 and mn not in NOWRITE and len(parts) > 1:
                    rd |= rng
                    wr |= rng
                else:
                    rd |= rng
            if mn.startswith('fst') or mn.startswith('fist'):
                # x87 stores: operand 0 written, not read
                wr |= rd
                rd = set()
        out.append((off, t, tg, mn, parts, rd, wr, lea))
    return out


def liveness(P):
    n = len(P)
    idx = {P[i][0]: i for i in range(n)}
    eh = any('fs:[0]' in p[1] for p in P)
    leas = sorted(set(p[7] for p in P if p[7] is not None))
    escaped = set()
    for k, b in enumerate(leas):
        top = leas[k + 1] if k + 1 < len(leas) else b + AGG
        escaped |= set(range(b, min(top, b + AGG)))
    if eh:
        escaped |= set(range(-0xc, 0))
    succ = []
    for i, (off, t, tg, mn, *_r) in enumerate(P):
        s = []
        if mn == 'ret' or mn == 'jmp' and tg is None:
            pass
        elif mn == 'jmp':
            s.append(idx.get(tg))
        else:
            if i + 1 < n:
                s.append(i + 1)
            if tg is not None and mn.startswith('j'):
                s.append(idx.get(tg))
        succ.append([x for x in s if x is not None])
    gen, kill = [], []
    for i, p in enumerate(P):
        rd, wr = set(p[5]), set(p[6])
        if p[3] == 'call':
            rd |= escaped
        gen.append(frozenset(rd))
        kill.append(frozenset(wr - rd))
    live_in = [frozenset()] * n
    live_out = [frozenset()] * n
    changed = True
    while changed:
        changed = False
        for i in range(n - 1, -1, -1):
            o = frozenset().union(*[live_in[s] for s in succ[i]]) if succ[i] else frozenset()
            ni = gen[i] | (o - kill[i])
            if o != live_out[i] or ni != live_in[i]:
                live_out[i], live_in[i] = o, ni
                changed = True
    return live_out, escaped, eh, succ


def reg_used_after(P, i, reg, succ):
    """the stored register is read again before it is redefined (straight-line, through fall-throughs)"""
    fam = REGS.get(SUB.get(reg, ''), ())
    j = i + 1
    seen = 0
    while j < len(P) and seen < 40:
        off, t, tg, mn, parts, *_r = P[j]
        if mn in ('call', 'ret'):
            return False
        toks = re.findall(r'[a-z]+', ' '.join(parts))
        dst = parts[0] if parts else ''
        srcs = ' '.join(parts[1:]) if mn in PURE else ' '.join(parts)
        if any(x in fam for x in re.findall(r'[a-z]+', srcs)) or '[' in dst and any(x in fam for x in re.findall(r'[a-z]+', dst)):
            return True
        if dst in fam and mn not in NOWRITE:
            return False
        if mn == 'jmp':
            if not succ[j]:
                return False
            j = succ[j][0]
        else:
            j += 1
        seen += 1
    return False


def events(ins):
    P = parse(ins)
    if not P or not any(p[1] == 'mov ebp, esp' for p in P[:6]):
        return []  # no ebp frame (ebp is a general register)
    live_out, escaped, eh, succ = liveness(P)
    preds = {}
    for i, s in enumerate(succ):
        for x in s:
            preds.setdefault(x, []).append(i)
    ev = []
    for i, (off, t, tg, mn, parts, rd, wr, lea) in enumerate(P):
        if mn != 'mov' or not wr or (eh and wr & set(range(-0xc, 0))):
            continue
        if wr & live_out[i]:
            continue
        src = parts[1]
        if src in SUB:
            kind = 'used' if reg_used_after(P, i, src, succ) else 'dead'
        elif re.fullmatch(r'-?(0x[0-9a-f]+|\d+)', src):
            kind = 'imm'
        else:
            continue
        ev.append((kind, i, min(wr)))
    # tern: two stores of one slot reaching one read; the read is the first use after a join
    stores = {}
    for i, p in enumerate(P):
        if p[3] == 'mov' and p[6] and not (p[6] & escaped):
            stores.setdefault(min(p[6]), []).append(i)
    for d, ss in stores.items():
        if len(ss) < 2:
            continue
        for a, b in zip(ss, ss[1:]):
            if b - a > 4 or b + 1 >= len(P):
                continue
            # `mov [m],K; jcc L; mov [m],r; L:` - a select materialised in its home
            if any(P[k][3].startswith('j') and P[k][3] != 'jmp' and P[k][2] == P[b + 1][0] for k in range(a + 1, b)):
                if d in live_out[b]:
                    ev.append(('tern', b, d))
    return ev


def scan(unit, addr, text, c, cod):
    r = fe.evaluate(c, text, addr)
    if 'error' in r:
        return dict(addr=addr, unit=unit, error=r['error'])
    rows = r['rows']
    lo = r['lo']
    lmap = tailscan.lines_of(cod, r['sym'], os.path.basename(unit).lower()) if cod else {}
    iins = sorted((x[0], x[1], x[3]) for k, x, cc in rows if x is not None)
    oins = sorted((x[0], x[1], x[3]) for k, o, x in rows if x is not None)
    ie, oe = events(iins), events(oins)
    # row index of each instruction
    irow, orow = {}, {}
    for n, (k, o, c_) in enumerate(rows):
        if o is not None:
            irow[o[0]] = n
        if c_ is not None:
            orow[c_[0]] = n
    rowc = {}
    for n, (k, o, c_) in enumerate(rows):
        if c_ is not None:
            rowc[n] = c_[0]

    def near_our(n):
        for dd in range(0, 30):
            for m in (n - dd, n + dd):
                if m in rowc:
                    return rowc[m]
        return None
    ikeys = [(k, irow[iins[i][0]], d, i) for k, i, d in ie]
    okeys = [(k, orow[oins[i][0]], d, i) for k, i, d in oe]

    def grp(k):
        return 'st' if k in ('used', 'dead', 'imm') else k
    used_o = set()
    img_only = []
    for k, n, d, i in ikeys:
        m = [j for j, (k2, n2, d2, i2) in enumerate(okeys) if j not in used_o and grp(k2) == grp(k) and abs(n2 - n) <= WIN]
        if m:
            used_o.add(m[0])
        else:
            img_only.append((k, n, d, i))
    our_only = [okeys[j] for j in range(len(okeys)) if j not in used_o]
    isites = []
    for k, n, d, i in img_only:
        oo = near_our(n)
        isites.append('%-4s %08X %-34s ebp%s  ~our %s %s' % (k, lo + iins[i][0], iins[i][1], '%+#x' % d,
                                                         '%x' % oo if oo is not None else '?', lmap.get(oo, '?')))
    osites = ['%-4s %4x %-34s ebp%s  %s' % (k, oins[i][0], oins[i][1], '%+#x' % d, lmap.get(oins[i][0], '?'))
              for k, n, d, i in our_only]
    score = sum(1 for k, *_ in img_only if k in ('used', 'tern')) + sum(1 for k, *_ in our_only if k in ('used', 'tern'))
    return dict(addr=addr, unit=unit, sym=r['sym'], diff=r['diff'], exact=r['exact'],
                img=len(ie), ours=len(oe), img_only=len(img_only), our_only=len(our_only), score=score,
                img_sites=isites, our_sites=osites)


def run_unit(unit, addrs, src, pfx):
    try:
        text = open(src, encoding='latin-1').read()
    except OSError:
        return [dict(addr=a, unit=unit, error='NO SOURCE') for a in addrs]
    c, cod = tailscan.compile_listing(unit, src, pfx)
    if c is None:
        return [dict(addr=a, unit=unit, error='COMPILE FAIL') for a in addrs]
    out = []
    for a in addrs:
        try:
            out.append(scan(unit, a, text, c, cod))
        except Exception as e:
            out.append(dict(addr=a, unit=unit, error=repr(e)))
    return out


def show(x, sites=True):
    if 'error' in x:
        return ['ERR %08X %s %s' % (x['addr'], x['unit'], x['error'])]
    l = ['%08X %-40s %-22s diff %4d  homes img %d ours %d  img-only %d ours-only %d' % (
        x['addr'], x['sym'][:40], x['unit'][4:], x['diff'], x['img'], x['ours'], x['img_only'], x['our_only'])]
    if sites:
        l += ['    IMG ' + s for s in x['img_sites']]
        l += ['    OUR ' + s for s in x['our_sites']]
    return l


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('unit', nargs='?')
    ap.add_argument('addr', nargs='?')
    ap.add_argument('--src')
    ap.add_argument('--pfx', default=os.environ.get('FE_PFX_BASE', '2'))
    ap.add_argument('--from', dest='frm')
    ap.add_argument('--all', action='store_true', help='with --from: include exact functions (validation)')
    ap.add_argument('--min', type=int, default=1)
    ap.add_argument('--sites', action='store_true')
    ap.add_argument('--json')
    a = ap.parse_args()
    if a.frm:
        work = {}
        for r in json.load(open(a.frm)):
            if a.all or not r.get('exact'):
                u = r['unit'] if r['unit'].startswith('src/') else 'src/' + r['unit']
                work.setdefault(u, []).append(int(str(r['addr']), 16))
        work = sorted(work.items())
        jobs = int(os.environ.get('FE_JOBS', '5'))
        fe.compile_cmd(work[0][0])
        base = int(a.pfx)

        def lane(k):
            res = []
            for i in range(k, len(work), jobs):
                u, ad = work[i]
                res += run_unit(u, ad, fe.R + '/' + u, str(base + k))
            return res
        allr = []
        with ThreadPoolExecutor(jobs) as ex:
            for o in ex.map(lane, range(jobs)):
                allr += o
        fl = [x for x in allr if 'error' not in x and x['score'] >= a.min]
        fl.sort(key=lambda x: (-x['score'], x['diff']))
        for x in fl:
            print('\n'.join(show(x, sites=a.sites)))
        print('functions %d, flagged %d, errors %d' % (len(allr), len(fl), sum('error' in x for x in allr)))
        if a.json:
            json.dump(allr, open(a.json, 'w'), indent=0)
        return
    unit = a.unit if a.unit.startswith('src/') else 'src/' + a.unit
    src = os.path.abspath(a.src) if a.src else fe.R + '/' + unit
    for x in run_unit(unit, [int(v, 16) for v in a.addr.split(',')], src, a.pfx):
        print('\n'.join(show(x)))


if __name__ == '__main__':
    main()
