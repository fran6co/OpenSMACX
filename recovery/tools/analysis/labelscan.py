#!/usr/bin/env python3
"""labelscan.py [--from FEALL.json] [unit:addr ...] [--label L] [--json F]
MATCHING_GUIDE 3.7 "Label block placement" screen. For every function with goto labels: each label's
LAST reaching goto site in source order, classified claiming / non-claiming (the rule's forms; `?` when
the form is not one the probe measured), the placement the rule predicts (after that site, or at the
label's fall-through predecessor), where our compile lays the label block and where the image lays it.
Placement is named by the source line of the instruction laid just before the label block; the image's
is found through the image targets of our goto jumps (fe alignment), so a block the image lays elsewhere
is still located. `ret` counts are printed too (a claimed label behind a pure-return predecessor costs one
extra `ret`). Rows marked `MOVE` are labels whose image predecessor differs from ours. Screen only; read
each with fe.py --all before respelling. Lane FE_PFX_BASE (two compiles per unit: fe and /FAsc)."""
import sys, os, re, json, argparse, collections
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE)); sys.path.insert(0, HERE)
import fe
from regprio import cod_listing, func_source, strip_code

KEY = {'case', 'default', 'public', 'private', 'protected'}
LAB = re.compile(r'^\s*([A-Za-z_]\w*)\s*:(?!:)')
GOTO = re.compile(r'\bgoto\s+(\w+)\s*;')


def toplevel(cond, op):
    d = 0
    for i in range(len(cond) - 1):
        c = cond[i]
        if c == '(':
            d += 1
        elif c == ')':
            d -= 1
        elif d == 0 and cond[i:i + 2] == op:
            return True
    return False


def if_cond(text):
    """text starting at `if (`: the condition"""
    i = text.find('(')
    d = 0
    for j in range(i, len(text)):
        if text[j] == '(':
            d += 1
        elif text[j] == ')':
            d -= 1
            if d == 0:
                return text[i + 1:j]
    return text[i + 1:]


def head(code, j):
    """the statement text that ends on line j and starts at its `if (` (conditions spanning lines)"""
    s = code[j]
    k = j
    while k > 0 and not re.search(r'\bif\s*\(', code[k]) and j - k < 12:
        k -= 1
        s = code[k] + ' ' + s
    m = re.search(r'\bif\s*\(', s)
    return s[m.start():] if m else None


def if_kind(c):
    return '||' if toplevel(c, '||') else '&&' if toplevel(c, '&&') else 'c'


def classify(code, n):
    """code: stripped lines of the function; n: index of the goto line -> (form, claiming True/False/None)"""
    l = code[n]
    m = LAB.match(l)
    if m and m.group(1) not in KEY and re.match(r'^\s*\w+\s*:\s*goto\s+\w+\s*;\s*$', l):
        return 'label-goto', True
    if re.search(r'\)\s*goto\s+\w+\s*;', l) and not re.search(r'\belse\b', l):
        h = head(code, n)
        if h:
            k = if_kind(if_cond(h))
            return 'if(%s) goto' % ('A||B' if k == '||' else 'A&&B' if k == '&&' else 'c'), k != '||'
    else_goto = re.search(r'\belse\s+goto\b', l)
    if else_goto or (re.match(r'^\s*goto\b', l) and n > 0 and re.search(r'\belse\s*\{\s*$', code[n - 1])):
        k = n if else_goto else n - 1
        d = 0
        for j in range(k, -1, -1):
            s = code[j] if j != k else code[j][:code[j].find('else')]
            d += s.count('}') - s.count('{')
            if d <= 0 and j != k and '{' in code[j]:
                h = head(code, j)
                if not h:
                    break
                kd = if_kind(if_cond(h))
                return 'if(%s){S}else goto' % ('A||B' if kd == '||' else 'A&&B' if kd == '&&' else 'c'), \
                    True if kd == '||' else None if kd == '&&' else False
        return 'else goto', None
    if re.match(r'^\s*goto\b', l):
        p = n - 1
        while p >= 0 and not code[p].strip():
            p -= 1
        prev = code[p].strip() if p >= 0 else ''
        nxt = code[n + 1].strip() if n + 1 < len(code) else ''
        if prev.endswith('{') and nxt.startswith('}'):
            h = head(code, p)
            if h:
                kd = if_kind(if_cond(h))
                return 'if(%s){goto}' % ('A||B' if kd == '||' else 'A&&B' if kd == '&&' else 'c'), kd != '||'
            return '{goto}', None
        if prev.endswith(';') or prev.endswith('}') or prev.endswith('{'):
            return '{S; goto}', True
        return 'goto', None
    return 'other', None


