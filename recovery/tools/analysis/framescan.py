#!/usr/bin/env python3
"""framescan.py [unit ...] [--addr A[,A]] [--from FEALL.json] [--all] [--jobs N] [--json OUT] [--slots] [--equal]
Frame-size scan: for every non-exact function (non-BYTE_EXACT markers, or the inexact rows of a feall
json with --from, or every marker with --all), read the local frame size from the prologue of the image
span and of our compile, and print the functions whose sizes differ.

Frame size = N of `sub esp, N` (after `push ebp; mov ebp, esp` and the EH registration pushes when
present), or of `mov eax, N; call __chkstk/_alloca_probe` for large frames.  `push ecx` used as a 4-byte
frame (VC6 for a single slot) counts as N=4.

Per function:  `FRAME <addr> <unit> <sym> img=0x.. ours=0x.. d=+N eh=img/ours ebp=img/ours diff=<fe rows>`
With --slots (default when --addr is given) the slot maps follow: each frame offset either side uses
(ebp-relative, or esp-relative tracked linearly through pushes/pops for /Oy functions, printed as the
offset from the frame top), the access widths seen (1/2/4/8 bytes, `&` = address taken by lea),
and the reference count.  Our slots carry /Fa variable names (`_name$ = -N`) where the listing has them.

The frame is the third rung of the structural order (EH states, calls, frame): a different N names a
wrong set of locals (a missing or extra variable, an aggregate of the wrong size or scope, an
address-taken local that should share a slot, an enregistered variable that lost allocation on one
side, a compiler temporary).  Screen only; confirm with `recover measure`."""
import sys, os, re, json, argparse, collections, subprocess
from concurrent.futures import ThreadPoolExecutor
T = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, T)
import fe
import capstone

md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
SLOT = re.compile(r'(?:(byte|word|dword|qword|tbyte) ptr )?\[(ebp|esp)(?: ([-+]) (0x[0-9a-f]+|\d+))?\]')
WID = {'byte': 1, 'word': 2, 'dword': 4, 'qword': 8, 'tbyte': 10, None: 0}


def insns(data, org, n=None):
    out = []
    for i in md.disasm(data, org):
        out.append((i.address - org, i.mnemonic, i.op_str, i.size))
    return out


def frame(ins):
    """-> (frame size, eh, ebp_frame, index of first body instruction)"""
    k = 0
    ebp = eh = False
    if len(ins) > 1 and ins[0][1:3] == ('push', 'ebp') and ins[1][1:3] == ('mov', 'ebp, esp'):
        ebp = True
        k = 2
    size = 0
    pend = None
    # the prologue: EH registration (push -1 / push handler / fs:[0]) interleaved with the frame
    # allocation (`sub esp, N`, or `mov eax, N` .. `call __chkstk`), up to the first callee-saved push
    while k < len(ins) and k < 14:
        _, mn, op, _ = ins[k]
        if 'fs:[0]' in op or (mn == 'push' and op == '-1' and k <= 2):
            eh = True
        elif mn == 'push' and eh and pend is None and not size and op not in ('ebx', 'esi', 'edi', 'ecx'):
            pass  # handler / previous registration
        elif mn == 'mov' and op.startswith('eax, ') and not size:
            try:
                pend = int(op[5:], 0)
            except ValueError:
                break
        elif mn == 'call' and pend is not None:
            size = pend
            pend = None
        elif mn == 'sub' and op.startswith('esp, ') and not size:
            size = int(op[5:], 0)
        elif mn == 'push' and op == 'ecx' and not size:
            size = 4
        else:
            break
        k += 1
    if pend is not None:  # a `mov eax, N` that was body code
        k -= 1
    return size, eh, ebp, k


