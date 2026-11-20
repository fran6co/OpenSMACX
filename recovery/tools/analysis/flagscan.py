"""flagscan.py [list|deck] ...: MATCHING_GUIDE 3.7 Layout "a flag skipping a shared tail is a goto" screen.

  list [unit ...] [--all]   source scan: bool/int locals initialised to a constant, set to another constant at
                            one or more sites and tested once later by an `if` with no else, every other
                            occurrence being the declaration. Only non-exact markers unless --all. Prints
                            unit addr name var sets test-line form flow (flow=ok: every set site falls straight
                            into the test, so the goto rewrite preserves behaviour).
  deck <unit> <addr> [--var V] [--write]
                            for each flow=ok skip-form flag of the function: the rewrite (sets become
                            `goto <v>_past;`, the test is dropped, the label follows the block, the declaration
                            goes) evaluated with sdeck against the base: (EH, named calls, cf counts, cf hunks,
                            rows). --write applies the best rewrite when its key beats the base.
Screen only; confirm with `recover measure`. Lanes FE_PFX_BASE.."""
import sys, os, re, json
S = os.path.dirname(os.path.abspath(__file__)); T = os.path.dirname(S); R = os.path.dirname(os.path.dirname(T))

TYPES = r'(?:int|BOOL|bool|char|uint8_t|int8_t|int32_t|uint32_t|unsigned(?:\s+int)?|short|int16_t|uint16_t)'
CONST = r'(?:-?\d+|false|true|FALSE|TRUE)'


def cval(s):
    s = s.strip()
    return {'false': 0, 'FALSE': 0, 'true': 1, 'TRUE': 1}.get(s, None) if not re.fullmatch(r'-?\d+', s) else int(s)


def mask(t):
    """comments and literal contents -> spaces (offsets kept)"""
    o = list(t); i = 0; n = len(t)
    while i < n:
        c = t[i]
        if t.startswith('//', i):
            j = t.find('\n', i); j = n if j < 0 else j
            for k in range(i, j): o[k] = ' '
            i = j
        elif t.startswith('/*', i):
            j = t.find('*/', i + 2); j = n if j < 0 else j + 2
            for k in range(i, j):
                if o[k] != '\n': o[k] = ' '
            i = j
        elif c in '"\'':
            j = i + 1
            while j < n and t[j] != c:
                j += 2 if t[j] == '\\' else 1
            for k in range(i + 1, min(j, n)): o[k] = ' '
            i = j + 1
        else:
            i += 1
    return ''.join(o)


def match_close(m, i, op='{', cl='}'):
    d = 0
    for j in range(i, len(m)):
        if m[j] == op: d += 1
        elif m[j] == cl:
            d -= 1
            if d == 0: return j
    return -1


def match_open(m, j, op='{', cl='}'):
    d = 0
    for i in range(j, -1, -1):
        if m[i] == cl: d += 1
        elif m[i] == op:
            d -= 1
            if d == 0: return i
    return -1


def skipws(m, i):
    while i < len(m) and m[i].isspace(): i += 1
    return i


def stmt_end(m, i):
    """end (exclusive) of the statement starting at i"""
    i = skipws(m, i)
    if m[i] == '{': return match_close(m, i) + 1
    w = re.match(r'(if|while|for|switch)\s*\(', m[i:])
    if w:
        p = match_close(m, i + w.end() - 1, '(', ')')
        e = stmt_end(m, p + 1)
        if w.group(1) == 'if':
            k = skipws(m, e)
            if re.match(r'else\b', m[k:]): e = stmt_end(m, k + 4)
        return e
    if re.match(r'else\b', m[i:]): return stmt_end(m, i + 4)
    if re.match(r'do\b', m[i:]):
        e = stmt_end(m, i + 2); return m.index(';', e) + 1
    d = 0
    for j in range(i, len(m)):
        if m[j] in '({[': d += 1
        elif m[j] in ')}]': d -= 1
        elif m[j] == ';' and d == 0: return j + 1
    return len(m)


def header_before(m, ob):
    """kind of the construct whose block opens at ob: if/else/loop/switch/plain/func"""
    k = ob - 1
    while k >= 0 and m[k].isspace(): k -= 1
    if m[k] == ')':
        p = match_open(m, k, '(', ')')
        w = re.search(r'(\w+)\s*$', m[:p])
        kw = w.group(1) if w else ''
        return {'if': 'if', 'while': 'loop', 'for': 'loop', 'switch': 'switch'}.get(kw, 'func')
    if re.search(r'\belse\s*$', m[:ob]): return 'else'
    if re.search(r'\bdo\s*$', m[:ob]): return 'loop'
    if m[k] in ';{}': return 'plain'
    return 'func'


