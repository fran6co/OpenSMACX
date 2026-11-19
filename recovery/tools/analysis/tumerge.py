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
`--deps` also moves the host's unmarked top-level pieces (objects, helpers, #define blocks) that only the moved
chunks name, to closure, each in front of the next moved chunk in file order; a declaration both sides use is
copied; a definition both sides use stays and is reported as SHARED.
A `$E` address (a static-initializer thunk) moves its marker blocks and the object definition (see extract_objects);
`--with NAME,..` moves the named definitions as well. A host left with nothing but its prelude is removed from the
tree and the build. After every regroup analysis/esym.py refreshes the `// symbol _$E<n>` lines of both sides.
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
        # trailing marker-only comment blocks (bodies that live in a header) are chunks of their own
        parts = split_orphans('// ORIGINAL: 0x00000000 \n' + pend + tail)
        parts[0] = parts[0].split('\n', 1)[1]
        for part in parts:
            ks = [int(x, 16) for x in ORIG.findall(part)]
            if ks:
                out.append([min(ks), part])
            elif out:
                out[-1][1] += part
            else:
                out.append([None, part])
    return out


LICENCE = re.compile(r'\A\s*/\*.*?GNU General Public.*?\*/\n?', re.S)


def split_foreign(path, t=None):
    if t is None:
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


def strip_code(s):
    """s with comments, string and character literals blanked"""
    out = []; i = 0; n = len(s)
    while i < n:
        if s.startswith('//', i):
            j = s.find('\n', i); j = n if j < 0 else j
            i = j; continue
        if s.startswith('/*', i):
            j = s.find('*/', i + 2); i = n if j < 0 else j + 2; out.append(' '); continue
        c = s[i]
        if c == '"' or c == "'":
            j = i + 1
            while j < n and s[j] != c:
                j += 2 if s[j] == '\\' else 1
            i = j + 1; out.append('0'); continue
        out.append(c); i += 1
    return ''.join(out)


def open_comment(s):
    """True when s ends inside a /* comment"""
    i = 0; n = len(s)
    while i < n:
        if s.startswith('//', i):
            j = s.find('\n', i); i = n if j < 0 else j; continue
        if s.startswith('/*', i):
            j = s.find('*/', i + 2)
            if j < 0:
                return True
            i = j + 2; continue
        c = s[i]
        if c == '"' or c == "'":
            j = i + 1
            while j < n and s[j] != c and s[j] != '\n':
                j += 2 if s[j] == '\\' else 1
            i = j + 1; continue
        i += 1
    return False


DEFINE = re.compile(r'^[ \t]*#[ \t]*define[ \t]+(\w+)(?:[^\n]*\\\n)*[^\n]*\n', re.M)


def pieces(s):
    """a chunk cut into its top-level pieces: every item (with the comments before it) and every #define block
    (with the comment lines directly above it); ''.join(pieces) == s"""
    cuts = set(items(s))
    for m in DEFINE.finditer(s):
        b = m.start()
        while b > 0:
            pb = s.rfind('\n', 0, b - 1) + 1
            if s[pb:b].lstrip().startswith('//'):
                b = pb
            else:
                break
        # only when the define is not inside an item (depth 0): its start must follow an item end or other pieces
        cuts.add(m.end())
        cuts.add(b)
    cuts = sorted(c for c in cuts if 0 < c < len(s))
    out = []; q = 0
    for c in cuts:
        out.append(s[q:c]); q = c
    out.append(s[q:])
    return [x for x in out if x]


def piece_names(pc):
    """(kind, {names}) of a piece: 'define', 'function' (has a body), 'object' (a definition), 'decl' (extern or
    prototype), '' (nothing declared: comments, marker blocks)"""
    m = DEFINE.search(pc)
    code = strip_code(pc)
    if m and not strip_code(pc[:m.start()] + pc[m.end():]).strip():
        return 'define', {m.group(1)}
    code = re.sub(r'^[ \t]*#[^\n]*(?:\\\n[^\n]*)*', '', code, flags=re.M)
    if not code.strip():
        return '', set()
    depth = 0; cut = len(code)
    for i, c in enumerate(code):
        if c in '([':
            if depth == 0 and c == '[':
                cut = i; break
            if depth == 0 and c == '(':
                cut = i; break
            depth += 1
        elif c in ')]':
            depth -= 1
        elif depth == 0 and c in '=;{':
            cut = i; break
    head = code[:cut]
    ids = re.findall(r'[A-Za-z_]\w*', head)
    if not ids:
        return '', set()
    name = ids[-1]
    first = ids[0]
    if first in ('struct', 'class', 'union', 'enum', 'typedef', 'namespace') and code[cut:cut + 1] == '{' or first == 'typedef':
        # a type: its tag, and the declarator after the closing brace
        tail = re.findall(r'[A-Za-z_]\w*', code[code.rfind('}') + 1:]) if '}' in code else ids[-1:]
        return 'type', set(ids[1:2]) | set(tail[-1:])
    if code[cut:cut + 1] == '(':
        # function: body or prototype
        d = 0; j = cut
        while j < len(code):
            if code[j] == '(':
                d += 1
            elif code[j] == ')':
                d -= 1
                if d == 0:
                    break
            j += 1
        rest = code[j + 1:].lstrip()
        if rest.startswith('{') or rest.startswith(':') or re.match(r'(?:const\s*)?\{', rest):
            return 'function', {name}
        if rest.startswith(';') or rest.startswith('const') or rest.startswith('throw'):
            return 'decl', {name}
        return 'object', {name}      # `T name(args);` style object or a macro-wrapped definition
    if first == 'extern' and '=' not in code[cut:cut + 1]:
        return 'decl', {name}
    return 'object', {name}


