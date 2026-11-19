# (h) a cap on cross-jumps per function: N independent sibling pairs `if (x) { use(a); f2(k,1); } else if (y) { use(b); f2(k,2); }`
# each followed by a distinct statement; count the f2 copies (2N = none merged, N = all merged).
import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + '/inline')
import ib
HDR = '#include "probe.h"\nextern void f2(int, int);\nstruct D { D(); ~D(); int v; };\n'
def fn(name, n, eh=False, pre=0):
    s = 'void %s(int a) {\n' % name + ('  D d;\n' if eh else '')
    for i in range(pre):
        s += '  if (g%d == %d) use(%d);\n' % (2 + i % 7, i, 5000 + i)
    for i in range(n):
        s += '  if (ga[%d] == 3) { use(%d); f2(%d, 1); } else if (ga[%d] == 4) { use(%d); f2(%d, 2); }\n  g = %d;\n' % (i, i, i, i + 1, i + 1000, i, i)
    return s + '}\n'
V = []
for n in [int(x) for x in (sys.argv[1] if len(sys.argv) > 1 else '5,20,50,100,200,400').split(',')]:
    V.append(('n%d' % n, n, fn('t_n%d' % n, n)))
    V.append(('n%d_eh' % n, n, fn('t_n%deh' % n, n, eh=True)))
src = HDR + ''.join(v for _, _, v in V)
asm, out = ib.compile_asm(None, os.environ.get('FE_PFX_BASE', '21'), text=src)
for k, n, v in V:
    name = re.findall(r'void (t_\w+)\(', v)[0]
    f = [x for x in ib.functions(asm) if re.search(r'\b' + name + r'\b', x[0])][0]
    c = sum(1 for l in f[2] if l.startswith('call') and '?f2@@' in l)
    print('%-10s pairs=%d f2-copies=%d merged=%d' % (k, n, c, 2 * n - c))
