#!/usr/bin/env python3
"""mattest.py [unit ...] [--all] [--json OUT]
Materialised flag tests present on one side only (MATCHING_GUIDE 3.7 "Flag tests: threaded or materialised").
For every non-exact function (--all: every marker) it counts, in the image and in our compile:
  mat   `mov r,K` / `xor r,r` then `test r,r; jcc` (a constant flag stored on one path and tested at a join:
        VC6 threads it away unless a loop, a call or a second use sits between the store and the test)
  set   `setcc r8` [`mov r2,r`] then `test r,r; jcc` (a computed bool local tested later: `int f = x != 0; if (f)`)
  orin  two conditional jumps with only straight code between, the first landing just after the second and the
        second jumping further on: an `||` whose arm is laid in place (the test computed as a value, 3.7)
  orout two such jumps to one forward target: the `||` (or `&&`) both of whose tests leave for one block
and prints the kinds whose counts differ, with image addresses and our offsets. Screen only; read the rows."""
import sys, os, re, json, argparse
from concurrent.futures import ThreadPoolExecutor
T = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, T)
import fe  # noqa: E402
MARK = re.compile(r'// ORIGINAL: 0x([0-9A-F]{8}) (\S+) 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)')
R32 = {'al': 'eax', 'bl': 'ebx', 'cl': 'ecx', 'dl': 'edx'}


def pats(X):
    out = []
    for i in range(len(X) - 2):
        a, b, c = X[i][1], X[i + 1][1], X[i + 2][1]
        m = re.match(r'test (e\w\w), (e\w\w)$', b)
        if not m or m.group(1) != m.group(2) or not c.startswith('j') or c.startswith('jmp'):
            continue
        r = m.group(1)
        if re.match(r'mov %s, (0x)?[0-9a-f]+$' % r, a) or a == 'xor %s, %s' % (r, r):
            out.append(('mat', X[i][0]))
        mm = re.match(r'set\w+ (\w\w)$', a)
        if mm and R32.get(mm.group(1)) == r:
            out.append(('set', X[i][0]))
        if i and re.match(r'mov %s, e\w\w$' % r, a):
            mm = re.match(r'set\w+ (\w\w)$', X[i - 1][1])
            if mm and a == 'mov %s, %s' % (r, R32.get(mm.group(1))):
                out.append(('set', X[i - 1][0]))
    return out


def jpairs(data, base):
    """orin / orout pairs from the raw code (real jump targets)."""
    ins = list(fe.md.disasm(data, base))
    out = []
    for i, x in enumerate(ins):
        if not (x.mnemonic.startswith('j') and x.mnemonic != 'jmp') or not x.op_str.startswith('0x'):
            continue
        t1 = int(x.op_str, 16)
        for j in range(i + 1, min(i + 14, len(ins))):
            y = ins[j]
            if y.mnemonic.startswith('j') or y.mnemonic.startswith('ret'):
                if y.mnemonic != 'jmp' and y.op_str.startswith('0x'):
                    t2 = int(y.op_str, 16)
                    if t1 == y.address + y.size and t2 > t1:
                        out.append(('orin', x.address - base))
                    elif t1 == t2 and t1 > y.address:
                        out.append(('orout', x.address - base))
                break
    return out


def scan_unit(unit, pfx, mode):
    text = open(fe.R + '/' + unit, encoding='latin-1').read()
    c, err = fe.compile_unit(unit, fe.R + '/' + unit, pfx)
    if c is None:
        return [('ERR', unit, err)]
    out = []
    for m in MARK.finditer(text):
        addr = int(m.group(1), 16)
        r = fe.evaluate(c, text, addr)
        if 'error' in r or (mode == 'nonexact' and r['exact']):
            continue
        O = [x[1] for x in r['rows'] if x[1] is not None]
        C = [x[2] for x in r['rows'] if x[2] is not None]
        po, pc = pats(O), pats(C)
        idn = fe.ident(text, addr)
        po += jpairs(fe.imgbytes(idn[1], idn[2]), idn[1])
        try:
            s = c.function(idn[0]); pc += jpairs(c.sections[s.section - 1].data, 0)
        except Exception:
            pass
        d = []
        for kd in ('mat', 'set', 'orin', 'orout'):
            a = [o for k, o in po if k == kd]; b = [o for k, o in pc if k == kd]
            if len(a) != len(b):
                d.append(dict(kind=kd, img=['%X' % o for o in a], ours=['+%X' % o for o in b]))
        if d:
            out.append(dict(unit=unit, addr='%08X' % addr, fn=r['sym'], exact=r['exact'], diff=r['diff'], d=d))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='*')
    ap.add_argument('--all', action='store_true')
    ap.add_argument('--json')
    ap.add_argument('--jobs', type=int, default=int(os.environ.get('FE_JOBS', '5')))
    ap.add_argument('--pfx-base', type=int, default=int(os.environ.get('FE_PFX_BASE', '21')))
    a = ap.parse_args()
    mode = 'all' if a.all else 'nonexact'
    cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
    units = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/')]
    units = [u for u in units if '/vendor/' not in u and u.endswith('.cpp')]
    if a.units:
        units = [u for u in units if u in a.units or os.path.basename(u) in a.units]
    fe.compile_cmd(units[0])

    def lane(j):
        r = []
        for i in range(j, len(units), a.jobs):
            r += scan_unit(units[i], a.pfx_base + j, mode)
        return r
    res = []
    with ThreadPoolExecutor(a.jobs) as ex:
        for o in ex.map(lane, range(a.jobs)):
            res += o
    for e in res:
        if isinstance(e, tuple):
            print('ERROR', e[1], (e[2] or '')[:200])
    res = [x for x in res if isinstance(x, dict)]
    if a.json:
        json.dump(res, open(a.json, 'w'), indent=0)
    for x in sorted(res, key=lambda x: (x['unit'], x['addr'])):
        print('%s %s %-40s %s diff=%d' % ('EXACT' if x['exact'] else 'NONEX', x['addr'], x['fn'][:40], x['unit'], x['diff']))
        for d in x['d']:
            print('   %-4s img %d %s  ours %d %s' % (d['kind'], len(d['img']), ','.join(d['img'][:8]), len(d['ours']), ','.join(d['ours'][:8])))
    print('functions=%d' % len(res))


if __name__ == '__main__':
    main()
