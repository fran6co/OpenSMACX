#!/usr/bin/env python3
"""ownerscan.py [unit ...] [--addr A,..] [--probes 8,32] [--repeat] [--swaps] [--json F] [--all]

Missing-inline-owner screen (MATCHING_GUIDE 2.8, "Missing owner, not missing budget").

For each non-exact function (or --addr), list the inline callees the IMAGE calls at more sites
than ours does (ours expands sites the image calls). Then every target function of the unit gets
a dead pass-1 probe at the top of its body, `if (__os_g[0] == 0x1234567) { __os_p(..); ... }`
with N charged 4-if inline probes (about 4.7 if-units each), and the unit is recompiled once per
probe size. Per callee it reports our call count at each probe size against the image's:

  budget   the probe turns our expanded sites into calls up to the image's count (pass-1 sites
           refused for budget: supply natural spend);
  partial  some flip, not all;
  owner    nothing flips (or only once the probe flips unrelated nested sites first): in the
           original these sites were inside an inline owner the function expanded; our source
           writes that owner out. Find a helper boundary, not spend.

For callees whose out-of-line copy makes calls, inlinemap's per-site E/c strings (image / ours /
ours at each probe size) are printed too. --repeat lists, for owner callees, the source lines of
the function that name the callee and every other function in the tree that contains the same
statement window (2-3 normalised lines): a shared block suggests a shared helper.
Screen only; prefixes FE_PFX_BASE.. (FE_JOBS lanes).
"""
import sys, os, re, json, glob, argparse, collections, threading
from concurrent.futures import ProcessPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.dirname(HERE))
import fe  # noqa: E402
import inlinemap as im  # noqa: E402

R = fe.R
PROBE_DECL = ('extern int __os_g[]; extern void __os_mk(int);\n'
              'static inline void __os_p(int k) { if (__os_g[k] == 1000) __os_mk(k); '
              'if (__os_g[k + 1] == 1000) __os_mk(k + 1); if (__os_g[k + 2] == 1000) __os_mk(k + 2); '
              'if (__os_g[k + 3] == 1000) __os_mk(k + 3); }\n')


def body_span(text, addr):
    """-> (marker_pos, open_brace_pos, close_brace_pos) of the function after the ORIGINAL marker"""
    m = re.search(r'^// ORIGINAL: 0x%08X ' % addr, text, re.M)
    if not m:
        return None
    p = m.start()
    # skip the marker's comment block and any further comment lines
    q = m.end()
    lines_end = q
    while True:
        nl = text.find('\n', lines_end)
        if nl < 0:
            return None
        nxt = text[nl + 1:nl + 3]
        lines_end = nl + 1
        if not text[lines_end:].lstrip(' ').startswith('//') and text[lines_end:lines_end + 1] != '\n':
            break
    # first '{' at paren depth 0 after the signature
    depth = 0
    i = lines_end
    while i < len(text):
        ch = text[i]
        if text.startswith('//', i):
            i = text.find('\n', i)
            continue
        if text.startswith('/*', i):
            i = text.find('*/', i) + 2
            continue
        if ch == '(':
            depth += 1
        elif ch == ')':
            depth -= 1
        elif ch == ';' and depth == 0:
            return None  # a declaration, not a definition
        elif ch == '{' and depth == 0:
            break
        i += 1
    ob = i
    d = 0
    j = ob
    while j < len(text):
        if text.startswith('//', j):
            j = text.find('\n', j)
            continue
        if text.startswith('/*', j):
            j = text.find('*/', j) + 2
            continue
        ch = text[j]
        if ch in '"\'':
            k = j + 1
            while k < len(text) and text[k] != ch:
                k += 2 if text[k] == '\\' else 1
            j = k + 1
            continue
        if ch == '{':
            d += 1
        elif ch == '}':
            d -= 1
            if d == 0:
                return p, ob, j
        j += 1
    return None


def probed(text, addrs, n):
    """whole-file variant with an n-probe dead block at the top of each target function"""
    if n == 0:
        return text
    spans = []
    for a in addrs:
        s = body_span(text, a)
        if s:
            spans.append(s)
    block = '\n    if (__os_g[0] == 0x1234567) { ' + ' '.join('__os_p(%d);' % (4 * k + 1) for k in range(n)) + ' }\n'
    out = text
    for p, ob, cb in sorted(spans, key=lambda s: -s[1]):
        out = out[:ob + 1] + block + out[ob + 1:]
    first = min((s[0] for s in spans), default=None)
    if first is None:
        return out
    return out[:first] + PROBE_DECL + out[first:]


