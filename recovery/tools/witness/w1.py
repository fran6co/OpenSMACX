#!/usr/bin/env python3
"""w1.py [unit ...] [--addr A[,A]] [--all] [--json OUT] [--mac PATH]

W1, call sites against the Mac witness. For every non-exact marker (all markers
with --all) whose name has a twin in the Mac SMACX Carbon build, compare the
twin's named callees (source-level: CodeWarrior keeps most calls) with ours
compiled with inlining off (/Ob0: every call the source spells stays a call).

Per function it prints the callee names whose counts differ:
  MAC-ONLY  the source called a helper that we do not call (hand-expanded body,
            missing helper, or a helper under another name)
  OURS-ONLY we call something the Mac source does not (invented helper, or a
            body the source wrote out)
Names CodeWarrior inlines or never emits (no Mac function of that name) are
reported separately as invisible. Unnamed Mac targets (the statically linked
runtime) are labelled by majority vote over aligned twins.

A difference is a candidate, not a fact: the 2003 Mac port can differ from the
1999 Windows source, and the pinned image decides. Screens only."""
import argparse, collections, json, os, re, struct, subprocess, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
import fe  # noqa: E402  (recovery/tools/fe.py: compile database, COFF, prefixes)
from witness.pef import Pef, demangle  # noqa: E402

R = fe.R
MAC = os.path.join(fe.MAIN, '.opensmacx/witness/mac/carbon/SMACX Carbon')
SCR = os.path.join(fe.SCR, 'w1')


def msvc_name(sym):
    """`?draw@Buffer@@QAEXXZ` -> `Buffer::draw`; `??0Foo@@QAE@XZ` -> `Foo::<ctor>`;
    `_strcat` -> `strcat`; `sub_401000` stays."""
    if sym.startswith('??0') or sym.startswith('??1'):
        m = re.match(r'^\?\?([01])([A-Za-z_0-9]+)@', sym)
        if m:
            return m.group(2) + ('::<ctor>' if m.group(1) == '0' else '::<dtor>')
    if sym.startswith('??_G') or sym.startswith('??_E'):
        m = re.match(r'^\?\?_[GE]([A-Za-z_0-9]+)@', sym)
        return (m.group(1) + '::<dtor>') if m else sym
    m = re.match(r'^\?([A-Za-z_0-9]+)@(?:([A-Za-z_0-9]+)@)?@', sym)
    if m:
        return (m.group(2) + '::' if m.group(2) else '') + m.group(1)
    if sym.startswith('_') and not sym.startswith('__'):
        return sym[1:].split('@')[0]
    return sym


def norm(name):
    return name.lower()


def markers(units):
    rows = []
    for unit in units:
        text = open(os.path.join(R, unit), encoding='latin-1').read()
        for m in re.finditer(r'^// ORIGINAL: (0x[0-9A-Fa-f]{8}) (\S+) ([^\n]*)', text, re.M):
            rows.append(dict(unit=unit, addr=int(m.group(1), 16), symbol=m.group(2),
                             exact='BYTE_EXACT' in m.group(3)))
    return rows


def compile_ob0(unit, pfx):
    cmd = fe.compile_cmd(unit)
    if not cmd:
        return None, 'unit not in compile_commands'
    os.makedirs(SCR, exist_ok=True)
    tag = re.sub(r'[^a-z0-9]', '_', unit.lower())
    obj = os.path.join(SCR, tag + '.obj')
    out = []
    for t in cmd:
        if t.startswith('/Fo'):
            t = '/Fo' + obj
        elif t.startswith('/Fd') or t.startswith('--repro-root') or t == '/W4':
            continue
        elif t.startswith('/Ob'):
            t = '/Ob0'
        out.append(t)
    env = dict(os.environ, VC6_PREFIX=f'{fe.MAIN}/.opensmacx/wine/worker-{pfx}')
    if os.path.exists(obj):
        os.remove(obj)
    p = subprocess.run(out, cwd=SCR, env=env, capture_output=True, text=True, stdin=subprocess.DEVNULL)
    if not os.path.exists(obj):
        return None, (p.stdout + p.stderr)[-800:]
    return fe.Coff(open(obj, 'rb').read()), None


