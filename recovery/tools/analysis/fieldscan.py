#!/usr/bin/env python3
"""fieldscan.py [--from FEALL.json] [unit ...] [--json F] [--strong] | <unit> --addr A[,..] [--src ALT.cpp]

Data-flow half of the behaviour screen (MATCHING_GUIDE 3.7): on every fe-aligned differing row where both
sides have the same mnemonic and the same register operands, report the numbers that differ:

  FIELD   a memory operand [reg + d] with another displacement (another field of the same object:
          ReportWin::draw_ops read Base+0x98 where the image reads +0xD4)
  IMM     an immediate with another value (a stored constant, a compare constant, a pushed argument,
          `* 3` against `* 4`, `>> 2` against `>> 3`)
  SCALE   an index scale `*2` against `*4`

Frame operands ([ebp/esp ...]) and stack adjustments are left out (slot choice and argument counts are other
screens). A hit is a lead: read it against the image (a field displacement can also differ through a folded array
index, `a[i - 1]` against a decremented register). Rows in fe's misaligned stretches pair unrelated instructions,
so hits whose neighbourhood is mostly differing rows are marked `~` (weak).
"""
import argparse
import json
import os
import re
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor

R = os.getcwd()
FE = R + '/recovery/tools/fe.py'
ROW = re.compile(r'^(!!|!>|  ) (?:([0-9a-f]+) (.*?))?\s*\|\s*(?:([0-9a-f]+)\s+(.*))?$')
NUM = re.compile(r'(?<![\w])(-?0x[0-9a-f]+|-?\d+)(?![\w])')


def toks(op):
    return NUM.split(op)


def compare(mi, oi, mo, oo):
    """kind and the differing numbers, or None"""
    if mi != mo:
        return None
    a, b = toks(oi), toks(oo)
    if len(a) != len(b):
        return None
    out = []
    for k in range(len(a)):
        if k % 2 == 0:
            if a[k] != b[k]:
                return None
            continue
        if a[k] == b[k]:
            continue
        # context of this number
        pre = ''.join(a[:k])
        if re.search(r'\[(?:ebp|esp)\b[^\]]*$', pre):
            return None
        if pre.endswith('*'):
            out.append(('SCALE', a[k], b[k]))
        elif re.search(r'\[[^\]]*$', pre):
            out.append(('FIELD', a[k], b[k]))
        else:
            out.append(('IMM', a[k], b[k]))
    if not out:
        return None
    if mi in ('add', 'sub') and oi.startswith('esp'):
        return None
    if mi in ('ret', 'fxch') or '[ebp - 4]' in oi:   # EH state numbers are the EH screen's
        return None
    return out


def scan_text(text):
    res = {}
    rows = []
    for line in text.split('\n'):
        m = re.match(r'FE ([0-9A-F]{8}) (\S+) exact=(\w+) diff=(\d+)', line)
        if m:
            res[m.group(1)] = dict(sym=m.group(2), exact=m.group(3) == 'True', diff=int(m.group(4)), hits=analyse(rows))
            rows = []
            continue
        r = ROW.match(line)
        if r:
            rows.append(r.groups())
    return res


REG = re.compile(r'\b(?:e?[abcd]x|[abcd][lh]|e?[sd]i|e?bp|e?sp)\b')


def regmask(s):
    return REG.sub('R', s)


