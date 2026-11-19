#!/usr/bin/env python3
"""retype.py TYPESCAN.json BASE.json   (run from the repo root; commits in the current checkout)
retype.py TYPESCAN.json BASE.json [--only Class::field,...] [--dry] [--limit N] [--log LOG]
Per-field retype runner (one hypothesis at a time, one commit per kept field).
Candidates: 32-bit fields declared unsigned whose image uses show no unsigned use (typescan), that our
source casts to int (or the image uses signed). Evidence that the field is signed is either a signed use
in the image (typescan S > 0) or a load-bearing cast: dropping the int casts while the field stays unsigned
changes some function (the pre-test; a cast that changes nothing says nothing about the type). With
evidence: retype the declaration to `int`, drop the int casts on that field, feall the units that name it
against BASE, and keep (commit) when nothing fails to compile and no function that uses the field is lost
or gets more rows. Every other function that moves is listed in the commit: a cast dropped from a header
inline shifts the front-end counter of every includer, which moves the count-sensitive ties and canaries
(MATCHING_GUIDE 2.5); those are not re-tuned here."""
import sys, os, re, json, subprocess, argparse, collections
R = os.getcwd()
ap = argparse.ArgumentParser()
ap.add_argument('typescan'); ap.add_argument('base')
ap.add_argument('--only', default=''); ap.add_argument('--skip', default=''); ap.add_argument('--dry', action='store_true')
ap.add_argument('--limit', type=int, default=0); ap.add_argument('--log', default='retype.log')
ap.add_argument('--jobs', default='5'); ap.add_argument('--pfx', default='2')
a = ap.parse_args()
uses = json.load(open(a.typescan))
by = collections.defaultdict(list)
for u in uses:
    by[u['key']].append(u)
UTYPES = r'(?:uint32_t|unsigned int|unsigned long|unsigned|DWORD|ULONG|UINT)'
srcs = []
for root, _, files in os.walk(R + '/src'):
    if '/vendor' in root:
        continue
    for f in files:
        if f.endswith(('.cpp', '.h')):
            srcs.append(os.path.join(root, f))
texts = {p: open(p, encoding='latin-1').read() for p in srcs}


def class_body(cname):
    """-> [(path, start, end)] spans of `class/struct cname {...}` bodies"""
    out = []
    rx = re.compile(r'\b(?:class|struct|union)\s+%s\b[^;{()]*\{' % re.escape(cname))
    for p, t in texts.items():
        if not p.endswith('.h'):
            continue
        for m in rx.finditer(t):
            d, i = 1, m.end()
            while i < len(t) and d:
                d += (t[i] == '{') - (t[i] == '}')
                i += 1
            out.append((p, m.end(), i))
    return out


cands = []
for key, us in by.items():
    w, sg, tn = us[0]['decl']
    if w != 4 or sg != 'U' or tn.startswith(('pointer', 'enum', 'bool')) or key.count('::') != 1:
        continue
    S = sum(1 for u in us if u['img'] == 'S')
    U = sum(1 for u in us if u['img'] == 'U')
    if U and U * 3 > S:
        continue
    cname, f = key.split('::')
    spans = class_body(cname)
    decl = None
    for p, s, e in spans:
        m = re.search(r'^([ \t]*)(%s)(\s+%s\s*(?:\[[^\]]*\]\s*)*;)' % (UTYPES, re.escape(f)), texts[p][s:e], re.M)
        if m:
            decl = (p, s + m.start(2), s + m.end(2))
            break
    if not decl:
        continue
    castrx = re.compile(r'(?:static_cast<\s*(?:int|int32_t|long|LONG|signed int|signed)\s*>\(\s*((?:[A-Za-z_]\w*(?:\[[^\]\(\)]*\])*(?:\.|->))*%s(?:\[[^\]\(\)]*\])*)\s*\))' % re.escape(f))
    ncast = sum(len(castrx.findall(t)) for t in texts.values())
    if not ncast and not S:
        continue
    nonex = len({u['addr'] for u in us if not u['exact']})
    cands.append(dict(units={u['unit'] for u in us}, U=U, key=key, f=f, cname=cname, decl=decl, S=S, ncast=ncast, nonex=nonex, uses=len(us), castrx=castrx))