def flows_to(m, i, target, body_lo):
    """does control fall from position i (after a statement) straight to target, through closing braces of
    if/else/plain blocks only (skipping else continuations)?"""
    while True:
        i = skipws(m, i)
        if i == target: return True
        if i >= len(m) or i < body_lo: return False
        if m[i] == '}':
            ob = match_open(m, i)
            kind = header_before(m, ob)
            if kind not in ('if', 'else', 'plain'): return False
            i += 1
            while True:  # skip else continuations of the closed if chain
                k = skipws(m, i)
                if re.match(r'else\b', m[k:]): i = stmt_end(m, k + 4)
                else: break
            continue
        if re.match(r'else\b', m[i:]):
            i = stmt_end(m, i + 4); continue
        if re.match(r'break\s*;', m[i:]):  # leave the innermost loop (not a switch)
            ob = i
            while True:
                ob = enclosing(m, ob)
                if ob < body_lo: return False
                kind = header_before(m, ob)
                if kind == 'switch': return False
                if kind == 'loop': break
            i = match_close(m, ob) + 1
            k = skipws(m, i)
            if re.match(r'while\s*\(', m[k:]): i = m.index(';', match_close(m, m.index('(', k), '(', ')')) + 1
            continue
        return False


def enclosing(m, i):
    d = 0
    for j in range(i - 1, -1, -1):
        if m[j] == '}': d += 1
        elif m[j] == '{':
            if d == 0: return j
            d -= 1
    return -1


def functions(text, m, exact_too):
    for mk in re.finditer(r'// ORIGINAL: 0x([0-9A-F]{8}) (\S+) 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)', text):
        if 'BYTE_EXACT' in mk.group(3) and not exact_too: continue
        # first top-level '{' after a ')' following the marker
        i = mk.end(); d = 0; ob = None
        while i < len(m):
            c = m[i]
            if c == '(': d += 1
            elif c == ')': d -= 1
            elif c == ';' and d == 0 and ob is None:
                ob = -1; break  # declaration only
            elif c == '{' and d == 0:
                ob = i; break
            i += 1
        if not ob or ob < 0: continue
        cb = match_close(m, ob)
        if cb < 0: continue
        yield int(mk.group(1), 16), mk.group(2), ob, cb


def cond_value(cond, v, val):
    c = re.sub(r'\b%s\b' % re.escape(v), str(val), cond)
    c = c.replace('!=', ' NE ').replace('==', ' EQ ').replace('!', ' not ').replace(' NE ', '!=').replace(' EQ ', '==')
    c = c.replace('&&', ' and ').replace('||', ' or ')
    try: return bool(eval(c, {}))
    except Exception: return None


def scan_fn(text, m, ob, cb):
    out = []
    body = m[ob:cb]
    for d in re.finditer(r'(?<![\w.])(?:const\s+)?%s\s+(\w+)\s*=\s*(%s)\s*;' % (TYPES, CONST), body):
        v = d.group(1); k0 = cval(d.group(2)); dpos = ob + d.start()
        occ = [ob + o.start() for o in re.finditer(r'(?<![\w.>])%s\b' % re.escape(v), body)]
        sets = []; tests = []; other = []
        for p in occ:
            if dpos <= p < ob + d.end(): continue
            a = re.match(r'%s\s*=\s*(%s)\s*;' % (re.escape(v), CONST), m[p:])
            pre = m[:p].rstrip()[-1:]
            if a and pre in ';{})' or (a and re.search(r'\belse\s*$', m[:p])):
                sets.append((p, p + a.end(), cval(a.group(1)))); continue
            # inside an if condition?
            q = m.rfind('(', 0, p)
            w = re.search(r'\bif\s*\($', m[:q + 1])
            tests.append(p) if w else other.append(p)
        if other or len(tests) != 1 or not sets: continue
        if any(s[2] == k0 for s in sets) or len(set(s[2] for s in sets)) != 1: continue
        k1 = sets[0][2]
        tp = tests[0]
        q = m.rfind('(', 0, tp); ifpos = m.rfind('if', 0, q)
        qc = match_close(m, q, '(', ')')
        cond = m[q + 1:qc]
        if not re.fullmatch(r'[\s!()\w=<>&|-]*', cond): continue
        # the condition must test only the flag (other operands make the goto non-equivalent)
        rest = re.sub(r'\b%s\b' % re.escape(v), '', cond)
        if re.search(r'[A-Za-z_]\w*', re.sub(r'\b(true|false|TRUE|FALSE)\b', '', rest)): continue
        c0, c1 = cond_value(cond, v, k0), cond_value(cond, v, k1)
        if c0 is None or c0 == c1: continue
        form = 'skip' if c0 and not c1 else 'enter'
        bs = skipws(m, qc + 1); be = stmt_end(m, bs)
        k = skipws(m, be)
        if re.match(r'else\b', m[k:]): continue
        if any(s[0] > ifpos for s in sets): continue
        flow = all(flows_to(m, s[1], ifpos, ob) for s in sets)
        out.append(dict(var=v, k0=k0, k1=k1, sets=sets, ifpos=ifpos, bs=bs, be=be, decl=(dpos, ob + d.end()),
                        form=form, flow=flow, cond=' '.join(cond.split())))
    return out


