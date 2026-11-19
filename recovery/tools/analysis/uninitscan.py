#!/usr/bin/env python3
"""uninitscan.py [unit ...] [--first] [--jobs N] [--json OUT]
Uninitialized-read screen (MATCHING_GUIDE 3.5, "the wrong variable of a packed frame slot"; about 3.5 min
tree-wide with FE_JOBS=5).

Decompiler transcriptions name locals by frame slot, so where the original packed two variables into one
slot a read can be attributed to the wrong one. That read then has no definition on its path: VC6 /W4 with
the optimiser on reports it as C4701 ("may be used without having been initialized"; C4700 when certain).
Example: base_build 0x004F81A0 read the supply-convoy counter l_34 where the image reads the unit score w
from their shared [ebp-0x34] (093c0ce64d, 4821 -> 3657 rows).

Every optimised game/Jackal/UV unit is compiled with its build flags plus /W4. VC6 reports each variable
only once per function, so by default each reported read gets `V = 0;` inserted at the start of its statement
in a throw-away copy and the unit is recompiled until no new site appears (up to 6 passes; aggregates that
cannot take `= 0` stop the enumeration for that unit, listed as ERR). `--first` keeps only the compiler's
first report. Each site prints with its function, fe state of the function (EXACT = claimed marker, else
`fe<rows>` from a fresh fe pass of that unit) and the source line.

Reading the output: most sites are flow false positives (the def and the read sit under the same test, a
switch without default over a closed range) or annotated original bugs. A candidate is a read whose path
can never have set the variable (its only defs sit in a mutually exclusive branch or switch case) while a
sibling variable that was set on that path shares the image slot it reads (framescan.py --addr --slots,
fe.py --all). Sites in BYTE_EXACT functions are original behaviour: record, do not change."""
import sys, os, re, json, argparse, subprocess
from concurrent.futures import ThreadPoolExecutor
H = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(H))
import fe

WARN = re.compile(r"\\([^\\(]+)\((\d+)\) : warning (C470[01]): local variable '(\w+)'")
STMT_BAD = re.compile(r'^\s*(\}|else\b|&&|\|\||\?|:|[-+*/|&^<>=,)]|case\b|default\b)')


def compile_w4(unit, path, pfx, tag):
    cmd = fe.compile_cmd(unit)
    if not cmd:
        return [], []
    obj = '%s/uw_%s.obj' % (fe.SCR, tag)
    out = []
    for t in cmd:
        if t.startswith('/Fo'):
            t = '/Fo' + obj
        elif t.startswith('/Fd') or t.startswith('--repro-root') or t in ('/W0', '/W1', '/W2', '/W3'):
            continue
        elif t == fe.R + '/' + unit:
            t = path
        out.append(t)
    if '/W4' not in out:
        out.insert(1, '/W4')
    env = dict(os.environ, VC6_PREFIX='%s/.opensmacx/wine/worker-%s' % (fe.MAIN, pfx))
    p = subprocess.run(out, cwd=fe.SCR, env=env, capture_output=True, text=True, errors='replace')
    os.path.exists(obj) and os.remove(obj)
    sites = [(int(m.group(2)), m.group(4), m.group(3)) for m in WARN.finditer(p.stdout)
             if m.group(1) == os.path.basename(path)]
    errs = [l for l in p.stdout.split('\n') if ' error ' in l]
    return sites, errs


