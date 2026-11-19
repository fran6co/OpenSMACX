#!/usr/bin/env python3
"""Mechanical unit merge behind `tumap.py --merge A.cpp [B.cpp ...] --into C.cpp [--dry]` and
`tumap.py --move 0xADDR[,0xADDR] --into C.cpp [--dry]`.

A file is cut into top-level items (a lexer skips comments, strings and preprocessor lines and tracks the
brace depth; an item ends at a depth-0 `}` or `;`). The comments and blank lines before an item belong
to it. Items are glued into chunks: an item without an ORIGINAL marker belongs to the next item of its
file that has one (trailing unmarked items to the last). A chunk's key is its lowest ORIGINAL address.

--merge: every chunk of A, B, ... is inserted into C immediately after the chunk of C (or an already
inserted one) that holds its address predecessor, or at the top of C (after the last #include and the
CANARY-PADS block) when it precedes all of C. C's own order is not changed. --top puts the moved chunks, in address
order, at the top of C whatever C holds (C's own order is often not address order). The prelude of A (licence
comment, #include lines) is dropped except for includes C lacks, which are added after C's last
#include; any other prelude text stays with A's first chunk. The emptied files are deleted with
`git rm`, and CMakeLists.txt, recovery/project.json (measurement.unit_flag_sets) and every
`// compile_context A` annotation are rewritten to C.
--move: the same for single functions (the source file is kept; only the includes named with
--include '"x.h"' are added). When C.cpp does not exist, `--new-like L.cpp [--after P.cpp]` creates it: the licence
and #include lines of L, listed after P (default L) in OPENSMACX_SOURCES, L's per-source property commands and
project.json flag set duplicated (and L's entry in the worktree's build/compile_commands.json, for fe).
No body is edited. Warnings name what a person must check: unit-level #defines before the includes,
preprocessor lines inside moved chunks, flag sets that differ."""
import os, re, sys, json, subprocess
HERE = os.path.dirname(os.path.abspath(__file__))
R = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
ORIG = re.compile(r'// ORIGINAL: 0x([0-9A-F]{8}) ')


def items(t):
    """end offsets (just past the line) of the top-level items of C++ text t"""
    out = []
    i, n, depth, par = 0, len(t), 0, 0
    bol = True
    while i < n:
        c = t[i]
        if c == '\n':
            bol = True; i += 1; continue
        if bol and c in ' \t':
            i += 1; continue
        if bol and c == '#':
            # preprocessor line with continuations
            while i < n:
                j = t.find('\n', i)
                if j < 0: j = n
                cont = t[j - 1:j] == '\\'
                i = j + 1
                if not cont: break
            bol = True; continue
        bol = False
        if t.startswith('//', i):
            j = t.find('\n', i); i = n if j < 0 else j; continue
        if t.startswith('/*', i):
            j = t.find('*/', i + 2); i = n if j < 0 else j + 2; continue
        if c == '"' or c == "'":
            j = i + 1
            while j < n and t[j] != c:
                j += 2 if t[j] == '\\' else 1
            i = j + 1; continue
        if c == '(': par += 1
        elif c == ')': par -= 1
        elif c == '{': depth += 1
        elif c == '}':
            depth -= 1
            if depth == 0 and par == 0:
                j = i + 1
                while j < n and t[j] in ' \t': j += 1
                if j < n and t[j] == ';':
                    i = j
                else:
                    # `} name = {..};` / `} var;` continue the same item
                    k = j
                    while k < n and t[k] in ' \t\n': k += 1
                    if not (k < n and (t[k].isalpha() or t[k] in '_*=') and re.match(r'[\w\s\*\[\]=,]*[;{]', t[k:k + 200]) and t[i - 1] != '\n' and False):
                        e = t.find('\n', i); e = n if e < 0 else e + 1
                        out.append(e); i = e; bol = True; continue
        if c == ';' and depth == 0 and par == 0:
            e = t.find('\n', i); e = n if e < 0 else e + 1
            out.append(e); i = e; bol = True; continue
        i += 1
    return out


def prelude_end(t):
    """offset just past the last #include line and a CANARY-PADS / HEADER-PADS block that follows it"""
    last = 0
    for m in re.finditer(r'^[ \t]*#[ \t]*include[^\n]*\n', t, re.M):
        last = m.end()
    m = re.compile(r'(?:[ \t]*//[^\n]*\n)*[ \t]*// CANARY-PADS:[^\n]*\n(?:[ \t]*(?:void|extern)[^\n]*Pad\w*(?:\(\))?;[ \t]*\n)*').match(t, last)
    if m:
        last = m.end()
    return last


