#!/usr/bin/env python3
"""schedscan.py <unit> <0xADDR> [--src ALT.cpp] [--pfx N] [--gap N] [--rows]

Scheduling / scratch-register diagnosis for a function whose difference from the image is
(mostly) which of eax/ecx/edx a value gets and in which order independent instructions come.
Read with MATCHING_GUIDE 3.2 "Scratch registers and scheduling" (rules S1..S6).

1. Windows. Differing runs of the fe.py alignment closer than --gap equal rows are merged.
   Each window is classified with eax/ecx/edx (and their 8/16-bit parts) abstracted to R:
     REGS     same instructions in the same order; only the scratch register names differ.
              `rot+k` when one rotation eax->ecx->edx (k steps) maps ours onto the image.
     SCHED    same instructions in another order (after abstraction): scheduling. The moved
              instructions are listed in image order.
     LOADS    same instructions with memory operands exchanged between rows: which value is
              loaded first / becomes the destination (rule S4: reference order of the operands).
     OTHER    different instructions: not a scheduling difference; read it with fe.py.
2. Cursor track (rule S1). VC6 hands out scratch registers round-robin eax -> ecx -> edx in
   code-generation order, continuing through the whole function. Every instruction that starts
   a new value in a scratch register (mov/movsx/movzx/lea r,..., xor r,r) is an allocation; a
   thiscall receiver (`mov ecx, X` right before its call) is fixed and is skipped. Allocations
   are paired along the abstracted alignment and each pair gets the offset
   (ours - image) mod 3. The track prints where the offset changes: the first change is where
   our source allocates one register more (offset 1) or one fewer (offset 2) than the
   original, somewhere in the code generated since the previous allocation in sync.
Screen only: confirm a fix with fe.py."""
import sys, os, re, argparse, difflib
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
import fe

SCR = ('eax', 'ecx', 'edx')
PARTS = {'eax': ('eax', 'ax', 'al', 'ah'), 'ecx': ('ecx', 'cx', 'cl', 'ch'), 'edx': ('edx', 'dx', 'dl', 'dh')}
RX = re.compile(r'\b(e?[acd]x|[acd][lh])\b')
ROT = {1: {'eax': 'ecx', 'ecx': 'edx', 'edx': 'eax'}, 2: {'eax': 'edx', 'ecx': 'eax', 'edx': 'ecx'}}


def owner(r):
    for k, v in PARTS.items():
        if r in v:
            return k
    return None


def abstract(ins):
    return RX.sub(lambda m: 'R' + ('8' if m.group(1)[-1] in 'lh' else ''), ins)


def rename(ins, perm):
    def sub(m):
        r = m.group(1)
        o = owner(r)
        return PARTS[perm[o]][PARTS[o].index(r)]
    return RX.sub(sub, ins)


def alloc_reg(seq, i):
    """scratch register newly defined by seq[i], or None"""
    ins = seq[i]
    m = re.match(r'(mov|movsx|movzx|lea) (e[acd]x|[acd][lh]), (.*)', ins)
    if m:
        dst = owner(m.group(2))
        if m.group(1) == 'mov' and m.group(2)[-1] in 'lh':
            return None  # byte load into a zeroed register: the xor allocated it
        if dst in [owner(r) for r in RX.findall(m.group(3))]:
            return None  # mov eax, [eax+4]: reuses the register
        if dst == 'ecx' and m.group(1) == 'mov':
            for j in range(i + 1, min(len(seq), i + 4)):
                if seq[j].startswith('call'):
                    return None  # thiscall receiver
                if 'ecx' in seq[j] or 'cl' in RX.findall(seq[j]):
                    break
        return dst
    m = re.match(r'xor (e[acd]x), (e[acd]x)$', ins)
    if m and m.group(1) == m.group(2):
        return m.group(1)
    return None


def merge(rs, gap):
    out = []
    for s, e in rs:
        if out and s - out[-1][1] <= gap:
            out[-1] = (out[-1][0], e)
        else:
            out.append((s, e))
    return out


