#!/usr/bin/env python3
"""run.py: when does VC6 give an inlined callee's locals a frame slot shared with the caller's locals?
(draw_intel 0x004A2210: has_fac's address-taken `mask` shares a slot with a later `faction` loop
counter in ours, alone in the image; NewTechWin::exec 0x00483900.)
Each variant: an inline callee with address-taken locals (`bitmask(id, &offset, &mask)`), called in an
early loop, then a later loop whose counter (and a second local) must live in memory.  We print the
frame offsets VC6 assigned to every named variable from the /FAs listing (`_name$ = -N`), so shared
slots show as equal offsets.
Usage: uv run python run.py [--show KEY]   (FE_PFX_BASE, default 21)"""
import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))) + '/inline')
import ib
PFX = os.environ.get('FE_PFX_BASE', '21')
HDR = '''#include "probe.h"
void bitmask(int id, int *offset, int *mask);
struct B { int fac[4]; int owner; };
extern B Bs[];
void take(int *);
'''
CALLEE = {
 'addr': '''inline int hf(int id, int b) {
  int offset; int mask;
  bitmask(id, &offset, &mask);
  return (Bs[b].fac[offset] & mask) != 0;
}
''',
 'addr_guard': '''inline int hf(int id, int b) {
  if (id >= 64) return 0;
  int offset; int mask;
  bitmask(id, &offset, &mask);
  int built = (Bs[b].fac[offset] & mask) != 0;
  if (built || !g2) return built;
  return 0;
}
''',
}
LATER = {
 # the later loop counter is forced to memory by taking its address
 'for_addr': '''  for (int faction = 1; faction < 8; ++faction) { take(&faction); use(faction); }\n''',
 # later loop counter in memory through register pressure (many live values)
 'for_plain': '''  for (int faction = 1; faction < 8; ++faction) { if (faction == g3) continue; use(ga[faction] + g4); use(faction); }\n''',
 'fn_scope_addr': '''  int faction2; for (faction2 = 1; faction2 < 8; ++faction2) { take(&faction2); }\n''',
 'for_pressure': '''  for (int faction = 1; faction < 8; ++faction) { int a = ga[faction]; int b = ga[faction + 8]; int c = ga[faction + 16]; int d = ga[faction + 24]; use(a); use(b); use(c); use(d); use(a + b + c + d + faction); }\n''',
 'for_pressure_pre': '''  int faction; for (faction = 1; faction < 8; ++faction) { int a = ga[faction]; int b = ga[faction + 8]; int c = ga[faction + 16]; int d = ga[faction + 24]; use(a); use(b); use(c); use(d); use(a + b + c + d + faction); }\n''',
 'block_addr': '''  { int faction3 = g5; take(&faction3); use(faction3); }\n''',
}
SITE = {
 'loop': '''  int hq = -1;
  for (int b = 0; b < g; ++b) { if (Bs[b].owner == s && hf(5, b)) { hq = b; break; } }
  use(hq);
''',
 'plain': '''  int hq = hf(5, s);\n  use(hq);\n''',
 'block': '''  { int hq = hf(5, s); use(hq); }\n''',
}


def slots(asm, name):
    i = asm.find(name)
    p = asm.find(' PROC NEAR', i)
    j = asm.rfind('_TEXT\tSEGMENT', 0, p)
    out = {}
    for m in re.finditer(r'^_(\w+)\$(\d*) = (-?\d+)', asm[j:p], re.M):
        out.setdefault(int(m.group(3)), []).append(m.group(1) + (('$' + m.group(2)) if m.group(2) else ''))
    return out


def run(V, show=None):
    for key, callee, site, later in V:
        src = HDR + CALLEE[callee] + 'void t_f(int s) {\n' + SITE[site] + LATER[later] + '}\n'
        asm, _ = ib.compile_asm(None, PFX, text=src)
        sl = slots(asm, 't_f')
        shared = {k: v for k, v in sl.items() if len(v) > 1}
        print('%-34s %s   shared=%s' % (key, ' '.join('%s:%d' % ('/'.join(v), k) for k, v in sorted(sl.items())), shared or '-'))
        if show == key:
            print(asm[asm.find('t_f'):][:3000])


if __name__ == '__main__':
    show = sys.argv[sys.argv.index('--show') + 1] if '--show' in sys.argv else None
    V = []
    for c in CALLEE:
        for s in SITE:
            for l in LATER:
                V.append(('%s/%s/%s' % (c, s, l), c, s, l))
    run(V, show)
