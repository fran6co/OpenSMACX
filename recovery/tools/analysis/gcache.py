#!/usr/bin/env python3
"""gcache.py [--from FEALL.json] [unit ...] [--addr A[,A]] [--min N] [--json F] [-v]
Global-cache screen (MATCHING_GUIDE 2.7 "A global cached across a loop"): per non-exact function, the
absolute globals (scalars and constant-offset fields: `[0xADDR]` operands) that one side reads from memory
more often than the other. The side with fewer reads keeps the value in a register web across blocks
(loaded once, reloaded only after calls); the other reloads it per use.

Image addresses are paired with our relocation symbols through fe's equal rows (and replace rows of the
same mnemonic). Per paired global it prints the read counts, the registers it is loaded into on each side
(`*` marks a callee-saved register: a web that spans a call) and the function's call count:

  GC <addr> <unit> <sym> diff=<rows> calls=<img>/<ours>
     <our symbol>+<k> (0x<image addr>)  reads img <n> ours <m>  loads img [regs] ours [regs]  IMG-WEB|OURS-WEB

IMG-WEB = the image reads the global fewer times (it caches it, ours reloads); OURS-WEB the reverse.
Screen only: read both listings (fe.py --all) before trying a spelling (early `continue` vs nested body,
a local copy, which statement uses the global last). Env: FE_PFX_BASE (lane)."""
import sys, os, re, json, argparse, collections
H = os.path.dirname(os.path.abspath(__file__))
T_ = os.path.dirname(H)
R = os.path.dirname(os.path.dirname(T_))
sys.path.insert(0, T_)
os.chdir(R)
import fe
import capstone
from capstone import x86 as X

md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
md.detail = True
CS = {'esi', 'edi', 'ebx', 'ebp'}


def absmem(ins):
    """-> (disp, is_read, dest_reg_or_None) for an operand [disp] with no base/index, else None."""
    for i, op in enumerate(ins.operands):
        if op.type == X.X86_OP_MEM and op.mem.base == 0 and op.mem.index == 0 and op.mem.segment == 0:
            disp = op.mem.disp & 0xffffffff
            write = bool(op.access & capstone.CS_AC_WRITE)
            read = bool(op.access & capstone.CS_AC_READ) or ins.mnemonic in ('push', 'cmp', 'test')
            dest = None
            if ins.mnemonic in ('mov', 'movsx', 'movzx') and i == 1 and ins.operands[0].type == X.X86_OP_REG:
                dest = ins.reg_name(ins.operands[0].reg)
            return disp, read and not (ins.mnemonic == 'mov' and i == 0), dest
    return None


def disraw(data, org):
    return {ins.address - org: ins for ins in md.disasm(data, org)}


