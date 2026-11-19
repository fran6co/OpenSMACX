#!/usr/bin/env python3
"""declpad.py top  OUT.jsonl [unit ...]             symbol-number sweep at the top of each unit
declpad.py report OUT.jsonl [--max 60] [--delta]     what the top sweep found
declpad.py syms <unit> <addr>                        symbol numbers of the globals a function references
declpad.py at <unit> <addr[,addr]> [--only NAME,..] [--full] [--extra file:line,..]
                                                     sweep before and after each referenced global's declaration
declpad.py padfree PF.jsonl [unit ...]               the top sweep with the unit's CANARY-PADS / HEADER-PADS blocks emptied
declpad.py common OUT.jsonl [PF.jsonl ...]           is there ONE shift for every unit (a count missing from a shared prefix)?
declpad.py who OUT.jsonl WHO.jsonl [--max 60]        `at` for every function the top sweep moves

MATCHING_GUIDE 2.5 "Bucket order of referenced symbols".  VC6's front end numbers every symbol of the unit from one
counter (the `$SG` number).  Some load-order and operand-order ties follow the numbers, modulo 64, of the file-scope
symbols the function references: a header global has the number the counter had at its extern, a literal the number it
had at the function.  So a tie on header globals follows the declaration counts before each of them, which pads behind
the includes never move.
`top` puts n = 0..63 prototypes (one symbol each) before the first line of the unit: every symbol shifts by n, so each
pair of symbols whose numbers differ modulo 64 changes bucket order for part of the sweep; a function that never changes
has no such tie.  One compile per n evaluates every marker of the unit.
`at` places the pads immediately before, and immediately after, the extern declaration of each referenced global in a
shadow copy of src/ (build/recovery/shadow/<prefix>, as outside.py; the worktree's headers are never touched): the
symbols declared before that place keep their numbers, the later ones shift.  It bisects on the pattern, so only the
declarations the tie follows are swept; the pattern is flat after the last of them.
Screens only.  FE_PFX_BASE / FE_JOBS lanes."""
import sys, os, re, json, threading, collections, subprocess
R = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
sys.path.insert(0, R + '/recovery/tools'); sys.path.insert(0, R + '/recovery/tools/analysis')
import fe
from concurrent.futures import ThreadPoolExecutor

BASE = int(os.environ.get('FE_PFX_BASE', '21')); JOBS = int(os.environ.get('FE_JOBS', '5'))
SCR = R + '/build/recovery/declpad'
PERIOD = 64
STEP = int(os.environ.get('DECLPAD_STEP', '1'))   # `at`/`who` sample every STEP-th pad count


def pads(n, tag='DeclPad'):
    return ''.join('void %s%d();' % (tag, i) for i in range(n))


def prefix(p, text):
    """pads before the first line without moving line numbers (a preprocessor first line gets `#line 1`)"""
    return p + ('\n#line 1\n' if text.lstrip().startswith('#') and p else '') + text


def markers(text):
    return [(int(m.group(1), 16), 'BYTE_EXACT' in m.group(2)) for m in
            re.finditer(r'// ORIGINAL: 0x([0-9A-F]{8}) \S+ 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)', text)]


def rows_all(c, text, addrs):
    d = {}
    for a in addrs:
        try:
            r = fe.evaluate(c, text, a)
        except Exception:
            continue
        if 'error' not in r:
            d[a] = 0 if r['exact'] else max(1, r['diff'])
    return d


def first_sg(c):
    v = [int(s.name[3:]) for s in c.symbols.values() if re.fullmatch(r'\$SG\d+', s.name)]
    return min(v) if v else None


def units_all():
    cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
    us = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/') and '/vendor/' not in e['file']]
    return [u for u in us if os.path.exists(R + '/' + u)]


