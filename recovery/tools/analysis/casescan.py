#!/usr/bin/env python3
"""casescan.py <unit> <addr[,addr]> [--src ALT.cpp] [-v] | --from FEALL.json [--json F] [--units u,..]
Switch case-label screen (MATCHING_GUIDE Z9 "Shared case labels"). For every jump-table dispatch
(`jmp [r*4+T]`, with or without the VC6 byte index table `mov cl,[r+B]`) in both the image and our
COMDAT, map each table index to its target block and compare:
  PART  the partition of indices by target differs: ours sends two indices to one block that the
        image keeps apart (`split`: write the cases apart), or the image shares one ours keeps apart
        (`merge`: write `case a: case b:`); `dflt` marks the group the bound check's default joins.
  TGT   same partition, but an index's image target does not align with ours (block moved/rewritten).
  BLK   the case block's straight run (to its first jmp/ret) differs in length: a cross-jumped tail
        on one side (`img>ours`: ours jumps into another case's tail the image wrote out).
  DFLT  one side sends a case value to the bound check's default block, the other to its own block.
  BIAS  the index bias (`add/sub/dec/lea` before the bound `cmp`) or the bound differs.
  NTAB  the dispatch counts differ (a table on one side is a compare tree on the other).
Indices are table indices; case value = index + bias (printed). Screen only."""
import sys, os, re, json, struct, argparse
S = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, S)
import fe

JMP = re.compile(r'dword ptr \[(e\w\w)\*4(?: \+ (0x[0-9a-f]+))?\]')
BYT = re.compile(r'byte ptr \[(e\w\w)(?: \+ (0x[0-9a-f]+))?\]')


def raw_insns(data, org):
    out = []
    for i in fe.md.disasm(data, org):
        out.append((i.address, i.mnemonic, i.op_str, i.size))
    return out


def dispatches(ins, rd32, rd8, start_of, in_code):
    """-> [dict(at, idx, n, bias, bound, dflt, targets[index])]; offsets in the side's own space"""
    res = []
    for k, (a, mn, op, sz) in enumerate(ins):
        if mn != 'jmp': continue
        m = JMP.search(op)
        if not m: continue
        reg = m.group(1); T = start_of(a, sz, op)
        if T is None: continue
        # walk back: optional byte table, bound cmp, default branch, bias
        B = None; bound = None; dflt = None; bias = 0; ireg = reg; jmn = 'ja'
        j = k - 1; lim = max(0, k - 12)
        while j >= lim:
            a2, mn2, op2, sz2 = ins[j]
            mb = BYT.search(op2)
            if mn2 in ('mov', 'movzx') and mb and B is None:
                B = start_of(a2, sz2, op2); ireg = mb.group(1)
            elif mn2 in ('ja', 'jae') and dflt is None:
                try: dflt = int(op2, 0); jmn = mn2
                except ValueError: pass
            elif mn2 == 'cmp' and bound is None and op2.startswith(ireg + ','):
                try: bound = int(op2.split(',')[1], 0) - (jmn == 'jae')
                except ValueError: pass
            elif bound is not None and op2.startswith(ireg + ',') and mn2 in ('add', 'sub'):
                try:
                    v = int(op2.split(',')[1], 0); v = v - (1 << 32) if v >= 1 << 31 else v
                    if abs(v) < 0x10000: bias = -v if mn2 == 'add' else v
                    break
                except ValueError: break
            elif bound is not None and (op2.startswith(ireg + ',') or op2 == ireg): break
            elif bound is not None and mn2 == 'dec' and op2 == ireg: bias = 1; break
            elif bound is not None and mn2 == 'lea' and op2.startswith(ireg + ','):
                mm = re.search(r'\[e\w\w ([+-]) (0x[0-9a-f]+|\d+)\]', op2)
                if mm: bias = int(mm.group(2), 0) * (1 if mm.group(1) == '-' else -1)
                break
            elif bound is not None and mn2 in ('call', 'ret') or mn2.startswith('j') and bound is not None: break
            j -= 1
        if bound is None: continue
        n = bound + 1
        if B is not None:
            idx = [rd8(B + i) for i in range(n)]
            nt = max(idx) + 1
            tab = [rd32(T + 4 * i) for i in range(nt)]
            tg = [tab[x] for x in idx]
        else:
            tg = [rd32(T + 4 * i) for i in range(n)]
        if any(t is None or not in_code(t) for t in tg): continue
        res.append(dict(at=a, n=n, bias=bias, dflt=dflt, targets=tg, bytes=B is not None))
    return res


