#!/usr/bin/env python3
"""regmap.py <unit> <0xADDR> [--src ALT.cpp] [--pfx N] [--min USES] [--ranges] [--reg esi,edi,ebx]
Which callee-saved register holds which role, in the image and in our compile, and what the other side has in
the same aligned rows.

A *tenure* starts at an instruction that loads the register (mov/lea/movsx/movzx/xor r,r/or r,-1/pop) and runs to
the last read before the next load, in address order.  Tenures are grouped by role = what was loaded:
  this            mov r, ecx in the prologue of a member function
  [slot]{name}    a frame or parameter slot: the variable is memory-homed and the register is a cached copy
                  (several ranges = "cached block by block"; `wb` = the register is written back to its slot)
  eax@line        a call result or computed value: the register is the variable's home for that tenure
  expr            anything else (address arithmetic, constants: compiler temporaries)
For ours, names and source lines come from the /FAsc listing; image slots get our names through the fe
alignment (majority vote over aligned rows), so a wrong or missing name there means the frames differ.

Each row ends with `-> other side:` a count of what the aligned instruction on the other side uses in the
operand positions where this side uses the register (another register, a named slot, or `-` when the row has
no partner).  Example reading: `IMG edi [0xc]{morale} ... -> ours: [0xc]{morale} 9` means the image caches
morale in edi where ours reads it from memory each time."""
import sys, os, re, subprocess, argparse, collections
T = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, T)
import fe

CS = ('ebx', 'esi', 'edi', 'ebp')
SUB = {'ebx': ('ebx', 'bx', 'bl', 'bh'), 'esi': ('esi', 'si'), 'edi': ('edi', 'di'), 'ebp': ('ebp', 'bp')}
LOADS = ('mov', 'lea', 'movsx', 'movzx', 'pop')


def listing(unit, src, pfx, sym):
    """/FAsc listing of one function -> {offset: (source line, instruction text)}"""
    cmd = fe.compile_cmd(unit)
    asm = fe.SCR + '/rm_%d.cod' % os.getpid()
    obj = fe.SCR + '/rm_%d.obj' % os.getpid()
    out = []
    for t in cmd:
        if t.startswith('/Fo'):
            t = '/Fo' + obj
        elif t.startswith('/Fd') or t.startswith('--repro-root') or t == '/W4':
            continue
        elif t == fe.R + '/' + unit:
            t = os.path.abspath(src)
        out.append(t)
    out.insert(-1, '/FAsc')
    out.insert(-1, '/Fa' + asm)
    env = dict(os.environ, VC6_PREFIX='%s/.opensmacx/wine/worker-%s' % (fe.MAIN, pfx))
    subprocess.run(out, cwd=fe.SCR, env=env, capture_output=True, text=True)
    t = open(asm, errors='replace').read()
    os.remove(asm)
    os.path.exists(obj) and os.remove(obj)
    i = t.find(sym + ' PROC')
    e = t.find(sym + ' ENDP', i)
    line = 0
    byoff = {}
    pend = None
    for l in t[i:e].split('\n'):
        l = l.rstrip('\r')
        m = re.match(r'; (\d+)\s*:', l)
        if m:
            line = int(m.group(1))
            continue
        ins = None
        m = re.match(r'  ([0-9a-f]{5})\t([0-9a-f ]+)(?:\t+ ?(.*))?$', l)
        if m:
            off = int(m.group(1), 16)
            if m.group(3):
                ins = m.group(3)
            else:
                pend = off
                continue
        else:
            m = re.match(r'\t[0-9a-f ]+\t+ ?(\S.*)$', l)
            if m and pend is not None:
                off = pend
                ins = m.group(1)
                pend = None
        if ins:
            byoff[off] = (line, ins)
    return byoff


def slot_of(op):
    m = re.search(r'\[ebp ([-+]) (0x[0-9a-f]+|\d+)\]', op)
    return (('-' if m.group(1) == '-' else '') + m.group(2)) if m else None


def split_ops(txt):
    mn, _, rest = txt.partition(' ')
    return mn, [o.strip() for o in rest.split(',')] if rest else []