def top(out, units, transform=None):
    os.makedirs(SCR, exist_ok=True)
    done = set()
    if os.path.exists(out):
        for l in open(out):
            try:
                done.add(json.loads(l)['unit'])
            except Exception:
                pass
    units = [u for u in (units or units_all()) if u not in done]
    fe.compile_cmd('src/main.cpp')
    f = open(out, 'a'); lk = threading.Lock()

    def lane(k):
        for u in units[k::JOBS]:
            text = open(R + '/' + u, encoding='latin-1').read()
            if transform:
                text = transform(text)
            ms = markers(text)
            if not ms:
                continue
            addrs = [a for a, _ in ms]; res = {}; sg = {}
            p = '%s/top_%d.cpp' % (SCR, BASE + k)
            for n in range(PERIOD):
                open(p, 'w', encoding='latin-1').write(prefix(pads(n), text))
                c, err = fe.compile_unit(u, p, BASE + k)
                if c is None:
                    res[n] = None; continue
                res[n] = rows_all(c, text, addrs); sg[n] = first_sg(c)
            if res[0] is None:
                rec = dict(unit=u, error='compile')
            else:
                var = {}
                for a in addrs:
                    col = [(res[n] or {}).get(a, -1) for n in range(PERIOD)]
                    if len(set(col)) > 1:
                        var['%08X' % a] = col
                rec = dict(unit=u, padfree=bool(transform), base={'%08X' % a: v for a, v in res[0].items()}, claimed=['%08X' % a for a, cl in ms if cl],
                           sg0=sg.get(0), var=var)
            with lk:
                f.write(json.dumps(rec) + '\n'); f.flush()
                print(u, 'markers', len(addrs), 'sensitive', len(rec.get('var', {})), flush=True)
    with ThreadPoolExecutor(JOBS) as ex:
        list(ex.map(lane, range(JOBS)))


def names():
    """address -> short name, from every marker of the tree"""
    out = {}
    for u in units_all():
        for m in re.finditer(r'// ORIGINAL: 0x([0-9A-F]{8}) (\S+)', open(R + '/' + u, encoding='latin-1').read()):
            s = m.group(2); q = re.match(r'\?(\w+)@(?:(\w+)@)?@', s)
            out[m.group(1)] = ('%s::%s' % (q.group(2), q.group(1)) if q.group(2) else q.group(1)) if q else s[:30]
    return out


def runs(col, pred):
    """cyclic runs of n where pred(col[n]) -> 'a-b,c' text"""
    ok = [pred(x) for x in col]; N = len(col)
    if all(ok):
        return 'all'
    if not any(ok):
        return '-'
    s = next(i for i in range(N) if ok[i] and not ok[i - 1]); out = []; i = s
    while True:
        if ok[i % N]:
            j = i
            while ok[(j + 1) % N] and (j + 1 - s) < N:
                j += 1
            out.append('%d' % (i % N) if j == i else '%d-%d' % (i % N, j % N)); i = j + 1
        else:
            i += 1
        if i - s >= N:
            break
    return ','.join(out)


def report(out, maxrows, delta):
    nm = names(); recs = [json.loads(l) for l in open(out)]
    recs = [r for r in recs if 'var' in r]
    tot = [0] * PERIOD; base_exact = 0; nfun = 0
    print('== functions the top sweep moves (rows at n=0; exact n; best rows and its n) ==')
    for r in sorted(recs, key=lambda r: r['unit']):
        nfun += len(r['base']); base_exact += sum(1 for v in r['base'].values() if v == 0)
        for n in range(PERIOD):
            tot[n] += sum(1 for a, v in r['base'].items() if (r['var'][a][n] if a in r['var'] else v) == 0)
        for a, col in sorted(r['var'].items()):
            b = col[0]; cl = a in r['claimed']
            if b > maxrows and not cl and min(x for x in col if x >= 0) != 0:
                continue
            best = min(x for x in col if x >= 0)
            print('%s %-34s %-28s %s base %3d  exact n: %-22s best %d at %s' % (
                a, nm.get(a, '?')[:34], os.path.basename(r['unit']), 'C' if cl else ' ', b, runs(col, lambda x: x == 0),
                best, runs(col, lambda x: x == best) if best else ''))
    print('markers', nfun, 'fe-exact at n=0', base_exact)
    if delta:
        print('== one delta at the top of every unit: exact markers per n ==')
        for n in range(PERIOD):
            print('  n=%2d exact %d (%+d)' % (n, tot[n], tot[n] - tot[0]))


# ---- symbol numbers of referenced globals -----------------------------------------------------------------
def referenced(c, sym):
    """decorated names of the data symbols the function's COMDAT relocates against, and its $SG numbers"""
    s = c.function(sym); sec = c.sections[s.section - 1]; data = []; sg = []; funcs = []
    for r in sec.relocations:
        n = c.symbols[r[1]].name
        if re.fullmatch(r'\$SG\d+', n):
            sg.append(int(n[3:]))
        elif re.match(r'\?\w+@@3', n) or re.match(r'\?\w+@\w+@@2', n) or re.fullmatch(r'_[A-Za-z]\w*', n):
            n in data or data.append(n)
        elif not n.startswith(('$', '__imp_', '.')):
            n in funcs or funcs.append(n)
    return data, sorted(set(sg)), funcs


