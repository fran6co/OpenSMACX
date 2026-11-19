#!/usr/bin/env python3
"""labelsubsets.py <unit> <0xADDR> [--max N] [--write]

Every subset of a function's label/table sites flipped between the written-out and the named form
(MATCHING_GUIDE 3.2 S7): `StringTable.get(Labels.strings_ptr[k])` <-> `label_get(k)` and
`StringTable.get(x)` <-> `table_text(x)`, one site per source line. schedscan --try covers single
sites and all sites at once; functions such as read_rules, ReportIf::draw_projects, ProdPicker::exec
and BaseWin::draw_commerce need a particular pair. 2^n compiles: functions with more than --max
sites (default 10) are refused. Prints the best subsets (bit i = site i flipped, in source order)
with fe rows; --write keeps the best one when it beats the base. A flip can move the /Ob2 budget:
compare analysis/callees.py before and after. Set FE_PFX_BASE / FE_JOBS."""
import sys, os, re, itertools, argparse
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
import fe, rules, perm
SITE = re.compile(r'StringTable\.get\(|label_get\(|table_text\(')


def flip(s):
    m = re.search(r'StringTable\.get\(Labels\.strings_ptr\[([^\]]+)\]\)', s)
    if m:
        return s.replace(m.group(0), 'label_get(%s)' % m.group(1), 1)
    m = re.search(r'label_get\(([^()]+)\)', s)
    if m:
        return s.replace(m.group(0), 'StringTable.get(Labels.strings_ptr[%s])' % m.group(1), 1)
    if 'StringTable.get(' in s:
        return s.replace('StringTable.get(', 'table_text(', 1)
    return s.replace('table_text(', 'StringTable.get(', 1)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('unit')
    ap.add_argument('addr')
    ap.add_argument('--max', type=int, default=10)
    ap.add_argument('--write', action='store_true')
    ap.add_argument('--top', type=int, default=8)
    a = ap.parse_args()
    unit = a.unit if a.unit.startswith('src/') else 'src/' + a.unit
    path = fe.R + '/' + unit
    t = open(path, encoding='latin-1').read()
    addr = int(a.addr, 16)
    marks = [(int(m.group(1), 16), m.start()) for m in rules.MARK.finditer(t)]
    me = [p for x, p in marks if x == addr]
    if not me:
        raise SystemExit('no marker')
    lo = me[0]
    hi = min([p for _, p in marks if p > lo] + [len(t)])
    L = t.split('\n')
    first, last = t.count('\n', 0, lo), t.count('\n', 0, hi)
    lines = [i for i in range(first, last) if SITE.search(L[i]) and not L[i].lstrip().startswith('//')]
    if not 1 <= len(lines) <= a.max:
        raise SystemExit('%d sites (need 1..%d)' % (len(lines), a.max))
    deck = []
    for bits in itertools.product('01', repeat=len(lines)):
        M = list(L)
        for b, i in zip(bits, lines):
            if b == '1':
                M[i] = flip(M[i])
        deck.append((''.join(bits), '\n'.join(M)))
    res = perm.run_deck(unit, '0x%08X' % addr, deck, jobs=int(os.environ.get('FE_JOBS', '5')))
    base = [r[1] for r in res if set(r[3]) == {'0'}][0]
    print('%d sites at lines %s; base %d rows' % (len(lines), ','.join(str(i + 1) for i in lines), base))
    for w, d, ex, key in res[:a.top]:
        print('  %4d%s %s' % (d, ' EXACT' if ex else '', key))
    if a.write and res[0][1] < base:
        open(path, 'w', encoding='latin-1').write(dict(deck)[res[0][3]])
        print('WROTE %s (%d -> %d)' % (res[0][3], base, res[0][1]))


if __name__ == '__main__':
    main()