JCC = {'je', 'jne', 'jg', 'jge', 'jl', 'jle', 'ja', 'jae', 'jb', 'jbe', 'js', 'jns'}
REG32 = re.compile(r'^e(ax|bx|cx|dx|si|di|bp)$')


def _flags(a, b, r):
    s = lambda x: x - (1 << 32) if x & (1 << 31) else x
    return dict(z=(r & 0xffffffff) == 0, sa=s(a & 0xffffffff), sb=s(b & 0xffffffff), ua=a & 0xffffffff, ub=b & 0xffffffff,
                sgn=bool(r & (1 << 31)))


def _cond(mn, f):
    if mn == 'je': return f['z']
    if mn == 'jne': return not f['z']
    if mn == 'jg': return f['sa'] > f['sb']
    if mn == 'jge': return f['sa'] >= f['sb']
    if mn == 'jl': return f['sa'] < f['sb']
    if mn == 'jle': return f['sa'] <= f['sb']
    if mn == 'ja': return f['ua'] > f['ub']
    if mn == 'jae': return f['ua'] >= f['ub']
    if mn == 'jb': return f['ua'] < f['ub']
    if mn == 'jbe': return f['ua'] <= f['ub']
    if mn == 'js': return f['sgn']
    if mn == 'jns': return not f['sgn']


def _jcc_after(ins, k):
    # the jcc that consumes the flags of ins[k]; a scheduled `mov` (not flag-setting) may sit between
    for j in (k + 1, k + 2):
        if j >= len(ins): return None
        if ins[j][1] in JCC: return j
        if ins[j][1] != 'mov': return None
    return None


def _unit(ins, k):
    """a compare-tree test unit at k: (reg, op, imm) followed by a jcc"""
    j = _jcc_after(ins, k)
    if j is None: return None
    a, mn, op, sz = ins[k]
    ps = [x.strip() for x in op.split(',')]
    if mn == 'dec' and REG32.match(op): return op
    if mn in ('cmp', 'sub', 'add') and len(ps) == 2 and REG32.match(ps[0]):
        try: int(ps[1], 0); return ps[0]
        except ValueError: return None
    if mn == 'test' and len(ps) == 2 and ps[0] == ps[1] and REG32.match(ps[0]): return ps[0]
    return None


def _run(ins, pos, start, reg, v):
    a0 = start; val = v & 0xffffffff; f = None; steps = 0; k = pos[start]
    while steps < 200:
        steps += 1
        a, mn, op, sz = ins[k]
        ps = [x.strip() for x in op.split(',')]
        if mn in ('cmp', 'sub', 'add') and len(ps) == 2 and ps[0] == reg:
            try: imm = int(ps[1], 0)
            except ValueError: return a
            r = val - imm if mn != 'add' else val + imm
            f = _flags(val, imm, r)
            if mn != 'cmp': val = r & 0xffffffff; f['sa'] = f['sb'] = None if mn == 'add' else f['sa']
            k += 1; continue
        if mn == 'dec' and op == reg:
            r = val - 1; f = _flags(val, 1, r); f['ua'] = None; val = r & 0xffffffff; k += 1; continue
        if mn == 'test' and ps == [reg, reg]:
            f = _flags(val, 0, val); f['ua'] = val; f['ub'] = 0; k += 1; continue
        if mn in JCC:
            if f is None: return a
            try: c = _cond(mn, f)
            except TypeError: return a
            if c:
                try: tg = int(op, 0)
                except ValueError: return a
                if tg not in pos: return tg
                k = pos[tg]
            else: k += 1
            continue
        if mn == 'mov' and not op.startswith(reg + ',') and f is not None:
            k += 1; continue
        if mn == 'jmp':
            try: tg = int(op, 0)
            except ValueError: return a
            if tg not in pos: return tg
            # a jmp to another test unit continues the tree
            if _unit(ins, pos[tg]) == reg: k = pos[tg]; continue
            return tg
        return a
    return None