def classify(img, ours):
    ai, ao = [abstract(x) for x in img], [abstract(x) for x in ours]
    if ai == ao:
        for k, rot in ROT.items():
            if [rename(x, rot) for x in ours] == img:
                return 'REGS', 'rot+%d' % k
        return 'REGS', ''
    if sorted(ai) == sorted(ao):
        return 'SCHED', ''
    mem = re.compile(r'(?:byte|word|dword) ptr \[[^\]]+\]')
    if [mem.sub('M', x) for x in ai] == [mem.sub('M', x) for x in ao] and sorted(
            m for x in img for m in mem.findall(x)) == sorted(m for x in ours for m in mem.findall(x)):
        return 'LOADS', ''
    return 'OTHER', ''


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('unit')
    ap.add_argument('addr')
    ap.add_argument('--src')
    ap.add_argument('--pfx', default=os.environ.get('FE_PFX_BASE', '27'))
    ap.add_argument('--gap', type=int, default=4, help='merge differing runs separated by at most this many equal rows')
    ap.add_argument('--rows', action='store_true', help='print the rows of every window')
    a = ap.parse_args()
    unit = a.unit if a.unit.startswith('src/') else 'src/' + a.unit
    path = os.path.abspath(a.src) if a.src else fe.R + '/' + unit
    text = open(path, encoding='latin-1').read()
    c, err = fe.compile_unit(unit, path, a.pfx)
    if c is None:
        raise SystemExit('COMPILE FAIL\n' + err)
    addr = int(a.addr, 16)
    res = fe.evaluate(c, text, addr)
    if 'error' in res:
        raise SystemExit(res['error'])
    rows, lo = res['rows'], res['lo']
    print('FE %08X %s diff=%d img=%d ours=%d' % (addr, res['sym'][:50], res['diff'], res['img'], res['ours']))

    # 1. windows
    rs, i = [], 0
    while i < len(rows):
        if rows[i][0] != '  ':
            j = i
            while j < len(rows) and rows[j][0] != '  ':
                j += 1
            rs.append((i, j))
            i = j
        else:
            i += 1
    kinds = {}
    for s, e in merge(rs, a.gap):
        img = [r[1][1] for r in rows[s:e] if r[1]]
        ours = [r[2][1] for r in rows[s:e] if r[2]]
        kind, det = classify(img, ours)
        kinds[kind] = kinds.get(kind, 0) + 1
        at = next((lo + r[1][0] for r in rows[s:e] if r[1]), None)
        print('%-5s %-6s %s  %d img / %d ours' % (kind, det, ('%x' % at) if at else '?', len(img), len(ours)))
        if kind == 'SCHED':
            ai, ao = [abstract(x) for x in img], [abstract(x) for x in ours]
            sm = difflib.SequenceMatcher(None, ao, ai, autojunk=False)
            for op, a1, a2, b1, b2 in sm.get_opcodes():
                if op in ('insert', 'replace'):
                    for k in range(b1, b2):
                        print('        image places `%s` %s' % (img[k], 'earlier' if ai[k] in ao[a2:] else 'later' if ai[k] in ao[:a1] else 'here'))
        if a.rows or kind in ('OTHER', 'LOADS'):
            for r in rows[s:e]:
                o, q = r[1], r[2]
                print('        %s %-6s %-40s | %s' % (r[0], ('%x' % (lo + o[0])) if o else '', o[1] if o else '', q[1] if q else ''))
    print('windows: %s' % ', '.join('%s %d' % kv for kv in sorted(kinds.items())))

    # 2. cursor track over the whole function, along an alignment of the abstracted listings
    I = [r for r in res['rows'] if r[1]]
    img_seq = [x[1][1] for x in sorted(I, key=lambda r: r[1][0])]
    img_off = [x[1][0] for x in sorted(I, key=lambda r: r[1][0])]
    O = [r for r in res['rows'] if r[2]]
    our_seq = [x[2][1] for x in sorted(O, key=lambda r: r[2][0])]
    sm = difflib.SequenceMatcher(None, [abstract(x) for x in img_seq], [abstract(x) for x in our_seq], autojunk=False)
    pairs = []
    for op, a1, a2, b1, b2 in sm.get_opcodes():
        if op == 'equal':
            for k in range(a2 - a1):
                ri, ro = alloc_reg(img_seq, a1 + k), alloc_reg(our_seq, b1 + k)
                if ri and ro:
                    pairs.append((img_off[a1 + k], (SCR.index(ro) - SCR.index(ri)) % 3, img_seq[a1 + k], our_seq[b1 + k]))
    print('\ncursor track (S1): %d paired allocations' % len(pairs))
    last, last_at = None, None
    changes = 0
    for at, d, si, so in pairs:
        if d != last:
            changes += 1
            if changes <= 40:
                print('  %x offset %d%s   image `%s`  ours `%s`' % (lo + at, d, '' if last is None else
                      ' (was %d since %x)' % (last, lo + last_at), si, so))
            last = d
        last_at = at
    nz = [p for p in pairs if p[1]]
    if not nz:
        print('  every paired allocation is in sync: the differences are order (S3/S4) or fixed registers.')
    else:
        print('  %d of %d paired allocations are out of sync; offset 1 = ours one allocation ahead, 2 = one behind.'
              % (len(nz), len(pairs)))


if __name__ == '__main__':
    main()
