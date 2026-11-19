#!/usr/bin/env python3
"""copycensus.py [unit ...] [--pfx N] [--json F] [--all] [--list]
Census of named field copies (MATCHING_GUIDE 2.7 "Named copies of a field"). For every fe-exact function
(BYTE_EXACT marker; --all takes every marker) it lists the single-assignment locals that localinplace.py
would respell (`int v = Obj[i].field;`, `v = p->field;`, `v = Global;`) and reads from our /FAsc listing,
which for an exact function is the image, what VC6 did with the copy:

  reg    the copy's line defines a callee-saved register (esi/edi/ebx; ebp under /Oy) that later lines read
  slot   the local has a frame slot that the copy's line stores (memory home; `reg+slot` = written back)
  none   neither: the value is consumed from a scratch register or folded into its uses

with the facts the allocator could use: number of uses, calls between the copy and its last use (`x` = the
value crosses a call, so it needs a callee-saved register or a slot), loop depth is not read. The table
printed at the end groups by (uses, crosses a call). --list prints one line per local.
Compiles each unit once on prefix --pfx (default FE_PFX_BASE or 21)."""
import sys, os, re, json, glob, argparse, collections, subprocess
H = os.path.dirname(os.path.abspath(__file__))
T_ = os.path.dirname(H)
R = os.path.dirname(os.path.dirname(T_))
sys.path.insert(0, T_)
sys.path.insert(0, H)
os.chdir(R)
import fe
import localinplace as lip

CS = ('esi', 'edi', 'ebx')


def unit_cod(unit, pfx):
    cmd = fe.compile_cmd(unit)
    asm = fe.SCR + '/cc_%d.cod' % os.getpid()
    obj = fe.SCR + '/cc_%d.obj' % os.getpid()
    out = []
    for t in cmd:
        if t.startswith('/Fo'):
            t = '/Fo' + obj
        elif t.startswith('/Fd') or t.startswith('--repro-root') or t == '/W4':
            continue
        out.append(t)
    out.insert(-1, '/FAsc')
    out.insert(-1, '/Fa' + asm)
    env = dict(os.environ, VC6_PREFIX='%s/.opensmacx/wine/worker-%s' % (fe.MAIN, pfx))
    subprocess.run(out, cwd=fe.SCR, env=env, capture_output=True, text=True)
    if not os.path.exists(asm):
        return None
    t = open(asm, errors='replace').read().replace('\r', '')
    os.remove(asm)
    os.path.exists(obj) and os.remove(obj)
    return t


def func(t, sym):
    """-> (frame {name: off}, ins [(line, text)]) of one function of a .cod listing"""
    i = t.find(sym + ' PROC')
    if i < 0:
        return None
    e = t.find(sym + ' ENDP', i)
    head = t[t.rfind('SEGMENT', 0, i):i]
    frame = {m.group(1): int(m.group(3)) for m in re.finditer(r'^_(\w+?)\$(\d*) = (-?\d+)$', head, re.M)}
    ins, line, pend, last = [], 0, False, -1
    for l in t[i:e].split('\n'):
        m = re.match(r'; (\d+)\s*:', l)
        if m:
            line = int(m.group(1))
            continue
        m = re.match(r'  ([0-9a-f]{5})\t([0-9a-f ]+)(?:\t+ ?(.*))?$', l)
        x = None
        if m:
            off = int(m.group(1), 16)
            if off < last:
                break
            last = off
            if m.group(3):
                x = m.group(3)
            else:
                pend = True
                continue
        else:
            m = re.match(r'\t[0-9a-f ]+\t+ ?(\S.*)$', l)
            if m and pend:
                x, pend = m.group(1), False
        if x:
            ins.append((line, re.sub(r'\s+', ' ', x.split(';')[0]).strip()))
    return frame, ins