def pull_deps(host_text, pe, moved_keys):
    """--move --deps: the unmarked pieces of the host that only the moved chunks use travel with them.
    -> (moved chunks with their pieces glued in file order, remaining host text, report lines)"""
    cs = chunks(host_text, pe)
    rep = []
    P = []          # [chunk index, text, kind, names, marked, moved]
    for ci, (k, s) in enumerate(cs):
        if k in moved_keys:
            P.append([ci, s, 'moved', set(), True, True]); continue
        for pc in pieces(s):
            kind, names = piece_names(pc)
            ms = list(re.finditer(r'// ORIGINAL: 0x[0-9A-F]{8} (\S+)', pc))
            if ms and kind == 'function' and not any(re.search(r'(?<![A-Za-z0-9_])' + re.escape(nm) + r'(?![A-Za-z0-9_])', m.group(1))
                                                      or m.group(1).startswith('??') for m in ms for nm in names):
                # the markers above this function name other functions (bodies that live in a header): the
                # marker comments stay, the unmarked function is a piece of its own
                q = ms[-1].start()
                o = pc.rfind('/*', 0, q)
                if o >= 0 and pc.find('*/', o, q) < 0:
                    e = pc.find('*/', q); e = pc.find('\n', e) + 1
                else:
                    e = pc.find('\n', q) + 1
                    while pc[e:].lstrip(' \t').startswith('//') and pc[e:e + 1] != '\n':
                        e = pc.find('\n', e) + 1
                P.append([ci, pc[:e], '', set(), True, False])
                P.append([ci, pc[e:], kind, names, False, False])
                continue
            marked = bool(ms) and kind in ('function', '')
            P.append([ci, pc, kind, names, marked, False])
    def idents(s):
        return set(re.findall(r'[A-Za-z_]\w*', strip_code(s)))
    ID = [idents(p[1]) for p in P]
    pre_ids = idents(host_text[:pe])
    changed = True
    copies = set()
    while changed:
        changed = False
        used_moved = set().union(*[ID[i] for i, p in enumerate(P) if p[5]]) if any(p[5] for p in P) else set()
        for i, p in enumerate(P):
            if p[5] or p[4] or not p[3] or i in copies:
                continue
            if not (p[3] & used_moved):
                continue
            stay = pre_ids.union(*[ID[j] for j, q in enumerate(P) if not q[5] and j != i])
            if p[3] & stay:
                if p[2] == 'decl':
                    copies.add(i); changed = True
                    rep.append('  dep COPIED (declaration both sides use): ' + ' '.join(sorted(p[3])))
                continue
            p[5] = True; changed = True
            rep.append('  dep moved: %s %s (%d lines)' % (p[2], ' '.join(sorted(p[3])), p[1].count('\n')))
    used_moved = set().union(*[ID[i] for i, p in enumerate(P) if p[5]]) if any(p[5] for p in P) else set()
    for i, p in enumerate(P):
        if not p[5] and not p[4] and i not in copies and p[3] & used_moved:
            rep.append('  dep SHARED, left in the host (%s %s; also used by code that stays)' % (p[2], ' '.join(sorted(p[3]))))
    # glue: every pulled piece goes in front of the next moved chunk in file order (the last one when none follows)
    order = [i for i, p in enumerate(P) if p[5] or i in copies]
    mchunks = [i for i in order if P[i][2] == 'moved']
    out = {i: '' for i in mchunks}
    tailadd = ''
    for i in order:
        if P[i][2] == 'moved':
            continue
        nxt = next((j for j in mchunks if j > i), None)
        txt = P[i][1] if P[i][1].startswith('\n') else '\n' + P[i][1]
        if nxt is None:
            tailadd += txt
        else:
            out[nxt] += txt
    moved = []
    for i in mchunks:
        k = cs[P[i][0]][0]
        moved.append([k, out[i] + P[i][1] + (tailadd if i == mchunks[-1] else '')])
    keep = host_text[:pe] + ''.join(p[1] for i, p in enumerate(P) if not p[5])
    return moved, keep, rep


