#!/usr/bin/env python3
"""initorder.py: the .bss law checked on the static-initializer thunks (MATCHING_GUIDE 2.5).
Each thunk of the CRT table 0x00682004-0x006826CC that starts `mov ecx, <object>` sits in its unit's .text
and names an object of that unit's .bss. Units follow link order in both sections, so, taking the units in
.text order, their object intervals [min, max] must not overlap: an object of a later unit below an earlier
unit's maximum means the two thunks are in ONE unit (with everything between them). Prints every unit's
interval and the violations; no compile."""
import sys, os, struct, bisect
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import fe, tumap
mk = tumap.load()
los = [m[0] for m in mk]
tab = fe.imgbytes(0x682004, 0x6826CC)
rows = []
for i in range(0, len(tab), 4):
    t = struct.unpack_from('<I', tab, i)[0]
    if not t:
        continue
    raw = fe.imgbytes(t, t + 8)
    if raw[0] != 0xB9:
        continue
    obj = struct.unpack_from('<I', raw, 1)[0]
    k = bisect.bisect_right(los, t) - 1
    rows.append((t, obj, mk[k][2] if k >= 0 and mk[k][0] <= t < mk[k][1] else '?'))
rows.sort()
units = []
for t, obj, u in rows:
    if units and units[-1][0] == u:
        units[-1][1].append((t, obj))
    else:
        units.append((u, [(t, obj)]))
hi = 0
bad = 0
for u, xs in units:
    lo_, hi_ = min(o for _, o in xs), max(o for _, o in xs)
    flag = ''
    if lo_ < hi:
        flag = '   <-- below the previous units\' maximum %08X: one unit with the units before it' % hi
        bad += 1
    hi = max(hi, hi_)
    print('%08X %-34s thunks=%-3d objects %08X..%08X%s' % (xs[0][0], u, len(xs), lo_, hi_, flag))
print('thunks with an object: %d, units: %d, violations: %d' % (len(rows), len(units), bad))
