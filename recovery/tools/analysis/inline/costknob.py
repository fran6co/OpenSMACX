#!/usr/bin/env python3
"""costknob.py TAG FILE OLD NEW [FILE OLD NEW ...] [--base it_base.json]

Tree-wide sensitivity of /Ob2 decisions to one inline callee's cost (guide 2.8, "Callee costs").
Applies the textual edits (FILE relative to the repository root) (each OLD must occur once; the files are restored afterwards, even on
error), runs ../inlinetree.py --all and compares every function's site disagreement with the image
against a baseline inlinetree run. The usual knob is a dead statement in the callee body, which
leaves every out-of-line copy unchanged:  `int k = 0;` (smallest that moves anything) or
`int k = 0; if (k == 12345) close();` (about one if-unit).
Prints the total, then each function whose disagreement moved (+ worse, - better).
Uses FE_PFX_BASE/FE_JOBS lanes; do not edit the tree while it runs."""
import sys, os, json, subprocess, collections, argparse
HERE = os.path.dirname(os.path.abspath(__file__))
R = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(HERE))))
ap = argparse.ArgumentParser()
ap.add_argument('tag')
ap.add_argument('edits', nargs='*')
ap.add_argument('--base', required=True, help='inlinetree --all output of the unedited tree')
ap.add_argument('--outdir', default=os.path.join(R, 'build/recovery/fe'))
a = ap.parse_args()
out = os.path.join(a.outdir, 'knob_%s.json' % a.tag)
orig = {}
try:
    for i in range(0, len(a.edits), 3):
        f, old, new = a.edits[i:i + 3]
        p = os.path.join(R, f)
        t = open(p, encoding='latin-1').read()
        orig.setdefault(p, t)
        if t.count(old) != 1:
            raise SystemExit('%s: OLD occurs %d times' % (f, t.count(old)))
        open(p, 'w', encoding='latin-1').write(t.replace(old, new))
    subprocess.run(['uv', 'run', 'python', os.path.join(os.path.dirname(HERE), 'inlinetree.py'), '--all', '--out', out, '--top', '0'],
                   cwd=R, capture_output=True)
finally:
    for p, t in orig.items():
        open(p, 'w', encoding='latin-1').write(t)


def load(f):
    d, name = collections.Counter(), {}
    for r in json.load(open(f)):
        if r['callee'] == '?':
            continue
        d[r['addr']] += sum(1 for x, y in zip(r['image'], r['ours']) if x != y) + abs(r['image'].count('E') - r['ours'].count('E'))
        name[r['addr']] = r['sym']
    return d, name


b, nb = load(a.base)
n, nn = load(out)
nb.update(nn)
print('%s: disagreement %d -> %d; matching functions broken %d, mismatching improved %d, worse %d' % (
    a.tag, sum(b.values()), sum(n.values()), sum(1 for k in n if n[k] and not b[k]),
    sum(1 for k in b if b[k] and n[k] < b[k]), sum(1 for k in b if b[k] and n[k] > b[k])))
for k in sorted(set(b) | set(n), key=lambda k: n[k] - b[k]):
    if n[k] != b[k]:
        print('  %+4d %s %s (%d -> %d)' % (n[k] - b[k], k, nb[k][:50], b[k], n[k]))