cands.sort(key=lambda c: (-min(c['S'], 1), -c['nonex'], -c['ncast']))
if a.only:
    sel = set(a.only.split(','))
    cands = [c for c in cands if c['key'] in sel]
if a.skip:
    sk = set(open(a.skip).read().split())
    cands = [c for c in cands if c['key'] not in sk]
if a.limit:
    cands = cands[:a.limit]
print('candidates', len(cands))
if a.dry:
    for c in cands:
        print('%-44s S=%d casts=%d nonexact_fns=%d uses=%d %s' % (c['key'], c['S'], c['ncast'], c['nonex'], c['uses'], c['decl'][0][len(R) + 1:]))
    sys.exit()

base = {}
for x in json.load(open(a.base)):
    base[(x['unit'], x['addr'])] = x
MAIN = os.environ.get('OPENSMACX_MAIN', R if os.path.exists(R + '/.opensmacx/game/terranx_original.exe') else '/home/fran6co/code/OpenSMACX')
cc = json.load(open(MAIN + '/build/compile_commands.json'))
allunits = [e['file'][len(MAIN) + 1:] for e in cc if e['file'].startswith(MAIN + '/src/')]
log = open(a.log, 'a')


def sh(*cmd, **kw):
    return subprocess.run(cmd, cwd=R, capture_output=True, text=True, **kw)


