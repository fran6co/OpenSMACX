#!/usr/bin/env python3
"""datawho.py [--cmds]: which unit defines each out-of-order object of datarun.py (MATCHING_GUIDE 2.5).
  H unit   hard: the object lies inside the interval of the objects that unit's static-initializer thunks build
           (initorder.py), so it is in that unit's .bss contribution;
  D unit   .data: the in-order neighbours on both sides (objects or the unit's own function literals) belong to
           one unit, so the object is inside that unit's .data contribution;
  S unit   .bss, soft: the in-order neighbours on both sides belong to one unit and a function of that unit
           references the object;
  N unit   the neighbours differ, and of the units from one to the other in link order exactly one references
           the object;
  ? a|b    undecided: the neighbours differ, or the enclosing unit never names the object (left where it is).
--cmds prints one objmove.py command per destination unit for the H and S rows."""
import sys, os, re, struct, bisect, subprocess, collections
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE)); sys.path.insert(0, HERE)
import fe, tumap
mk = tumap.load()
los = [m[0] for m in mk]
tab = fe.imgbytes(0x682004, 0x6826CC)
iv = {}
for i in range(0, len(tab), 4):
    t = struct.unpack_from('<I', tab, i)[0]
    if not t:
        continue
    k = bisect.bisect_right(los, t) - 1
    if not (k >= 0 and mk[k][0] <= t < mk[k][1]):
        continue
    # every `mov ecx, <object>` of the thunk: one thunk constructs all objects of a grouped declaration
    raw = fe.imgbytes(t, mk[k][1])
    u = mk[k][2]
    for j in range(len(raw) - 4):
        if raw[j] == 0xB9:
            obj = struct.unpack_from('<I', raw, j + 1)[0]
            if 0x6A7000 <= obj < 0x9C2200:
                lo, hi = iv.get(u, (obj, obj))
                iv[u] = (min(lo, obj), max(hi, obj))
out = subprocess.run([sys.executable, HERE + '/datarun.py'], capture_output=True, text=True, cwd=fe.R).stdout
rows = []
for l in out.split('\n'):
    m = re.match(r' ! ([0-9A-F]{8}) (\S+)\s+(\S+)\s+between (\S+) \.\. (\S+)', l)
    if m:
        rows.append((int(m.group(1), 16), 'src/' + m.group(2), m.group(3), m.group(4), m.group(5)))
addrs = sorted({r[0] for r in rows})
refs = collections.defaultdict(collections.Counter)
aset = set(addrs)
for flo, fhi, unit, name, exact, path in mk:
    b = fe.imgbytes(flo, fhi)
    for i in range(len(b) - 3):
        v = struct.unpack_from('<I', b, i)[0]
        if v in aset:
            refs[v][unit] += 1
cmds = collections.defaultdict(list)
# link order of the units (main .text run), for the candidates between two neighbours
runs = {}
for lo, hi, unit, name, exact, path in mk:
    runs.setdefault(unit, []).append((lo, hi))
rank = {}
cur = None; best = {}
for lo, hi, u in sorted((lo, hi, u) for u, r in runs.items() for lo, hi in r):
    if cur and cur[2] == u:
        cur = (cur[0], hi, u)
    else:
        if cur and cur[1] - cur[0] > best.get(cur[2], (0, 0))[0]:
            best[cur[2]] = (cur[1] - cur[0], cur[0])
        cur = (lo, hi, u)
if cur and cur[1] - cur[0] > best.get(cur[2], (0, 0))[0]:
    best[cur[2]] = (cur[1] - cur[0], cur[0])
rank = {u: v[1] for u, v in best.items()}
BSS = 0x6A708C
for a, own, nm, bef, aft in rows:
    hard = [u for u, (lo, hi) in iv.items() if lo <= a <= hi and u != own]
    rf = ' '.join('%s:%d' % (u[4:], n) for u, n in refs[a].most_common(4))
    b_, a_ = 'src/' + bef, 'src/' + aft
    cand = [u for u in rank if b_ in rank and a_ in rank and rank[b_] <= rank[u] <= rank[a_]]
    named = [u for u in cand if refs[a].get(u)]
    if hard:
        tag = 'H ' + hard[0][4:]; cmds[hard[0]].append(a)
    elif bef == aft and bef != '-' and a < BSS:
        tag = 'D ' + bef; cmds[b_].append(a)
    elif bef == aft and bef != '-' and refs[a].get(b_):
        tag = 'S ' + bef; cmds[b_].append(a)
    elif bef != aft and len(named) == 1 and named[0] != own:
        tag = 'N ' + named[0][4:]; cmds[named[0]].append(a)
    else:
        tag = '? %s|%s' % (bef, aft)
    print('%08X %-26s %-40s %-34s refs %s' % (a, own[4:], nm[:40], tag, rf))
# in-order objects are not above suspicion: any bound object inside another unit's thunk interval is that unit's
flagged = {r[0] for r in rows}
for dp, dn, fn in os.walk(fe.R + '/src'):
    for f in fn:
        if not f.endswith('.cpp'):
            continue
        p = os.path.relpath(os.path.join(dp, f), fe.R)
        for l in open(os.path.join(dp, f), encoding='latin-1'):
            m = re.match(r'\s*// RECOVERY_OBJECT: v\d (0x[0-9A-F]{8}) (?:0x[0-9A-F]{8} )?(\S+)', l)
            if not m:
                continue
            a = int(m.group(1), 16)
            if a in flagged:
                continue
            for u, (lo, hi) in iv.items():
                if lo <= a <= hi and u != p:
                    print('%08X %-26s %-40s %-34s (in order, inside the thunk interval %08X..%08X)' % (a, p[4:], m.group(2)[:40], 'H ' + u[4:], lo, hi))
                    cmds[u].append(a)
if '--cmds' in sys.argv:
    for u, xs in sorted(cmds.items()):
        print('uv run python recovery/tools/analysis/objmove.py --into %s --addr %s' % (u, ','.join('%X' % x for x in xs)))