def scan(c, text, addr):
    r = fe.evaluate(c, text, addr)
    if 'error' in r or r['exact']:
        return None
    lo = r['lo']
    idn = fe.ident(text, addr)
    _, ilo, ihi = idn
    I = disraw(fe.imgbytes(ilo, ihi), ilo)
    s = c.function(r['sym'])
    sec = c.sections[s.section - 1]
    O = disraw(sec.data, 0)
    rel = {}
    for ro, si, ty in sec.relocations:
        rel[ro] = (c.symbols[si].name if si in c.symbols else '?')

    def okey(ins):
        a = absmem(ins)
        if not a:
            return None
        for k in range(ins.size):
            if ins.address + k in rel:
                return ('%s+%d' % (rel[ins.address + k], a[0]), a)
        return None

    def ikey(ins):
        a = absmem(ins)
        if not a or not (0x400000 <= a[0] < 0xa00000):
            return None
        return (a[0], a)
    pair = collections.defaultdict(collections.Counter)
    for k, o, cc in r['rows']:
        if o is None or cc is None:
            continue
        io, oo = I.get(o[0]), O.get(cc[0])
        if io is None or oo is None or io.mnemonic != oo.mnemonic:
            continue
        a, b = ikey(io), okey(oo)
        if a and b:
            pair[b[0]][a[0]] += 1
    imap, best = {}, {}
    for ok, cnt in pair.items():
        ia, n = cnt.most_common(1)[0]
        # one image address claimed by two of our symbols is misalignment: keep the stronger pairing
        if ia in best and best[ia][1] >= n:
            continue
        if ia in best:
            imap.pop(best[ia][0], None)
        best[ia] = (ok, n)
        imap[ok] = ia
    stats = collections.defaultdict(lambda: dict(ir=0, orr=0, il=[], ol=[]))
    icalls = sum(1 for x in I.values() if x.mnemonic == 'call' and x.address < ihi)
    ocalls = sum(1 for x in O.values() if x.mnemonic == 'call')
    inv = {v: k for k, v in imap.items()}
    for off, ins in I.items():
        k = ikey(ins)
        if k and k[0] in inv:
            st = stats[inv[k[0]]]
            if k[1][1]:
                st['ir'] += 1
            if k[1][2]:
                st['il'].append(k[1][2])
    for off, ins in O.items():
        k = okey(ins)
        if k and k[0] in imap:
            st = stats[k[0]]
            if k[1][1]:
                st['orr'] += 1
            if k[1][2]:
                st['ol'].append(k[1][2])
    out = []
    for ok, st in stats.items():
        if st['ir'] != st['orr']:
            tag = 'IMG-WEB' if st['ir'] < st['orr'] else 'OURS-WEB'
            out.append(dict(key=ok, img=imap[ok], ir=st['ir'], orr=st['orr'],
                            il=st['il'], ol=st['ol'], tag=tag))
    return dict(addr=addr, sym=r['sym'], diff=r['diff'], icalls=icalls, ocalls=ocalls, g=out)


def fmtregs(rs):
    return '[' + ','.join(x + ('*' if x in CS else '') for x in rs) + ']'


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='*')
    ap.add_argument('--from', dest='frm')
    ap.add_argument('--addr')
    ap.add_argument('--min', type=int, default=1)
    ap.add_argument('--json')
    ap.add_argument('--pfx', default=fe.PFX)
    ap.add_argument('-v', action='store_true')
    a = ap.parse_args()
    todo = collections.defaultdict(list)
    if a.frm:
        for e in json.load(open(a.frm)):
            if not e.get('exact'):
                todo[e['unit']].append(int(e['addr'], 16))
    for u in a.units:
        u = u if u.startswith('src/') else 'src/' + u
        if a.addr:
            todo[u] += [int(x, 16) for x in a.addr.split(',')]
    if a.units and a.frm:
        keep = {u if u.startswith('src/') else 'src/' + u for u in a.units}
        todo = {u: v for u, v in todo.items() if u in keep}
    res = []
    for unit, addrs in sorted(todo.items()):
        text = open(R + '/' + unit, encoding='latin-1').read()
        c, err = fe.compile_unit(unit, R + '/' + unit, a.pfx)
        if c is None:
            print('COMPILE FAIL', unit)
            continue
        for addr in addrs:
            try:
                x = scan(c, text, addr)
            except Exception as e:
                print('ERR %08X %s' % (addr, e))
                continue
            if not x:
                continue
            g = [y for y in x['g'] if abs(y['ir'] - y['orr']) >= a.min]
            if not g:
                continue
            x['unit'] = unit
            res.append(x)
            print('GC %08X %s %s diff=%d calls=%d/%d' % (addr, unit, x['sym'][:50], x['diff'], x['icalls'], x['ocalls']))
            for y in sorted(g, key=lambda y: -abs(y['ir'] - y['orr'])):
                print('   %-40s (0x%06X) reads img %d ours %d  loads img %s ours %s  %s' % (
                    y['key'][:40], y['img'], y['ir'], y['orr'], fmtregs(y['il']), fmtregs(y['ol']), y['tag']))
            sys.stdout.flush()
    if a.json:
        json.dump(res, open(a.json, 'w'), indent=1)


if __name__ == '__main__':
    main()
