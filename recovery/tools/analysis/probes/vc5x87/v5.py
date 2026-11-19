#!/usr/bin/env python3
"""v5.py: compile probe text with the Caviar library's VC5 recipe (/O2 /Ob1 --compiler=vc5) and return
each function's listing.  Library for the vc5x87 probe scripts; run from the repo root.
  compile(text, tag) -> {function name: [instruction, ...]}   (labels kept as `$Lnn:` lines)
Lane: FE_PFX_BASE (VC5 runs under its own prefix, VC5_PREFIX, default ~/opt/vc5/wineprefix)."""
import os, re, subprocess
HERE = os.path.dirname(os.path.abspath(__file__))
R = os.path.abspath(HERE + '/../../../../..')
MAIN = R if '/.opensmacx/worktrees/' not in R else R.split('/.opensmacx/worktrees/')[0]
PFX = os.environ.get('FE_PFX_BASE', '21')


def compile(text, tag='p', extra=()):
    scr = R + '/build/recovery/fe/vc5x87'
    os.makedirs(scr, exist_ok=True)
    base = '%s/%s_%s' % (scr, tag, PFX)
    src = base + '.cpp'
    open(src, 'w').write(text)
    cmd = [MAIN + '/recovery/environment/vc6-cl', '/nologo', '/TP', '/DWIN32', '/D_WINDOWS', '/GX', '/DNDEBUG',
           '-MT', '/O2', '/Ob2', '--compiler=vc5', '/Ob1'] + list(extra) + \
          ['/FAs', '/Fa' + base + '.asm', '/Fo' + base + '.obj', '-c', src]
    if os.path.exists(base + '.asm'):
        os.remove(base + '.asm')
    env = dict(os.environ, VC6_PREFIX='%s/.opensmacx/wine/worker-%s' % (MAIN, PFX))
    p = subprocess.run(cmd, cwd=scr, env=env, capture_output=True, text=True)
    if not os.path.exists(base + '.asm'):
        raise SystemExit("compile failed\n" + (p.stdout + p.stderr)[:3000])
    asm = open(base + '.asm', errors='replace').read()
    out = {}
    for m in re.finditer(r'^(\S+)\s+PROC NEAR[^\n]*\n(.*?)^\1\s+ENDP', asm, re.S | re.M):
        name = m.group(1)
        dm = re.match(r'\?(\w+)@@', name)
        name = dm.group(1) if dm else name.lstrip('_')
        lines = []
        for l in m.group(2).split('\n'):
            if l.startswith('\t;') or l.startswith(';'):
                continue
            mm = re.match(r'^(\$\w+):', l)
            if mm:
                lines.append(mm.group(1) + ':')
                continue
            if l.startswith('\t'):
                s = re.sub(r'\s+', ' ', l.strip().split(';')[0].strip())
                if s:
                    lines.append(s)
        out[name] = lines
    return out


def show(fs, only=None, filt=None):
    for k, v in fs.items():
        if only and not k.startswith(only):
            continue
        print('==', k)
        for l in v:
            if filt is None or filt(l):
                print('   ', l)