def scan(unit, pfx, first):
    lines = open(fe.R + '/' + unit, encoding='latin-1').read().split('\n')
    tmp = '%s/src/zz_uninit_%s_%s' % (fe.R, pfx, os.path.basename(unit))
    ins, found, err, agg = {}, {}, None, set()
    zero = lambda v: ('*(char *)&%s = 0;' if v in agg else '%s = 0;') % v  # aggregates: address taken, no warning
    try:
        passes = 0
        while passes < (1 if first else 6):
            passes += 1
            text, back = [], []
            for n, l in enumerate(lines, 1):
                if n in ins:
                    text.append(re.match(r'\s*', l).group(0) + ' '.join(zero(v) for v in ins[n]))
                    back.append(('ins', n))
                text.append(l)
                back.append(n)
            open(tmp, 'w', encoding='latin-1').write('\n'.join(text))
            ws, errs = compile_w4(unit, tmp, pfx, pfx)
            if errs:
                m = re.search(r'\((\d+)\) : error', errs[0])
                b = back[int(m.group(1)) - 1] if m and 0 < int(m.group(1)) <= len(back) else None
                if isinstance(b, tuple) and set(ins[b[1]]) - agg:
                    agg.update(ins[b[1]])
                    passes -= 1
                    continue
                err = errs[0].strip()[-160:]
                break
            new = []
            for tl, v, code in ws:
                ol = back[tl - 1] if 0 < tl <= len(back) else None
                if isinstance(ol, int) and (ol, v) not in found:
                    found[(ol, v)] = code
                    new.append((ol, v))
            if not new:
                break
            for ol, v in new:
                s = ol
                while s > 1 and not re.search(r'[;{}]\s*(//.*)?$', lines[s - 2]):
                    s -= 1
                if not STMT_BAD.match(lines[s - 1]):
                    ins.setdefault(s, []).append(v)
    finally:
        os.path.exists(tmp) and os.remove(tmp)
    if not found and not err:
        return []
    # functions: marker spans, fe rows for unclaimed markers
    text = '\n'.join(lines)
    marks = [(i + 1, m.group(1), 'BYTE_EXACT' in m.group(2)) for i, l in enumerate(lines)
             for m in [re.match(r'// ORIGINAL: 0x([0-9A-F]{8}) \S+ \S+([^\n]*)', l)] if m]
    c, _ = fe.compile_unit(unit, fe.R + '/' + unit, pfx)
    out, rows = [], {}
    for (ln, v), code in sorted(found.items()):
        prev = [x for x in marks if x[0] < ln]
        if not prev:
            continue
        _, addr, claimed = prev[-1]
        st = 'EXACT'
        if not claimed:
            if addr not in rows:
                r = fe.evaluate(c, text, int(addr, 16)) if c else {'error': 'compile'}
                rows[addr] = 'fe%d' % r['diff'] if 'diff' in r else '?'
            st = rows[addr]
        sym = fe.ident(text, int(addr, 16))
        out.append(dict(unit=unit, line=ln, var=v, code=code, addr=addr, sym=sym[0] if sym else '?', state=st,
                        src=lines[ln - 1].strip()))
    if err:
        out.append(dict(unit=unit, error=err))
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('units', nargs='*')
    ap.add_argument('--first', action='store_true', help='only the compiler\'s first report per variable')
    ap.add_argument('--jobs', type=int, default=int(os.environ.get('FE_JOBS', '5')))
    ap.add_argument('--json')
    a = ap.parse_args()
    base = int(os.environ.get('FE_PFX_BASE', '21'))
    cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
    units = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/')
             and ('/Og' in e['command'].split() or '/O2' in e['command'].split())]
    if a.units:
        units = [u for u in units if u in a.units or os.path.basename(u) in a.units]

    def lane(k):
        r = []
        for i in range(k, len(units), a.jobs):
            r += scan(units[i], str(base + k), a.first)
        return r
    res = []
    with ThreadPoolExecutor(a.jobs) as ex:
        for o in ex.map(lane, range(a.jobs)):
            res += o
    for r in sorted(res, key=lambda r: (r['unit'], r.get('line', 0))):
        if 'error' in r:
            print('ERR  %s enumeration stopped: %s' % (r['unit'], r['error']))
        else:
            print('%-7s %s %-28s %6d %-24s %s %s | %s' % (r['state'], r['code'], r['unit'], r['line'], r['var'],
                                                         r['addr'], r['sym'][:40], r['src'][:80]))
    n = [r for r in res if 'error' not in r]
    print('sites %d in non-exact functions, %d in exact functions' % (
        sum(r['state'] != 'EXACT' for r in n), sum(r['state'] == 'EXACT' for r in n)))
    if a.json:
        json.dump(res, open(a.json, 'w'), indent=1)


if __name__ == '__main__':
    main()
