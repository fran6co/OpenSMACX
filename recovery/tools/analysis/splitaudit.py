#!/usr/bin/env python3
"""splitaudit.py <unit> <0xADDR> [--min K] | --from FEALL.json [--rows N] [--min K] [--keep DIR]
Split screen (MATCHING_GUIDE 3.3 "One name over two image homes"): the reverse of imgmerge.py.

For every named local of ours it collects, from `imgmerge.py --sites all`, the image home of each paired
access with its source line, and prints the variables whose accesses fall into two or more image homes
(K+ paired accesses each) over DISJOINT source line ranges: one name of ours that the image keeps as two
variables.  Structs and arrays (interleaved line ranges over neighbouring homes) are left out.

A hit is a pointer, not a verdict.  Typical causes: a reusesweep rename into a function-scope variable that
the image contradicts (the web's stores land in another home), a decompiler reuse (battle_compute's
is_bombard), or two block-locals of one name.  Read the two homes with `imgmerge.py --sites A,B`: the other
occupants of the second home name the variable the value belongs to (Path::find: the step cost shares the
home of cur_cost, not x_cur).  Compare candidates with `imgmerge.py --plan` (frame, slot-map difference,
rows): several image-contradicted renames still read best as they are (TutWin::tour, capture_base, probe,
setup_player), because VC6 packs the separate block-local somewhere else again.
Env: FE_PFX_BASE (lane prefix)."""
import sys, os, re, json, argparse, collections, subprocess

H = os.path.dirname(os.path.abspath(__file__))
R = os.path.dirname(os.path.dirname(os.path.dirname(H)))
REG = re.compile(r'^(e?[abcd]x|e?[sd]i|e?[sb]p|[abcd][lh])$')


def sites(unit, addr):
    env = dict(os.environ, FE_JOBS='1')
    return subprocess.run(['uv', 'run', 'python', os.path.join(H, 'imgmerge.py'), unit, addr, '--sites', 'all'],
                          capture_output=True, text=True, cwd=R, env=env).stdout


def audit(text, k):
    acc = collections.defaultdict(lambda: collections.defaultdict(list))
    slot = None
    for l in text.split('\n'):
        m = re.match(r'== image slot (-?0x[0-9a-f]+)', l)
        if m:
            slot = m.group(1)
            continue
        m = re.match(r'\s+([0-9a-f]{6}) .*\| L(\d+)\s+.*?\s(\S+)\s*$', l)
        if not m or slot is None:
            continue
        addr, line, name = m.group(1), int(m.group(2)), m.group(3)
        if not re.match(r'^[A-Za-z_]\w*(#\d+)?$', name) or name.startswith('T0x') or name == 'this' or REG.match(name):
            continue
        acc[name][slot].append((line, addr))
    out = []
    for v, sl in acc.items():
        big = {s: x for s, x in sl.items() if len(x) >= k}
        if len(big) < 2:
            continue
        rng = sorted(((min(a for a, _ in x), max(a for a, _ in x), s, len(x), x[0][1]) for s, x in big.items()))
        if all(rng[i][1] < rng[i + 1][0] for i in range(len(rng) - 1)):
            out.append((v, rng))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('unit', nargs='?')
    ap.add_argument('addr', nargs='?')
    ap.add_argument('--from', dest='src')
    ap.add_argument('--rows', type=int, default=30)
    ap.add_argument('--min', type=int, default=2)
    ap.add_argument('--keep', help='directory for the --sites dumps')
    a = ap.parse_args()
    if a.src:
        todo = [(r['unit'].replace('src/', ''), '0x' + r['addr'], r['diff'], r.get('sym', ''))
                for r in json.load(open(a.src)) if not r['exact'] and r['diff'] >= a.rows]
        todo.sort(key=lambda t: -t[2])
    else:
        todo = [(a.unit, a.addr, 0, '')]
    for unit, addr, rows, sym in todo:
        t = sites(unit, addr)
        if a.keep:
            os.makedirs(a.keep, exist_ok=True)
            open(os.path.join(a.keep, addr[2:] + '.txt'), 'w').write(t)
        hits = audit(t, a.min)
        if hits:
            print('%s %s %s rows=%d' % (addr, unit, sym[:40], rows))
            for v, rng in hits:
                print('    %-26s %s' % (v, '  '.join('[%s] x%d L%d-%d @%s' % (s, n, lo, hi, ad)
                                                       for lo, hi, s, n, ad in rng)))
            sys.stdout.flush()


if __name__ == '__main__':
    main()