def split_orphans(s):
    """a segment whose leading comments hold several ORIGINAL markers (bodies that live in a header leave
    a marker-only comment block) is cut before the comment block of each marker after the first"""
    ms = [m.start() for m in ORIG.finditer(s)]
    cuts = []
    for p in ms[1:]:
        o = s.rfind('/*', 0, p)
        if o >= 0 and s.find('*/', o, p) < 0:
            b = o
        else:
            b = s.rfind('\n', 0, p) + 1
            while b > 0:
                pb = s.rfind('\n', 0, b - 1) + 1
                if s[pb:b].lstrip().startswith('//'):
                    b = pb
                else:
                    break
        b = s.rfind('\n', 0, b) + 1
        if b > (cuts[-1] if cuts else 0) and ORIG.search(s, cuts[-1] if cuts else 0, b):
            cuts.append(b)
    out = []; q = 0
    for c in cuts:
        out.append(s[q:c]); q = c
    out.append(s[q:])
    return out


def chunks(t, start=0):
    """[(key|None, text)] of t[start:]; unmarked items glue forward"""
    ends = [e for e in items(t) if e > start]
    segs = []; p = start
    for e in ends:
        segs.append(t[p:e]); p = e
    tail = t[p:]
    out = []; pend = ''
    for s in segs:
        for part in split_orphans(s):
            ks = [int(x, 16) for x in ORIG.findall(part)]
            pend += part
            if ks:
                out.append([min(ks), pend]); pend = ''
    if pend or tail:
        if out:
            out[-1][1] += pend + tail
        else:
            out.append([None, pend + tail])
    return out


LICENCE = re.compile(r'\A\s*/\*.*?GNU General Public.*?\*/\n?', re.S)


def split_foreign(path):
    t = open(path, encoding='latin-1').read()
    pe = prelude_end(t)
    pre = t[:pe]
    incs = re.findall(r'^[ \t]*#[ \t]*include[^\n]*\n', pre, re.M)
    rest = LICENCE.sub('', pre)
    rest = re.sub(r'^[ \t]*#[ \t]*include[^\n]*\n', '', rest, flags=re.M)
    defs = re.findall(r'^[ \t]*#[ \t]*(?:define|pragma|undef|if|ifdef|ifndef)[^\n]*', rest, re.M)
    cs = chunks(t, pe)
    if rest.strip() and cs:
        cs[0][1] = '// From ' + os.path.relpath(path, R) + ' (unit prelude):\n' + rest.strip('\n') + '\n' + cs[0][1]
    return incs, defs, cs


def insert(C, cs_c, foreign, top=False):
    """cs_c: [[key, text]] chunks of C in file order; foreign: [[key, text]]; returns new chunk list"""
    out = [list(c) for c in cs_c]
    placed = set()
    for k, s in sorted(foreign, key=lambda x: (x[0] is None, x[0] or 0)):
        best = -1; bk = -1
        for i, (ck, _) in enumerate(out):
            if top and (i, ck) not in placed:
                continue
            if ck is not None and k is not None and ck < k and ck > bk:
                best, bk = i, ck
        if not s.startswith('\n'):
            s = '\n' + s
        out.insert(best + 1, [k, s])
        placed = {(i + (i > best), ck) for i, ck in placed} | {(best + 1, k)}
    return out


