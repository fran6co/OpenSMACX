#!/usr/bin/env python3
"""run.py [--oy] [--asm] [--pfx N] <probe.cpp>... [-f func]
Compile each probe file with the game's unit flags (/Og /Ot /Gs /Gy /Ob2 /Oy-; --oy uses /Oy)
and print, per function, the home of every probe variable at each of its sites:
  name  a:esi b:edi c:ebx d:[d]      (reg, [slot] = reloaded from the frame, r/[s] = changes along the function)
A variable x is defined by def_x() and read by use_x(x) / sink_x = x (see probe.h).  --asm prints the listing."""
import sys, os, re, subprocess, argparse
HERE = os.path.dirname(os.path.abspath(__file__))
R = os.path.abspath(HERE + '/../../../..')
MAIN = os.environ.get('OPENSMACX_MAIN', R if os.path.exists(R + '/.opensmacx/game/terranx_original.exe') else '/home/fran6co/code/OpenSMACX')
CS = ('ebx', 'esi', 'edi', 'ebp')
SUB = {'bl': 'ebx', 'bx': 'ebx', 'si': 'esi', 'di': 'edi'}


def compile_asm(src, pfx, oy=False, extra=()):
    scr = os.environ.get('FE_SCRATCH', R + '/build/recovery/fe') + '/regalloc'
    os.makedirs(scr, exist_ok=True)
    base = '%s/%s_%s' % (scr, os.path.basename(src)[:-4], pfx)
    cmd = [MAIN + '/recovery/environment/vc6-cl', '/nologo', '/TP', '/GX', '/DNDEBUG', '-MT', '/Og', '/Ot', '/Gs', '/Gy',
           '/Ob2', '/Oy' if oy else '/Oy-'] + list(extra) + ['-I' + HERE, '/FAs', '/Fa' + base + '.asm', '/Fo' + base + '.obj', '-c', os.path.abspath(src)]
    env = dict(os.environ, VC6_PREFIX='%s/.opensmacx/wine/worker-%s' % (MAIN, pfx))
    if os.path.exists(base + '.asm'):
        os.remove(base + '.asm')
    p = subprocess.run(cmd, cwd=scr, env=env, capture_output=True, text=True)
    if not os.path.exists(base + '.asm'):
        raise SystemExit('compile failed: ' + src + '\n' + p.stdout + p.stderr)
    err = [l for l in (p.stdout + p.stderr).split('\n') if ' error ' in l]
    if err:
        raise SystemExit('\n'.join(err))
    return open(base + '.asm', errors='replace').read()


def functions(asm):
    """-> [(name, [instruction lines])] in file order"""
    out = []
    for m in re.finditer(r'^(\S+) PROC NEAR\s*; ([^\n,]+)[^\n]*\n(.*?)^\1 ENDP', asm, re.S | re.M):
        ins = [l.strip().split(';')[0].strip() for l in m.group(3).split('\n') if l.startswith('\t') and not l.startswith('\t;')]
        out.append((m.group(2).strip(), [i for i in ins if i]))
    return out


def home_of(ins, k, reg):
    """where did scratch register `reg` (read at ins[k]) come from: callee-saved reg or [slot]"""
    for j in range(k - 1, max(-1, k - 12), -1):
        m = re.match(r'(?:mov|movsx|movzx)\s+%s, (.*)' % reg, ins[j])
        if m:
            src = m.group(1)
            if src in CS:
                return src
            if src in SUB:
                return SUB[src]
            s = re.search(r'_(\w+)\$\d*\[', src)
            if s:
                return '[%s]' % s.group(1)
            return '?' + src.replace('DWORD PTR ', '')
    return '?'


def homes(ins):
    seen = {}
    for k, l in enumerate(ins):
        v = None
        m = re.match(r'call\s+\?(def|use)_(\w)@@', l)
        if m and m.group(1) == 'use':
            v = m.group(2)
            for j in range(k - 1, max(-1, k - 8), -1):
                p = re.match(r'push\s+(.*)', ins[j])
                if p:
                    a = p.group(1)
                    s = re.search(r'_(\w+)\$\d*\[', a)
                    h = a if a in CS else ('[%s]' % s.group(1) if s else home_of(ins, j, a))
                    break
            else:
                h = '?'
        elif m:
            v = m.group(2)
            h = '?'
            for j in range(k + 1, min(len(ins), k + 6)):
                p = re.match(r'mov\s+(.*), eax$', ins[j])
                if p:
                    a = p.group(1)
                    s = re.search(r'_(\w+)\$\d*\[', a)
                    h = a if a in CS else ('[%s]' % s.group(1) if s else '?' + a)
                    break
                if ins[j].startswith('call'):
                    break
            if h == '?':
                continue  # value consumed in eax; later sites tell
        else:
            m = re.match(r'mov\s+DWORD PTR \?sink_(\w)@@3HA, (\w+)', l)
            if m:
                v = m.group(1)
                h = m.group(2) if m.group(2) in CS else home_of(ins, k, m.group(2))
        if v:
            seen.setdefault(v, [])
            if not seen[v] or seen[v][-1] != h:
                seen[v].append(h)
    return seen


def report(src, pfx, oy=False, only=None, show=False):
    asm = compile_asm(src, pfx, oy)
    text = open(src).read()
    res = {}
    for name, ins in functions(asm):
        if only and name not in only:
            continue
        h = homes(ins)
        saved = [i.split()[1] for i in ins[:8] if i.startswith('push') and i.split()[1] in CS and i.split()[1] != 'ebp']
        frame = next((i for i in ins[:4] if i.startswith('sub\tesp') or i == 'push\tecx'), '')
        res[name] = h
        note = re.search(r'((?://[^\n]*\n)+)[^\n]*\b%s\(' % re.escape(name), text)
        if note:
            print('  ' + note.group(1).strip().replace('\n', '\n  '))
        print('%-14s %s   saved=%s%s' % (name, ' '.join('%s:%s' % (v, '/'.join(x)) for v, x in sorted(h.items())), ','.join(saved),
                                         ' frame' if frame else ''))
        if show:
            print('\n'.join('      ' + i for i in ins))
    return res


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('src', nargs='+')
    ap.add_argument('--oy', action='store_true')
    ap.add_argument('--asm', action='store_true')
    ap.add_argument('--pfx', default=os.environ.get('FE_PFX_BASE', '2'))
    ap.add_argument('-f', action='append')
    a = ap.parse_args()
    for s in a.src:
        print('== %s%s' % (os.path.basename(s), ' (/Oy)' if a.oy else ''))
        report(s, a.pfx, a.oy, a.f, a.asm)
