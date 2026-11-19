#!/usr/bin/env python3
"""workarounds.py [--feall F] [--kind K,..] [--file SUBSTR] [--json F] [--table] [--summary]

Stand-in debt inventory (AGENTS.md "Byte-exact first, workarounds allowed"; MATCHING_GUIDE 4.x
"Stand-in debt"). Pure text screen over src/ plus an fe whole-tree result for exactness.

Items, one per source line:
  WORKAROUND(<k>)  a tag (comment or trailing); kind `wa:<k>`. A tag at file scope serves the
                   functions that use the identifier declared on the next code line (or the
                   addresses its comment names); a tag inside a body serves that function.
  probe-def        definition of a budget probe: a static/inline function whose name says
                   budget/probe/spend (`*_budget_probe*`, `*_spend`, `*_budget`, `*_probe<N>`,
                   `picker_spend`, `bb_probe`, ...) and that has no ORIGINAL marker.
  probe-call       a line inside a marked body that calls a probe (count = calls on the line).
  fnptr            a function-pointer local initialised with a function (`range_call`, `project_call`):
                   an unused one is a pass-1 spend, a used one a call forcer (MATCHING_GUIDE 2.8).
  pragma           `#pragma auto_inline|inline_depth|optimize` (region open and close counted apart).
  pads             CANARY-PADS / HEADER-PADS block (count stand-ins for the front-end symbol counter);
                   served = the `// CANARY:` lines above it plus addresses in its comment.
  volatile         a `volatile` cast or local inside a marked body (forced reload stand-in);
                   declarations in headers and uv/ thread code are left out.
  forceinline      `__forceinline` on a definition.

Served functions are ORIGINAL markers; `x` = fe-exact now (from --feall, default
recovery/tools/feall.json; run `feall.py --all --out F` first for a fresh one), `-` = not exact,
`?` = not measured. Evidence = the comment block the item sits under (first 160 chars).
"""
import os, re, sys, json, glob, argparse, collections

HERE = os.path.dirname(os.path.abspath(__file__))
R = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))

MARK = re.compile(r'// ORIGINAL: 0x([0-9A-F]{8}) (\S+) 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)')
TAG = re.compile(r'WORKAROUND\((\w+)\)')
PROBE_NAME = re.compile(r'^(?:\w*budget_probe\w*|\w+_spend\d*|\w*_budget\d*|\w+_probe\d*(?:_in)?|bb_probe|dw_probe|'
                        r'picker_spend|\w+_budget_step|\w+_probe_\w+)$')
_DEF = re.compile(r'^\s*([A-Za-z_][\w:<>\s\*&]*?[\s\*&:])([~\w]+)\s*\(')


class _Def:
    """match(line) -> object with group(1) = the defined name, or None (a type then a name then `(`)."""
    @staticmethod
    def match(line):
        if '(' not in line:
            return None
        m = _DEF.match(line)
        if not m or not m.group(1).strip() or m.group(1).strip() in ('return', 'else', 'case', 'new', 'delete', 'throw'):
            return None
        if re.search(r'\b(return|if|while|for|switch|sizeof)\s*$', m.group(1)):
            return None
        n = m.group(2)
        return type('M', (), {'group': staticmethod(lambda k, n=n: n if k == 1 else None)})()


DEF = _Def
FNPTR = re.compile(r'\(\s*(?:__\w+\s+)?\*\s*(?:const\s+)?(\w+)\s*\)\s*\([^;]*\)\s*=\s*&?([\w:]+)\s*;')
PRAGMA = re.compile(r'^\s*#\s*pragma\s+(auto_inline|inline_depth|optimize)\s*\(([^)]*)\)')
PADS = re.compile(r'//\s*(CANARY-PADS|HEADER-PADS):\s*(\w+)\s+(\w+)')
ADDR = re.compile(r'0x00([4-6][0-9A-Fa-f]{5})\b')
REAL_NAMES = {'weapon_budget', 'armor_budget', '__alloca_probe'}