EMARK = re.compile(r'// ORIGINAL: 0x([0-9A-F]{8}) (\S+) 0x([0-9A-Fa-f]+)-0x([0-9A-Fa-f]+)[^\n]*\n((?://(?! ORIGINAL:)[^\n]*\n)*)')
ENAME = re.compile(r'_?\$E\d+$')


def extract_objects(ts, addrs, withnames=()):
    """--move of static-initializer thunks: a `$E` marker is a comment block (the compiler writes the function), so
    what moves is the marker block, the blocks of the `$E` functions its image code refers to (the atexit
    callback, the constructor and registration halves) and the definition of the object it initializes: the
    piece under the `// RECOVERY_OBJECT` comment whose address the thunk's image bytes hold (when exactly one
    does; otherwise name the definitions with --with). One chunk per thunk, keyed by its address, the marker
    blocks directly above the definition, so the destination keeps initializer order = thunk address order.
    -> (host text without them, [[key, text]], report)"""
    import struct
    sys.path.insert(0, os.path.dirname(HERE))
    import fe
    ms = {int(m.group(1), 16): m for m in EMARK.finditer(ts)}
    em = {a: m for a, m in ms.items() if ENAME.match(m.group(2))}
    heads = sorted(a for a in addrs if a in em)
    rep = []
    if not heads and not withnames:
        return ts, [], rep, set()
    def refs(a):
        m = em[a]; lo, hi = int(m.group(3), 16), int(m.group(4), 16)
        b = fe.imgbytes(lo, hi); out = set()
        for i in range(len(b) - 3):
            v = struct.unpack_from('<I', b, i)[0]
            out.add(v); out.add((lo + i + 4 + v) & 0xffffffff)
        return out
    objs = {int(m.group(1), 16): m.start() for m in re.finditer(r'// RECOVERY_OBJECT: v\d+ 0x([0-9A-F]{8}) ', ts)}
    groups = []; taken = set()
    for a in heads:
        if a in taken:
            continue
        grp = [a]; todo = [a]; oaddr = set()
        while todo:
            x = todo.pop()
            r = refs(x)
            for y in em:
                if y in r and y not in grp:
                    grp.append(y); todo.append(y)
            oaddr |= {o for o in objs if o in r}
        taken |= set(grp)
        groups.append((a, sorted(grp), sorted(oaddr)))
    pe = prelude_end(ts)
    # pieces of the body, to find object definitions
    body = ts[pe:]
    pcs = []
    for k, s in chunks(ts, pe):
        pcs += pieces(s)
    assert ''.join(pcs) == body
    kinds = [piece_names(pc) for pc in pcs]
    pull = {}       # piece index -> group key
    extra = [i for i, (kind, nm) in enumerate(kinds) if nm & set(withnames) and kind in ('object', 'function', 'define', 'decl', 'type')]
    out = []
    for a, grp, oaddr in groups:
        idx = []
        if len(oaddr) == 1:
            line = '0x%08X ' % oaddr[0]
            idx = [i for i, pc in enumerate(pcs) if re.search(r'// RECOVERY_OBJECT: v\d+ ' + line, pc) and kinds[i][0] == 'object']
        if not idx and not (extra and len(groups) == 1):
            rep.append('  WARN thunk 0x%08X: %s; its definition was not moved (name it with --with)' % (
                a, 'no RECOVERY_OBJECT address in its code' if not oaddr else 'object addresses ' + ' '.join('0x%08X' % o for o in oaddr)))
        for i in idx[:1]:
            pull[i] = a
        rep.append('  thunk 0x%08X: markers %s, object %s' % (a, ' '.join('0x%08X' % g for g in grp),
                                                               ' '.join(sorted(kinds[i][1])[0] for i in idx[:1]) or '-'))
    for i in extra:
        if i not in pull:
            pull[i] = groups[0][0] if groups else None
            rep.append('  --with: %s %s' % (kinds[i][0], ' '.join(sorted(kinds[i][1]))))
    res = []
    gone_blocks = []
    for a, grp, oaddr in groups:
        txt = ''.join(em[g].group(0) for g in grp)
        gone_blocks += [em[g].group(0) for g in grp]
        defs = ''.join(pcs[i].lstrip('\n') for i in sorted(pull) if pull[i] == a)
        res.append([a, '\n' + txt + defs])
    if not groups and extra:
        res.append([None, ''.join(pcs[i] for i in extra)])
    keep_body = ''.join(pc for i, pc in enumerate(pcs) if i not in pull)
    keep = ts[:pe] + keep_body
    for blk in gone_blocks:
        if blk in keep:
            keep = keep.replace(blk, '', 1)
        else:
            rep.append('  WARN marker block not found again after the object pieces were removed')
    moved_names = set()
    for i in pull:
        if kinds[i][0] == 'object':
            moved_names |= kinds[i][1]
    return keep, res, rep, moved_names


