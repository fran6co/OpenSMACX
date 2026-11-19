#!/usr/bin/env python3
"""findpat.py <name> [--exact|--all] : scan every ORIGINAL marker's image span for a named instruction pattern.
Patterns (regexes over 'mnemonic op_str' rows, consecutive within a window):
  addsub  `sub A,B` then `lea A,[A + B - K]` / `add A,B` within 3 rows: (x - y) + y kept unsimplified
  bytecse `mov r8,byte ptr [...]` then two or more `movsx r32, r8` of that byte register (byte held, extended per use)
  negcopy `neg dword ptr [ebp + N]` / `neg r; mov [ebp + N], r` followed by a load of [ebp + N]: negate in place, copy
Prints address, function, marker status (X = BYTE_EXACT claimed) and the matched rows.
Usage: uv run python recovery/tools/analysis/findpat.py addsub [--nonexact]"""
import sys, os, re, glob
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import fe
R = fe.R
MARK = re.compile(r'// ORIGINAL: (0x[0-9A-Fa-f]+) (\S+) (0x[0-9A-Fa-f]+)-(0x[0-9A-Fa-f]+)(.*)')

def markers():
    for f in sorted(glob.glob(R + '/src/**/*.cpp', recursive=True)):
        for line in open(f, encoding='latin-1'):
            m = MARK.search(line)
            if m:
                yield f[len(R) + 1:], int(m.group(1), 16), m.group(2), int(m.group(3), 16), int(m.group(4), 16), 'BYTE_EXACT' in m.group(5)

def rows(lo, hi):
    return [(i.address, i.mnemonic, i.op_str) for i in fe.md.disasm(fe.imgbytes(lo, hi), lo)]

def addsub(L):
    out = []
    for k, (a, m, o) in enumerate(L):
        if m != 'sub':
            continue
        p = [x.strip() for x in o.split(',')]
        if len(p) != 2 or not re.match(r'e[a-d]x|e[sd]i|ebp$', p[1]) or not re.match(r'e[a-d]x|e[sd]i|ebp$', p[0]):
            continue
        A, B = p
        for j in range(k + 1, min(k + 4, len(L))):
            a2, m2, o2 = L[j]
            if m2 == 'lea' and re.match(r'%s, \[(%s \+ %s|%s \+ %s)( [-+] 0x[0-9a-f]+| [-+] \d+)?\]' % (A, A, B, B, A), o2):
                out.append((a, ['%s %s' % (m, o), '%s %s' % (m2, o2)]))
            if m2 == 'add' and o2 in ('%s, %s' % (A, B),):
                out.append((a, ['%s %s' % (m, o), '%s %s' % (m2, o2)]))
            if o2.split(',')[0].strip() in (A, B) and m2 not in ('push', 'cmp', 'test'):
                break
    return out

def bytecse(L):
    out = []
    for k, (a, m, o) in enumerate(L):
        mm = re.match(r'([a-d]l), byte ptr \[(.*)\]$', o) if m == 'mov' else None
        if not mm:
            continue
        r8 = mm.group(1); n = 0; seen = []
        for j in range(k + 1, min(k + 40, len(L))):
            a2, m2, o2 = L[j]
            if m2 == 'movsx' and o2.endswith(', ' + r8):
                n += 1; seen.append('%x' % a2)
            elif o2.split(',')[0].strip() in (r8, 'e' + r8[0] + 'x') and m2 not in ('push', 'cmp', 'test') and not (m2 == 'movsx' and o2.endswith(r8)):
                break
        if n >= 2:
            out.append((a, ['%s %s' % (m, o), 'movsx x%d at %s' % (n, ','.join(seen))]))
    return out

def negcopy(L):
    out = []
    for k, (a, m, o) in enumerate(L):
        if m == 'neg' and o.startswith('dword ptr [ebp'):
            slot = o[len('dword ptr '):]
            for j in range(k + 1, min(k + 6, len(L))):
                if slot in L[j][2] and L[j][1] == 'mov':
                    out.append((a, ['%s %s' % (m, o), '%s %s' % (L[j][1], L[j][2])])); break
    return out

PAT = {'addsub': addsub, 'bytecse': bytecse, 'negcopy': negcopy}
if __name__ == '__main__':
    pat = PAT[sys.argv[1]]; nonexact = '--nonexact' in sys.argv
    n = 0
    for f, addr, name, lo, hi, exact in markers():
        if nonexact and exact:
            continue
        try:
            L = rows(lo, hi)
        except Exception:
            continue
        for a, r in pat(L):
            n += 1
            print('%x %s %-45s %s | %s' % (a, 'X' if exact else '.', name[:45], f, ' ; '.join(r)))
    print('hits', n)