def counts_img(lo, hi):
    return collections.Counter(n for a, n in im.img_stream(lo, hi))


def counts_ours(c, sym):
    return collections.Counter(n for a, n in im.our_stream(c, sym))


def short(k):
    k = k.lstrip('?')
    return k.split('@')[0]


NORM = re.compile(r'\s+')


def norm(line):
    line = re.sub(r'//.*', '', line)
    return NORM.sub(' ', line).strip()


_TREE = None


def tree_lines():
    global _TREE
    if _TREE is None:
        _TREE = []
        for p in glob.glob(R + '/src/**/*.cpp', recursive=True):
            t = open(p, encoding='latin-1').read().split('\n')
            _TREE.append((p[len(R) + 1:], [norm(x) for x in t]))
    return _TREE


def owner_of(path, lines, i):
    """nearest ORIGINAL marker symbol above line i of a file (raw text lines not kept: re-read)"""
    t = open(R + '/' + path, encoding='latin-1').read().split('\n')
    for k in range(i, -1, -1):
        m = re.match(r'// ORIGINAL: 0x([0-9A-F]{8}) (\S+)', t[k])
        if m:
            return '%s %s' % (m.group(1), m.group(2)[:50])
    return '?'


def repeats(text, addr, name, unit='', width=(2, 3)):
    """source lines naming `name` in the function, and other places with the same statement window"""
    s = body_span(text, addr)
    if not s:
        return []
    lines = text[s[1]:s[2]].split('\n')
    nl = [norm(x) for x in lines]
    base_line = text[:s[1]].count('\n') + 1
    out = []
    pat = re.compile(r'\b%s\s*\(' % re.escape(name))
    for i, l in enumerate(nl):
        if not pat.search(l):
            continue
        hits = []
        for w in width:
            for lo_ in range(i - w + 1, i + 1):
                win = [x for x in nl[max(0, lo_):lo_ + w]]
                if len(win) < w or any(len(x) < 4 or x in ('{', '}', '} else {') for x in win):
                    continue
                if sum(len(x) for x in win) < 40 * w // 2:
                    continue
                found = []
                for path, tl in tree_lines():
                    for j in range(len(tl) - w + 1):
                        if tl[j] == win[0] and tl[j:j + w] == win:
                            if path == unit and base_line <= j + 1 <= base_line + len(lines):
                                continue  # the function itself
                            found.append((w, path, j + 1))
                if len(found) <= 4:  # a statement window common everywhere is an idiom, not a helper
                    hits += found
        out.append((base_line + i, l, hits))
    return out

def lane(args):
    k, work, probes, base, jobs, a = args
    res = []
    for wi in range(k, len(work), jobs):
        u, t, addrs = work[wi]
        objs = {}
        for n in probes:
            v = probed(t, addrs, n)
            path = '%s/os_%d_%d.cpp' % (fe.SCR, k, n)
            open(path, 'w', encoding='latin-1').write(v)
            c, err = fe.compile_unit(u, path, str(base + k))
            objs[n] = (c, v)
        if objs[0][0] is None:
            res.append(dict(unit=u, error='compile failed'))
            continue
        for ad in addrs:
            idn = fe.ident(t, ad)
            if not idn:
                continue
            sym, lo, hi = idn
            try:
                ci = counts_img(lo, hi)
                cos = {n: (counts_ours(objs[n][0], sym) if objs[n][0] is not None else None) for n in probes}
            except Exception as e:
                res.append(dict(unit=u, addr='%08X' % ad, error=repr(e)))
                continue
            co = cos[0]
            inl = [n for n in ci if im.SPANS.get(n) and not n.startswith('??0') and not n.startswith('?__os')]
            if not a.swaps:  # count-differing callees only (analyse adds them itself); --swaps tracks every callee
                inl = [n for n in inl if any(cos[m] is not None and cos[m][n] != ci[n] for m in probes)]
            per = {}
            if True:
                for n in probes:
                    if objs[n][0] is None:
                        continue
                    try:
                        per[n] = im.analyse(objs[n][0], sym, lo, hi, inl)[3]
                    except Exception:
                        per[n] = {}
            cand = {n for n in inl if ci[n] > co[n]}
            swaps = {}  # callee -> site indexes the image calls and ours expands
            for cal, st in per.get(0, {}).items():
                bad = [q for q, (x, y) in enumerate(st) if x and y and x[2] == 'c' and y[2] == 'E']
                if bad:
                    swaps[cal] = bad
                    cand.add(cal)
            if a.all:
                cand |= set(inl)
            if not cand:
                continue
            rows = []
            pc = [cos[n]['?__os_p'] if cos[n] is not None else 0 for n in probes]
            for cal in sorted(cand):
                seq = [cos[n][cal] if cos[n] is not None else None for n in probes]
                need = ci[cal] - co[cal]
                got = max((x for x in seq if x is not None), default=co[cal]) - co[cal]
                sites = {}
                for n in probes:
                    st = per.get(n, {}).get(cal)
                    if st:
                        ri, ro, lv = im.strings(st)
                        sites['img'] = ri
                        sites['p%d' % n] = ro
                flips = []
                for q in swaps.get(cal, []):
                    f = [n for n in probes[1:] if len(sites.get('p%d' % n, '')) > q and sites['p%d' % n][q] == 'c']
                    flips.append((q + 1, f[0] if f else None))
                if swaps.get(cal):
                    nf = sum(1 for q, f in flips if f is not None)
                    kind = 'budget' if nf == len(flips) else 'partial' if nf else 'owner'
                else:
                    kind = 'budget' if got >= need else 'partial' if got > 0 else 'owner'
                rows.append(dict(callee=cal, img=ci[cal], ours=seq, kind=kind, probe_calls=pc, sites=sites,
                                 called_sites=['%d:%s' % (q, 'p%d' % f if f is not None else 'never') for q, f in flips]))
            rec = dict(unit=u, addr='%08X' % ad, sym=sym, callees=rows)
            if a.repeat:
                rec['repeats'] = {r['callee']: repeats(t, ad, short(r['callee']), u)
                                  for r in rows if r['kind'] != 'budget' and not r['callee'].startswith('??')}
            res.append(rec)
    return res