def trees(ins):
    """compare trees: [dict(at, reg, vals{v: target}, dflt)] in the side's own offsets"""
    pos = {x[0]: i for i, x in enumerate(ins)}
    units = {}
    for k in range(len(ins)): 
        r = _unit(ins, k)
        if r: units[ins[k][0]] = r
    # successors of each unit (fallthrough + jcc target) that are units on the same register
    inner = set()
    for a, r in units.items():
        k = _jcc_after(ins, pos[a])
        nx = ins[k + 1][0] if k + 1 < len(ins) else None
        try: tg = int(ins[k][2], 0)
        except ValueError: tg = None
        for b in (nx, tg):
            if b in units and units[b] == r: inner.add(b)
            elif b in pos and ins[pos[b]][1] == 'jmp':
                try: b2 = int(ins[pos[b]][2], 0)
                except ValueError: continue
                if units.get(b2) == r: inner.add(b2)
    res = []
    for a, r in units.items():
        if a in inner: continue
        consts = set()
        # constants: walk the reachable units collecting cumulative offsets
        seen = set(); stack = [(a, 0)]
        while stack:
            b, off = stack.pop()
            if (b, off) in seen or len(seen) > 400: continue
            seen.add((b, off)); k = pos[b]
            _, mn, op, _ = ins[k]; ps = [x.strip() for x in op.split(',')]
            imm = int(ps[1], 0) if mn in ('cmp', 'sub', 'add') else (1 if mn == 'dec' else 0)
            if mn == 'add': imm = -imm
            imm = imm - (1 << 32) if imm >= 1 << 31 else imm
            consts.add(off + imm)
            noff = off + imm if mn in ('sub', 'add', 'dec') else off
            kj = _jcc_after(ins, k)
            try: tg = int(ins[kj][2], 0)
            except ValueError: tg = None
            nx = ins[kj + 1][0] if kj + 1 < len(ins) else None
            for b2 in (nx, tg):
                if b2 in pos and ins[pos[b2]][1] == 'jmp':
                    try: b2 = int(ins[pos[b2]][2], 0)
                    except ValueError: continue
                if units.get(b2) == r: stack.append((b2, noff))
        if len(seen) < 3: continue
        vals = set()
        for c_ in consts: vals.update((c_ - 1, c_, c_ + 1))
        vm = {}
        for v in sorted(vals):
            tg = _run(ins, pos, a, r, v)
            if tg is not None: vm[v] = tg
        cnt = {}
        for v, tg in vm.items(): cnt[tg] = cnt.get(tg, 0) + 1
        if len(cnt) < 3: continue
        dflt = vm.get(min(vm) if vm else None)
        res.append(dict(at=a, reg=r, vals=vm, dflt=dflt))
    return sorted(res, key=lambda x: x['at'])


def straight(insmap, addrs, t, fn):
    """instructions from t to the first jmp/ret (inclusive), as text"""
    out = []
    try: k = addrs.index(t)
    except ValueError: return None
    while k < len(addrs) and len(out) < 400:
        a, mn, op, sz = insmap[addrs[k]]
        out.append(mn)
        if mn == 'jmp' or mn.startswith('ret'): break
        k += 1
    return out