def plain(n):
    m = re.match(r'\?(\w+)@(?:(\w+)@)?@', n)
    return m.group(1) if m else n[1:]


def decl_sites(name, files):
    """(rel, line index) of the declarations of a global: extern lines first, else a depth-0 definition"""
    out = []
    rx = re.compile(r'^[ \t]*(?:extern\b[^;()\n]*?[\s\*&]|(?:static\s+|const\s+|volatile\s+)*[A-Za-z_][\w:<>]*[\s\*&]+(?:const\s+)?)%s\b\s*(?:\[[^\]\n]*\]\s*)*(?:=|;|,)' % re.escape(name), re.M)
    for rel, t in files.items():
        for m in rx.finditer(t):
            ls = t.rfind('\n', 0, m.start()) + 1
            pre = t[:ls]
            depth = pre.count('{') - pre.count('}')
            ext = 'extern' in m.group(0)
            if not ext and (depth != 0 or rel.endswith('.h')):
                continue
            prev = re.sub(r'//[^\n]*', '', pre).rstrip()
            if prev and prev[-1] not in ';}{' and not prev.endswith('*/') and not prev.split('\n')[-1].lstrip().startswith('#'):
                continue   # a continuation line
            out.append((rel, t.count('\n', 0, ls), ext))
    return out


def unit_files(unit):
    import outside
    hs = outside.load_headers(); text = open(R + '/' + unit, encoding='latin-1').read()
    inc = outside.closure(unit, text, hs)
    files = {k: v for k, v in hs.items() if k in inc}; files[unit] = text
    return files


def insert(files, edits):
    """edits: [(rel, line index, mode, text)] -> overlay {rel: text}.  mode 'pre' prepends to the line, 'post' puts
    the text after the line's first `;`, 'top' before the first line of the file"""
    ov = {}
    for rel in {e[0] for e in edits}:
        L = files[rel].split('\n'); head = ''
        for _, i, mode, tx in [e for e in edits if e[0] == rel]:
            if mode == 'top':
                head += tx
            elif mode == 'post':
                j = L[i].index(';') + 1
                L[i] = L[i][:j] + tx + L[i][j:]
            else:
                L[i] = tx + L[i]
        ov[rel] = prefix(head, '\n'.join(L))
    return ov


def after_includes(text):
    """line index of the first blank or `//` line after the unit's last #include"""
    L = text.split('\n')
    last = max(i for i, l in enumerate(L[:4000]) if re.match(r'\s*#\s*include\b', l))
    for i in range(last + 1, len(L)):
        s = L[i].strip()
        if s == '' or s.startswith('//'):
            return i
    return last + 1


def positions(unit, addrs, k=None):
    """-> (positions [(counter, label, rel, line index, mode)] in declaration order, files, notes).  A position is a
    place pads can go: the top of the unit, before and after the declaration of each global the functions reference,
    and behind the includes.  `counter` is the front end's symbol number at that place (the number the next declared
    symbol takes): a `char *p = "zq<i>";` probe there takes two numbers, the pointer and then its literal `$SG<n>`."""
    import outside
    k = BASE if k is None else k
    files = unit_files(unit); text = files[unit]
    outside.shadow(k)
    c, _ = outside.compile_shadow(unit, {}, k, str(k))
    if c is None:
        raise SystemExit('compile failed: %s' % unit)
    data = []; notes = []
    for a in addrs:
        d, sg, funcs = referenced(c, fe.ident(text, a)[0])
        data += [n for n in d if n not in data]
        if sg:
            notes.append('%08X literals $SG %s' % (a, ' '.join('%d(%d)' % (x, x % 64) for x in sg[:6])))
    pos = [('(top of unit)', unit, 0, 'top')]; missing = []
    for n in data:
        ds = decl_sites(plain(n), files)
        ds = [d for d in ds if d[2]] or ds
        if not ds:
            n.startswith('?') and missing.append(plain(n))
            continue
        for rel, li, ext in ds:
            pos.append((plain(n), rel, li, 'pre'))
            if ';' in files[rel].split('\n')[li]:
                pos.append((plain(n) + '+', rel, li, 'post'))
    pos.append(('(behind the includes)', unit, after_includes(text), 'pre'))
    if missing:
        notes.append('no declaration found: ' + ' '.join(missing))
    ov = insert(files, [(rel, li, mode, 'char *DeclPadProbe%d = "zq%d";' % (i, i)) for i, (lab, rel, li, mode) in enumerate(pos)])
    c2, _ = outside.compile_shadow(unit, ov, k, str(k))
    if c2 is None:
        raise SystemExit('probe compile failed for %s' % unit)
    got = {}
    for s in c2.symbols.values():
        if re.fullmatch(r'\$SG\d+', s.name) and s.section > 0:
            d = c2.sections[s.section - 1].data[s.value:s.value + 12]
            if d.startswith(b'zq') and b'\0' in d:
                got[int(d[2:d.index(b'\0')])] = int(s.name[3:])
    rank = {i: r for r, i in enumerate(sorted(got, key=lambda i: got[i]))}
    out = []
    for i, (lab, rel, li, mode) in enumerate(pos):
        if i in got:
            out.append((got[i] - 1 - 2 * rank[i], lab, rel, li, mode))
    out.sort(key=lambda x: (x[0], x[1].endswith('+')))
    seen = set(); keep = []
    for x in out:   # a global declared twice: the first declaration numbers it
        if x[1] in seen:
            continue
        seen.add(x[1]); keep.append(x)
    return keep, files, notes


