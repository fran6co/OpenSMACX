#!/usr/bin/env python3
"""probesweep.py [--write] [--each] [unit ...]

Dead budget-probe retirement screen (MATCHING_GUIDE 2.8, "Judge a probe removal by inline decisions").
For every function that holds `*_budget_probe*(...)` statement lines: the function without them
(--each: also without each line alone), measured by (named call difference to the image, fe rows)
against the unit as written, plus the unit's other markers that change.  A removal is KEEP when the
named call difference does not grow, the function's rows do not grow and no other marker of the unit
loses exactness; --write applies the KEEP removals (all lines of a function, one function at a time,
re-measured cumulatively).  Probe definitions are left in place: removing an unused definition
shifts the unit's front-end counter (guide 2.8), do that once per batch.
Prefixes FE_PFX_BASE.. with FE_JOBS lanes (one unit per lane).  Screen only.
"""
import sys, os, re, json, argparse
from concurrent.futures import ThreadPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.dirname(HERE))
import fe  # noqa: E402
import callees  # noqa: E402

R = fe.R
PROBE = re.compile(r'^[ \t]*\w*budget_probe\w*\([^;{}]*\);[^\n]*\n', re.M)
MARK = re.compile(r'// ORIGINAL: 0x([0-9A-F]{8}) \S+ 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+')


def measure(unit, text, pfx, tag):
    path = os.path.join(fe.SCR, 'probesweep_%s_%s.cpp' % (pfx, tag))
    open(path, 'w', encoding='latin-1').write(text)
    c, err = fe.compile_unit(unit, path, pfx)
    if c is None:
        return None
    out = {}
    for m in MARK.finditer(text):
        a = int(m.group(1), 16)
        try:
            r = fe.evaluate(c, text, a)
            out[a] = (r['diff'], r['exact'], callees.named_diff(c, text, a))
        except Exception:
            pass
    return out


def spans(text):
    ms = list(MARK.finditer(text))
    return [(int(m.group(1), 16), m.start(), ms[i + 1].start() if i + 1 < len(ms) else len(text)) for i, m in enumerate(ms)]


def unit_job(arg):
    unit, pfx, write, each = arg
    text = open(R + '/' + unit, encoding='latin-1').read()
    lines = []
    base = measure(unit, text, pfx, 'b')
    if base is None:
        return ['%s COMPILE FAIL' % unit]
    for addr, lo, hi in spans(text):
        ps = [m for m in PROBE.finditer(text, lo, hi) if 'static' not in m.group(0)]
        if not ps:
            continue
        cands = [('all', ps)] + ([('L%d' % (text.count('\n', 0, m.start()) + 1), [m]) for m in ps] if each and len(ps) > 1 else [])
        for key, sel in cands:
            t2 = text
            for m in reversed(sel):
                t2 = t2[:m.start()] + t2[m.end():]
            r = measure(unit, t2, pfx, 'v')
            if r is None or addr not in r or addr not in base:
                lines.append('%s %08X %s COMPILE/MEASURE FAIL' % (unit, addr, key))
                continue
            b, v = base[addr], r[addr]
            lost = [a for a in base if base[a][1] and not r.get(a, (0, False))[1]]
            gained = [a for a in base if not base[a][1] and r.get(a, (0, False))[1]]
            others = [a for a in base if a != addr and base[a] != r.get(a)]
            keep = v[2] <= b[2] and v[0] <= b[0] and not lost
            lines.append('%s %08X %-4s probes %2d  calls %d -> %d  rows %d -> %d  lost %s gained %s others-moved %d  %s' % (
                unit, addr, key, len(sel), b[2], v[2], b[0], v[0], ['%08X' % a for a in lost], ['%08X' % a for a in gained], len(others),
                'KEEP' if keep else ('calls-ok' if v[2] <= b[2] and not lost else '')))
            if keep and write and key == 'all':
                text = t2
                base = r
                open(R + '/' + unit, 'w', encoding='latin-1').write(text)
                break
    return lines


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='*')
    ap.add_argument('--write', action='store_true')
    ap.add_argument('--each', action='store_true')
    a = ap.parse_args()
    cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
    units = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/')]
    if a.units:
        units = [u for u in units if u in a.units or os.path.basename(u) in a.units]
    work = []
    for u in units:
        try:
            t = open(R + '/' + u, encoding='latin-1').read()
        except OSError:
            continue
        if any('static' not in m.group(0) for m in PROBE.finditer(t)):
            work.append(u)
    jobs = int(os.environ.get('FE_JOBS', '5'))
    base = int(os.environ.get('FE_PFX_BASE', '21'))
    callees.addr2sym(), callees.iat()
    fe.compile_cmd(work[0] if work else '')

    def lane(k):
        out = []
        for i in range(k, len(work), jobs):
            out += unit_job((work[i], str(base + k), a.write, a.each))
        return out
    with ThreadPoolExecutor(jobs) as ex:
        for o in ex.map(lane, range(jobs)):
            for ln in o:
                print(ln)


if __name__ == '__main__':
    sys.exit(main())