def uses_reg(txt, r):
    return re.search(r'\b(%s)\b' % '|'.join(SUB[r]), txt) is not None


def is_load(txt, r):
    mn, ops = split_ops(txt)
    if not ops or ops[0] != r:
        return None
    if mn in LOADS:
        if mn == 'pop':
            return 'pop'
        src = ops[1] if len(ops) > 1 else ''
        if mn == 'lea' or not uses_reg(src, r):
            return src if mn != 'lea' else 'lea ' + src
        return None
    if mn == 'xor' and len(ops) == 2 and ops[1] == r:
        return '0'
    if mn == 'or' and len(ops) == 2 and ops[1] in ('0xffffffff', '-1'):
        return '-1'
    return None


def counterpart(a, b, r):
    """what does text b have where text a has register r (same instruction shape)?"""
    if b is None:
        return ['-']
    ma, oa = split_ops(a)
    mb, ob = split_ops(b)
    if len(oa) != len(ob):
        return ['?']
    res = []
    for x, y in zip(oa, ob):
        if not uses_reg(x, r):
            continue
        if '[' not in x:
            if '[' in y:
                s = slot_of(y)
                res.append('[%s]' % s if s else 'mem')
            else:
                res.append(y)
        else:
            if '[' not in y:
                res.append('?')
                continue
            ta = re.findall(r'[a-z0-9<>]+', x[x.index('['):])
            tb = re.findall(r'[a-z0-9<>]+', y[y.index('['):])
            if len(ta) != len(tb):
                res.append('?')
            else:
                res.extend(q for p, q in zip(ta, tb) if p in SUB[r])
    return res or ['?']


class Side:
    def __init__(self, rows, me, base):
        # rows: fe rows; me: 1 for image, 2 for ours
        self.ins = []  # (addr, text, partner text or None, row kind)
        for k, o, c in rows:
            mine, other = (o, c) if me == 1 else (c, o)
            if mine is not None:
                self.ins.append((base + mine[0], mine[1], other[1] if other else None, k))
        self.names = {}
        self.lines = {}
        self.this = None
        for a, t, p, k in self.ins[:14]:
            for r in CS:
                if t == 'mov %s, ecx' % r:
                    self.this = r

    def tenures(self, regs):
        out = []
        for r in regs:
            cur = None
            for n, (a, t, p, k) in enumerate(self.ins):
                src = is_load(t, r)
                if src == 'pop':
                    cur = None
                    continue
                if src is not None:
                    role = src
                    s = slot_of(src)
                    if n < 14 and src == 'ecx' and self.this == r:
                        role = 'this'
                    elif s and not src.startswith('lea'):
                        role = '[%s]' % s
                    elif src in ('eax', 'ecx', 'edx') or src.startswith('dword ptr [e') or src.startswith('lea'):
                        role = src
                    cur = dict(reg=r, role=role, slot=s if role.startswith('[') else None, start=a, end=a, uses=0, other=collections.Counter(), wb=set(), n=n)
                    out.append(cur)
                    continue
                if cur is None:
                    continue
                if t.startswith('push ' + r) and n < 8:
                    continue
                if uses_reg(t, r):
                    cur['uses'] += 1
                    cur['end'] = a
                    for c in counterpart(t, p, r):
                        cur['other'][c] += 1
                    m = re.match(r'mov (?:dword ptr )?\[ebp ([-+]) (0x[0-9a-f]+|\d+)\], %s$' % r, t)
                    if m:
                        cur['wb'].add(('-' if m.group(1) == '-' else '') + m.group(2))
        return out


