#!/usr/bin/env python3
"""run.py: base/index order of the two vtordisp stores of an inlined ~RadioButton (MATCHING_GUIDE 3.4).
The final ~Popup teardown expands ~Dialogs -> ~RadioButton, whose prologue writes the two vtordisp slots:
`lea edx,[eax-0x18]; mov [X+Y-4],edx` (GraphicWin) and `lea edx,[eax-0xa30]; mov [X+Y-4],edx` (Dialog).
Per store it prints `T` when `this` is the base ([esi+eax-4]) and `i` when the vbtable offset is ([eax+esi-4]).
The probe is one function of faction.cpp's header context: `Popup popup; popup.start(..);` N lines of
`parse_says(i%4, get_title(k + i), -1, -1);`, `return popup.exec();`, compiled with faction.cpp's recipe.
Usage: uv run python run.py census          the image and ours at the 11 image expansions
       uv run python run.py sweep LO HI      the pattern for N = LO..HI-1
       uv run python run.py split            the same N split before/after the Popup (position test)
(FE_PFX_BASE.., FE_JOBS lanes; run from the repo root.)"""
import sys, os, re, subprocess
from concurrent.futures import ThreadPoolExecutor
R = os.getcwd(); sys.path.insert(0, R + '/recovery/tools')
import fe
PFX = int(os.environ.get('FE_PFX_BASE', '21')); JOBS = int(os.environ.get('FE_JOBS', '5'))
SCR = os.environ.get('FE_SCRATCH', R + '/build/recovery/fe')
HDR = ''.join(open(R + '/src/faction.cpp', encoding='latin-1').readlines()[17:61])  # defines + includes


def scan(c):
    names = {}
    for s in c.symbols.values():
        if s.section > 0 and s.value == 0 and s.name[:1] in '?_': names.setdefault(s.section, s.name)
    out = {}
    for i, sec in enumerate(c.sections, 1):
        if not sec.name.startswith('.text'): continue
        ins = list(fe.md.disasm(bytes(sec.data), 0)); pats = ''
        for a, b in zip(ins, ins[1:]):
            m = re.match(r'(\w+), \[(\w+) - (0x18|0xa30)\]', a.op_str)
            m2 = re.match(r'dword ptr \[(\w+) \+ (\w+) - 4\], (\w+)', b.op_str)
            if a.mnemonic == 'lea' and m and b.mnemonic == 'mov' and m2:
                pats += 'i' if m2.group(1) == m.group(2) else 'T'
        if pats: out[names.get(i, '?')] = pats
    return out


def compile_probe(body, lane):
    p = '%s/vtordisp_%d.cpp' % (SCR, lane)
    open(p, 'w', encoding='latin-1').write(HDR + '\n' + body)
    c, err = fe.compile_unit('src/faction.cpp', p, PFX + lane)
    return scan(c) if c else 'ERR ' + str(err)


def probe(n_before, n_after):
    f = lambda r: ''.join('    parse_says(%d, get_title(k + %d), -1, -1);\n' % (i % 4, i) for i in r)
    return ('int __cdecl probe_a(int k) {\n%s    Popup popup;\n    popup.start("PROPOSAL", 0x100000);\n%s'
            '    return popup.exec();\n}\n' % (f(range(n_before)), f(range(n_before, n_before + n_after))))


def run(keys, mk):
    fe.compile_cmd('src/faction.cpp'); res = {}
    def lane(l):
        for j in range(l, len(keys), JOBS):
            r = compile_probe(mk(keys[j]), l)
            res[j] = r if isinstance(r, str) else r.get('?probe_a@@YAHH@Z', '-')
    with ThreadPoolExecutor(JOBS) as ex: list(ex.map(lane, range(JOBS)))
    return [res[j] for j in range(len(keys))]


SITES = [('designwin.cpp', 0x00438460), ('diplowin.cpp', 0x004415C0), ('console_commands.cpp', 0x004D61A0),
         ('base.cpp', 0x004F07E0), ('netdaemon.cpp', 0x00534400), ('faction.cpp', 0x00541100),
         ('faction.cpp', 0x00545E80), ('faction.cpp', 0x0054DCF0), ('alpha.cpp', 0x00589D30),
         ('orders.cpp', 0x005947C0), ('technology.cpp', 0x005BB000)]


def census():
    def one(a):
        j, (u, ad) = a
        o = subprocess.run([sys.executable, R + '/recovery/tools/fe.py', u, '0x%08X' % ad, '--all', '--pfx',
                            str(PFX + j % JOBS)], cwd=R, capture_output=True, text=True).stdout
        im = us = ''
        for l in o.split('\n'):
            p = l.split('|')
            if len(p) < 2: continue
            a_ = re.search(r'\[(\w+) \+ (\w+) - 4\], edx', p[0]); b_ = re.search(r'\[(\w+) \+ (\w+) - 4\], edx', p[1])
            if a_ and 'ebp' not in p[0]: im += 'T' if a_.group(1) == 'esi' else 'i'
            if b_ and 'ebp' not in p[1]: us += 'T' if b_.group(1) == 'esi' else 'i'
        return u, ad, im, us
    with ThreadPoolExecutor(JOBS) as ex:
        for u, ad, im, us in ex.map(one, enumerate(SITES)):
            print('%-22s 0x%08X image %s ours %s %s' % (u, ad, im, us, '' if im == us else '<-'))


if __name__ == '__main__':
    cmd = sys.argv[1] if len(sys.argv) > 1 else 'census'
    if cmd == 'census': census()
    elif cmd == 'sweep':
        ks = list(range(int(sys.argv[2]), int(sys.argv[3])))
        for k, r in zip(ks, run(ks, lambda n: probe(0, n))): print(k, r)
    elif cmd == 'split':
        ks = [(a, 120 - a) for a in (0, 40, 80, 120)] + [(a, 76 - a) for a in (0, 38, 76)]
        for k, r in zip(ks, run(ks, lambda k: probe(*k))): print(k, r)
