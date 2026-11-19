#!/usr/bin/env python3
"""census.py [--show N]: cross-jump sites in the image's BYTE_EXACT game functions (MATCHING_GUIDE 3.7,
"Cross-jumping ground truth").  A site is an unconditional `jmp` whose target is reached by fall-through from the
instruction before it (a jump into straight-line code).  Each site is classified by the instruction at the target
(call, add esp, push, mov to memory, mov reg, other) and by what the jumping block holds just before the `jmp`
(push = the arm ends inside a call's argument list).  Run from the worktree root."""
import sys, os, re, glob, collections
sys.path.insert(0, os.getcwd() + '/recovery/tools'); import fe
show = int(sys.argv[sys.argv.index('--show') + 1]) if '--show' in sys.argv else 0
M = re.compile(r'^// ORIGINAL: 0x([0-9A-F]+) (\S+) 0x([0-9A-F]+)-0x([0-9A-F]+)\S* BYTE_EXACT', re.M)
fns = []
for f in glob.glob('src/**/*.cpp', recursive=True):
    if '/uv/' in f or '/caviar/' in f: continue
    for m in M.finditer(open(f, encoding='latin-1').read()):
        fns.append((f, m.group(2), int(m.group(3), 16), int(m.group(4), 16)))
tgt = collections.Counter(); pre = collections.Counter(); nf = 0; ex = []
for f, sym, lo, hi in fns:
    I = list(fe.md.disasm(fe.imgbytes(lo, hi), lo))
    at = {i.address: k for k, i in enumerate(I)}
    hit = 0
    for k, i in enumerate(I):
        if i.mnemonic != 'jmp' or not i.op_str.startswith('0x'): continue
        t = int(i.op_str, 16)
        if t not in at or at[t] == 0: continue
        p = I[at[t] - 1]
        if p.mnemonic in ('jmp', 'ret') : continue
        x = I[at[t]]
        c = 'call' if x.mnemonic == 'call' else 'add esp' if x.op_str.startswith('esp') and x.mnemonic == 'add' else \
            'push' if x.mnemonic == 'push' else 'mov mem' if x.mnemonic == 'mov' and '[' in x.op_str.split(',')[0] else \
            'mov reg' if x.mnemonic in ('mov', 'lea', 'xor') else 'pop/epilogue' if x.mnemonic == 'pop' else x.mnemonic
        tgt[c] += 1
        b = I[k - 1] if k else None
        pre['push before jmp' if b and b.mnemonic == 'push' else 'other before jmp'] += 1
        hit += 1
        if show and len(ex) < show: ex.append('%s %x -> %x (%s)' % (sym, i.address, t, c))
    nf += 1 if hit else 0
print('%d exact functions, %d with a jump into fall-through code, %d sites' % (len(fns), nf, sum(tgt.values())))
print('instruction at the target:', ', '.join('%s %d' % kv for kv in tgt.most_common()))
print('jumping block ends with:', ', '.join('%s %d' % kv for kv in pre.most_common()))
for e in ex: print('  ' + e)
