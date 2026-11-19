#!/usr/bin/env python3
"""x87sym.py <unit> <addr> [--ours] [--names off=name,...]
Symbolic x87 evaluation of one function (MATCHING_GUIDE 3.8, VC5 x87): replays the straight-line x87 code of
the image (or, with --ours, of our compile of <unit>) and prints every store to memory as an expression tree
over its loads, with the operand order the code used (`a*b` = `fld a; fmul b`, left operand is the one on the
stack).  Memory operands are named by their address text; esp-relative names are normalised to the entry esp
(call pushes and `add/sub esp` are tracked), so image and ours line up.  Loads of a slot the function stored
earlier are shown as that stored expression (in braces), so spills are visible as `{...}`.
Branches are ignored (straight-line reading): use it on the x87 blocks.  Run from the repo root."""
import sys, os, re
sys.path.insert(0, os.getcwd() + '/recovery/tools')
import fe


def listing(unit, addr, ours):
    text = open('src/' + unit if not unit.startswith('src/') else unit, encoding='latin-1').read()
    sym, lo, hi = fe.ident(text, addr)
    if not ours:
        return [(i.mnemonic, i.op_str) for i in fe.md.disasm(fe.imgbytes(lo, hi), lo)]
    u = unit if unit.startswith('src/') else 'src/' + unit
    c, err = fe.compile_unit(u, os.path.abspath(u), int(os.environ.get('FE_PFX_BASE', '21')))
    if err:
        raise SystemExit(err)
    s = c.function(sym)
    data = c.sections[s.section - 1].data
    return [(i.mnemonic, i.op_str) for i in fe.md.disasm(bytes(data), 0)]


def run(ins, names=None):
    names = names or {}
    st = []          # x87 stack, st[0] = top
    esp = 0          # offset of esp from entry esp
    mem = {}
    regs = {}
    out = []

    def memname(op):
        m = re.match(r'(?:dword|qword|tbyte) ptr \[(.*)\]', op)
        if not m:
            return op
        a = m.group(1).replace(' ', '')
        mm = re.match(r'esp([+-]0x[0-9a-f]+|[+-]\d+)?$', a)
        if mm:
            off = int(mm.group(1), 0) if mm.group(1) else 0
            a = 'S%+d' % (off + esp)
            a = names.get(a, a)
        else:
            a = names.get(a, a)
        return a

    def load(op):
        n = memname(op)
        return mem.get(n, n) if n in mem else n

    def sti(op):
        m = re.match(r'st\((\d)\)', op)
        return int(m.group(1)) if m else 0
    for mn, op in ins:
        ops = [o.strip() for o in op.split(',')] if op else []
        try:
            if mn == 'sub' and ops[0] == 'esp':
                esp -= int(ops[1], 0)
            elif mn == 'add' and ops[0] == 'esp':
                esp += int(ops[1], 0)
            elif mn == 'push':
                esp -= 4
            elif mn == 'pop':
                esp += 4
            elif mn == 'call':
                st = []
            elif mn == 'fld':
                if ops[0].startswith('st'):
                    st.insert(0, st[sti(ops[0])])
                else:
                    v = load(ops[0])
                    st.insert(0, '{%s}' % v if memname(ops[0]) in mem else v)
            elif mn in ('fild',):
                st.insert(0, 'int(%s)' % memname(ops[0]))
            elif mn == 'fldz':
                st.insert(0, '0')
            elif mn == 'fld1':
                st.insert(0, '1')
            elif mn == 'fxch':
                i = sti(ops[0]) if ops else 1
                st[0], st[i] = st[i], st[0]
            elif mn in ('fst', 'fstp'):
                if ops[0].startswith('st'):
                    st[sti(ops[0])] = st[0]
                else:
                    n = memname(ops[0])
                    mem[n] = st[0]
                    out.append('%s = %s' % (n, st[0]))
                if mn == 'fstp':
                    st.pop(0)
            elif mn in ('fistp', 'fist'):
                out.append('%s = (int)%s' % (memname(ops[0]), st[0]))
                if mn == 'fistp':
                    st.pop(0)
            elif mn in ('fsin', 'fcos', 'fchs', 'fabs', 'fsqrt'):
                st[0] = '%s(%s)' % (mn[1:], st[0])
            elif mn[:4] in ('fmul', 'fadd', 'fsub', 'fdiv') or mn[:5] in ('fsubr', 'fdivr'):
                base = mn.rstrip('p') if mn.endswith('p') and mn not in ('fsubp', 'fdivp', 'faddp', 'fmulp', 'fsubrp', 'fdivrp') else mn
                pop = mn.endswith('p')
                o = {'fmul': '*', 'fadd': '+', 'fsub': '-', 'fdiv': '/', 'fsubr': '-', 'fdivr': '/'}[mn[:-1] if pop else mn]
                rev = 'r' in (mn[:-1] if pop else mn)
                if len(ops) == 1 and not ops[0].startswith('st'):
                    a, b = st[0], load(ops[0])
                    if memname(ops[0]) in mem:
                        b = '{%s}' % b
                    st[0] = '(%s%s%s)' % ((b, o, a) if rev else (a, o, b))
                else:
                    if len(ops) == 2:
                        d, s = sti(ops[0]), sti(ops[1])
                    elif pop:
                        d, s = (sti(ops[0]) if ops else 1), 0
                    else:
                        d, s = 0, sti(ops[0])
                    a, b = st[d], st[s]
                    st[d] = '(%s%s%s)' % ((b, o, a) if rev else (a, o, b))
                    if pop:
                        st.pop(0)
            elif mn in ('fcomp', 'fcom', 'fcompp'):
                out.append('cmp %s ? %s' % (st[0], ops[0] if ops else st[1]))
                if mn == 'fcomp':
                    st.pop(0)
                if mn == 'fcompp':
                    st.pop(0); st.pop(0)
        except (IndexError, KeyError) as e:
            out.append('!! %s %s (%s)' % (mn, op, e))
    return out


if __name__ == '__main__':
    a = sys.argv[1:]
    ours = '--ours' in a
    names = {}
    if '--names' in a:
        for kv in a[a.index('--names') + 1].split(','):
            k, v = kv.split('=')
            names[k] = v
    a = [x for i, x in enumerate(a) if not x.startswith('--') and (i == 0 or a[i - 1] != '--names')]
    for line in run(listing(a[0], int(a[1], 16), ours), names):
        print(line)