def unit_flags(u):
    d = json.load(open(R + '/recovery/project.json'))['measurement']['unit_flag_sets']
    return sorted(set(d.get(u, {}).get('flags', [])) - {'/Z7'})


def merge(srcs, into, dry=False, addrs=None, top=False, with_inc=None, new_like=None, after=None, deps=False, with_names=None):
    skipped = []; deprep = []; objnames = set()
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
        ts = open(R + '/' + s, encoding='latin-1').read()
        ocs = []
        if addrs is not None:
            ts, ocs, rep, onames = extract_objects(ts, addrs, with_names or ())
            deprep += rep
            objnames |= onames
        incs, defs, cs = split_foreign(R + '/' + s, ts)
        if addrs is not None and deps:
            cs, keep_src[s], rep = pull_deps(ts, prelude_end(ts), addrs)
            deprep += rep
            cs = ocs + cs
        elif addrs is not None:
            mine = [c for c in cs if c[0] in addrs]
            # a moved function takes only its own item text: re-chunk without glue
            keep = ts
            for c in mine:
                keep = keep.replace(c[1], '', 1)
            keep_src[s] = keep
            cs = ocs + mine
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
            if open_comment(c[1]):
                # a marker block whose comment was only closed by the next block's `*/`
                c[1] = c[1].rstrip('\n') + '\n*/\n'
                warn.append('%s chunk %s: unterminated /* comment closed' % (s, '0x%08X' % c[0] if c[0] else '-'))
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
    body = ''.join(c[1] for c in new)
    for nm in sorted(objnames):
        # the destination's own `extern T name;` is redundant now, and after the definition it is harmful: VC6
        # emits the dynamic initializer of a class object a second time at a later redeclaration
        rx = re.compile(r'^[ \t]*extern\s+[^;{}()=]*\b' + re.escape(nm) + r'\b[^;{}()=]*;[^\n]*\n', re.M)
        for m in list(rx.finditer(body))[::-1]:
            deprep.append('  local declaration removed from %s: %s' % (into, m.group(0).strip()))
            body = body[:m.start()] + body[m.end():]
        for m in list(rx.finditer(pre))[::-1]:
            deprep.append('  local declaration removed from %s: %s' % (into, m.group(0).strip()))
            pre = pre[:m.start()] + pre[m.end():]
    out = pre + body
    print('MERGE %s -> %s: %d chunks moved, %d includes added' % (' '.join(srcs), into, len(foreign), len(add_inc)))
    for k, s in foreign:
        print('  chunk %s %d lines' % ('0x%08X' % k if k else '-', s.count('\n')))
    if skipped:
        print('  includes of the source file NOT added (pass --include NAME for the ones the moved bodies need): ' + ' '.join(skipped))
    for w in deprep + warn:
        print(w if w.startswith('  ') else '  WARN ' + w)
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
        empty = []
        for s, keep in keep_src.items():
            kp = prelude_end(keep)
            if not ORIG.search(keep) and not strip_code(keep[kp:]).strip():
                empty.append(s)         # nothing but the prelude is left: the file was not a unit
            else:
                open(R + '/' + s, 'w', encoding='latin-1').write(keep)
        if empty:
            for nt in rewrite_build(empty, into, dry):
                print('  ' + nt)
            for s in empty:
                subprocess.run(['git', 'rm', '-q', '-f', s], cwd=R, check=True)
                print('  %s is empty after the move: removed' % s)
    # static-initializer symbol numbers follow the unit's counter: refresh them on both sides
    import esym
    pfx = int(os.environ.get('FE_PFX_BASE', '21'))
    for u in [into] + [s for s in srcs if os.path.exists(R + '/' + s)]:
        try:
            esym.run([u], write=True, pfx=pfx, quiet=True)
        except SystemExit as e:
            print('  ESYM %s: %s' % (u, str(e)[:600]))
    return 0