def analyse(rows):
    hits = []
    if os.environ.get('FS_REGMASK'):
        # register allocation differs at many rows: compare with general registers masked (frame operands stay
        # recognisable because ebp/esp are masked after the frame test below)
        rows = [(f, ia, it, oa, ot) for f, ia, it, oa, ot in rows]
    for i, (flag, ia, it, oa, ot) in enumerate(rows):
        if flag != '!!' or not ia or not oa:
            continue
        mi, _, oi = it.strip().partition(' ')
        mo, _, oo = ot.strip().partition(' ')
        oi, oo = oi.strip(), oo.strip()
        if os.environ.get('FS_REGMASK'):
            if re.search(r'\[(?:ebp|esp)\b', oi) or re.search(r'\[(?:ebp|esp)\b', oo) or oi.startswith('esp'):
                continue
            oi, oo = regmask(oi), regmask(oo)
        d = compare(mi, oi, mo, oo)
        if not d:
            continue
        lo, hi = max(0, i - 4), min(len(rows), i + 5)
        bad = sum(1 for x in rows[lo:hi] if x[0] == '!!') - 1
        hits.append(dict(i=i, img=ia, ours=oa, ins_img=it.strip(), ins_ours=ot.strip(),
                         kinds=d, weak=bad >= 4, why=''))
    # two loads in the other order (`mov edi,[a]; mov ebx,[b]` against b, a): scheduling, not data
    for h in hits:
        for g in hits:
            if g is not h and abs(g['i'] - h['i']) <= 3 and len(h['kinds']) == len(g['kinds']) == 1:
                (k1, a1, b1), (k2, a2, b2) = h['kinds'][0], g['kinds'][0]
                if k1 == k2 and a1 == b2 and b1 == a2:
                    h['why'] = 'swap'
    # the same operands in another order within a few rows (registers may differ): scheduling
    for h in hits:
        if h['why'] or len(h['kinds']) != 1:
            continue
        k, x, y = h['kinds'][0]
        lo, hi = max(0, h['i'] - 8), min(len(rows), h['i'] + 9)
        near_i = ' '.join((r[2] or '') for r in rows[lo:hi])
        near_o = ' '.join((r[4] or '') for r in rows[lo:hi])
        pat = lambda v: re.compile(r'(?<![\w])' + re.escape(v) + r'(?![\w])')
        if pat(x).search(near_o) and pat(y).search(near_i):
            h['why'] = 'order'
    # both instructions occur elsewhere on the other side: the rows pair across a layout difference
    ti = {regmask((r[2] or '').strip()) for r in rows}
    to = {regmask((r[4] or '').strip()) for r in rows}
    for h in hits:
        if not h['why'] and regmask(h['ins_img']) in to and regmask(h['ins_ours']) in ti:
            h['why'] = 'moved'
    # one displacement delta repeated through the function: a pointer induction variable with another base
    from collections import Counter
    def delta(h):
        k, x, y = h['kinds'][0]
        return int(x, 0) - int(y, 0) if k == 'FIELD' and len(h['kinds']) == 1 else None
    c = Counter(delta(h) for h in hits if delta(h) is not None)
    for h in hits:
        if not h['why'] and delta(h) is not None and c[delta(h)] >= 3:
            h['why'] = 'iv%+d' % delta(h)
    return hits


def run_unit(unit, addrs, pfx, src=None):
    p = subprocess.run(['uv', 'run', 'python', FE, unit, ','.join(addrs), '--all', '--pfx', str(pfx)] + (['--src', src] if src else []),
                       cwd=R, capture_output=True, text=True)
    return unit, scan_text(p.stdout)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='*')
    ap.add_argument('--from', dest='frm')
    ap.add_argument('--addr')
    ap.add_argument('--json')
    ap.add_argument('--src', help='single-unit mode: compile this file in place of the unit')
    ap.add_argument('--strong', action='store_true', help='leave out weak hits')
    ap.add_argument('-v', action='store_true')
    a = ap.parse_args()
    base = int(os.environ.get('FE_PFX_BASE', '21'))
    jobs = int(os.environ.get('FE_JOBS', '1'))
    work = {}
    if a.frm:
        for x in json.load(open(a.frm)):
            if x.get('diff', -1) > 0 and not x.get('exact'):
                work.setdefault(x['unit'], []).append(x['addr'])
        if a.units:
            work = {u: v for u, v in work.items() if u in a.units or os.path.basename(u) in a.units}
    else:
        u = a.units[0]
        work[u if u.startswith('src/') else 'src/' + u] = [x.replace('0x', '').upper() for x in a.addr.split(',')]
    items = sorted(work.items())

    def lane(k):
        return [run_unit(u, ['0x' + x for x in ad], base + k, a.src) for u, ad in items[k::jobs]]
    out = []
    with ThreadPoolExecutor(jobs) as ex:
        for part in ex.map(lane, range(jobs)):
            out += part
    flat = []
    for unit, res in out:
        for addr, r in res.items():
            for h in r['hits']:
                if a.strong and (h['weak'] or h['why']):
                    continue
                flat.append(dict(unit=unit, addr=addr, sym=r['sym'], diff=r['diff'], **h))
    if a.json:
        json.dump(flat, open(a.json, 'w'), indent=0)
    for h in sorted(flat, key=lambda h: (h['weak'], bool(h['why']), h['unit'], h['addr'], h['img'])):
        k = ','.join('%s %s/%s' % x for x in h['kinds'])
        print('%s%-6s %-7s %s %-34s img %s %-40s ours %s %s' % ('~' if h['weak'] else ' ', h['kinds'][0][0], h['why'], h['addr'],
              h['sym'][:34], h['img'], h['ins_img'][:40], h['ours'], h['ins_ours'][:40]))
    print('hits', len(flat), 'strong', sum(1 for h in flat if not h['weak'] and not h['why']))


if __name__ == '__main__':
    sys.exit(main())
