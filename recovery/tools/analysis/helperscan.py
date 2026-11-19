#!/usr/bin/env python3
"""helperscan.py [NAME ...] [--min N] [--thr R] [--top K] [--json F]

Invented-helper census (MATCHING_GUIDE 2.8 "Referenced owners too"). An invented helper is a function
definition under src/ with no ORIGINAL marker of its own that sits under a WORKAROUND(inline|decl) tag
(or is named on the command line). For each one: its hosts (ORIGINAL-marked bodies whose source calls
it) and, per host, the reverse owner scan of rscan.py (every other marker's image body found inside the
host's image body, score*prec >= --thr, min --min tokens), so a real header inline or same-unit function
that plays the helper's role can be read off the list. Budget probes (names that say probe/spend/budget)
are left out unless named. Screen only: replace the helper with the real owner, then fe the hosts."""
import sys, os, re, glob, json, argparse
H = os.path.dirname(os.path.abspath(__file__))
R = os.path.dirname(os.path.dirname(os.path.dirname(H)))
ap = argparse.ArgumentParser()
ap.add_argument('names', nargs='*')
ap.add_argument('--min', type=int, default=4)
ap.add_argument('--thr', type=float, default=0.85)
ap.add_argument('--top', type=int, default=8)
ap.add_argument('--json')
a = ap.parse_args()
sys.stdout.reconfigure(line_buffering=True)
sys.argv = [sys.argv[0]]
sys.path.insert(0, H)
sys.path.insert(0, os.path.dirname(H))
import orphanowner as oo

PROBE = re.compile(r'probe|spend|budget|stand_in|sink|pad', re.I)
DEF = re.compile(r'^(?:static\s+|inline\s+|__forceinline\s+)+[\w:<>\s\*&]*?\b(\w+)\s*\(([^;{]*)\)\s*(?:const\s*)?\{?\s*$', re.M)
MRE = re.compile(r'// ORIGINAL: 0x([0-9A-F]{8}) ')


def helpers():
    out = {}
    for p in sorted(glob.glob(R + '/src/**/*.cpp', recursive=True) + glob.glob(R + '/src/**/*.h', recursive=True)):
        t = open(p, encoding='latin-1').read()
        lines = t.split('\n')
        for m in DEF.finditer(t):
            name = m.group(1)
            ln = t.count('\n', 0, m.start())
            above = '\n'.join(lines[max(0, ln - 12):ln + 1])
            if 'ORIGINAL:' in '\n'.join(lines[max(0, ln - 3):ln]):
                continue
            if a.names:
                if name not in a.names:
                    continue
            else:
                if not re.search(r'WORKAROUND\((inline|decl)\)', above) or PROBE.search(name):
                    continue
                # the tag must be in the comment block directly above
                blk = []
                k = ln - 1
                while k >= 0 and (lines[k].strip().startswith('//') or lines[k].strip().startswith('template')):
                    blk.append(lines[k])
                    k -= 1
                if not any('WORKAROUND' in b for b in blk) and 'WORKAROUND' not in lines[ln]:
                    continue
            out.setdefault(name, []).append((os.path.relpath(p, R), ln + 1))
    return out


def hosts_of(name, ms):
    pat = re.compile(r'\b' + re.escape(name) + r'\s*\(')
    res = []
    byfile = {}
    for m in ms:
        byfile.setdefault(m['unit'], []).append(m)
    for unit, mm in byfile.items():
        t = open(R + '/' + unit, encoding='latin-1').read().split('\n')
        mm = sorted(mm, key=lambda m: m['line'])
        for i, m in enumerate(mm):
            end = mm[i + 1]['line'] - 1 if i + 1 < len(mm) else len(t)
            body = '\n'.join(t[m['line']:end])
            if pat.search(body) and not m['frag']:
                res.append(m)
    return res


if os.environ.get("HS_LIST"):
    hs = helpers(); print(len(hs)); print(" ".join(sorted(hs))); sys.exit()
ms = oo.markers()
a2s = oo.a2s_map(ms)
hs = helpers()
report = {}
for name, defs in sorted(hs.items()):
    hosts = hosts_of(name, ms)
    print('== %s  def %s  hosts %d' % (name, ' '.join('%s:%d' % d for d in defs), len(hosts)))
    rep = []
    for h in hosts[:6]:
        Hs = oo.toks(h['lo'], h['hi'], a2s)
        res = []
        for m in ms:
            if m['frag'] or m['addr'] == h['addr'] or m['hi'] - m['lo'] < 4 or m['hi'] - m['lo'] > (h['hi'] - h['lo']) // 2:
                continue
            O = oo.toks(m['lo'], m['hi'], a2s)
            if len(O) < a.min:
                continue
            w = oo.best_window(O, Hs)
            if not w or w['score'] * w['prec'] < a.thr:
                continue
            res.append((w['score'] * w['prec'], len(O), m, w, oo.referenced(m['addr'])))
        res.sort(key=lambda r: (-r[0], -r[1]))
        print('  host %08X %s (%s) %d tok' % (h['addr'], h['sym'][:50], h['unit'], len(Hs)))
        for sc, n, m, w, ref in res[:a.top]:
            same = 'SAME' if m['unit'] == h['unit'] else ''
            print('    %.3f n=%3d %08X %-44s %-28s ref=%s %s' % (sc, n, m['addr'], m['sym'][:44], m['unit'][4:32], ref, same))
        rep.append(dict(host=h['addr'], sym=h['sym'], cands=[dict(addr=m['addr'], sym=m['sym'], unit=m['unit'], score=sc, n=n)
                                                              for sc, n, m, w, ref in res[:a.top]]))
    report[name] = dict(defs=defs, hosts=rep)
if a.json:
    json.dump(report, open(a.json, 'w'), indent=1)
