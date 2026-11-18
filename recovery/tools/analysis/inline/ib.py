#!/usr/bin/env python3
"""ib.py: compile an inline-budget probe file with the game unit flags and report, per function,
which call sites of each inline function were expanded (E) or kept as calls (c).

Probe conventions (see probe.h):
  * an inline function body calls mk(K) / dk(K) where K is the site's constant argument, so a
    site is "expanded" when `push K; call mk` appears in the caller and "called" when
    `push K; call <inline function>` appears;
  * destructor teardown sites: write `at(i);` immediately before a return; the tail after it
    (following unconditional jumps) is scanned for `call dm` (expanded ~D) or `call ??1D@@` (called).
Usage: python3 ib.py <probe.cpp> [-f func] [--asm] [--oy] [--pfx N] [--states]
"""
import sys, os, re, subprocess, argparse
HERE = os.path.dirname(os.path.abspath(__file__))
R = os.path.abspath(HERE + '/../../../..')
MAIN = os.environ.get('OPENSMACX_MAIN', R if os.path.exists(R + '/.opensmacx/game/terranx_original.exe') else '/home/fran6co/code/OpenSMACX')


def compile_asm(src, pfx, oy=False, extra=(), text=None):
    scr = os.environ.get('FE_SCRATCH', R + '/build/recovery/fe') + '/inline'
    os.makedirs(scr, exist_ok=True)
    tag = os.path.basename(src)[:-4] if src else 'deck'
    base = '%s/%s_%s' % (scr, tag, pfx)
    if text is not None:
        src = base + '_in.cpp'
        open(src, 'w').write(text)
    cmd = [MAIN + '/recovery/environment/vc6-cl', '/nologo', '/TP', '/GX', '/DNDEBUG', '-MT', '/Og', '/Ot', '/Gs', '/Gy',
           '/Ob2', '/Oy' if oy else '/Oy-', '/W4'] + list(extra) + ['-I' + HERE, '/FAs', '/Fa' + base + '.asm', '/Fo' + base + '.obj', '-c', os.path.abspath(src)]
    env = dict(os.environ, VC6_PREFIX='%s/.opensmacx/wine/worker-%s' % (MAIN, pfx))
    if os.path.exists(base + '.asm'):
        os.remove(base + '.asm')
    p = subprocess.run(cmd, cwd=scr, env=env, capture_output=True, text=True)
    out = p.stdout + p.stderr
    if not os.path.exists(base + '.asm'):
        raise SystemExit('compile failed: ' + src + '\n' + out)
    err = [l for l in out.split('\n') if ' error ' in l]
    if err:
        raise SystemExit('\n'.join(err))
    return open(base + '.asm', errors='replace').read(), out


def functions(asm):
    out = []
    for m in re.finditer(r'^(\S+) PROC NEAR\s*; ([^\n,]+)[^\n]*\n(.*?)^\1 ENDP', asm, re.S | re.M):
        lines = []
        for l in m.group(3).split('\n'):
            if l.startswith('\t;'):
                continue
            mm = re.match(r'^(\$\w+):', l)
            if mm:
                lines.append(mm.group(1) + ':')
                continue
            if l.startswith('\t'):
                s = l.strip().split(';')[0].strip()
                if s:
                    lines.append(s)
        out.append((m.group(2).strip(), m.group(1), lines))
    return out


def demangle_name(sym):
    m = re.match(r'\?\?1(\w+)@@', sym)
    if m:
        return '~' + m.group(1)
    m = re.match(r'\?(\w+)@', sym)
    return m.group(1) if m else sym


def last_push(lines, k):
    for j in range(k - 1, max(-1, k - 10), -1):
        m = re.match(r'push\s+(-?\d+)$', lines[j])
        if m:
            return int(m.group(1))
        if lines[j].startswith('call') or lines[j].endswith(':'):
            return None
    return None


def sites(lines):
    """-> list of (k, callee, 'E'|'c') in listing order for every pushed-constant site."""
    res = []
    for k, l in enumerate(lines):
        m = re.match(r'call\s+(\S+)', l)
        if not m:
            continue
        sym = m.group(1)
        name = demangle_name(sym)
        K = last_push(lines, k)
        if K is None:
            continue
        if name in ('mk',):
            res.append((K, name, 'E'))
        elif name not in ('at', 'dm', 'dm2', 'dm3', 'cond', 'call', 'use', 'sink') and K is not None:
            res.append((K, name, 'c'))
    return res


