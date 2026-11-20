#!/usr/bin/env python3
"""xjhostscan.py [unit ...] [--from TAILSCAN.json] [--json OUT]: cross-jump host screen (MATCHING_GUIDE 3.7,
"Cross-jump host" and "Host orientation").

Runs `tailscan.py --tree` (or reads its --json output) and classifies every HOST join whose host copy differs
between the image and ours by where each side's host sits among the join's predecessors in OUR source order:

  ORIENT-LAST   image hosts the last copy, ours the first: the orientation family (3.7 "Host orientation"):
                not an arm-order fix; look for the construct that flips our orientation (an `if (A || B) return;`
                bare early exit, an if/else-if chain around the first copies, explicit vs implicit final return).
  ORIENT-FIRST  the reverse.
  ORDER         the image hosts a middle copy (or ours does): by the last-copy rule the image's host copy was last
                in the original's pre-layout block order: move that statement/arm after the others, or reach the
                copies ours hosts by goto (a goto block keeps its source position).
Prints one line per join with both predecessor lists; 50 s tree-wide with FE_JOBS=5 (tailscan's cost).
Screen only: the orientation is brittle (it flips with constructs elsewhere in the function), so read each
join with fe.py --all and try the order change in a deck before editing."""
import sys, os, re, json, argparse, subprocess, tempfile

H = os.path.dirname(os.path.abspath(__file__))
TOK = re.compile(r'(H:|x:)?L(\d+)\[')


def preds(line):
    out = []
    for m in TOK.finditer(line):
        out.append((m.group(1) or '', int(m.group(2))))
    return out


def classify(img, ours):
    if not img or not ours:
        return None
    hi = [i for i, (t, _) in enumerate(img) if t == 'H:']
    ho = [i for i, (t, _) in enumerate(ours) if t == 'H:']
    if not hi or not ho:
        return None
    hi, ho = hi[0], ho[0]
    n = len(img)
    if hi == n - 1 and ho == 0:
        return 'ORIENT-LAST'
    if hi == 0 and ho == len(ours) - 1:
        return 'ORIENT-FIRST'
    return 'ORDER'


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='*')
    ap.add_argument('--from', dest='src')
    ap.add_argument('--json')
    a = ap.parse_args()
    if a.src:
        data = json.load(open(a.src))
    else:
        fd, tmp = tempfile.mkstemp(suffix='.json')
        os.close(fd)
        subprocess.run(['uv', 'run', 'python', H + '/tailscan.py', '--tree'] + a.units + ['--json', tmp],
                       stdout=subprocess.DEVNULL, check=True)
        data = json.load(open(tmp))
        os.unlink(tmp)
    rows = []
    for x in data:
        if not x.get('host'):
            continue
        L = x['lines']
        for i, l in enumerate(L):
            if not l.startswith('  HOST'):
                continue
            img = ours = None
            for k in range(i + 1, min(i + 4, len(L))):
                if L[k].strip().startswith('img  preds:'):
                    img = preds(L[k])
                elif L[k].strip().startswith('ours preds:'):
                    ours = preds(L[k])
            c = classify(img, ours)
            if c is None:
                continue
            ih = next(n for t, n in img if t == 'H:')
            oh = next(n for t, n in ours if t == 'H:')
            rows.append(dict(addr=x['addr'], unit=x['unit'], sym=x['sym'], diff=x['diff'], kind=c,
                             img_host=ih, ours_host=oh, n=len(img),
                             img=' '.join(t + 'L%d' % n for t, n in img),
                             ours=' '.join(t + 'L%d' % n for t, n in ours)))
    order = {'ORDER': 0, 'ORIENT-LAST': 1, 'ORIENT-FIRST': 2}
    rows.sort(key=lambda r: (order[r['kind']], r['diff']))
    for r in rows:
        print('%-12s %s %-26s %-32s diff=%-4d copies=%d img-host L%d ours-host L%d' % (
            r['kind'], r['addr'], r['unit'][4:], r['sym'][:32], r['diff'], r['n'], r['img_host'], r['ours_host']))
        print('    img  %s' % r['img'])
        print('    ours %s' % r['ours'])
    print('joins', len(rows), ' '.join('%s=%d' % (k, sum(1 for r in rows if r['kind'] == k)) for k in order))
    if a.json:
        json.dump(rows, open(a.json, 'w'), indent=0)


if __name__ == '__main__':
    main()
