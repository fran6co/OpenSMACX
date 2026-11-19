#!/usr/bin/env python3
"""retire.py WA.json [--write] [--kinds probe-call,fnptr,pragma,forceinline,volatile] [unit ...]

Stand-in retirement screen over `workarounds.py --json` items (MATCHING_GUIDE 4.x "Stand-in debt").
For each mechanical stand-in in a .cpp unit, the unit without it is compiled and every marker of the unit
measured (fe rows, exactness, named-call difference to the image, as probesweep.py):
  probe-call   the probe statements of one function removed (all lines of the function together,
               then each line alone when there are several); definitions stay (removing an unused
               definition shifts the unit's front-end counter, do it once per batch);
  fnptr        the function-pointer local deleted and its uses in that body spelled as the plain call;
  pragma       an `auto_inline/inline_depth/optimize` region's open and close lines deleted;
  forceinline  `__forceinline` spelled `inline`;
  volatile     `*(volatile T *)&x` read as `x`, `static_cast<const volatile T *>(p)` as `p`.
Verdicts: DEAD = every marker of the unit keeps its exactness, rows and named calls (the stand-in did
nothing measurable); KEEP = nothing gets worse and something improves; else the markers that move.
--write applies DEAD and KEEP removals cumulatively, one unit at a time (re-measured after each).
Prefixes FE_PFX_BASE.. with FE_JOBS lanes (one unit per lane). Screen only.
"""
import sys, os, re, json, argparse, collections
from concurrent.futures import ThreadPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.dirname(HERE))
import fe  # noqa: E402
import callees  # noqa: E402
import probesweep  # noqa: E402

R = fe.R


def strip_probe_calls(line, probe_re):
    """Remove `probe(args);` statements from a line; '' when nothing else is left but a comment."""
    new = re.sub(r'\b(' + probe_re + r')\s*\([^;{}]*\)\s*;', '', line)
    new = re.sub(r'^([ \t]*)(' + probe_re + r')\s*;?\s*(?=//|$)', r'\1', new)  # bare macro use
    code = new.split('//', 1)[0]
    if not code.strip():
        return None
    return new


def candidates(text, items, probe_re):
    lines = text.split('\n')
    out = []
    byfn = collections.defaultdict(list)
    for x in items:
        if x['kind'] == 'probe-call':
            key = tuple(s['addr'] for s in x['served'])
            byfn[key].append((x['line'] - 1, x.get('orig', x['line'])))
    for key, idx in byfn.items():
        def apply(sel):
            ls = list(lines)
            for i, _ in sorted(sel, reverse=True):
                n = strip_probe_calls(ls[i], probe_re)
                if n is None:
                    del ls[i]
                else:
                    ls[i] = n
            return '\n'.join(ls)
        out.append(('probe %s all' % '/'.join(key), apply(idx), list(key)))
        if len(idx) > 1:
            for i, o in idx:
                out.append(('probe %s L%d' % ('/'.join(key), o), apply([(i, o)]), list(key)))
    for x in items:
        i = x['line'] - 1
        served = [s['addr'] for s in x['served']]
        if x['kind'] == 'fnptr':
            name, target = x['subject'].split('=', 1)
            target = target.lstrip(':') if target.startswith('::') else target
            ls = list(lines)
            # the enclosing body: from the declaration to the end of the block it is declared in
            depth, j = 0, i
            while j < len(ls):
                depth += ls[j].count('{') - ls[j].count('}')
                if depth < 0:
                    break
                j += 1
            for k in range(i + 1, min(j + 1, len(ls))):
                ls[k] = re.sub(r'\b' + re.escape(name) + r'\s*\(', target + '(', ls[k])
            del ls[i]
            out.append(('fnptr %s L%d' % (name, x['orig']), '\n'.join(ls), served))
        elif x['kind'] == 'pragma-open':
            what = re.match(r'(\w+)', x['subject']).group(1)
            ls = list(lines)
            close = next((k for k in range(i + 1, len(ls)) if re.match(r'\s*#\s*pragma\s+' + what + r'\b', ls[k])), None)
            if close is None:
                continue
            del ls[close]
            del ls[i]
            out.append(('pragma %s L%d' % (x['subject'], x['orig']), '\n'.join(ls), served))
        elif x['kind'] == 'forceinline':
            ls = list(lines)
            ls[i] = ls[i].replace('__forceinline', 'inline')
            ls[i] = re.sub(r'\binline\s+static\b', 'static inline', ls[i])
            ls[i] = re.sub(r'\bstatic\s+inline\s+inline\b', 'static inline', ls[i])
            out.append(('forceinline %s L%d' % (x['subject'], x['orig']), '\n'.join(ls), served))
        elif x['kind'] in ('volatile', 'wa:volatile'):
            ls = list(lines)
            n = re.sub(r'\*\s*\(\s*(?:const\s+)?volatile\s+[\w\s]+?\*\s*\)\s*&\s*', '', ls[i])
            n = re.sub(r'static_cast<\s*(?:const\s+)?volatile\s+([\w:]+)\s*\*\s*>\s*\(([^()]*)\)', r'\2', n)
            n = re.sub(r'\*\s*reinterpret_cast<\s*volatile\s+[\w:]+\s*\*\s*>\s*\(\s*&([^()]*)\)', r'\1', n)
            n = re.sub(r'\bvolatile\s+(?=\w)', '', n) if n == ls[i] and not re.search(r'\(\s*volatile', n) else n
            if n != ls[i]:
                ls[i] = n
                out.append(('volatile L%d' % (x['orig']), '\n'.join(ls), served))
    return out


