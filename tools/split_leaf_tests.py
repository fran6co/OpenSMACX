#!/usr/bin/env python3
"""Parse tests/recovery_leaf_tests.cpp into top-level items.

Shared by the analysis pass and the emit pass so both see the same item
boundaries.  Nothing here interprets C++ semantics beyond brace depth,
comment/string stripping and declarator-name extraction.
"""
import re

SRC = 'tests/recovery_leaf_tests.cpp'

KW = set('''void int char bool float double long short unsigned signed const constexpr static extern
inline struct class union enum namespace return if else for while do switch case break continue new
delete this true false nullptr sizeof alignas alignof typedef using template typename operator public
private protected virtual friend explicit mutable volatile auto decltype static_assert noexcept goto
default try catch throw register thread_local'''.split())


def strip_comments(lines):
    out = []
    inblock = False
    for l in lines:
        s = ''
        j = 0
        while j < len(l):
            c = l[j]
            if inblock:
                if l.startswith('*/', j):
                    inblock = False
                    j += 2
                    continue
                j += 1
                continue
            if l.startswith('//', j):
                break
            if l.startswith('/*', j):
                inblock = True
                j += 2
                continue
            if c == '"':
                s += ' '
                j += 1
                while j < len(l):
                    if l[j] == '\\':
                        j += 2
                        continue
                    if l[j] == '"':
                        j += 1
                        break
                    j += 1
                continue
            if c == "'":
                s += ' '
                j += 1
                while j < len(l):
                    if l[j] == '\\':
                        j += 2
                        continue
                    if l[j] == "'":
                        j += 1
                        break
                    j += 1
                continue
            s += c
            j += 1
        out.append(s)
    return out


def parse(path=SRC):
    lines = open(path).read().split('\n')
    clean = strip_comments(lines)
    depth = 0
    startdepth = []
    for s in clean:
        startdepth.append(depth)
        for c in s:
            if c == '{':
                depth += 1
            elif c == '}':
                depth -= 1
    assert depth == 0, depth
    n = len(lines)
    items = []
    i = 1
    inns = False
    while i <= n:
        s = clean[i - 1]
        if not s.strip():
            i += 1
            continue
        if not inns and startdepth[i - 1] == 0 and re.match(r'^\s*namespace\s*\{\s*$', s):
            items.append({'start': i, 'end': i, 'kind': 'nsopen'})
            inns = True
            i += 1
            continue
        if inns and startdepth[i - 1] == 1 and s.strip() == '}':
            items.append({'start': i, 'end': i, 'kind': 'nsclose'})
            inns = False
            i += 1
            continue
        base = 1 if inns else 0
        st = i
        d = startdepth[i - 1]
        assert d == base, (i, d, base, s)
        j = i
        while j <= n:
            for c in clean[j - 1]:
                if c == '{':
                    d += 1
                elif c == '}':
                    d -= 1
            t = clean[j - 1].rstrip()
            if d == base and (t.endswith(';') or t.endswith('}')
                              or re.match(r'^\s*#', clean[j - 1])):
                break
            j += 1
        items.append({'start': st, 'end': j, 'kind': 'item', 'base': base,
                      'inns': inns})
        i = j + 1
    for x in items:
        raw = '\n'.join(lines[x['start'] - 1:x['end']])
        x['raw'] = raw
        x['clean'] = '\n'.join(clean[x['start'] - 1:x['end']])
    classify(items, lines)
    return lines, clean, items


TESTRE = re.compile(r'^void (test_\w+)\(\)')


def classify(items, lines):
    for x in items:
        if x['kind'] != 'item':
            x['k'] = x['kind']
            x['defs'] = []
            x['ids'] = []
            continue
        head = lines[x['start'] - 1].strip()
        c = x['clean']
        m = TESTRE.match(head)
        if m:
            x['k'] = 'test'
            x['name'] = m.group(1)
            x['defs'] = [m.group(1)]
        elif head.startswith('#'):
            x['k'] = 'pp'
            x['defs'] = []
        else:
            x['k'] = item_kind(c)
            x['defs'] = declared_names(c)
        x['ids'] = sorted(set(re.findall(r'[A-Za-z_]\w*', c)))


def item_kind(c):
    s = c.lstrip()
    if re.match(r'^enum\s*\{', s):
        return 'type'
    if re.match(r'^(?:const\s+)?(?:struct|union)\s*\{[^{}]*\}\s*[A-Za-z_]', s, re.S):
        return 'var'
    if re.match(r'^(struct|class|union|enum)\b', s) and '{' in s.split(';')[0]:
        return 'type'
    if re.match(r'^(using|typedef)\b', s):
        return 'alias'
    if re.match(r'^static_assert\b', s):
        return 'assert'
    stop = len(c)
    for ch in ('{', '='):
        p = c.find(ch)
        if p >= 0:
            stop = min(stop, p)
    proto = c[:stop]
    if '(' in proto and ')' in proto and not proto.rstrip().endswith(','):
        return 'func'
    return 'var'


def declared_names(c):
    s = c.lstrip()
    m = re.match(r'^(?:struct|class|union|enum)\s+(?:class\s+)?(\w+)\s*(?:[:{])', s)
    if m:
        return [m.group(1)]
    # An unnamed enum contributes its enumerators to the enclosing scope.
    if re.match(r'^enum\s*\{', s):
        body = s[s.index('{') + 1:s.rindex('}')]
        out = []
        for part in body.split(','):
            mm = re.match(r'\s*([A-Za-z_]\w*)', part)
            if mm:
                out.append(mm.group(1))
        return out
    # An unnamed struct/union used as a declarator type: the name follows '}'.
    mm = re.match(r'^(?:const\s+)?(?:struct|union)\s*\{[^{}]*\}\s*([A-Za-z_]\w*)', s, re.S)
    if mm:
        return [mm.group(1)]
    m = re.match(r'^using\s+(\w+)\s*=', s)
    if m:
        return [m.group(1)]
    if re.match(r'^static_assert\b', s):
        return []
    stop = len(c)
    for ch in ('{', '='):
        p = c.find(ch)
        if p >= 0:
            stop = min(stop, p)
    proto = c[:stop]
    fm = re.findall(r'([A-Za-z_]\w*(?:::[A-Za-z_]\w*)*)\s*\(', proto)
    if fm and '(' in proto:
        cand = fm[0]
        if cand.split('::')[-1] not in KW:
            return [cand]
    toks = [t for t in re.findall(r'[A-Za-z_]\w*', proto.split('[')[0]) if t not in KW]
    if toks:
        return [toks[-1]]
    # anonymous struct / enum declarators: name sits before '[' or '='
    toks = [t for t in re.findall(r'[A-Za-z_]\w*', proto) if t not in KW]
    return toks[-1:] if toks else []


if __name__ == '__main__':
    lines, clean, items = parse()
    import collections
    print(collections.Counter(x['k'] for x in items))