def vote_slots(rows, ournames):
    """image slot -> our variable name through aligned rows"""
    v = collections.defaultdict(collections.Counter)
    for k, o, c in rows:
        if o is None or c is None:
            continue
        so, sc = slot_of(o[1]), slot_of(c[1])
        if so and sc and sc in ournames:
            v[so][ournames[sc]] += 1
    return {s: c.most_common(1)[0][0] for s, c in v.items()}


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('unit')
    ap.add_argument('addr')
    ap.add_argument('--src')
    ap.add_argument('--pfx', default=os.environ.get('FE_PFX_BASE', '2'))
    ap.add_argument('--min', type=int, default=2, help='hide roles with fewer register reads (default 2)')
    ap.add_argument('--ranges', action='store_true', help='list every tenure of a role, not only the first and the count')
    ap.add_argument('--reg', default='ebx,esi,edi')
    a = ap.parse_args()
    unit = a.unit if a.unit.startswith('src/') else 'src/' + a.unit
    addr = int(a.addr, 16)
    src = os.path.abspath(a.src) if a.src else fe.R + '/' + unit
    text = open(src, encoding='latin-1').read()
    c, err = fe.compile_unit(unit, src, a.pfx)
    if c is None:
        raise SystemExit('COMPILE FAIL\n' + str(err))
    r = fe.evaluate(c, text, addr)
    if 'error' in r:
        raise SystemExit(r['error'])
    lst = listing(unit, src, a.pfx, r['sym'])
    srclines = text.split('\n')
    regs = a.reg.split(',')
    if not any(re.match(r'mov ebp, esp', x[1][1]) for x in r['rows'][:3] if x[1]):
        regs = regs + ['ebp'] if 'ebp' not in regs else regs
    img, our = Side(r['rows'], 1, r['lo']), Side(r['rows'], 2, 0)
    ournames = {}
    for k, o, cc in r['rows']:
        if cc is None or cc[0] not in lst:
            continue
        s = slot_of(cc[1])
        m = re.search(r'_(\w+)\$\d*\[ebp', lst[cc[0]][1])
        if s and m:
            ournames.setdefault(s, m.group(1))
    imgnames = vote_slots(r['rows'], ournames)
    print('FE %08X %s exact=%s diff=%d img=%d ours=%d' % (addr, r['sym'][:50], r['exact'], r['diff'], r['img'], r['ours']))
    for tag, side, names, oname in (('IMG', img, imgnames, ournames), ('OUR', our, ournames, imgnames)):
        groups = collections.OrderedDict()
        for t in sorted(side.tenures(regs), key=lambda t: t['start']):
            groups.setdefault((t['reg'], t['role']), []).append(t)
        print('--- %s: register, role, first tenure, tenures, reads  -> what the %s has there' % (tag, 'image' if tag == 'OUR' else 'ours'))
        for (reg, role), ts in sorted(groups.items(), key=lambda g: (regs.index(g[0][0]), g[1][0]['start'])):
            uses = sum(t['uses'] for t in ts)
            if uses < a.min:
                continue
            other = collections.Counter()
            for t in ts:
                other.update(t['other'])
            label = role
            if ts[0]['slot']:
                label += '{%s}' % names.get(ts[0]['slot'], '?')
            wb = sorted({w for t in ts for w in t['wb']})
            if wb:
                label += ' wb:' + ','.join('[%s]{%s}' % (w, names.get(w, '?')) for w in wb)
            if tag == 'OUR' and ts[0]['start'] in lst and not ts[0]['slot'] and role != 'this':
                ln = lst[ts[0]['start']][0]
                label += '  @%d: %s' % (ln, srclines[ln - 1].strip()[:60])

            def fmt(x):
                m = re.fullmatch(r'\[(-?(?:0x)?[0-9a-f]+)\]', x)
                return '%s{%s}' % (x, oname.get(m.group(1), '?')) if m else x
            oth = ' '.join('%s:%d' % (fmt(x), n) for x, n in other.most_common(5))
            fmtaddr = (lambda x: '%x' % x)
            print('%s %s %s-%s x%-3d r%-4d %-40s -> %s' % (tag, reg, fmtaddr(ts[0]['start']), fmtaddr(max(t['end'] for t in ts)), len(ts), uses, label, oth))
            if a.ranges:
                for t in ts:
                    print('        %s-%s r%d' % (fmtaddr(t['start']), fmtaddr(t['end']), t['uses']))


if __name__ == '__main__':
    main()