def _raw_calls(coff, symbol):
    try:
        s = coff.function(symbol)
    except Exception:
        return None
    sec = coff.sections[s.section - 1]
    out = []
    for off, target, kind in sorted(sec.relocations):
        if kind != 20:   # IMAGE_REL_I386_REL32: call/jmp rel32
            continue
        sy = coff.symbols.get(target)
        if sy is not None:
            out.append(sy.name)
    return out


def our_calls(coff, symbol, mac_names=None, depth=4):
    """Ordered callee names. With `mac_names`, a callee that has no Mac function
    (CodeWarrior inlined it, or the recovery invented it) is replaced by its own
    callees, recursively, as CodeWarrior's expansion would show them."""
    raw = _raw_calls(coff, symbol)
    if raw is None:
        return None
    out = []
    for sym in raw:
        name = msvc_name(sym)
        if mac_names is not None and depth and norm(name) not in mac_names:
            inner = our_calls(coff, sym, mac_names, depth - 1)
            if inner is not None:
                out.extend(inner)
                continue
        out.append(name)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='*')
    ap.add_argument('--addr')
    ap.add_argument('--all', action='store_true')
    ap.add_argument('--json')
    ap.add_argument('--mac', default=MAC)
    ap.add_argument('--pfx', default=fe.PFX)
    a = ap.parse_args()

    mac = Pef(a.mac)
    mac_by = collections.defaultdict(list)
    for f in mac.functions:
        mac_by[norm(demangle(f[2]))].append(f)
    mac_names = set(mac_by)

    units = a.units or sorted({e['file'][len(R) + 1:] for e in json.load(open(fe.MAIN + '/build/compile_commands.json'))
                               if e['file'].endswith('.cpp') and '/src/' in e['file']})
    want = {int(x, 16) for x in a.addr.split(',')} if a.addr else None
    report = []
    for unit in units:
        rows = [r for r in markers([unit]) if (a.all or not r['exact']) and (want is None or r['addr'] in want)]
        if not rows:
            continue
        coff, err = compile_ob0(unit, a.pfx)
        if coff is None:
            print('COMPILE-ERROR', unit, err.strip().splitlines()[-1] if err else '')
            continue
        for r in rows:
            key = norm(msvc_name(r['symbol']))
            twins = mac_by.get(key)
            if not twins:
                continue
            ours = our_calls(coff, r['symbol'], mac_names)
            if ours is None:
                continue
            best = None
            for tw in twins:   # overloads: the twin whose callee set is closest
                mc = [x for x in mac.calls(tw) if not x.startswith('@')]
                score = sum((collections.Counter(map(norm, mc)) & collections.Counter(map(norm, ours))).values())
                if best is None or score > best[0]:
                    best = (score, tw, mc)
            _, tw, mc = best
            mcount = collections.Counter(map(norm, mc))
            ocount = collections.Counter(map(norm, ours))
            mac_only = {k: (mcount[k], ocount[k]) for k in mcount if mcount[k] > ocount[k]}
            ours_only = {k: (mcount[k], ocount[k]) for k in ocount if ocount[k] > mcount[k] and k in mac_names}
            invisible = sorted(k for k in ocount if k not in mac_names and k not in mcount)
            rec = dict(unit=unit, addr='0x%08X' % r['addr'], symbol=r['symbol'], mac=tw[2], exact=r['exact'],
                       mac_calls=len(mc), our_calls=len(ours), mac_only=mac_only, ours_only=ours_only,
                       invisible=invisible)
            report.append(rec)
            if mac_only or ours_only:
                print('%s %-40s mac=%d ours=%d' % (rec['addr'], msvc_name(r['symbol'])[:40], len(mc), len(ours)))
                for k, (m_, o_) in sorted(mac_only.items(), key=lambda kv: -(kv[1][0] - kv[1][1])):
                    print('    MAC-ONLY  %-32s mac %3d ours %3d' % (k, m_, o_))
                for k, (m_, o_) in sorted(ours_only.items(), key=lambda kv: -(kv[1][1] - kv[1][0])):
                    print('    OURS-ONLY %-32s mac %3d ours %3d' % (k, m_, o_))
    if a.json:
        json.dump(report, open(a.json, 'w'), indent=1)
    differ = [x for x in report if x['mac_only'] or x['ours_only']]
    print('W1: %d functions with a Mac twin, %d with differing named calls' % (len(report), len(differ)))


if __name__ == '__main__':
    main()