def classify(text, c, frame, ins):
    l0 = c['line']
    l1 = text.count('\n', 0, c['de']) + 1
    ln = lambda o: text.count('\n', 0, o) + 1
    use_lines = sorted(ln(u) for u in c['uses'])
    idx = [k for k, (l, x) in enumerate(ins) if l0 <= l <= l1]
    if not idx:
        return None
    first, last_def = idx[0], idx[-1]
    reg = None
    regk = 0
    slot = False
    width = 'int'
    SUBR = {'al': 'eax', 'ax': 'eax', 'cl': 'ecx', 'cx': 'ecx', 'dl': 'edx', 'dx': 'edx', 'bl': 'ebx', 'bx': 'ebx',
            'si': 'esi', 'di': 'edi'}
    scratch = set()
    slotre = re.compile(r'mov (?:DWORD|WORD|BYTE) PTR _%s\$\d*\[e[bs]p[^\]]*\], (\w+)' % re.escape(c['name']))
    for k in idx:
        x = ins[k][1]
        m = re.match(r'(mov|movsx|movzx|lea) (\w+), (.*)', x)
        if m:
            d = SUBR.get(m.group(2), m.group(2))
            if d in CS:
                reg, regk = d, k
            elif d in ('eax', 'ecx', 'edx') and '[' in m.group(3):
                scratch.add(d)
        if re.match(r'movsx', x) or re.search(r'BYTE PTR|WORD PTR', x) and not re.search(r'DWORD PTR', x):
            width = 'narrow'
        if c['name'] in frame and slotre.match(x):
            slot = True
    # the copy's own register move or slot store is often listed under the next source line
    for kk in range(last_def + 1, min(len(ins), last_def + 13)):
        l, x = ins[kk]
        m = re.match(r'mov (e[sd]i|ebx), (e[acd]x)$', x)
        if m and m.group(2) in scratch and not reg:
            reg, regk = m.group(1), kk
            continue
        m = slotre.match(x)
        if m and SUBR.get(m.group(1), m.group(1)) in scratch and c['name'] in frame:
            slot = True
            continue
        m = re.match(r'(?:mov|movsx|movzx|lea|xor|pop) (\w+),', x)
        if m:
            scratch.discard(SUBR.get(m.group(1), m.group(1)))
        if x.startswith('call'):
            scratch.clear()
        if not scratch:
            break
    # the register must be read on a later line before it is redefined
    if reg:
        live = False
        for l, x in ins[regk + 1:]:
            if re.match(r'(?:mov|movsx|movzx|lea|pop) %s,' % reg, x) or x.startswith('xor %s, %s' % (reg, reg)):
                break
            if re.search(r'\b%s\b' % reg, x) or (reg == 'ebx' and re.search(r'\bb[lhx]\b', x)):
                live = True
                break
        if not live:
            reg = None
    # calls strictly between the copy and the first instruction of its last use line
    ul = use_lines[-1] if use_lines else l1
    lastk = next((k for k in range(last_def + 1, len(ins)) if ins[k][0] == ul), last_def)
    calls = sum(1 for l, x in ins[last_def + 1:lastk] if x.startswith('call'))
    home = '+'.join(h for h in (('reg' if reg else ''), ('slot' if slot else '')) if h) or 'none'
    # an address temporary of the same base in a callee-saved register on the copy's lines
    return dict(home=home, reg=reg, uses=len(c['uses']), calls=calls, width=width)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='*')
    ap.add_argument('--pfx', default=os.environ.get('FE_PFX_BASE', '21'))
    ap.add_argument('--json')
    ap.add_argument('--all', action='store_true')
    ap.add_argument('--list', action='store_true')
    a = ap.parse_args()
    units = a.units or sorted(os.path.relpath(p, R) for p in glob.glob(R + '/src/**/*.cpp', recursive=True))
    rows = []
    for unit in units:
        text = open(R + '/' + unit, encoding='latin-1').read()
        marks = [(int(m.group(1), 16), 'BYTE_EXACT' in m.group(0)) for m in re.finditer(r'// ORIGINAL: 0x([0-9A-F]{8}) \S+ [^\n]*', text)]
        marks = [m for m in marks if m[1] or a.all]
        if not marks:
            continue
        todo = []
        for addr, ex in marks:
            try:
                bs, be, m, cands = lip.candidates(text, addr)
            except BaseException:
                continue
            if cands:
                todo.append((addr, ex, cands))
        if not todo:
            continue
        try:
            cod = unit_cod(unit, a.pfx)
        except BaseException:
            cod = None
        if cod is None:
            continue
        for addr, ex, cands in todo:
            idn = fe.ident(text, addr)
            f = idn and func(cod, idn[0])
            if not f:
                continue
            for c in cands:
                r = classify(text, c, f[0], f[1])
                if r is None:
                    continue
                r.update(unit=unit, addr='%08X' % addr, name=c['name'], expr=c['expr'], exact=ex, respelled=len(c['revs']))
                rows.append(r)
                if a.list:
                    print('%s %s %-20s %-9s uses %-2d calls %-2d %-6s respelled %-2d = %s' % (
                        unit, r['addr'], r['name'], r['home'], r['uses'], r['calls'], r['width'], r['respelled'], r['expr'][:60]), flush=True)
    if a.json:
        json.dump(rows, open(a.json, 'w'), indent=1)
    tab = collections.defaultdict(collections.Counter)
    for r in rows:
        u = r['uses'] if r['uses'] < 4 else ('4-7' if r['uses'] < 8 else '8+')
        tab[(str(u), 'x' if r['calls'] else '-')][r['home']] += 1
    hs = ['reg', 'reg+slot', 'slot', 'none']
    print('%d locals in %d functions' % (len(rows), len({(r['unit'], r['addr']) for r in rows})))
    print('uses call ' + ' '.join('%9s' % h for h in hs))
    for k in sorted(tab):
        print('%-4s %-4s ' % k + ' '.join('%9d' % tab[k][h] for h in hs))


if __name__ == '__main__':
    main()