def syms(unit, addr):
    pos, files, notes = positions(unit, [addr])
    for cnt, lab, rel, li, mode in pos:
        if not lab.endswith('+'):
            print('%7d  mod64 %2d  %-28s %s:%d' % (cnt, cnt % 64, lab, rel, li + 1))
    print('\n'.join(notes))


def glyph(col):
    return ''.join('.' if x == 0 else '?' if x < 0 else str(x) if x < 10 else '#' for x in col)


def at(unit, addrs, only=None, full=False, extra=(), quiet=False):
    """Sweeps n = 0..63 pads at positions between the top of the unit and the end of its includes, bisecting: a
    stretch is split while the patterns at its two ends differ, so only the declarations a tie follows are swept.
    -> [(addr, position, rows per n)] for the positions swept"""
    import outside
    pos, files, notes = positions(unit, addrs)
    for lab, rel, li in extra:
        pos.append((-1, lab, rel, li, 'pre'))
    if only:
        pos = [x for x in pos if x[1].rstrip('+') in only or x[1].startswith('(')]
    text = files[unit]
    for k in range(JOBS):
        outside.shadow(BASE + k)
    lk = threading.Lock(); ctr = [0]; tls = threading.local(); ex = ThreadPoolExecutor(JOBS); P = {}

    def job(j):
        i, n = j
        if not hasattr(tls, 'k'):
            with lk:
                tls.k = BASE + ctr[0]; ctr[0] += 1
        x = pos[i]
        ov = insert(files, [(x[2], x[3], x[4], pads(n))]) if n else {}
        c, _ = outside.compile_shadow(unit, ov, tls.k, str(tls.k))
        return rows_all(c, text, addrs) if c is not None else {}

    def pattern(i):
        if i not in P:
            ds = list(ex.map(job, [(i, n) for n in range(0, PERIOD, STEP)]))
            P[i] = tuple(tuple(d.get(a, -1) for d in ds) for a in addrs)
        return P[i]

    def split(lo, hi):
        if hi - lo < 2 or pattern(lo) == pattern(hi):
            return
        mid = (lo + hi) // 2
        pattern(mid); split(lo, mid); split(mid, hi)
    if full:
        for i in range(len(pos)):
            pattern(i)
    else:
        split(0, len(pos) - 1)
    res = []
    quiet or print('\n'.join(notes))
    for ai, a in enumerate(addrs):
        quiet or print('== %08X %s: rows per n = 0..63%s pads at each position (. exact, digit rows, # >= 10)' % (a, unit, '' if STEP == 1 else ' step %d' % STEP))
        prev = None
        for i in sorted(P):
            x = pos[i]; col = P[i][ai]
            mark = ' ' if prev is None or prev == col else '*'
            quiet or print('%s%7d mod64 %2d %-26s %-30s %s' % (mark, x[0], x[0] % 64, x[1][:26], '%s:%d' % (os.path.basename(x[2]), x[3] + 1), glyph(col)))
            res.append((a, x, col)); prev = col
        quiet or print('  (* = the pattern differs from the previous listed position: a symbol of the tie is declared between them)')
    return res


