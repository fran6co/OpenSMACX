"""rscan.py HOSTADDR [--min N] [--thr R]: every marker (referenced or not) whose body appears inside the host's image body."""
import sys, os
sys.argv_saved = list(sys.argv)
H = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, H)
sys.path.insert(0, os.path.dirname(H))
args = sys.argv_saved[1:]
host = int(args[0], 16)
mn = int(args[args.index('--min') + 1]) if '--min' in args else 4
thr = float(args[args.index('--thr') + 1]) if '--thr' in args else 0.9
sys.argv = [sys.argv[0]]
import orphanowner as oo
ms = oo.markers()
a2s = oo.a2s_map(ms)
h = [m for m in ms if m['addr'] == host and not m['frag']][0]
Hs = oo.toks(h['lo'], h['hi'], a2s)
print('host', h['sym'], len(Hs), 'tokens')
res = []
for m in ms:
    if m['frag'] or m['addr'] == host or m['hi'] - m['lo'] < 4 or m['hi'] - m['lo'] > (h['hi'] - h['lo']) // 2:
        continue
    O = oo.toks(m['lo'], m['hi'], a2s)
    if len(O) < mn:
        continue
    w = oo.best_window(O, Hs)
    if not w or w['score'] < thr or w['prec'] < thr:
        continue
    ref = oo.referenced(m['addr'])
    res.append((w['score'] * w['prec'], len(O), m, w, ref))
res.sort(key=lambda r: (-r[0], -r[1]))
for sc, n, m, w, ref in res:
    print('%.3f n=%3d %08X %-50s %-28s ref=%s img %x-%x named=%s' % (sc, n, m['addr'], m['sym'][:50], m['unit'][4:32], ref,
          Hs[w['ilo']][0], Hs[w['ihi'] - 1][0], w['named']))