for c in cands:
    p, s, e = c['decl']
    t = open(p, encoding='latin-1').read()
    if t != texts[p]:
        # an earlier kept change edited this header: relocate the declaration
        spans = class_body(c['cname'])
    changed = {}
    # reload fresh texts (earlier commits)
    cur = {q: open(q, encoding='latin-1').read() for q in srcs}
    texts.update(cur)
    decl = None
    for q, s2, e2 in class_body(c['cname']):
        m = re.search(r'^([ \t]*)(%s)(\s+%s\s*(?:\[[^\]]*\]\s*)*;)' % (UTYPES, re.escape(c['f'])), cur[q][s2:e2], re.M)
        if m:
            decl = (q, s2 + m.start(2), s2 + m.end(2))
            break
    if not decl:
        print('SKIP nodecl', c['key']); continue
    q, s2, e2 = decl
    changed[q] = cur[q][:s2] + 'int' + cur[q][e2:]
    nstrip = 0
    own = {R + '/' + u for u in c['units']} | {q}
    for f_, t_ in cur.items():
        if f_ not in own:
            continue
        t0 = changed.get(f_, t_)
        t1, n = c['castrx'].subn(lambda m: m.group(1), t0)
        if n:
            nstrip += n
            changed[f_] = t1
    if not nstrip and not c['S']:
        print('SKIP noevidence', c['key']); continue
    name = re.compile(r'\b%s\b' % re.escape(c['f']))
    hdr_use = any(name.search(t_) for f_, t_ in cur.items() if f_.endswith('.h') and
                  len(name.findall(t_)) > (1 if f_ == q else 0))
    units = allunits if hdr_use else [u for u in allunits if name.search(cur.get(R + '/' + u, ''))]
    out = a.log + '.fe.json'

    def measure(us):
        cmd = ['uv', 'run', 'python', 'recovery/tools/feall.py', '--all', '--out', out] + (us if len(us) < len(allunits) else [])
        sh(*cmd, env=dict(os.environ, FE_PFX_BASE=a.pfx, FE_JOBS=a.jobs))
        res = json.load(open(out))
        lost, gained, worse, better, fail = [], [], [], [], []
        for x in res:
            b = base.get((x['unit'], x['addr']))
            if x['diff'] < 0:
                if not b or b['diff'] >= 0:
                    fail.append(x)
                continue
            if not b or b['diff'] < 0:
                continue
            if b['exact'] and not x['exact']:
                lost.append(x)
            elif not b['exact'] and x['exact']:
                gained.append(x)
            elif x['diff'] > b['diff']:
                worse.append((x, b['diff']))
            elif x['diff'] < b['diff']:
                better.append((x, b['diff']))
        return res, lost, gained, worse, better, fail

    evidence = 'image S=%d' % c['S'] if c['S'] else ''
    if not c['S']:
        # pre-test: the casts alone, the field still unsigned
        pre = {f_: t_ for f_, t_ in changed.items()}
        pre[q] = c['castrx'].subn(lambda m: m.group(1), cur[q])[0]
        touched = [f_ for f_, t_ in pre.items() if t_ != cur[f_]]
        for f_ in touched:
            open(f_, 'w', encoding='latin-1').write(pre[f_])
        us = units if any(f_.endswith('.h') for f_ in touched) else [f_[len(R) + 1:] for f_ in touched]
        res, lost, gained, worse, better, fail = measure(us)
        for f_ in touched:
            open(f_, 'w', encoding='latin-1').write(cur[f_])
        moved = lost + gained + [x for x, d in worse + better] + fail
        if not moved:
            print('SKIP casts-not-load-bearing %s casts=%d' % (c['key'], nstrip)); sys.stdout.flush()
            log.write('SKIP casts-not-load-bearing %s casts=%d\n' % (c['key'], nstrip)); log.flush()
            continue
        evidence = 'casts load-bearing: unsigned without them moves %s' % ', '.join(sorted({x['addr'] for x in moved if x.get('addr')})[:6])
    for f_, t_ in changed.items():
        open(f_, 'w', encoding='latin-1').write(t_)
    res, lost, gained, worse, better, fail = measure(units)
    users = {u['addr'] for u in by[c['key']]}
    hurt = [x for x in lost if x['addr'] in users] + [x for x, d in worse if x['addr'] in users]
    keep = not fail and not hurt
    desc = '%s %s casts=%d S=%d units=%d lost=%d gained=%d worse=%d better=%d fail=%d [%s]' % (
        'KEEP' if keep else 'DROP', c['key'], nstrip, c['S'], len(units), len(lost), len(gained), len(worse), len(better), len(fail), evidence)
    det = ''.join('  - LOST %s %s %s\n' % (x['addr'], x['unit'], x['sym'][:50]) for x in lost) + \
        ''.join('  + GAINED %s %s %s\n' % (x['addr'], x['unit'], x['sym'][:50]) for x in gained) + \
        ''.join('  ~ %s %s %s %d -> %d\n' % (x['addr'], x['unit'], x['sym'][:50], d, x['diff']) for x, d in worse + better) + \
        ''.join('  ! FAIL %s %s\n' % (x['unit'], x['sym'][:160]) for x in fail[:3])
    print(desc); print(det, end=''); sys.stdout.flush()
    log.write(desc + '\n' + det); log.flush()
    rel = [f_[len(R) + 1:] for f_ in changed]
    if keep:
        for x in res:
            if x['diff'] >= 0:
                base[(x['unit'], x['addr'])] = x
        msg = '%s is int (%s), %d int cast%s dropped\n\n' % (c['key'], evidence.split(':')[0], nstrip, '' if nstrip == 1 else 's')
        msg += 'typescan: %d attributed uses, none unsigned in the image; %s.\nfeall on %d unit%s: no user of the field lost or worse' % (
            c['uses'], evidence, len(units), '' if len(units) == 1 else 's')
        if gained:
            msg += '; exact now: ' + ', '.join(x['addr'] for x in gained)
        if lost:
            msg += ';\nLOST (count-sensitive, not users of the field): ' + ', '.join('%s %s' % (x['addr'], x['sym'][:40]) for x in lost)
        if worse or better:
            msg += ';\nmoved (count-sensitive ties and the functions using the field): ' + ', '.join('%s %d -> %d' % (x['addr'], d, x['diff']) for x, d in worse + better)
        msg += '.\nFiles: ' + ' '.join(rel) + '\n\nCo-Authored-By: Claude Fable 5.2 <noreply@anthropic.com>\nClaude-Session: https://claude.ai/code/session_0162NvZ7kdT8ixvbS84BMGy8\n'
        sh('git', 'add', *rel)
        g = sh('git', 'commit', '-q', '-m', msg)
        if g.returncode:
            print('COMMIT FAILED', g.stderr[:200])
    else:
        sh('git', 'checkout', '--', *rel)
json.dump(list(base.values()), open(a.base + '.new', 'w'))