def who(src, out, maxrows):
    """`at` for every function the top sweep moves (claimed markers, or rows at n = 0 up to maxrows); appends
    {unit, addr, positions: [[counter, label, file:line, rows per n]]} lines to out"""
    done = set()
    if os.path.exists(out):
        done = {json.loads(l)['unit'] for l in open(out)}
    f = open(out, 'a')
    for l in open(src):
        r = json.loads(l)
        if 'var' not in r or r['unit'] in done:
            continue
        addrs = [int(a, 16) for a, col in sorted(r['var'].items()) if a in r['claimed'] or col[0] <= maxrows or min(x for x in col if x >= 0) == 0]
        if not addrs:
            continue
        try:
            res = at(r['unit'], addrs)
        except SystemExit as e:
            print('SKIP', r['unit'], e); continue
        by = collections.OrderedDict()
        for a, x, col in res:
            by.setdefault(a, []).append([x[0], x[1], '%s:%d' % (x[2], x[3] + 1), list(col)])
        for a, ps in by.items():
            f.write(json.dumps(dict(unit=r['unit'], addr='%08X' % a, positions=ps)) + '\n')
        f.flush()


PADBLOCK = re.compile(r'(// (?:CANARY|HEADER)-PADS: \w+ (\w+)[^\n]*\n)((?:[^\n]*\b\2\d+\b[^\n]*\n)*)')


def strip_pads(text):
    """the unit without the lines of its CANARY-PADS / HEADER-PADS blocks"""
    return PADBLOCK.sub(lambda m: m.group(1), text)


def pad_units():
    return [u for u in units_all() if PADBLOCK.search(open(R + '/' + u, encoding='latin-1').read())]


def common(files):
    """one shift at the top of every unit: the n at which no marker that is exact now (pads in place) is lost.
    files: top-sweep outputs; a unit swept with its pad blocks emptied (padfree) is judged on that sweep, against the
    markers its ordinary record has exact."""
    ordn = {}; pf = {}
    for fn in files:
        for l in open(fn):
            r = json.loads(l)
            if 'var' in r:
                (pf if r.get('padfree') else ordn)[r['unit']] = r
    ok = set(range(PERIOD)); nm = names()
    for u in sorted(set(ordn) | set(pf)):
        r = pf.get(u) or ordn[u]; now = ordn.get(u, r)['base']
        need = [a for a, v in now.items() if v == 0 and (a in r['var'] or r['base'].get(a) != 0)]
        if not need:
            continue
        col = lambda a: r['var'].get(a) or [r['base'].get(a, -1)] * PERIOD
        good = {n for n in range(PERIOD) if all(col(a)[n] == 0 for a in need)}
        print('%-30s %s exact n: %-40s [%s]' % (u[4:], 'padfree' if u in pf else '       ', runs([0 if n in good else 1 for n in range(PERIOD)], lambda x: x == 0),
              ', '.join(nm.get(a, a) for a in need)))
        ok &= good
    print('common n over all units:', sorted(ok) or 'none')


if __name__ == '__main__':
    mode = sys.argv[1] if len(sys.argv) > 1 else ''
    if mode == 'top':
        top(sys.argv[2], sys.argv[3:])
    elif mode == 'report':
        mx = int(sys.argv[sys.argv.index('--max') + 1]) if '--max' in sys.argv else 60
        report(sys.argv[2], mx, '--delta' in sys.argv)
    elif mode == 'syms':
        syms(sys.argv[2], int(sys.argv[3], 16))
    elif mode == 'at':
        only = sys.argv[sys.argv.index('--only') + 1].split(',') if '--only' in sys.argv else None
        extra = []
        if '--extra' in sys.argv:   # rel:line[,rel:line]: further pad positions
            for x in sys.argv[sys.argv.index('--extra') + 1].split(','):
                rel, ln = x.rsplit(':', 1); extra.append(('@%s:%s' % (os.path.basename(rel), ln), rel, int(ln) - 1))
        at(sys.argv[2], [int(x, 16) for x in sys.argv[3].split(',')], only, '--full' in sys.argv, extra)
    elif mode == 'padfree':
        top(sys.argv[2], sys.argv[3:] or pad_units(), strip_pads)
    elif mode == 'common':
        common(sys.argv[2:])
    elif mode == 'who':
        who(sys.argv[2], sys.argv[3], int(sys.argv[sys.argv.index('--max') + 1]) if '--max' in sys.argv else 60)
    else:
        sys.exit(__doc__)