def scan(unit, addr, only=None):
    path = fe.R + '/' + unit
    text = open(path, encoding='latin-1').read()
    first, lines = func_source(text, addr)
    code = [strip_code(l) for l in lines]
    labs = {}
    gotos = collections.defaultdict(list)
    for i, l in enumerate(code):
        m = LAB.match(l)
        if m and m.group(1) not in KEY and not l.strip().startswith(('std::', '::')):
            labs[m.group(1)] = i
        for g in GOTO.finditer(l):
            gotos[g.group(1)].append(i)
    labs = {k: v for k, v in labs.items() if k in gotos and (not only or k in only)}
    if not labs:
        return []
    c, err = fe.compile_unit(unit, path, fe.PFX)
    if c is None:
        return [dict(addr='%08X' % addr, error='compile ' + (err or ''))]
    r = fe.evaluate(c, text, addr)
    if 'error' in r:
        return [dict(addr='%08X' % addr, error=r['error'])]
    items, _ = cod_listing(unit, path, fe.PFX, r['sym'])
    off2line = {}
    line_first = {}
    cur = None
    for it in items:
        if it[0] == 'line':
            cur = it[1]
        elif it[0] == 'ins' and cur is not None:
            off2line[it[1]] = cur
            line_first.setdefault(cur, it[1])
    rows = r['rows']
    c2o = {}
    for k, o, c_ in rows:
        if o is not None and c_ is not None:
            c2o[c_[0]] = o
    ours_seq = [c_ for k, o, c_ in rows if c_ is not None]
    ours_seq.sort()
    img_seq = sorted([o for k, o, c_ in rows if o is not None])
    o2c = {}
    for k, o, c_ in rows:
        if o is not None and c_ is not None:
            o2c[o[0]] = c_[0]
    rets_i = sum(1 for o in img_seq if o[1].startswith('ret'))
    rets_o = sum(1 for c_ in ours_seq if c_[1].startswith('ret'))
    out = []

    def srcline(ln):
        return ln - first if ln is not None else None   # index into lines

    def line_of_c(off):
        return off2line.get(off)

    for L, li in sorted(labs.items(), key=lambda kv: kv[1]):
        sites = gotos[L]
        last = max(sites)
        form, claim = classify(code, last)
        # our label offset: first instruction of the first listed line at or after the label line
        abs_lab = first + li
        cands = [ln for ln in line_first if abs_lab <= ln <= abs_lab + 6]
        if not cands:
            out.append(dict(addr='%08X' % addr, unit=unit, label=L, error='label line not in listing'))
            continue
        loff = line_first[min(cands)]
        # all instruction offsets of that line: the label block may start at any of them after a jump
        targets = collections.Counter(c_[3] for c_ in ours_seq if c_[3] is not None)
        lab_offs = [o for o, ln in off2line.items() if ln == min(cands)]
        loff = min(lab_offs, key=lambda o: (-targets.get(o, 0), o))
        idx = next(i for i, c_ in enumerate(ours_seq) if c_[0] == loff) if any(c_[0] == loff for c_ in ours_seq) else None
        ours_pred = line_of_c(ours_seq[idx - 1][0]) if idx else None
        ours_pred_ins = ours_seq[idx - 1][1] if idx else ''
        # image label address: targets of image jumps aligned with our jumps to loff
        itg = collections.Counter()
        for k, o, c_ in rows:
            if c_ is not None and c_[3] == loff and o is not None and o[3] is not None:
                itg[o[3]] += 1
        if itg:
            iaddr = itg.most_common(1)[0][0]
        else:
            iaddr = c2o.get(loff, (None,))[0]
        img_pred = img_pred_ins = None
        if iaddr is not None:
            j = next((i for i, o in enumerate(img_seq) if o[0] == iaddr), None)
            if j:
                img_pred_ins = img_seq[j - 1][1]
                for jj in range(j - 1, -1, -1):
                    if img_seq[jj][0] in o2c:
                        img_pred = line_of_c(o2c[img_seq[jj][0]])
                        break
        pred_site = srcline(last) + first if claim else None
        pred = ('after site %d' % (first + last)) if claim else ('fall-through (after %d)' % (first + li - 1)) if claim is False else '?'
        out.append(dict(addr='%08X' % addr, unit=unit, sym=r['sym'][:40], label=L, label_line=first + li,
                        sites=[first + s for s in sites], last=first + last, form=form, claiming=claim,
                        predicted=pred, ours_after=ours_pred, ours_ins=ours_pred_ins,
                        img_after=img_pred, img_ins=img_pred_ins,
                        img_label='%x' % (r['lo'] + iaddr) if iaddr is not None else None,
                        move=ours_pred != img_pred, rets=(rets_i, rets_o), rows=r['diff']))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('targets', nargs='*')
    ap.add_argument('--from', dest='frm')
    ap.add_argument('--label', default='')
    ap.add_argument('--json')
    ap.add_argument('--moved', action='store_true', help='print only labels whose image predecessor differs')
    a = ap.parse_args()
    tg = []
    if a.frm:
        for e in json.load(open(a.frm)):
            if not e.get('exact'):
                tg.append((e['unit'], int(e['addr'], 16)))
    for t in a.targets:
        u, ad = t.split(':')
        tg.append((u if u.startswith('src/') else 'src/' + u, int(ad, 16)))
    only = set(a.label.split(',')) - {''}
    res = []
    for u, ad in tg:
        t = open(fe.R + '/' + u, encoding='latin-1').read()
        try:
            first, lines = func_source(t, ad)
        except SystemExit:
            continue
        if not GOTO.search('\n'.join(strip_code(l) for l in lines)):
            continue
        for e in scan(u, ad, only):
            res.append(e)
            if 'error' in e:
                print('ERR', e['addr'], e.get('label', ''), e['error'][:200], flush=True)
                continue
            if a.moved and not e['move']:
                continue
            print('%s %s %-10s %-4s lab@%d last@%d %-22s claim=%-5s pred=%-26s ours_after=%s img_after=%s %s rets=%s/%s rows=%d [img prev: %s | ours prev: %s]' % (
                e['addr'], e['sym'][:28], e['label'], 'MOVE' if e['move'] else '', e['label_line'], e['last'], e['form'],
                e['claiming'], e['predicted'], e['ours_after'], e['img_after'], e['img_label'], e['rets'][0], e['rets'][1],
                e['rows'], e['img_ins'], e['ours_ins']), flush=True)
    if a.json:
        json.dump(res, open(a.json, 'w'), indent=1)


if __name__ == '__main__':
    main()
