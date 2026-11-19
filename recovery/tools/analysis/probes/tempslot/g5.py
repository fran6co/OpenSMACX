#!/usr/bin/env python3
"""g5.py: which liveness does VC6's slot packing use? A spilled scalar G (memory-homed by pressure from
p1..p3) and an address-taken block-local Z in a region R where G is dead by flow but not lexically.
Z sharing G's offset = packing saw G dead in R."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import t as T
HDR = '''#include "probe.h"
void take(int *);
'''
PRE = '''  int p1 = ga[1], p2 = ga[2], p3 = ga[3], G;
  for (G = 0; G < g; ++G) { for (int v2 = 0; v2 < g; ++v2) { call(); use(p1 + v2); use(p2 + v2); use(p3 + v2); } }
'''
POST = '''  call(); use(p1); use(p2); use(p3); use(s);
'''
R = '{ int Z; take(&Z); }'
CASES = {
 'base_dead': R,
 'live_after': R + ' use(G);',
 'ifelse': 'if (g) { %s } else { use(G); }' % R,
 'ifelse_rev': 'if (g) { use(G); } else { %s }' % R,
 'reassign': R + ' G = ga[s + 9]; call(); use(G);',
 'loop_redef': 'for (int q = 0; q < g; ++q) { use(G); %s G = ga[q]; }' % R,
 'goto_over': 'if (g) goto skip; %s skip: use(G);' % R,
 'goto_back': 'use(G); if (g2) goto out; %s use(G); out: ;' % R,
 'scope_end': '{ int H = G; call(); use(H); } %s' % R,
 'cond_use': 'if (g) use(G); %s' % R,
 'cond_use_after': '%s if (g) use(G);' % R,
}
V = ['void %s(int s) {\n%s  %s\n%s}\n' % (k, PRE, v, POST) for k, v in CASES.items()]
if __name__ == '__main__':
    T.show(HDR + ''.join(V), asm_of=sys.argv[1] if len(sys.argv) > 1 else None)
