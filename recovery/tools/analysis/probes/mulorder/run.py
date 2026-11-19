#!/usr/bin/env python3
"""run.py: operand order of `stride * y` index multiplies (MATCHING_GUIDE 3.4).
For a global stride G declared int or unsigned, writes each spelling of `T[G*y + (x>>1)]` and reports
which operand VC6 loads into the destination register first: `G` = `mov r,[G]; imul r,y`, `y` =
`mov r,y; imul r,[G]`.  y is a stack parameter (mem), a register-held value (reg, used in a loop) or a
local computed from a call (call).
Usage: uv run python run.py   (FE_PFX_BASE, default 21)"""
import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))) + '/inline')
import ib
PFX = os.environ.get('FE_PFX_BASE', '21')

SPELL = {
  'G*y': 'G * y', 'y*G': 'y * G', '(int)G*y': '(int)G * y', 'y*(int)G': 'y * (int)G',
}
YKIND = {
  'mem': ('int x, int y', ''),
  'call': ('int x', 'int y = cond();'),
  'live': ('int x', 'int y = cond(); use(y);'),
  'short': ('int v', 'int y = S[v].y; int x = S[v].x;'),
  'shortlive': ('int v', 'int y = S[v].y; int x = S[v].x; sink2(x, y);'),
  'addr': ('int x', 'int y; get(&y);'),
}
POST = {'live': 'use(y); ', 'shortlive': 'sink2(x, y); '}

def fn(name, spell, yk):
    params, pre = YKIND[yk]
    return ('int %s(%s) { %s int r = T[%s + (x >> 1)].v; %sreturn r; }\n' % (name, params, pre, SPELL[spell], POST.get(yk, '')))

def first(lines):
    for i, l in enumerate(lines):
        if l.startswith('imul'):
            # find the mov into the imul's destination register just before
            dst = l.split()[1].rstrip(',')
            for j in range(i - 1, -1, -1):
                m = re.match(r'mov\s+%s, (.*)$' % dst, lines[j])
                if m:
                    return 'G' if '?G@@' in m.group(1) else 'y'
            return '?'
    return 'none'

if __name__ == '__main__':
    out = {}
    for decl in ('int', 'unsigned int'):
        src = '#include "probe.h"\nstruct Tile { int v; int w; int z; };\nextern Tile T[];\nextern %s G;\nstruct SV { short x, y; };\nextern SV S[];\nvoid sink2(int, int);\nvoid get(int *);\n' % decl
        names = []
        for s in SPELL:
            for yk in YKIND:
                n = 't_%d_%s' % (len(names), yk)
                names.append((n, s, yk))
                src += fn(n, s, yk)
        asm, _ = ib.compile_asm(None, PFX, text=src)
        F = {f[0].split('(')[0].strip(): f[2] for f in ib.functions(asm)}
        for n, s, yk in names:
            L = [l for k, l in F.items() if k.endswith(n)]
            seq = ' ; '.join(l for l in L[0] if not l.startswith(('push ebp','mov ebp','pop','ret','push e'))) if L else ''
            print('%-13s %-10s y=%-5s first=%s | %s' % (decl, s, yk, first(L[0]) if L else '??', seq[:110]))