def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='*')
    ap.add_argument('--addr', default='')
    ap.add_argument('--probes', default='8,32')
    ap.add_argument('--repeat', action='store_true')
    ap.add_argument('--json')
    ap.add_argument('--all', action='store_true', help='also list callees that agree')
    ap.add_argument('--swaps', action='store_true', help='per-site E/c for every callee (slow on large functions)')
    a = ap.parse_args()
    probes = [0] + [int(x) for x in a.probes.split(',') if x]
    want = {int(x, 16) for x in a.addr.split(',') if x}
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
        addrs = []
        for m in re.finditer(r'^// ORIGINAL: 0x([0-9A-F]{8}) \S+ 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)', t, re.M):
            ad = int(m.group(1), 16)
            if want and ad not in want:
                continue
            if not want and 'BYTE_EXACT' in m.group(2):
                continue
            if '// compile_context' in t[m.end():m.end() + 400].split('\n\n')[0]:
                continue
            addrs.append(ad)
        if addrs:
            work.append((u, t, addrs))
    jobs = int(os.environ.get('FE_JOBS', '5'))
    base = int(os.environ.get('FE_PFX_BASE', '21'))
    fe.compile_cmd(work[0][0] if work else '')

    out = []
    with ProcessPoolExecutor(jobs) as ex:
        for o in ex.map(lane, [(k, work, probes, base, jobs, a) for k in range(jobs)]):
            out += o
    if a.json:
        json.dump(out, open(a.json, 'w'), indent=1)
    print('probe sizes (4-if inlines at the top): %s' % probes)
    for r in sorted(out, key=lambda r: (r.get('unit', ''), r.get('addr', ''))):
        if 'error' in r:
            print('%s %s ERROR %s' % (r.get('unit'), r.get('addr', ''), r['error']))
            continue
        print('\n%s %s %s' % (r['addr'], r['unit'], r['sym'][:60]))
        for c in r['callees']:
            print('  %-7s %-40s img calls %3d  ours %s  (probe calls %s)' % (
                c['kind'], c['callee'][:40], c['img'], '/'.join('-' if x is None else str(x) for x in c['ours']),
                '/'.join(str(x) for x in c['probe_calls'])))
            for k, v in c['sites'].items():
                print('          %-5s %s' % (k, v))
            if c['called_sites']:
                print('          image-called sites ours expands (site:first probe that flips it): ' + ' '.join(c['called_sites']))
        for cal, reps in r.get('repeats', {}).items():
            for line, text_, hits in reps:
                print('  site %s:%d  %s' % (cal, line, text_[:90]))
                seen = set()
                for w, path, ln in hits:
                    if (path, ln) in seen:
                        continue
                    seen.add((path, ln))
                    print('      x%d %s:%d  in %s' % (w, path, ln, owner_of(path, None, ln - 1)))
                    if len(seen) >= 8:
                        break


if __name__ == '__main__':
    main()
