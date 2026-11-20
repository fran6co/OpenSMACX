#!/usr/bin/env python3
"""teardowns.py <unit> <0xADDR> [FIRST_KEY] [--src ALT.cpp]

Per scope-exit teardown, image against ours: the calls each teardown leaves, from its first call
(FIRST_KEY, default `?close@Popup`: the expanded ~Popup's first call) to the return, following jumps
(`|` = a jump into another teardown's copy, i.e. a cross-jumped tail, MATCHING_GUIDE 2.8 rule 9).
Read a teardown's decision from its calls: `~Scroll` = called, `Scr.close ~FlatButton ...` = expanded.
Call counts hide this: 28 identical `call ~BasePop` tails cross-jump into one or two copies, which
read as "~BasePop expanded where ~Scroll is refused" (2026-11-20 par-4, terraform)."""
import sys, os, re, argparse
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.dirname(HERE))
sys.path.insert(0, os.path.dirname(os.path.dirname(HERE)))
import fe  # noqa: E402
import inlinemap as im  # noqa: E402
import callees as _callees  # noqa: E402
import predict  # noqa: E402
im.CRT.update(_callees.CRT)


def short(n):
    m = re.match(r'\?\?1(\w+)', n)
    if m:
        return '~' + m.group(1)
    m = re.match(r'\?(\w+)@(\w+)', n)
    return m.group(2)[:3] + '.' + m.group(1) if m else n


def walk(ins, j, is_jmp, target, callee):
    toks, hops = [], 0
    while j < len(ins) and hops <= 5:
        if ins[j] == 'RET':
            toks.append('RET')
            break
        t = target(ins[j])
        if is_jmp(ins[j]):
            toks.append('|')
            hops += 1
            if t is None:
                break
            j = t
            continue
        c = callee(ins[j])
        if c:
            toks.append(short(c))
        j += 1
    return toks


def ours(unit, src, sym, first):
    asm = predict.compile_listing(unit, src, os.environ.get('FE_PFX_BASE', '21'), False)
    m = re.search(r'^%s PROC NEAR.*?^%s ENDP' % (re.escape(sym), re.escape(sym)), asm, re.S | re.M)
    ins = []
    for l in m.group(0).split('\n'):
        ml = re.match(r'^(\$\w+):', l)
        if ml:
            ins.append(ml.group(1) + ':')
        elif l.startswith('\t') and not l.startswith('\t;'):
            s = l.strip().split(';')[0].strip()
            if s:
                ins.append('RET' if s.startswith('ret') else s)
    lab = {x[:-1]: i for i, x in enumerate(ins) if x.endswith(':')}

    def target(s):
        mj = re.match(r'jmp\s+(\$\w+)', s)
        return lab[mj.group(1)] + 1 if mj and mj.group(1) in lab else None

    def callee(s):
        mc = re.match(r'call\s+(\S+)', s)
        return mc.group(1) if mc else None
    return [walk(ins, i + 1, lambda s: s.startswith('jmp'), target, callee)
            for i, s in enumerate(ins) if callee(s) and im.key(callee(s)) == first]


def image(lo, hi, first):
    ins = list(fe.md.disasm(fe.imgbytes(lo, hi), lo))
    at = {x.address: i for i, x in enumerate(ins)}

    def tgt(x):
        try:
            return int(x.op_str, 16)
        except ValueError:
            return None

    def name(t):
        return im.key(im.CRT.get(t) or im.A2S.get(t, 'sub_%x' % t))
    seq = ['RET' if x.mnemonic == 'ret' else x for x in ins]
    return [walk(seq, i + 1, lambda x: x != 'RET' and x.mnemonic == 'jmp',
                 lambda x: at.get(tgt(x)) if x != 'RET' and x.mnemonic == 'jmp' else None,
                 lambda x: name(tgt(x)) if x != 'RET' and x.mnemonic == 'call' and tgt(x) else None)
            for i, x in enumerate(ins) if x.mnemonic == 'call' and tgt(x) and name(tgt(x)) == first]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('unit')
    ap.add_argument('addr')
    ap.add_argument('first', nargs='?', default='?close@Popup')
    ap.add_argument('--src')
    a = ap.parse_args()
    unit = a.unit if a.unit.startswith('src/') else 'src/' + a.unit
    src = a.src or fe.R + '/' + unit
    text = open(src, encoding='latin-1').read()
    sym, lo, hi = fe.ident(text, int(a.addr, 16))
    I, O = image(lo, hi, a.first), ours(unit, src, sym, a.first)
    for k in range(max(len(I), len(O))):
        i = ' '.join(I[k]) if k < len(I) else '-'
        o = ' '.join(O[k]) if k < len(O) else '-'
        print('%2d %s %s' % (k + 1, '  ' if i == o else '!!', i))
        if i != o:
            print('      ours %s' % o)


if __name__ == '__main__':
    main()