def lineno(text, p): return text.count('\n', 0, p) + 1


def rewrite(text, f):
    v = f['var']; lab = '%s_past' % v
    edits = [(s[0], s[1], 'goto %s;' % lab) for s in f['sets']]
    blk = text[f['bs']:f['be']]
    nxt = skipws(text, f['be'])
    tail = ';' if text[nxt] == '}' else ''
    edits.append((f['ifpos'], f['be'], blk + '\n%s:%s' % (lab, tail)))
    dl, dh = f['decl']
    ls = text.rfind('\n', 0, dl) + 1
    le = text.find('\n', dh)
    if text[ls:dl].strip() == '' and text[dh:le].strip() == '': edits.append((ls, le + 1, ''))
    else: edits.append((dl, dh, ''))
    for a, b, s in sorted(edits, reverse=True): text = text[:a] + s + text[b:]
    return text


def units(args):
    if args: return [a if a.startswith('src/') else 'src/' + a for a in args]
    cc = json.load(open(R + '/build/compile_commands.json'))
    return sorted(e['file'][len(R) + 1:] for e in cc if e['file'].startswith(R + '/src/'))


def main():
    a = sys.argv[1:]
    if not a or a[0] not in ('list', 'deck'): print(__doc__); return
    if a[0] == 'list':
        alls = '--all' in a; us = [x for x in a[1:] if not x.startswith('--')]
        for u in units(us):
            try: text = open(R + '/' + u, encoding='latin-1').read()
            except Exception: continue
            m = mask(text)
            for addr, sym, ob, cb in functions(text, m, alls):
                for f in scan_fn(text, m, ob, cb):
                    print('%s 0x%08X %s %s sets=%s test=%d form=%s flow=%s cond=%s' % (
                        u, addr, sym[:40], f['var'], ','.join(str(lineno(text, s[0])) for s in f['sets']),
                        lineno(text, f['ifpos']), f['form'], 'ok' if f['flow'] else 'no', f['cond']))
        return
    unit, addr = a[1], int(a[2], 16)
    unit = unit if unit.startswith('src/') else 'src/' + unit
    var = a[a.index('--var') + 1] if '--var' in a else None
    sys.path.insert(0, T); sys.path.insert(0, S)
    import fe, sdeck
    text = open(R + '/' + unit, encoding='latin-1').read(); m = mask(text)
    deck = [('base', text)]
    for ad, sym, ob, cb in functions(text, m, True):
        if ad != addr: continue
        for f in scan_fn(text, m, ob, cb):
            if (var and f['var'] != var) or f['form'] != 'skip' or not f['flow']: continue
            deck.append(('goto:' + f['var'], rewrite(text, f)))
    if len(deck) == 1: print('no flow=ok skip-form flag'); return
    _, lo, hi = fe.ident(text, addr)
    out = sdeck.run(unit[4:], addr, lo, hi, deck)
    if '--write' in a:
        res = dict(out); base = res.get('base')
        best = [kv for kv in out if kv[0] != 'base' and kv[1] is not None]
        if best and (base is None or best[0][1] < base):
            open(R + '/' + unit, 'w', encoding='latin-1').write(dict(deck)[best[0][0]])
            print('wrote', best[0][0])


if __name__ == '__main__':
    main()
