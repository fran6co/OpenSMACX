#!/usr/bin/env python3
"""retcensus.py [unit ...] [--json OUT] [--show CTX/FORM] [--all]
Return-statement census over the BYTE_EXACT functions (MATCHING_GUIDE 3.7 "Return-tail copies"): one /FAsc
compile per unit, then for every `return` statement of an exact function, how VC6 emitted it:

  COPY    the statement's own code ends in `ret` (it has its own epilogue copy)
  JMP     its code ends in `jmp` (value set here, epilogue shared)
  FOLD    no instruction is attributed to the line (the branch that guards it targets another statement's
          copy: cross-jumped or a conditional branch straight to a shared tail)
  FALL    code that falls into the next line (the last statement's epilogue, or a value set before a shared tail)

against the statement's context, read from the source text:

  last        the last statement of the function body
  guard       `if (c) return ..;` with a single condition (no && / ||), the return alone in the block
  guard&&     the same with && only;   guard||   with || (or mixed)
  after       preceded by another statement in the same block (`S; return ..;`): an unconditional path
  case        directly under a `case`/`default` label
  else        the only statement of an `else` block
and whether the returned expression is a constant shared with the function's LAST return (`=last`) or not.
Screen/census only (our listing equals the image for exact functions)."""
import sys, os, re, json, argparse, collections, subprocess
from concurrent.futures import ThreadPoolExecutor
T = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, T)
import fe  # noqa: E402
MARK = re.compile(r'// ORIGINAL: 0x([0-9A-F]{8}) (\S+) 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)')


def listing(unit, pfx):
    cmd = fe.compile_cmd(unit)
    if not cmd:
        return None
    asm = fe.SCR + '/rc_%s.cod' % pfx
    obj = fe.SCR + '/rc_%s.obj' % pfx
    out = []
    for t in cmd:
        if t.startswith('/Fo'):
            t = '/Fo' + obj
        elif t.startswith('/Fd') or t.startswith('--repro-root') or t == '/W4':
            continue
        out.append(t)
    out.insert(-1, '/FAsc')
    out.insert(-1, '/Fa' + asm)
    env = dict(os.environ, VC6_PREFIX='%s/.opensmacx/wine/worker-%s' % (fe.MAIN, pfx))
    os.path.exists(asm) and os.remove(asm)
    subprocess.run(out, cwd=fe.SCR, env=env, capture_output=True, text=True)
    if not os.path.exists(asm):
        return None
    t = open(asm, errors='replace').read().replace('\r', '')
    os.remove(asm)
    os.path.exists(obj) and os.remove(obj)
    return t


def context(L, n):
    """L: source lines (0-based list), n: 1-based line of the return statement."""
    s = L[n - 1].strip()
    j = n - 2
    while j >= 0 and (not L[j].strip() or L[j].strip().startswith('//')):
        j -= 1
    prev = L[j].strip() if j >= 0 else ''
    m = re.match(r'(?:\}\s*)?(?:else\s+)?if\s*\((.*)\)\s*(?:\{\s*)?return\b', s)
    if m:
        cond = m.group(1)
    elif prev.endswith('{') or prev.endswith(')'):
        # collect the whole controlling statement (it may span lines)
        k = j
        txt = prev
        while k > 0 and not re.match(r'(\}\s*)?(else\s+)?(if|for|while|switch|do)\b|\}?\s*else\b', L[k].strip()) and not L[k].strip().endswith(';') and k > j - 8:
            k -= 1
            txt = L[k].strip() + ' ' + txt
        if re.match(r'(\}\s*)?else\s*\{?$', L[k].strip()) or re.match(r'\}?\s*else\s*\{$', prev):
            return 'else'
        if not re.match(r'(\}\s*)?(else\s+)?if\b', L[k].strip()):
            return 'other'
        cond = txt
    elif re.match(r'(case\b.*|default\s*):$', prev):
        return 'case'
    elif prev.endswith(';') or prev.endswith('}'):
        return 'after'
    else:
        return 'other'
    if '||' in cond:
        return 'guard||'
    if '&&' in cond:
        return 'guard&&'
    return 'guard'