def rewrite_build(gone, into, dry):
    notes = []
    p = R + '/CMakeLists.txt'
    t = open(p).read()
    for g in gone:
        # drop the token from every command; drop a property command left without sources
        def fix(m):
            body = m.group(0)
            if g not in re.findall(r'[\w/\.]+\.cpp', body):
                return body
            srcs = re.findall(r'src/[\w/\.]+\.cpp', body)
            if set(srcs) == {g}:
                notes.append('CMakeLists.txt: removed %s' % ' '.join(body.split())[:110])
                return ''
            body2 = re.sub(r'\n[ \t]*' + re.escape(g) + r'(?=[\s\)])', '', body)
            if body2 == body:
                body2 = re.sub(re.escape(g) + r'[ \t]*', '', body)
            return body2
        t = re.sub(r'(?:^#[^\n]*\n)*^(?:set_source_files_properties|set_property)\([^)]*\)\n', fix, t, flags=re.M)
        t2 = re.sub(r'^[ \t]*' + re.escape(g) + r'[ \t]*\n', '', t, flags=re.M)
        if t2 != t:
            notes.append('CMakeLists.txt: removed %s from source lists' % g)
        t = t2
    if not dry:
        open(p, 'w').write(t)
    p = R + '/recovery/project.json'
    t = open(p).read()
    for g in gone:
        m = re.search(r'\n[ \t]*"' + re.escape(g) + r'": \{\n[ \t]*"flags": \[[^\]]*\]\n[ \t]*\},?', t)
        if m:
            s, e = m.start(), m.end()
            if not m.group(0).endswith(','):
                # last entry: remove the comma of the previous one
                s2 = t.rfind(',', 0, s)
                if t[s2 + 1:s].strip() == '':
                    s = s2
            t = t[:s] + t[e:]
            notes.append('recovery/project.json: removed unit_flag_sets[%s]' % g)
    if not dry:
        json.loads(t)
        open(p, 'w').write(t)
    return notes


def create_unit(new, like, after, dry):
    """a new unit: the licence and the #include lines of `like` (its unit prelude otherwise stays behind), listed
    after `after` in OPENSMACX_SOURCES, with every per-source property command and the project.json flag set
    of `like` duplicated, and the worktree's build/compile_commands.json entry copied"""
    notes = []
    tl = open(R + '/' + like, encoding='latin-1').read()
    m = LICENCE.match(tl)
    pre = (m.group(0) if m else '') + ''.join(re.findall(r'^[ \t]*#[ \t]*include[^\n]*\n', tl[:prelude_end(tl)], re.M))
    p = R + '/CMakeLists.txt'
    t = open(p).read()
    b0 = t.index('set(OPENSMACX_SOURCES'); b1 = t.index(')', b0)
    head, lst, rest = t[:b0], t[b0:b1], t[b1:]
    a_ = re.search(r'^([ \t]*)' + re.escape(after) + r'[ \t]*\n', lst, re.M)
    if not a_:
        raise SystemExit('--after %s is not in OPENSMACX_SOURCES' % after)
    lst = lst[:a_.end()] + a_.group(1) + new + '\n' + lst[a_.end():]
    notes.append('CMakeLists.txt: %s listed after %s' % (new, after))
    n = [0]
    def dup(m_):
        n[0] += 1
        return m_.group(0) + ('\n' + m_.group(1) + new if m_.group(1) is not None and m_.group(2) == '\n' else ' ' + new)
    rest = re.sub(r'(?:(?<=\n)([ \t]+)|(?<=[ (]))' + re.escape(like) + r'(?=([\s)]))', dup, rest)
    notes.append('CMakeLists.txt: %d property command(s) of %s extended to %s' % (n[0], like, new))
    if not dry:
        open(p, 'w').write(head + lst + rest)
    p = R + '/recovery/project.json'
    t = open(p).read()
    m = re.search(r'\n([ \t]*)"' + re.escape(like) + r'": (\{\n[ \t]*"flags": \[[^\]]*\]\n[ \t]*\})(,?)', t)
    if m:
        ins = '\n' + m.group(1) + '"' + new + '": ' + m.group(2)
        t = t[:m.end(2)] + ',' + ins + m.group(3) + t[m.end():]
        notes.append('recovery/project.json: unit_flag_sets[%s] copied from %s' % (new, like))
        if not dry:
            json.loads(t)
            open(p, 'w').write(t)
    p = R + '/build/compile_commands.json'
    if os.path.exists(p) and not dry:
        d = json.load(open(p))
        if not any(e['file'] == R + '/' + new for e in d):
            for i, e in enumerate(d):
                if e['file'] == R + '/' + like:
                    d.insert(i + 1, {k: v.replace(like, new) for k, v in e.items()})
                    json.dump(d, open(p, 'w'), indent=1)
                    notes.append('build/compile_commands.json (worktree copy): entry for %s' % new)
                    break
    if not dry:
        open(R + '/' + new, 'w', encoding='latin-1').write(pre)
        subprocess.run(['git', 'add', '-N', new], cwd=R, check=True)
    return notes


def unit_flags(u):
    d = json.load(open(R + '/recovery/project.json'))['measurement']['unit_flag_sets']
    return sorted(set(d.get(u, {}).get('flags', [])) - {'/Z7'})