def strip(text):
    """Blank comments, strings and chars, keeping newlines and offsets."""
    out = []
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if text.startswith('//', i):
            j = text.find('\n', i)
            j = n if j < 0 else j
            out.append(' ' * (j - i)); i = j
        elif text.startswith('/*', i):
            j = text.find('*/', i + 2)
            j = n if j < 0 else j + 2
            out.append(re.sub(r'[^\n]', ' ', text[i:j])); i = j
        elif c in '"\'':
            j = i + 1
            while j < n and text[j] != c and text[j] != '\n':
                j += 2 if text[j] == '\\' else 1
            j = min(j + 1, n)
            out.append(c + ' ' * (j - i - 2) + (c if j - i >= 2 else '')); i = j
        else:
            out.append(c); i += 1
    return ''.join(out)


class Fn:
    __slots__ = ('lo', 'hi', 'marks', 'name')


def functions(code_lines):
    """Top-level brace blocks that are function bodies: (lo, hi) 0-based line ranges.
    namespace/extern "C"/class/struct/enum blocks are descended into."""
    blocks = []
    stack = []  # (line, is_container)
    header = []
    for ln, line in enumerate(code_lines):
        for ch in line:
            if ch == '{':
                hdr = ''.join(header[-400:])
                container = bool(re.search(r'\b(namespace|extern\s*$|class|struct|union|enum)\b[^;{}()]*$', hdr)) \
                    or re.search(r'extern\s+"\s*"\s*$', hdr) is not None
                depth_fn = any(f for _, _, f in stack)
                if not depth_fn and not container and re.search(r'\)\s*(const\s*)?(throw\s*\([^)]*\)\s*)?$|\)\s*:[^;{}]*$', hdr):
                    stack.append((ln, False, True))
                else:
                    stack.append((ln, container, False))
                header = []
            elif ch == '}':
                if stack:
                    s, c, isfn = stack.pop()
                    if isfn:
                        blocks.append((s, ln))
                header = []
            elif ch == ';':
                header = []
            else:
                header.append(ch)
        header.append(' ')
    return blocks


def base_name(sym):
    """Unqualified source name of a (possibly mangled) symbol."""
    if sym.startswith('??1'):
        return '~' + sym[3:].split('@')[0]
    if sym.startswith('??0'):
        return sym[3:].split('@')[0]
    if sym.startswith('??'):
        return ''
    if sym.startswith('?'):
        return sym[1:].split('@')[0]
    return sym.lstrip('_').split('@')[0]


def scan_file(path, rel, exact):
    text = open(path, encoding='latin-1').read()
    lines = text.split('\n')
    code = strip(text).split('\n')
    marks = [(i, m.group(1), m.group(2), 'BYTE_EXACT' in m.group(3)) for i, l in enumerate(lines)
             for m in [MARK.search(l)] if m]
    blocks = functions(code)
    fns = []
    for lo, hi in sorted(blocks):
        f = Fn(); f.lo, f.hi = lo, hi
        f.marks = []
        f.name = ' '.join(code[max(0, lo - 8):lo + 1])
        fns.append(f)
    # a marker belongs to the first block after it whose header names the function (the marker's
    # symbol, or the `// symbol` line under it); else to the first block after it
    for k, (i, a, s, c) in enumerate(marks):
        nxt = marks[k + 1][0] if k + 1 < len(marks) else len(lines)
        names = {base_name(s)}
        for j in range(i + 1, min(i + 8, len(lines))):
            sm = re.match(r'\s*// symbol\s+(\S+)', lines[j])
            if sm:
                names.add(base_name(sm.group(1)))
        after = [f for f in fns if f.lo > i]
        named = [f for f in fns if any(n and re.search(r'(?<![\w~])' + re.escape(n) + r'\s*\(', f.name) for n in names)]
        pick = next((f for f in named if i < f.lo < nxt + 40), None)
        if pick is None and named:
            pick = min(named, key=lambda f: abs(f.lo - i))
        if pick is None:  # helpers sit between a marker and its function: take the last block before the next marker
            between = [f for f in after if f.lo < nxt]
            pick = between[-1] if between else None
        if pick is not None:
            pick.marks.append((a, s, c))
    line_fn = {}
    for f in fns:
        if f.marks:
            for k in range(f.lo, f.hi + 1):
                line_fn[k] = f
    return text, lines, code, marks, fns, line_fn


def comment_block(lines, i):
    """Comment lines ending at i (inclusive, if a comment) or directly above i."""
    j = i
    if not lines[j].strip().startswith('//'):
        j -= 1
    out = []
    while j >= 0 and lines[j].strip().startswith('//') and not MARK.search(lines[j]) \
            and not lines[j].strip().startswith('// RECOVERY_') and not lines[j].strip().startswith('// symbol'):
        out.append(lines[j].strip()[2:].strip()); j -= 1
    return ' '.join(reversed(out))


