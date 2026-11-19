#!/usr/bin/env python3
"""g13.py: is a maybe-uninitialised variable's home live where none of its stores reaches? A is stored only on
one arm and read under the same condition later (by flow the read is reachable from entry through the other
arm); an address-taken block-local Z sits on the other arm. Z sharing A's offset = packing uses liveness AND
reaching definition (A's home is free where no store of A reaches), not plain flow liveness."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import t as T
HDR = '''#include "probe.h"
void take(int *);
'''
LOOP = 'for (int v2 = 0; v2 < g; ++v2) { call(); use(p1 + v2); use(p2 + v2); use(p3 + v2); use(A + v2); }'
CASES = {
 # A stored on the then-arm only, read later under the same test: by flow live through the else-arm
 'maybe_uninit': 'if (s) { A = ga[4]; %s } else { int Z; take(&Z); } call(); use(p1); use(p2); use(p3); if (s) use(A);' % LOOP,
 # control: A stored before the branch, so it is live and defined over the else-arm
 'defined': 'A = ga[4]; if (s) { %s } else { int Z; take(&Z); } call(); use(p1); use(p2); use(p3); if (s) use(A);' % LOOP,
 # control: A never read after the branch
 'dead_after': 'if (s) { A = ga[4]; %s } else { int Z; take(&Z); } call(); use(p1); use(p2); use(p3);' % LOOP,
 # two variables on exclusive arms, each read after the join under its own test (go_to base_id/max_range)
 'exclusive': 'int B; if (s) { A = ga[4]; %s } else { B = ga[5]; for (int v3 = 0; v3 < g; ++v3) { call(); use(p1 + v3); use(p2 + v3); use(p3 + v3); use(B + v3); } } call(); use(p1); use(p2); use(p3); if (s) use(A); else use(B);' % LOOP,
}
V = ['void %s(int s) {\n  int p1 = ga[1], p2 = ga[2], p3 = ga[3], A;\n  %s\n}\n' % (k, v) for k, v in CASES.items()]
if __name__ == '__main__':
    T.show(HDR + ''.join(V), asm_of=sys.argv[1] if len(sys.argv) > 1 else None)