def merge(srcs, into, dry=False, addrs=None, top=False, with_inc=None, new_like=None, after=None):
    skipped = []
    into = os.path.relpath(os.path.abspath(into), R) if os.path.isabs(into) else into
    made = []
    if not os.path.exists(R + '/' + into):
        if not new_like:
            raise SystemExit('%s does not exist (pass --new-like UNIT [--after UNIT] to create it)' % into)
        if dry:
            raise SystemExit('--dry cannot create %s' % into)
        made = create_unit(into, new_like, after or new_like, dry)
    srcs = [os.path.relpath(os.path.abspath(s), R) if os.path.isabs(s) else s for s in srcs]
    tc = open(R + '/' + into, encoding='latin-1').read()
    pe = prelude_end(tc)
    cs_c = chunks(tc, pe)
    have = set(re.findall(r'#[ \t]*include[ \t]*([<"][^>"]+[>"])', tc[:pe]))
    add_inc = []; foreign = []; warn = []
    keep_src = {}
    for s in srcs:
        if unit_flags(s) != unit_flags(into):
            warn.append('flags differ: %s %s | %s %s' % (s, ' '.join(sorted(set(unit_flags(s)) - set(unit_flags(into)))) or '-',
                                                         into, ' '.join(sorted(set(unit_flags(into)) - set(unit_flags(s)))) or '-'))
        incs, defs, cs = split_foreign(R + '/' + s)
        if addrs is not None:
            mine = [c for c in cs if c[0] in addrs]
            # a moved function takes only its own item text: re-chunk without glue
            ts = open(R + '/' + s, encoding='latin-1').read()
            keep = ts
            for c in mine:
                keep = keep.replace(c[1], '', 1)
            keep_src[s] = keep
            cs = mine
        elif defs:
            warn.append('%s: unit-level preprocessor lines before/among the includes: %s' % (s, '; '.join(d.strip() for d in defs)))
        for inc in incs:
            name = re.search(r'([<"][^>"]+[>"])', inc).group(1)
            if addrs is not None and name not in have and name not in (with_inc or ()):
                skipped.append(name)
                continue
            if name not in have:
                have.add(name); add_inc.append(inc if inc.endswith('\n') else inc + '\n')
        for c in cs:
            if re.search(r'^[ \t]*#[ \t]*(?!include)\w+', c[1], re.M):
                warn.append('%s chunk %s holds preprocessor lines' % (s, '0x%08X' % c[0] if c[0] else '-'))
            foreign.append(c)
    new = insert(into, cs_c, foreign, top)
    pre = tc[:pe]
    if add_inc:
        last = 0
        for m in re.finditer(r'^[ \t]*#[ \t]*include[^\n]*\n', pre, re.M):
            last = m.end()
        pre = pre[:last] + ''.join(add_inc) + pre[last:]
    out = pre + ''.join(c[1] for c in new)
    print('MERGE %s -> %s: %d chunks moved, %d includes added' % (' '.join(srcs), into, len(foreign), len(add_inc)))
    for k, s in foreign:
        print('  chunk %s %d lines' % ('0x%08X' % k if k else '-', s.count('\n')))
    if skipped:
        print('  includes of the source file NOT added (pass --include NAME for the ones the moved bodies need): ' + ' '.join(skipped))
    for w in warn:
        print('  WARN ' + w)
    notes = list(made)
    if addrs is None:
        notes += rewrite_build(srcs, into, dry)
    for nt in notes:
        print('  ' + nt)
    if dry:
        return 0
    open(R + '/' + into, 'w', encoding='latin-1').write(out)
    if addrs is None:
        for s in srcs:
            subprocess.run(['git', 'rm', '-q', '-f', s], cwd=R, check=True)
        # compile_context annotations
        for dp, dn, fn in os.walk(R + '/src'):
            for f in fn:
                if f.endswith(('.cpp', '.h')):
                    p = os.path.join(dp, f)
                    t = open(p, encoding='latin-1').read()
                    t2 = t
                    for s in srcs:
                        t2 = re.sub(r'(// compile_context )' + re.escape(s) + r'(?=\s)', r'\1' + into, t2)
                    if t2 != t:
                        open(p, 'w', encoding='latin-1').write(t2)
                        print('  compile_context rewritten in ' + os.path.relpath(p, R))
    else:
        for s, keep in keep_src.items():
            open(R + '/' + s, 'w', encoding='latin-1').write(keep)
    return 0