def scan_unit(unit, pfx, allm):
    text = open(fe.R + '/' + unit, encoding='latin-1').read()
    marks = {m.group(2): ('BYTE_EXACT' in m.group(3)) for m in MARK.finditer(text)}
    if not any(marks.values()) and not allm:
        return []
    t = listing(unit, pfx)
    if t is None:
        return []
    L = text.split('\n')
    rows = []
    for m in re.finditer(r'^(\S+)[ \t]PROC NEAR.*?\n(.*?)^\1[ \t]ENDP', t, re.S | re.M):
        sym = m.group(1)
        if sym not in marks or not (marks[sym] or allm):
            continue
        chunks = []  # (line, [ins])
        own = True
        for l in m.group(2).split('\n'):
            mm = re.match(r'; File (.*)$', l)
            if mm:
                own = mm.group(1).strip().lower().replace('\\', '/').endswith(os.path.basename(unit).lower())
                continue
            mm = re.match(r'; (\d+)\s*:', l)
            if mm:
                n = int(mm.group(1)) if own else -1
                if chunks and not chunks[-1][1]:
                    chunks[-1][0].append(n)
                else:
                    chunks.append(([n], []))
                continue
            mm = re.match(r'  [0-9a-f]{5}\t[0-9a-f ]+\t+ ?(\S.*)$', l)
            if mm and chunks:
                chunks[-1][1].append(mm.group(1).strip())
        def trivial(n):
            s = L[n - 1].strip() if 0 < n <= len(L) else 'x'
            return s in ('}', '') or s.startswith('//')
        seen = {}
        for k, (ns, ins) in enumerate(chunks):
            for q, n in enumerate(ns):
                if not (0 < n <= len(L) and re.search(r'\breturn\b', L[n - 1])):
                    continue
                if not all(trivial(x) for x in ns[q + 1:]):
                    form = 'FOLD'
                else:
                    nxt = chunks[k + 1] if k + 1 < len(chunks) else None
                    allins = list(ins)
                    epi = nxt and nxt[1] and all(re.match(r'(pop|leave|ret|add\s+esp|mov\s+esp|mov\s+ecx, DWORD PTR __\$EH|mov\s+DWORD PTR fs)', i) for i in nxt[1])
                    if nxt and (all(trivial(x) for x in nxt[0]) or epi) and not (ins and re.match(r'jmp|ret', ins[-1])):
                        allins += nxt[1]
                    if any(i.startswith('ret') for i in allins):
                        form = 'COPY'
                    elif not ins:
                        form = 'FOLD'
                    elif ins[-1].startswith('jmp'):
                        form = 'JMP'
                    else:
                        form = 'FALL'
                order = ['FOLD', 'FALL', 'JMP', 'COPY']
                if n not in seen or order.index(form) > order.index(seen[n]):
                    seen[n] = form
        if not seen:
            continue
        lastn = max(seen)
        mv = re.search(r'return\s*([^;]*);', L[lastn - 1])
        lastv = mv.group(1).strip() if mv else None
        for n, form in sorted(seen.items()):
            ctx = 'last' if n == lastn else context(L, n)
            mv = re.search(r'return\s*([^;]*);', L[n - 1])
            v = mv.group(1).strip() if mv else None
            same = '=last' if (n != lastn and v == lastv) else ''
            rows.append(dict(unit=unit, sym=sym, line=n, ctx=ctx + same, form=form, nret=len(seen), src=L[n - 1].strip()[:80]))
    return rows


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='*')
    ap.add_argument('--json')
    ap.add_argument('--show')
    ap.add_argument('--all', action='store_true')
    a = ap.parse_args()
    units = a.units
    if not units:
        units = sorted({e for e in (os.path.relpath(os.path.join(d, f), fe.R) for d, _, fs in os.walk(fe.R + '/src') for f in fs if f.endswith('.cpp'))
                        if fe.compile_cmd(e)})
    base = int(os.environ.get('FE_PFX_BASE', '21'))
    jobs = int(os.environ.get('FE_JOBS', '5'))
    lanes = [units[i::jobs] for i in range(jobs)]

    def lane(k):
        out = []
        for u in lanes[k]:
            try:
                out += scan_unit(u, str(base + k), a.all)
            except Exception as e:  # noqa: BLE001
                print('ERR', u, e, file=sys.stderr)
        return out
    rows = []
    with ThreadPoolExecutor(jobs) as ex:
        for o in ex.map(lane, range(jobs)):
            rows += o
    if a.json:
        json.dump(rows, open(a.json, 'w'))
    tab = collections.Counter((r['ctx'], r['form']) for r in rows)
    ctxs = sorted({c for c, _ in tab}, key=lambda c: -sum(tab[(c, f)] for f in ('COPY', 'JMP', 'FOLD', 'FALL')))
    print('%-16s %6s %6s %6s %6s' % ('context', 'COPY', 'JMP', 'FOLD', 'FALL'))
    for c in ctxs:
        print('%-16s %6d %6d %6d %6d' % (c, tab[(c, 'COPY')], tab[(c, 'JMP')], tab[(c, 'FOLD')], tab[(c, 'FALL')]))
    print('return statements: %d in %d functions' % (len(rows), len({(r['unit'], r['sym']) for r in rows})))
    if a.show:
        c, f = a.show.split('/')
        for r in rows:
            if r['ctx'] == c and r['form'] == f:
                print('  %s:%d %s  %s' % (r['unit'], r['line'], r['sym'][:40], r['src']))


if __name__ == '__main__':
    main()