def scan(c, text, addr, verbose=False):
    idn = fe.ident(text, addr)
    if not idn: return dict(error='no marker')
    sym, lo, hi = idn
    try: s = c.function(sym)
    except Exception: return dict(error='nosymbol')
    sec = c.sections[s.section - 1]; data = sec.data
    rel = {r[0]: r for r in sec.relocations}
    res = fe.evaluate(c, text, addr)
    if 'error' in res: return res
    omap = {}
    for k, o, cc in res['rows']:
        if o is not None and cc is not None and o[1] == cc[1]: omap[o[0]] = cc[0]
    # image side
    ib = fe.imgbytes(lo, hi + 0x2000)
    iins = raw_insns(fe.imgbytes(lo, hi), lo)
    def i_rd32(va):
        o = va - lo
        return struct.unpack_from('<I', ib, o)[0] - lo if 0 <= o <= len(ib) - 4 else None
    def i_rd8(va):
        o = va - lo
        return ib[o] if 0 <= o < len(ib) else 0
    def i_start(a, sz, op):
        if not lo <= a < hi or '+ 0x' not in op: return None
        b = fe.imgbytes(a, a + sz)
        return struct.unpack_from('<I', b, sz - 4)[0]
    icode = {a - lo for a, *_ in iins}
    idis = dispatches(iins, i_rd32, i_rd8, i_start, lambda t: t in icode)
    for d in idis: d['at'] -= lo; d['dflt'] = None if d['dflt'] is None else d['dflt'] - lo
    # our side
    def o_sym(off):
        r = rel.get(off)
        if not r: return None
        sy = c.symbols.get(r[1])
        if sy is None or sy.section != s.section: return None
        return sy.value + struct.unpack_from('<I', data, off)[0]
    oins = raw_insns(data, 0)
    def o_rd32(off):
        return o_sym(off) if 0 <= off <= len(data) - 4 else None
    def o_rd8(off): return data[off] if 0 <= off < len(data) else 0
    def o_start(a, sz, op): return o_sym(a + sz - 4)
    ocode = {a for a, *_ in oins}
    odis = dispatches(oins, o_rd32, o_rd8, o_start, lambda t: t in ocode)
    iaddrs = [a - lo for a, *_ in iins]; imap = {a - lo: x for a, *x in [(x[0],) + x for x in iins]}
    imap = {x[0] - lo: x for x in iins}; oaddrs = [x[0] for x in oins]; omp = {x[0]: x for x in oins}
    out = dict(sym=sym, addr='%08X' % addr, diff=res['diff'], nimg=len(idis), nours=len(odis), finds=[])
    F = out['finds']
    if verbose:
        for x in odis: print('  ours table at %x n=%d bias=%d dflt=%s' % (x['at'], x['n'], x['bias'], x['dflt'] is not None and '%x' % x['dflt']))
    if len(idis) != len(odis): F.append(('NTAB', 'image %d tables, ours %d' % (len(idis), len(odis))))
    used = set()
    for d in idis:
        e = None
        if d['at'] in omap:
            e = next((x for x in odis if x['at'] == omap[d['at']]), None)
        if e is None:
            e = next((x for x in odis if id(x) not in used and x['n'] == d['n']), None) or next((x for x in odis if id(x) not in used), None)
        if e is None: F.append(('NTAB', 'image table at %x (n=%d) unmatched' % (lo + d['at'], d['n']))); continue
        used.add(id(e))
        tag = 'table %x' % (lo + d['at'])
        if d['bias'] != e['bias'] or d['n'] != e['n']:
            F.append(('BIAS', '%s image bias %d n %d, ours bias %d n %d' % (tag, d['bias'], d['n'], e['bias'], e['n'])))
            if d['bias'] != e['bias']: continue
        n = min(d['n'], e['n'])
        gi = {}; go = {}
        for i in range(n):
            gi.setdefault(d['targets'][i], []).append(i); go.setdefault(e['targets'][i], []).append(i)
        pi = sorted(tuple(v) for v in gi.values()); po = sorted(tuple(v) for v in go.values())
        cv = lambda i: i + d['bias']
        if pi != po:
            for grp in po:
                parts = {tuple(x for x in grp if x in g) for g in pi} - {()}
                if len(parts) > 1:
                    df = ['dflt' if d['dflt'] is not None and d['targets'][p[0]] == d['dflt'] else '' for p in parts]
                    F.append(('PART', '%s split: ours shares %s, image keeps %s' % (tag, [cv(x) for x in grp], ' | '.join('%s%s' % ([cv(x) for x in p], '(' + f + ')' if f else '') for p, f in zip(sorted(parts), df)))))
            for grp in pi:
                parts = {tuple(x for x in grp if x in g) for g in po} - {()}
                if len(parts) > 1:
                    df = 'dflt' if d['dflt'] is not None and d['targets'][grp[0]] == d['dflt'] else ''
                    F.append(('PART', '%s merge: image shares %s%s, ours keeps %s' % (tag, [cv(x) for x in grp], '(' + df + ')' if df else '', ' | '.join(str([cv(x) for x in p]) for p in sorted(parts)))))
        else:
            for grp in pi:
                ti = d['targets'][grp[0]]; to = e['targets'][grp[0]]
                if omap.get(ti) != to:
                    F.append(('TGT', '%s cases %s image %x -> ours %x (aligned %s)' % (tag, [cv(x) for x in grp], lo + ti, to, '%x' % omap[ti] if ti in omap else '-')))
        for i in range(n):
            ii = d['dflt'] is not None and d['targets'][i] == d['dflt']; oo = e['dflt'] is not None and e['targets'][i] == e['dflt']
            if ii != oo and (i == 0 or (d['targets'][i] == d['dflt']) != (d['targets'][i - 1] == d['dflt']) or (e['targets'][i] == e['dflt']) != (e['targets'][i - 1] == e['dflt'])):
                F.append(('DFLT', '%s case %d: %s' % (tag, cv(i), 'ours joins the default block, image has its own' if oo else 'image joins the default block, ours has its own')))
        # straight-run lengths per image group
        for grp in pi:
            ti = d['targets'][grp[0]]; to = e['targets'][grp[0]]
            si = straight(imap, iaddrs, ti, None); so = straight(omp, oaddrs, to, None)
            if si is not None and so is not None and len(si) != len(so):
                F.append(('BLK', '%s cases %s run img %d ours %d (%s) at img %x ours %x' % (tag, [cv(x) for x in grp], len(si), len(so), 'img>ours' if len(si) > len(so) else 'ours>img', lo + ti, to)))
        if verbose:
            print('  %s n=%d bias=%d dflt=%s img: %s' % (tag, d['n'], d['bias'], d['dflt'] and '%x' % (lo + d['dflt']), ' '.join('%d:%x' % (cv(i), lo + t) for i, t in enumerate(d['targets']))))
            print('  %s ours n=%d bias=%d: %s' % (' ' * len(tag), e['n'], e['bias'], ' '.join('%d:%x' % (cv(i), t) for i, t in enumerate(e['targets']))))
    # compare trees
    def _rb(x):
        if x[1] in JCC or x[1] == 'jmp':
            try: return (x[0] - lo, x[1], hex(int(x[2], 0) - lo), x[3])
            except ValueError: pass
        return (x[0] - lo,) + tuple(x[1:])
    iloc = [_rb(x) for x in iins]
    itr = trees(iloc); otr = trees([x for x in oins if x[0] < res_end(res)])
    out['nimg'] += len(itr); out['nours'] += len(otr)
    if len(itr) != len(otr): F.append(('NTREE', 'image %d compare trees, ours %d' % (len(itr), len(otr))))
    used = set()
    for d in itr:
        e = None
        if d['at'] in omap: e = next((x for x in otr if x['at'] == omap[d['at']]), None)
        if e is None: e = next((x for x in otr if id(x) not in used and set(x['vals']) == set(d['vals'])), None)
        if e is None:
            F.append(('NTREE', 'image tree at %x (%s, values %s) unmatched' % (lo + d['at'], d['reg'], sorted(v for v, tg in d['vals'].items() if tg != d['dflt'])[:12]))); continue
        used.add(id(e)); tag = 'tree %x' % (lo + d['at'])
        V = sorted(set(d['vals']) | set(e['vals']))
        gi = {}; go = {}
        for v in V:
            gi.setdefault(d['vals'].get(v, d['dflt']), []).append(v); go.setdefault(e['vals'].get(v, e['dflt']), []).append(v)
        pi = sorted(tuple(x) for x in gi.values()); po = sorted(tuple(x) for x in go.values())
        if verbose:
            print('  %s img: %s' % (tag, ' '.join('%d:%x' % (v, lo + d['vals'].get(v, d['dflt'])) for v in V if d['vals'].get(v, d['dflt']) != d['dflt'])))
            print('  %s ours: %s' % (' ' * len(tag), ' '.join('%d:%x' % (v, e['vals'].get(v, e['dflt'])) for v in V if e['vals'].get(v, e['dflt']) != e['dflt'])))
        if pi != po:
            for grp in po:
                parts = {tuple(x for x in grp if x in g) for g in pi} - {()}
                if len(parts) > 1:
                    F.append(('PART', '%s split: ours shares %s, image keeps %s' % (tag, list(grp)[:12], ' | '.join(str(list(p)[:8]) + ('(dflt)' if d['vals'].get(p[0], d['dflt']) == d['dflt'] else '') for p in sorted(parts)))))
            for grp in pi:
                parts = {tuple(x for x in grp if x in g) for g in po} - {()}
                if len(parts) > 1:
                    F.append(('PART', '%s merge: image shares %s%s, ours keeps %s' % (tag, list(grp)[:12], '(dflt)' if d['vals'].get(grp[0], d['dflt']) == d['dflt'] else '', ' | '.join(str(list(p)[:8]) + ('(dflt)' if e['vals'].get(p[0], e['dflt']) == e['dflt'] else '') for p in sorted(parts)))))
        else:
            for grp in pi:
                ti = d['vals'].get(grp[0], d['dflt']); to = e['vals'].get(grp[0], e['dflt'])
                if omap.get(ti) != to and ti != d['dflt']:
                    F.append(('TGT', '%s cases %s image %x -> ours %x (aligned %s)' % (tag, list(grp)[:8], lo + ti, to, '%x' % omap[ti] if ti in omap else '-')))
        for grp in pi:
            ti = d['vals'].get(grp[0], d['dflt']); to = e['vals'].get(grp[0], e['dflt'])
            si = straight(imap, iaddrs, ti, None); so = straight(omp, oaddrs, to, None)
            if si is not None and so is not None and len(si) != len(so):
                F.append(('BLK', '%s cases %s run img %d ours %d (%s) at img %x ours %x' % (tag, list(grp)[:8], len(si), len(so), 'img>ours' if len(si) > len(so) else 'ours>img', lo + ti, to)))
    return out


