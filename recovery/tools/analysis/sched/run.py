#!/usr/bin/env python3
"""run.py [--oy] [--pfx N] [-f REGEX] [--full] <probe.cpp>...
Compile a scheduling probe file with the game's unit flags (/Og /Ot /Gs /Gy /Ob2 /GX, /Oy- or --oy)
and print each function's body (prologue/epilogue trimmed unless --full), one instruction per line,
frame-slot names kept as the /FAs listing gives them (_x$[ebp]).
Functions are grouped by the name before the first `__`: the first member of a group is printed in full,
later members print `= <first>` when identical, else a side-by-side diff against the first.
Prefix: FE_PFX_BASE (use your own; par-5 uses 27)."""
import sys, os, re, subprocess, argparse, difflib
HERE = os.path.dirname(os.path.abspath(__file__))
R = os.path.abspath(HERE + '/../../../..')
MAIN = os.environ.get('OPENSMACX_MAIN', R if os.path.exists(R + '/.opensmacx/game/terranx_original.exe') else '/home/fran6co/code/OpenSMACX')


def compile_asm(src, pfx, oy=False, extra=()):
    scr = os.environ.get('FE_SCRATCH', R + '/build/recovery/fe') + '/sched'
    os.makedirs(scr, exist_ok=True)
    base = '%s/%s_%s' % (scr, os.path.basename(src)[:-4], pfx)
    cmd = [MAIN + '/recovery/environment/vc6-cl', '/nologo', '/TP', '/GX', '/DNDEBUG', '-MT', '/Og', '/Ot', '/Gs', '/Gy',
           '/Ob2', '/Oy' if oy else '/Oy-'] + list(extra) + ['-I' + HERE, '/FAs', '/Fa' + base + '.asm', '/Fo' + base + '.obj', '-c', os.path.abspath(src)]
    env = dict(os.environ, VC6_PREFIX='%s/.opensmacx/wine/worker-%s' % (MAIN, pfx))
    if os.path.exists(base + '.asm'):
        os.remove(base + '.asm')
    p = subprocess.run(cmd, cwd=scr, env=env, capture_output=True, text=True)
    err = [l for l in (p.stdout + p.stderr).split('\n') if ' error ' in l]
    if err or not os.path.exists(base + '.asm'):
        raise SystemExit('compile failed: ' + src + '\n' + '\n'.join(err or [p.stdout + p.stderr]))
    return open(base + '.asm', errors='replace').read()


def clean(l):
    l = l.split(';')[0].strip().replace('DWORD PTR ', 'd ').replace('BYTE PTR ', 'b ').replace('WORD PTR ', 'w ')
    l = re.sub(r'\?(\w+)@@\S*', r'\1', l)
    l = re.sub(r'\?(\w+)@(\w+)@@\S*', r'\2::\1', l)
    return re.sub(r'\s+', ' ', l)


def functions(asm, full=False):
    out = []
    for m in re.finditer(r'^(\S+) PROC NEAR\s*; ([^\n,(]+)[^\n]*\n(.*?)^\1 ENDP', asm, re.S | re.M):
        ins = []
        for l in m.group(3).split('\n'):
            if l.startswith('\t;') or not l.strip():
                continue
            if l.startswith('$L') or l.startswith('$'):
                ins.append(l.split(':')[0] + ':')
                continue
            if l.startswith('\t'):
                c = clean(l)
                if c:
                    ins.append(c)
        if not full:
            while ins and re.match(r'(push (ebp|ebx|esi|edi)|mov ebp, esp|sub esp, \d+|push ecx)$', ins[0]):
                ins.pop(0)
            while ins and re.match(r'(pop (ebp|ebx|esi|edi)|ret.*|mov esp, ebp|leave)$', ins[-1]):
                ins.pop()
        out.append((m.group(2).strip(), ins))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('src', nargs='+')
    ap.add_argument('--oy', action='store_true')
    ap.add_argument('--full', action='store_true')
    ap.add_argument('--pfx', default=os.environ.get('FE_PFX_BASE', '27'))
    ap.add_argument('-f', default=None)
    ap.add_argument('--cut', default=None, help='REGEX[@K]: keep only the instructions after the K-th (default 1st) match')
    ap.add_argument('--upto', default=None, help='REGEX: stop at the first match after the cut')
    ap.add_argument('--brief', action='store_true', help='one line per function (instructions joined by "; ")')
    a = ap.parse_args()
    for s in a.src:
        print('== %s%s' % (os.path.basename(s), ' (/Oy)' if a.oy else ''))
        first = {}
        for name, ins in functions(compile_asm(s, a.pfx, a.oy), a.full):
            if a.f and not re.search(a.f, name):
                continue
            if a.cut:
                rx, _, k = a.cut.partition('@')
                hits = [i for i, x in enumerate(ins) if re.search(rx, x)]
                k = int(k or 1)
                ins = ins[hits[k - 1] + 1:] if len(hits) >= k else ['<no cut>']
            if a.upto:
                hit = [i for i, x in enumerate(ins) if re.search(a.upto, x)]
                if hit:
                    ins = ins[:hit[0] + 1]
            if a.brief:
                print('%-24s %s' % (name, '; '.join(ins)))
                continue
            g = name.split('__')[0]
            if g not in first:
                first[g] = (name, ins)
                print('--- %s' % name)
                print('\n'.join('    ' + i for i in ins))
                continue
            n0, i0 = first[g]
            if ins == i0:
                print('--- %s = %s' % (name, n0))
                continue
            print('--- %s (vs %s)' % (name, n0))
            sm = difflib.SequenceMatcher(None, i0, ins, autojunk=False)
            for op, a1, a2, b1, b2 in sm.get_opcodes():
                if op == 'equal':
                    continue
                for k in range(max(a2 - a1, b2 - b1)):
                    x = i0[a1 + k] if a1 + k < a2 else ''
                    y = ins[b1 + k] if b1 + k < b2 else ''
                    print('    %-36s | %s' % (x, y))


if __name__ == '__main__':
    main()