def verdict(base, r, served):
    lost = [a for a in base if base[a][1] and not r.get(a, (0, False))[1]]
    gained = [a for a in base if not base[a][1] and r.get(a, (0, False))[1]]
    worse = [a for a in base if a in r and (r[a][0] > base[a][0] or (r[a][2] or 0) > (base[a][2] or 0))]
    better = [a for a in base if a in r and (r[a][0] < base[a][0] or (r[a][2] or 0) < (base[a][2] or 0))]
    missing = [a for a in base if a not in r]
    if lost or worse or missing:
        v = 'NO'
    elif better or gained:
        v = 'KEEP'
    else:
        v = 'DEAD'
    return v, lost, gained, worse, better


def unit_job(arg):
    unit, items, probe_re, pfx, write = arg
    text = open(R + '/' + unit, encoding='latin-1').read()
    base = probesweep.measure(unit, text, pfx, 'rb')
    if base is None:
        return ['%s COMPILE FAIL (base)' % unit]
    out = []
    done = set()
    # re-derive candidates after each applied removal (line numbers move)
    while True:
        cands = [c for c in candidates(text, items_now(text, items), probe_re) if c[0] not in done]
        applied = False
        for key, t2, served in cands:
            if t2 == text:  # the rewrite did not apply (a definition line, an expression use)
                done.add(key); continue
            r = probesweep.measure(unit, t2, pfx, 'rv')
            if r is None:
                out.append('%s %-44s COMPILE FAIL' % (unit, key)); done.add(key); continue
            v, lost, gained, worse, better = verdict(base, r, [int(a, 16) for a in served])
            fmt = lambda xs: ','.join('%08X' % a for a in xs)
            sv = ' '.join('%s:%d->%d' % (a, base.get(int(a, 16), (-1,))[0], r.get(int(a, 16), (-1,))[0]) for a in served[:3])
            out.append('%s %-44s %-4s served %s lost[%s] gained[%s] worse[%s] better[%s]' % (
                unit, key[:44], v, sv, fmt(lost), fmt(gained), fmt(worse), fmt(better)))
            done.add(key)
            if write and v in ('DEAD', 'KEEP'):
                text, base = t2, r
                open(R + '/' + unit, 'w', encoding='latin-1').write(text)
                applied = True
                break
        if not applied:
            break
    return out


_ITEMS_SRC = {}


def items_now(text, items):
    """Re-locate items after edits by their source line text (subject) - recomputed from scratch."""
    return _relocate(text, items)


def _relocate(text, items):
    lines = text.split('\n')
    out = []
    used = set()
    for x in sorted(items, key=lambda x: x['line']):
        want = x['_text']
        # nearest unused line with the same text
        cand = [i for i, l in enumerate(lines) if l == want and (i, x['kind']) not in used]
        if not cand:
            continue
        i = min(cand, key=lambda k: abs(k - (x['line'] - 1)))
        used.add((i, x['kind']))
        y = dict(x); y['orig'] = x['line']; y['line'] = i + 1
        out.append(y)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('json')
    ap.add_argument('units', nargs='*')
    ap.add_argument('--write', action='store_true')
    ap.add_argument('--kinds', default='probe-call,fnptr,pragma,forceinline,volatile')
    a = ap.parse_args()
    items = json.load(open(a.json))
    kinds = a.kinds.split(',')
    items = [x for x in items if x['file'].endswith('.cpp') and any(x['kind'].startswith(k) or x['kind'] == 'wa:' + k for k in kinds)]
    if a.units:
        items = [x for x in items if x['file'] in a.units or os.path.basename(x['file']) in a.units]
    names = set()
    for x in items:
        if x['kind'] == 'probe-call':
            names.add(x['subject'].split(' x')[0])
    probe_re = '|'.join(sorted(names, key=len, reverse=True)) or r'\b\B'
    byunit = collections.defaultdict(list)
    for x in items:
        x['_text'] = open(R + '/' + x['file'], encoding='latin-1').read().split('\n')[x['line'] - 1]
        byunit[x['file']].append(x)
    work = sorted(byunit)
    jobs = int(os.environ.get('FE_JOBS', '5'))
    pbase = int(os.environ.get('FE_PFX_BASE', '21'))
    callees.addr2sym(), callees.iat()
    if work:
        fe.compile_cmd(work[0])

    def lane(k):
        o = []
        for i in range(k, len(work), jobs):
            o += unit_job((work[i], byunit[work[i]], probe_re, str(pbase + k), a.write))
        return o
    with ThreadPoolExecutor(jobs) as ex:
        for o in ex.map(lane, range(jobs)):
            for ln in o:
                print(ln, flush=True)


if __name__ == '__main__':
    sys.exit(main())