def slots(ins, size, ebp, start):
    """-> {offset from frame top (negative = local, positive = parameter): [widths set, addr_taken, refs]}"""
    res = collections.defaultdict(lambda: [set(), False, 0])
    depth = 0  # bytes pushed below the frame (esp frames)
    for off, mn, op, sz in ins[start:]:
        for m in SLOT.finditer(op):
            base = m.group(2)
            d = int(m.group(4), 0) if m.group(4) else 0
            if m.group(3) == '-':
                d = -d
            if base == 'ebp':
                if not ebp:
                    continue
                key = d  # ebp-relative: locals negative, return address +4, params from +8
            else:
                if ebp:
                    continue
                key = d - depth - size  # esp-relative -> offset from the frame top (return address at +0 after pushes)
                # params: ret at frame top + saved regs; keep raw so that both sides compare alike
            e = res[key]
            if mn == 'lea':
                e[1] = True
            else:
                e[0].add(WID[m.group(1)])
            e[2] += 1
        if not ebp:
            if mn == 'push':
                depth += 4
            elif mn == 'pop':
                depth -= 4
            elif mn == 'add' and op.startswith('esp, '):
                try:
                    depth -= int(op[5:], 0)
                except ValueError:
                    pass
            elif mn == 'sub' and op.startswith('esp, '):
                try:
                    depth += int(op[5:], 0)
                except ValueError:
                    pass
            elif mn in ('ret', 'jmp'):
                depth = 0
    return res


def fa_names(unit, src, sym, pfx):
    """/Fa variable offsets for one function of our compile -> {offset: name}"""
    cmd = fe.compile_cmd(unit)
    asm = fe.SCR + '/fs_%d_%s.asm' % (os.getpid(), pfx)
    obj = fe.SCR + '/fs_%d_%s.obj' % (os.getpid(), pfx)
    out = []
    for t in cmd:
        if t.startswith('/Fo'):
            t = '/Fo' + obj
        elif t.startswith('/Fd') or t.startswith('--repro-root') or t == '/W4':
            continue
        elif t == fe.R + '/' + unit:
            t = os.path.abspath(src)
        out.append(t)
    out.insert(-1, '/Fa' + asm)
    env = dict(os.environ, VC6_PREFIX='%s/.opensmacx/wine/worker-%s' % (fe.MAIN, pfx))
    subprocess.run(out, cwd=fe.SCR, env=env, capture_output=True, text=True)
    names = collections.defaultdict(list)
    try:
        t = open(asm, errors='replace').read()
    except OSError:
        return names
    finally:
        os.path.exists(asm) and os.remove(asm)
        os.path.exists(obj) and os.remove(obj)
    i = t.find(sym + ' PROC')
    if i < 0:
        return names
    j = t.rfind('_TEXT\tSEGMENT', 0, i)
    for l in t[j:i].split('\n'):
        m = re.match(r'_?(\S+?)\$(?:\d*) = (-?\d+)', l)
        if m:
            names[int(m.group(2))].append(m.group(1))
    return names


def fmt(e):
    w = '/'.join(str(x) for x in sorted(e[0]) if x) or '-'
    return '%s%s x%d' % ('&' if e[1] else '', w, e[2])


def print_slots(si, so, names, ebp_i, ebp_o):
    keys = sorted(set(si) | set(so))
    for k in keys:
        a = fmt(si[k]) if k in si else ''
        b = fmt(so[k]) if k in so else ''
        nm = ','.join(names.get(k, [])) if ebp_o else ''
        mark = '  ' if (k in si) == (k in so) else ('<-' if k in si else '->')
        print('    %s %6s  img %-16s ours %-16s %s' % (mark, ('-0x%x' % -k) if k < 0 else '+0x%x' % k, a, b, nm))


