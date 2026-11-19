"""roles.py <unit> <src> <sym> <0xaddr> [pfx] -> rows: var line our_off img_addr img_slot our_slot flag
Joins the /FAsc listing (source line + variable names per instruction offset) with the fe diff alignment."""
import sys, os, re, subprocess, shutil
sys.path.insert(0, 'recovery/tools')
import fe
unit, src, func, addr = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4]
pfx = sys.argv[5] if len(sys.argv) > 5 else '2'
cmd = fe.compile_cmd(unit)
out = []
asm = fe.SCR + '/ro_%d.cod' % os.getpid()
obj = fe.SCR + '/ro_%d.obj' % os.getpid()
for t in cmd:
    if t.startswith('/Fo'):
        t = '/Fo' + obj
    elif t.startswith('/Fd') or t.startswith('--repro-root') or t == '/W4':
        continue
    elif t == fe.R + '/' + unit:
        t = os.path.abspath(src)
    out.append(t)
out.insert(-1, '/FAsc')
out.insert(-1, '/Fa' + asm)
env = dict(os.environ, VC6_PREFIX=f'{fe.MAIN}/.opensmacx/wine/worker-{pfx}')
subprocess.run(out, cwd=fe.SCR, env=env, capture_output=True, text=True)
t = open(asm, errors='replace').read()
i = t.find(func + ' PROC')
e = t.find(func + ' ENDP', i)
line = 0
byoff = {}
pend = None
for l in t[i:e].split('\n'):
    l = l.rstrip('\r')
    m = re.match(r'; (\d+)\s*:', l)
    if m:
        line = int(m.group(1))
        continue
    ins = None
    m = re.match(r'  ([0-9a-f]{5})\t([0-9a-f ]+)(?:\t+ ?(.*))?$', l)
    if m:
        off = int(m.group(1), 16)
        if m.group(3):
            ins = m.group(3)
        else:
            pend = off
            continue
    else:
        m = re.match(r'\t[0-9a-f ]+\t+ ?(\S.*)$', l)
        if m and pend is not None:
            off = pend
            ins = m.group(1)
            pend = None
    if ins:
        v = re.findall(r'_(\w+)\$\d*\[ebp', ins) + ['T' + x for x in re.findall(r'(-?\d+)\+\[ebp', ins)]
        byoff[off] = (line, v, ins)
if os.environ.get('ROLES_COD'):
    shutil.copy(asm, os.environ['ROLES_COD'])
os.remove(asm)
os.path.exists(obj) and os.remove(obj)
p = subprocess.run([sys.executable, 'recovery/tools/fe.py', unit, addr, '--pfx', pfx, '--src', src, '--all'],
                   capture_output=True, text=True)
if os.environ.get('ROLES_DIFF'):
    open(os.environ['ROLES_DIFF'], 'w').write(p.stdout)
for l in p.stdout.split('\n'):
    if '|' not in l:
        continue
    a, b = l.split('|', 1)
    mo = re.match(r'\s*([0-9a-f]+)\s', b)
    if not mo:
        continue
    off = int(mo.group(1), 16)
    if off not in byoff:
        continue
    ln, vs, ins = byoff[off]
    ia = a[3:9].strip()
    sa = re.findall(r'ebp ([-+]) (0x[0-9a-f]+|\d+)\]', a)
    sb = re.findall(r'ebp ([-+]) (0x[0-9a-f]+|\d+)\]', b)
    islot = ','.join(s + v for s, v in sa) if sa else '?'
    oslot = ','.join(s + v for s, v in sb) if sb else '?'
    for v in vs:
        print(v, ln, '%x' % off, ia or '-', islot, oslot, '=' if a[:2] == '  ' else 'D')