def res_end(res):
    return max([r[2][0] + 1 for r in res['rows'] if r[2] is not None] or [1 << 30])


def unitpath(u):
    if not u.endswith(('.cpp', '.c')): u += '.cpp'
    u = u if u.startswith('src/') else 'src/' + u
    if not fe.compile_cmd(u):
        hits = [k[len(fe.R) + 1:] for k in fe._CC if os.path.basename(k) == os.path.basename(u)]
        if len(hits) == 1: u = hits[0]
    return u


def main():
    ap = argparse.ArgumentParser(); ap.add_argument('unit', nargs='?'); ap.add_argument('addr', nargs='?')
    ap.add_argument('--src'); ap.add_argument('--from', dest='frm'); ap.add_argument('--json'); ap.add_argument('-v', action='store_true')
    ap.add_argument('--units', default=''); ap.add_argument('--all', action='store_true', help='print functions without findings too')
    a = ap.parse_args()
    work = {}
    if a.frm:
        for r in json.load(open(a.frm)):
            if r.get('exact') or r['addr'] == '?': continue
            if a.units and os.path.basename(r['unit']) not in a.units.split(','): continue
            work.setdefault(r['unit'], []).append(int(r['addr'], 16))
    else:
        work[unitpath(a.unit)] = [int(x, 16) for x in a.addr.split(',')]
    allres = []
    for u, addrs in sorted(work.items()):
        src = os.path.abspath(a.src) if a.src else fe.R + '/' + u
        text = open(src, encoding='latin-1').read()
        c, err = fe.compile_unit(u, src, fe.PFX)
        if c is None: print('COMPILE FAIL', u, err); continue
        for ad in addrs:
            try: r = scan(c, text, ad, a.v)
            except Exception as ex: r = dict(error=repr(ex)[:120])
            r['unit'] = u; allres.append(r)
            if 'error' in r:
                if not a.frm: print('ERR %08X %s' % (ad, r['error']))
                continue
            if r['nimg'] == 0 and r['nours'] == 0 and not a.all: continue
            if not r['finds'] and not a.all and a.frm: continue
            print('%08X %-40s %s diff=%d tables img=%d ours=%d' % (ad, r['sym'][:40], os.path.basename(u), r['diff'], r['nimg'], r['nours']))
            for k, msg in r['finds']: print('   %-4s %s' % (k, msg))
    if a.json: json.dump(allres, open(a.json, 'w'), indent=0)


if __name__ == '__main__': main()