def tails(lines):
    """at(i) sites: -> {i: [token...]} with token 'E<n>' (expanded inline destructor: saw dm<n>) or 'c<X>'
    (call of ~X), in execution order from the at(i) call, following unconditional jumps (tail-merged
    return paths share one copy)."""
    labels = {l[:-1]: i for i, l in enumerate(lines) if l.endswith(':')}

    def walk(j):
        toks = []
        steps = 0
        while j < len(lines) and steps < 600:
            steps += 1
            s = lines[j]
            if s.startswith('ret'):
                break
            m = re.match(r'jmp\s+(?:SHORT\s+)?(\$\w+)$', s)
            if m:
                j = labels.get(m.group(1), len(lines)) + 1
                continue
            if s.startswith('jmp\t??1'):
                toks.append('c' + demangle_name(s.split()[-1])[1:])
                break
            m = re.match(r'call\s+(\S+)', s)
            if m:
                n = demangle_name(m.group(1))
                if n.startswith('dm'):
                    toks.append('E' + n[2:])
                elif n.startswith('~'):
                    toks.append('c' + n[1:])
                elif n == 'at':
                    break
                else:
                    toks.append(m.group(1) if n.startswith('?') or '@' in m.group(1) else n)
            j += 1
        return toks

    res = {}
    for k, l in enumerate(lines):
        m = re.match(r'push\s+(-?\d+)$', l)
        if not m or k + 1 >= len(lines):
            continue
        nxt = lines[k + 1]
        j = None
        q = k + 1
        while q < len(lines) and lines[q].endswith(':'):
            q += 1
        if q < len(lines) and re.match(r'call\s+\?at@@', lines[q]):
            j = q + 1
        else:
            mm = re.match(r'jmp\s+(?:SHORT\s+)?(\$\w+)$', nxt)
            if mm and mm.group(1) in labels:
                q = labels[mm.group(1)] + 1
                if q < len(lines) and re.match(r'call\s+\?at@@', lines[q]):
                    j = q + 1
        if j is not None:
            res[int(m.group(1))] = walk(j)
    return res


def report(src, pfx, oy=False, only=None, show=False, text=None, quiet=False):
    asm, out = compile_asm(src, pfx, oy, text=text)
    res = {}
    for name, sym, lines in functions(asm):
        if only and name not in only:
            continue
        s = sites(lines)
        t = tails(lines)
        res[name] = (s, t, lines)
        if quiet:
            continue
        order = sorted(s, key=lambda x: (x[0] if x[0] is not None else 1 << 30))
        print('%-18s %s' % (name, ' '.join('%s%s' % (st, k) for k, cal, st in order)))
        if t:
            print('%-18s tails %s' % ('', ' '.join('%d:%s' % (i, ''.join(t[i]) or '-') for i in sorted(t))))
        if show:
            print('\n'.join('      ' + i for i in lines))
    return res


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('src', nargs='+')
    ap.add_argument('--oy', action='store_true')
    ap.add_argument('--asm', action='store_true')
    ap.add_argument('--pfx', default=os.environ.get('FE_PFX_BASE', '21'))
    ap.add_argument('-f', action='append')
    a = ap.parse_args()
    for s in a.src:
        print('== %s' % os.path.basename(s))
        report(s, a.pfx, a.oy, a.f, a.asm)


def compile_in_unit(unit, text, pfx, name='inunit'):
    """compile `text` with the production command of game unit `unit` (its include paths, defines and
    flags) and return the /FAs listing.  Lets a probe use the real headers (Popup, Scroll, ...)."""
    sys.path.insert(0, os.path.dirname(os.path.dirname(HERE)))
    import fe
    cmd = fe.compile_cmd(unit)
    if not cmd:
        raise SystemExit('unit not in compile_commands: ' + unit)
    scr = os.environ.get('FE_SCRATCH', R + '/build/recovery/fe') + '/inline'
    os.makedirs(scr, exist_ok=True)
    base = '%s/%s_%s' % (scr, name, pfx)
    src = base + '.cpp'
    open(src, 'w').write(text)
    out = []
    for t in cmd:
        if t.startswith('/Fo'):
            t = '/Fo' + base + '.obj'
        elif t.startswith('/Fd') or t.startswith('--repro-root') or t == '/W4':
            continue
        elif t == R + '/' + unit:
            t = src
        out.append(t)
    out.insert(1, '/FAs')
    out.insert(2, '/Fa' + base + '.asm')
    env = dict(os.environ, VC6_PREFIX='%s/.opensmacx/wine/worker-%s' % (MAIN, pfx))
    if os.path.exists(base + '.asm'):
        os.remove(base + '.asm')
    p = subprocess.run(out, cwd=scr, env=env, capture_output=True, text=True)
    if not os.path.exists(base + '.asm'):
        raise SystemExit('compile failed\n' + (p.stdout + p.stderr)[-3000:])
    return open(base + '.asm', errors='replace').read()