def next_code(lines, code, i):
    j = i + 1
    while j < len(lines) and (not code[j].strip()):
        j += 1
    return j if j < len(lines) else None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--feall', default=os.path.join(R, 'recovery/tools/feall.json'))
    ap.add_argument('--kind')
    ap.add_argument('--file')
    ap.add_argument('--json')
    ap.add_argument('--table', action='store_true', help='print every item')
    ap.add_argument('--summary', action='store_true', help='counts only')
    ap.add_argument('--root', default=R)
    a = ap.parse_args()
    exact = {}
    try:
        for r in json.load(open(a.feall)):
            if r.get('diff', -1) >= 0:
                exact[r['addr']] = exact.get(r['addr'], False) or r['exact']
    except Exception as e:
        print('no feall (%s): exactness unknown' % e, file=sys.stderr)
    paths = sorted(glob.glob(a.root + '/src/**/*.cpp', recursive=True) + glob.glob(a.root + '/src/**/*.h', recursive=True))
    files = {}
    allmarks = {}
    for p in paths:
        rel = os.path.relpath(p, a.root)
        files[rel] = scan_file(p, rel, exact)
        for (_, ad, s, c) in files[rel][3]:
            allmarks[ad] = (s, rel)
    marked = {s.split('@')[0].lstrip('?') for s, _ in allmarks.values()}
    # probe names: definitions without a marker whose name says probe/budget/spend
    probes = set()
    for rel, (text, lines, code, marks, fns, line_fn) in files.items():
        for i, l in enumerate(code):
            m = DEF.match(l)
            if not m:
                continue
            n = m.group(1) or m.group(2)
            if n and PROBE_NAME.match(n) and n not in REAL_NAMES and n not in marked and i not in line_fn:
                probes.add(n)
    # usage index: identifier -> set of marker addrs whose body uses it
    use = collections.defaultdict(set)
    for rel, (text, lines, code, marks, fns, line_fn) in files.items():
        for f in fns:
            if not f.marks:
                continue
            body = '\n'.join(code[f.lo:f.hi + 1])
            for w in set(re.findall(r'\b[A-Za-z_]\w*\b', body)):
                for (ad, s, c) in f.marks:
                    use[w].add(ad)
    # macros that expand to probe calls are probes too
    for rel, (text, lines, code, marks, fns, line_fn) in files.items():
        for i, l in enumerate(lines):
            m = re.match(r'\s*#\s*define\s+(\w+)', l)
            if m and probes:
                k, body = i, l
                while lines[k].rstrip().endswith('\\') and k + 1 < len(lines):
                    k += 1; body += lines[k]
                if re.search(r'\b(' + '|'.join(probes) + r')\s*\(', body):
                    probes.add(m.group(1))
    probe_re = re.compile(r'\b(' + '|'.join(sorted(probes, key=len, reverse=True)) + r')\b\s*\(?') if probes else None

    items = []

    def served_of(rel, i, ident=None, comment=''):
        f = files[rel][5].get(i)
        if f:
            return [ad for ad, _, _ in f.marks], 'body'
        s = []
        if ident:
            s = sorted(use.get(ident, ()))
        for m in ADDR.finditer(comment):
            ad = ('00' + m.group(1)).upper()
            if ad in allmarks and ad not in s:
                s.append(ad)
        return s, 'scope'

    def add(rel, i, kind, served, how, evidence, subject=''):
        items.append(dict(file=rel, line=i + 1, kind=kind, subject=subject, served=served, how=how,
                          evidence=re.sub(r'\s+', ' ', evidence)[:160]))

    for rel, (text, lines, code, marks, fns, line_fn) in files.items():
        is_h = rel.endswith('.h')
        for i, l in enumerate(lines):
            c = code[i]
            # WORKAROUND tags
            for m in TAG.finditer(l):
                kind = 'wa:' + m.group(1)
                trailing = bool(c.strip())
                ev = l.split('//', 1)[1] if '//' in l else l
                ev = ev.strip() if trailing else comment_block(lines, i)
                subj = ''
                if trailing and ((probe_re and probe_re.search(c)) or FNPTR.search(c)):
                    continue  # counted as a probe-call / fnptr item
                if trailing:
                    subj = (re.findall(r'\b(\w+)\s*\(', c) or re.findall(r'\b(\w+)\b', c) or [''])[0]
                    served, how = served_of(rel, i, subj, ev)
                else:
                    # skip tags inside a continuing comment block: one item per block
                    if i > 0 and TAG.search(lines[i - 1]) and lines[i - 1].strip().startswith('//') and \
                            TAG.search(lines[i - 1]).group(1) == m.group(1):
                        continue
                    j = i
                    while j + 1 < len(lines) and lines[j + 1].strip().startswith('//'):
                        j += 1
                    ev = ' '.join(x.strip()[2:].strip() for x in lines[i:j + 1])
                    nj = next_code(lines, code, j)
                    if nj is not None and nj - j <= 2:
                        cl = code[nj]
                        dm = DEF.match(cl) or None
                        fp = FNPTR.search(cl)
                        if fp:
                            subj = fp.group(1)
                        elif dm:
                            subj = dm.group(1) or dm.group(2)
                        else:
                            w = re.findall(r'\b([A-Za-z_]\w*)\s*(?:\[[^\]]*\])?\s*(?:=[^=]|;|\(|$)', cl)
                            subj = w[0] if w else ''
                    served, how = served_of(rel, i, subj, ev)
                add(rel, i, kind, served, how, ev, subj)
            if not c.strip():
                continue
            # probe definitions and calls
            m = DEF.match(c)
            if m and (m.group(1) or m.group(2)) in probes and i not in line_fn:
                n = m.group(1) or m.group(2)
                served, how = served_of(rel, i, n, comment_block(lines, i))
                add(rel, i, 'probe-def', served, how, comment_block(lines, i), n)
            elif probe_re and i in line_fn:
                calls = probe_re.findall(c)
                if calls:
                    served, how = served_of(rel, i)
                    add(rel, i, 'probe-call', served, how, comment_block(lines, i) or (l.split('//', 1)[1] if '//' in l else ''),
                        '%s x%d' % (calls[0], len(calls)))
            fp = FNPTR.search(c)
            if fp and i in line_fn:
                served, how = served_of(rel, i)
                add(rel, i, 'fnptr', served, how, comment_block(lines, i), '%s=%s' % fp.groups())
            pm = PRAGMA.match(c)
            if pm:
                arg = pm.group(2).strip()
                state = 'close' if arg in ('on', '"", on', '8') or arg.endswith('on') and pm.group(1) == 'auto_inline' else 'open'
                if pm.group(1) == 'optimize' and arg.replace(' ', '') == '"",on':
                    state = 'close'
                if pm.group(1) == 'inline_depth' and arg in ('8', ''):
                    state = 'close'
                # served = functions defined until the matching close
                served = []
                if state == 'open':
                    for k in range(i + 1, len(lines)):
                        pk = PRAGMA.match(code[k])
                        if pk and pk.group(1) == pm.group(1):
                            break
                        f = line_fn.get(k)
                        if f:
                            served += [ad for ad, _, _ in f.marks if ad not in served]
                add(rel, i, 'pragma-' + state, served, 'region', comment_block(lines, i), '%s(%s)' % (pm.group(1), arg))
            if '__forceinline' in c:
                dm = re.search(r'\b(\w+)\s*\(', c[c.find('__forceinline'):])
                n = dm.group(1) if dm else ''
                served, how = served_of(rel, i, n, comment_block(lines, i))
                add(rel, i, 'forceinline', served, how, comment_block(lines, i), n)
            if re.search(r'\bvolatile\b', c) and i in line_fn and not rel.startswith('src/uv/') and not is_h:
                served, how = served_of(rel, i)
                tagged = 'WORKAROUND' in l or 'WORKAROUND(volatile)' in comment_block(lines, i)
                if not tagged:  # a tag item already lists tagged lines
                    add(rel, i, 'volatile', served, how, comment_block(lines, i) or c.strip(), c.strip()[:60])
        # pad blocks
        for i, l in enumerate(lines):
            m = PADS.search(l)
            if not m:
                continue
            n = 0
            k = i + 1
            while k < len(lines) and m.group(3) in lines[k]:
                n += 1; k += 1
            canaries = []
            j = i - 1
            while j >= 0 and lines[j].strip().startswith('//'):
                cm = re.search(r'CANARY: 0x([0-9A-F]{8})', lines[j])
                if cm:
                    canaries.append(cm.group(1))
                j -= 1
            ev = comment_block(lines, i - 1) if i else ''
            for am in ADDR.finditer(ev):
                ad = ('00' + am.group(1)).upper()
                if ad in allmarks and ad not in canaries:
                    canaries.append(ad)
            add(rel, i, 'pads:' + m.group(1).split('-')[0].lower(), canaries, 'canary', ev, '%s %s x%d' % (m.group(2), m.group(3), n))

    if a.kind:
        ks = a.kind.split(',')
        items = [x for x in items if any(x['kind'].startswith(k) for k in ks)]
    if a.file:
        items = [x for x in items if a.file in x['file']]

    def st(ad):
        return 'x' if exact.get(ad) is True else '-' if ad in exact else '?'

    for x in items:
        x['served'] = [dict(addr=ad, sym=allmarks.get(ad, ('?', ''))[0], exact=st(ad)) for ad in x['served']]
        e = [s['exact'] for s in x['served']]
        x['wide'] = x['how'] == 'scope' and len(x['served']) > 25
        x['status'] = 'none' if not e else 'all-exact' if all(v == 'x' for v in e) else 'none-exact' if all(v != 'x' for v in e) else 'mixed'
    if a.json:
        json.dump(items, open(a.json, 'w'), indent=1)

    def short(sym):
        m = re.match(r'\?+([^@]+)@([^@]+)@@', sym)
        if m and not m.group(2)[0].isupper() is False and not sym.startswith('??'):
            pass
        if sym.startswith('?'):
            parts = sym.lstrip('?').split('@@')[0].split('@')
            return '::'.join(reversed(parts)) if len(parts) > 1 and parts[1] else parts[0]
        return sym

    if a.table:
        for x in items:
            sv = ' '.join('%s%s' % (short(s['sym'])[:28], s['exact']) for s in x['served'][:4])
            if len(x['served']) > 4:
                sv += ' +%d' % (len(x['served']) - 4)
            if x['wide']:
                sv = '(%d users, %d exact)' % (len(x['served']), sum(s['exact'] == 'x' for s in x['served']))
            print('%s:%d\t%s\t%s\t[%s]\t%s\t| %s' % (x['file'], x['line'], x['kind'], x['subject'][:30],
                                                     'wide' if x['wide'] else x['status'], sv, x['evidence'][:100]))
    cnt = collections.Counter(x['kind'] for x in items)
    st_ = collections.Counter((x['kind'], x['status']) for x in items)
    calls = sum(int(x['subject'].rsplit('x', 1)[1]) for x in items if x['kind'] == 'probe-call')
    print('%-18s %6s %9s %6s %10s %5s' % ('kind', 'items', 'all-exact', 'mixed', 'none-exact', 'none'))
    for k in sorted(cnt):
        print('%-18s %6d %9d %6d %10d %5d' % (k, cnt[k], st_[(k, 'all-exact')], st_[(k, 'mixed')], st_[(k, 'none-exact')], st_[(k, 'none')]))
    print('probe names %d, probe call sites %d (lines %d); files %d' % (len(probes), calls, cnt['probe-call'],
                                                                       len({x['file'] for x in items})))
    print('wide items (a header/type stand-in whose identifier more than 25 functions use): %d' % sum(x['wide'] for x in items))
    fn = collections.Counter(s['addr'] for x in items if not x['kind'].startswith('pads') and not x['wide'] for s in x['served'])
    print('functions served by a non-pad, non-wide stand-in: %d (%d exact now)' % (len(fn), sum(1 for ad in fn if exact.get(ad) is True)))
    pf = collections.defaultdict(int)
    for x in items:
        if x['kind'] in ('probe-call', 'fnptr'):
            for sv in x['served']:
                pf[(sv['addr'], sv['sym'], sv['exact'])] += int(x['subject'].rsplit('x', 1)[1]) if x['kind'] == 'probe-call' else 1
    print('functions carrying probe calls / function-pointer stand-ins: %d (%d exact now)' % (len(pf), sum(1 for k in pf if k[2] == 'x')))
    if a.table:
        for (ad, sym, e), n in sorted(pf.items(), key=lambda t: -t[1]):
            print('  %s %s %-40s %d' % (e, ad, short(sym)[:40], n))


if __name__ == '__main__':
    main()
