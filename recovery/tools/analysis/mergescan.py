#!/usr/bin/env python3
"""mergescan.py <unit> <addr[,addr]> [--src ALT.cpp] [--pfx N] | --from FEALL.json [--pfx N] [--json F]

Tail-merge reference screen (MATCHING_GUIDE 2.7 "Tail-merged references count").  VC6 allocates
registers BEFORE it tail-merges / cross-jumps identical code, so every written-out copy of a merged
statement counts for the candidates it references, although only one copy is left in the code.
CSE, copy propagation and dead-code removal run BEFORE allocation: references they remove count
nothing.  So the same instructions can come from sources with different priorities, and the lever is
the number of source copies of a merged statement:

  - ours writes a statement in several arms and VC6 merges them (`MERGED` lines): each copy counts
    for its operands.  If the image ranks one of those operands LOWER than ours, the original wrote
    the statement once after the join (say_orders: three `strcat(text, ...)` arms -> one strcat
    with the label chosen in the arms, exact).
  - ours reaches one statement through `goto` or a join (`GOTO` lines) where the image ranks an
    operand of that statement HIGHER: the original wrote the statement out in each arm (popp:
    `start(caption, ...)` in every failing arm instead of `goto plain`, exact).

A MERGED line is a source line whose call has no `call` instruction attributed to it in our /FAsc
listing while another line of the function calls the same callee (an inline expansion is not
reported: its callee is never called out of line).  Each line lists the identifiers of the call's
arguments; compare them with the register pairs candset.py reports as ordered differently."""
import sys, os, re, json, argparse, collections
from concurrent.futures import ProcessPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.dirname(HERE))
import fe
import regprio

KW = {'if', 'while', 'for', 'switch', 'return', 'sizeof', 'case', 'else', 'do', 'static_cast',
      'reinterpret_cast', 'const_cast', 'new', 'delete'}


def calls_on(code):
    """[(callee, args text)] of the call expressions on a stripped source line"""
    out = []
    for m in re.finditer(r'([A-Za-z_]\w*)\s*\(', code):
        if m.group(1) in KW:
            continue
        depth, k = 1, m.end()
        while k < len(code) and depth:
            depth += {'(': 1, ')': -1}.get(code[k], 0)
            k += 1
        out.append((m.group(1), code[m.end():k - 1]))
    return out


def scan(unit, addr, src=None, pfx='8'):
    text = open(src or fe.R + '/' + unit, encoding='latin-1').read()
    a = int(addr, 16)
    try:
        idn = fe.ident(text, a)
    except Exception as e:
        return None
    first, lines = regprio.func_source(text, a)
    items, _ = regprio.cod_listing(unit, src or fe.R + '/' + unit, pfx, idn[0])
    called = collections.defaultdict(list)   # line -> [call operand text]
    cur = None
    for it in items:
        if it[0] == 'line':
            cur = it[1]
        elif it[0] == 'ins' and it[2].startswith('call ') and cur is not None:
            called[cur].append(it[2][5:])
    allcalls = [c for v in called.values() for c in v]

    def out_of_line(name):
        return [c for c in allcalls if re.search(r'\b\??%s@' % re.escape(name), c) or c.strip('_') == name]
    merged, gotos = [], []
    stmt = {}
    # join continuation lines: a statement's calls are attributed to its first line by /FAsc
    n = 0
    while n < len(lines):
        ln = first + n
        code = regprio.strip_code(lines[n])
        k = n
        while code.count('(') > code.count(')') and k + 1 < len(lines):
            k += 1
            code += ' ' + regprio.strip_code(lines[k]).strip()
        stmt[ln] = (code, k - n)
        n = k + 1
    for ln, (code, extra) in stmt.items():
        m = re.search(r'\bgoto\s+(\w+)', code)
        if m:
            gotos.append((ln, m.group(1)))
        span = range(ln, ln + extra + 1)
        here = [c for l in span for c in called.get(l, [])]
        for name, args in calls_on(code):
            if not out_of_line(name):
                continue
            if any(re.search(r'\b\??%s@' % re.escape(name), c) or c.strip('_') == name for c in here):
                continue
            ids = sorted(set(re.findall(r'\b([A-Za-z_]\w*)\b', args)) - KW)
            others = sorted({l for l, v in called.items() for c in v
                             if re.search(r'\b\??%s@' % re.escape(name), c) or c.strip('_') == name})
            merged.append((ln, name, args.strip()[:60], ids, others))
    return {'unit': unit, 'addr': addr, 'sym': idn[0], 'merged': merged, 'gotos': gotos}


def show(r):
    if not r:
        return
    print('MS %s %s  merged=%d gotos=%d' % (r['addr'], r['sym'][:60], len(r['merged']), len(r['gotos'])))
    for ln, name, args, ids, others in r['merged']:
        print('  MERGED L%-5d %s(%s)  ids=%s  kept at L%s' % (ln, name, args, ','.join(ids), ',L'.join(map(str, others))))
    for ln, lab in r['gotos']:
        print('  GOTO   L%-5d -> %s' % (ln, lab))


def _lane(job):
    rows, pfx = job
    out = []
    for r in rows:
        try:
            out.append(scan(r['unit'], r['addr'], None, pfx))
        except Exception as e:
            print('MS %s FAILED %s' % (r['addr'], e), file=sys.stderr)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('unit', nargs='?')
    ap.add_argument('addr', nargs='?')
    ap.add_argument('--src')
    ap.add_argument('--pfx', default=os.environ.get('FE_PFX_BASE', '8'))
    ap.add_argument('--from', dest='frm', help='feall.py --out JSON: scan its non-exact functions')
    ap.add_argument('--json')
    a = ap.parse_args()
    if a.frm:
        rows = [r for r in json.load(open(a.frm)) if not r['exact']]
        nj = int(os.environ.get('FE_JOBS', '5'))

        res = []
        with ProcessPoolExecutor(nj) as ex:  # cod_listing names its files by pid
            for o in ex.map(_lane, [(rows[i::nj], str(int(a.pfx) + i)) for i in range(nj)]):
                res += [x for x in o if x]
        res.sort(key=lambda r: -len(r['merged']))
        for r in res:
            if r['merged'] or r['gotos']:
                show(r)
        if a.json:
            json.dump(res, open(a.json, 'w'), indent=1)
        return
    for ad in a.addr.split(','):
        show(scan(a.unit, ad, a.src, a.pfx))


if __name__ == '__main__':
    main()