def scan_unit(unit, addrs, pfx, want_slots, want_equal):
    src = fe.R + '/' + unit
    text = open(src, encoding='latin-1').read()
    c, err = fe.compile_unit(unit, src, pfx)
    if c is None:
        return [('ERR', unit, err)]
    out = []
    ctxc = {}
    c0 = c
    for addr in addrs:
        idn = fe.ident(text, addr)
        if not idn:
            continue
        sym, lo, hi = idn
        cc_ = c0
        # `// compile_context <tu>`: measured in that unit's compile, as fe.py and `recover measure` do
        m = re.search(r'// ORIGINAL: 0x%08X [^\n]*\n((?://(?! ORIGINAL)[^\n]*\n)*)' % addr, text)
        mc = m and re.search(r'// compile_context (\S+)', m.group(1))
        if mc and mc.group(1) != unit:
            if mc.group(1) not in ctxc:
                ctxc[mc.group(1)] = fe.compile_unit(mc.group(1), fe.R + '/' + mc.group(1), pfx)[0]
            if ctxc[mc.group(1)] is not None:
                cc_ = ctxc[mc.group(1)]
        c = cc_
        try:
            s = c.function(sym)
        except Exception:
            continue
        sec = c.sections[s.section - 1]
        I = insns(fe.imgbytes(lo, hi), lo)
        O = insns(sec.data, 0)
        fi = frame(I)
        fo = frame(O)
        if fi[0] == fo[0] and fi[1] == fo[1] and fi[2] == fo[2] and not want_equal:
            continue
        r = fe.evaluate(c, text, addr)
        rec = dict(unit=unit, addr='%08X' % addr, sym=sym, img=fi[0], ours=fo[0], d=fo[0] - fi[0],
                   eh=[fi[1], fo[1]], ebp=[fi[2], fo[2]], diff=r.get('diff', -1), exact=r.get('exact', False))
        if want_slots:
            si = slots(I, fi[0], fi[2], fi[3])
            so = slots(O, fo[0], fo[2], fo[3])
            names = fa_names(unit, src, sym, pfx) if fo[2] else {}
            rec['slots'] = (si, so, names)
        out.append(rec)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='*')
    ap.add_argument('--addr', default='')
    ap.add_argument('--from', dest='src_json')
    ap.add_argument('--all', action='store_true')
    ap.add_argument('--jobs', type=int, default=int(os.environ.get('FE_JOBS', '5')))
    ap.add_argument('--pfx-base', type=int, default=int(os.environ.get('FE_PFX_BASE', '21')))
    ap.add_argument('--json')
    ap.add_argument('--slots', action='store_true')
    ap.add_argument('--equal', action='store_true', help='also list functions whose frames agree (with --addr)')
    a = ap.parse_args()
    cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
    units = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/')]
    if a.units:
        units = [u for u in units if u in a.units or os.path.basename(u) in a.units]
    only = {int(x, 16) for x in a.addr.split(',') if x}
    inexact = None
    if a.src_json:
        inexact = {int(x['addr'], 16) for x in json.load(open(a.src_json)) if not x.get('exact') and x['addr'] != '?'}
    work = []
    rx = re.compile(r'// ORIGINAL: 0x([0-9A-F]{8}) \S+ 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)')
    for u in units:
        try:
            t = open(fe.R + '/' + u, encoding='latin-1').read()
        except OSError:
            continue
        addrs = []
        for m in rx.finditer(t):
            ad = int(m.group(1), 16)
            if only:
                ok = ad in only
            elif inexact is not None:
                ok = ad in inexact
            else:
                ok = a.all or 'BYTE_EXACT' not in m.group(2)
            if ok:
                addrs.append(ad)
        if addrs:
            work.append((u, addrs))
    want_slots = a.slots or bool(only)

    fe.compile_cmd(work[0][0] if work else 'src/x.cpp')  # fill the command cache before threading

    def lane(k):
        r = []
        for i in range(k, len(work), a.jobs):
            r += scan_unit(work[i][0], work[i][1], a.pfx_base + k, want_slots, a.equal)
        return r
    res = []
    with ThreadPoolExecutor(a.jobs) as ex:
        for o in ex.map(lane, range(a.jobs)):
            res += o
    errs = [x for x in res if isinstance(x, tuple)]
    res = [x for x in res if isinstance(x, dict)]
    # largest structural payoff first: frame is the first differing rung when EH agrees
    res.sort(key=lambda x: (x['eh'][0] != x['eh'][1], -abs(x['d']) if x['d'] else 0, -x['diff']))
    for x in res:
        print('FRAME %s %-28s %-40s img=0x%x ours=0x%x d=%+d eh=%s/%s ebp=%s/%s diff=%d%s' % (
            x['addr'], x['unit'][4:], x['sym'][:40], x['img'], x['ours'], x['d'], int(x['eh'][0]), int(x['eh'][1]),
            int(x['ebp'][0]), int(x['ebp'][1]), x['diff'], ' EXACT' if x['exact'] else ''))
        if want_slots:
            print_slots(*x['slots'], x['ebp'][0], x['ebp'][1])
    for e in errs:
        print('ERROR', e[1], (e[2] or '')[:200])
    print('functions with differing frames: %d (EH differs: %d)' % (
        sum(1 for x in res if x['img'] != x['ours'] or x['ebp'][0] != x['ebp'][1]), sum(1 for x in res if x['eh'][0] != x['eh'][1])))
    if a.json:
        for x in res:
            x.pop('slots', None)
        json.dump(res, open(a.json, 'w'), indent=0)


if __name__ == '__main__':
    main()
