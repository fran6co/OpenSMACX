#!/usr/bin/env python3
"""t.py: compile one probe text and print each function's frame: per offset, the named symbols with their
named-access counts, and `~N` = number of anonymous accesses (`-N+[ebp]`, compiler temporaries).
Usage: t.py FILE.cpp [--asm FUNC]"""
import sys, os, re, collections
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))) + '/inline')
import ib
PFX = os.environ.get('FE_PFX_BASE', '21')


def frames(asm):
    out = []
    for m in re.finditer(r'^(\S+)\s+PROC NEAR[^\n]*\n(.*?)^\1\s+ENDP', asm, re.S | re.M):
        name = m.group(1)
        j = asm.rfind('_TEXT\tSEGMENT', 0, m.start())
        syms = collections.defaultdict(list)
        for mm in re.finditer(r'^(\S+) = (-?\d+)', asm[j:m.start()], re.M):
            syms[int(mm.group(2))].append(mm.group(1))
        acc = collections.Counter()
        anon = collections.Counter()
        for mm in re.finditer(r'(\S+)\[ebp\]', m.group(2)):
            acc[mm.group(1)] += 1
        for mm in re.finditer(r'(-?\d+)\+\[ebp\]', m.group(2)):
            anon[int(mm.group(1))] += 1
        out.append((name, syms, acc, anon, m.group(2)))
    return out


def show(src_text, only=None, asm_of=None, quiet=False):
    asm, _ = ib.compile_asm(None, PFX, text=src_text)
    res = {}
    for name, syms, acc, anon, body in frames(asm):
        nm = re.sub(r'^\?|@.*$', '', name)
        if only and nm not in only:
            continue
        slots = []
        for off in sorted(set(syms) | set(anon), reverse=True):
            names = ['%s(%d)' % (re.sub(r'^_|\$$', '', x), acc[x]) for x in syms.get(off, [])]
            if anon[off]:
                names.append('~%d' % anon[off])
            slots.append('%d:%s' % (off, '/'.join(names)))
        res[nm] = slots
        if not quiet:
            print('%-10s %s' % (nm, '  '.join(slots)))
        if asm_of == nm:
            print('\n'.join(l for l in body.split('\n') if l and not l.startswith(';')))
    return res


if __name__ == '__main__':
    a = sys.argv[1:]
    asm_of = a[a.index('--asm') + 1] if '--asm' in a else None
    show(open(a[0]).read(), asm_of=asm_of)
