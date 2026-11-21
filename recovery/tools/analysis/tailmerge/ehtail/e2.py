"""par-6 r54: use_ability's teardown pair in a Popup-shaped probe.  P : B { pad; Sc s; } with Sc : G { FB f1, f2;
~Sc() { sclose(); } } and ~P() { close(); }: every return expands close, sclose, ~FB x2, ~G (state K), state -1,
~B.  R1 = lock-fail return, R2 = return after the action, inside `if (g)`; later returns are the host.  Prints
copies of `call ~G` (1 per kept teardown) and the state stores taken from a register."""
import sys, os, re
T = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, T + '/inline'); import ib
def hdr(pad):
    return '''extern void f0(); extern int cond(); extern void use(int); extern int lock(int *, int, int, int, int); extern void act(int, int);
extern int g, g2, g3;
struct B { B(); ~B(); void close(); int x[40]; };
struct G { G(); virtual ~G(); int x[40]; };
struct FB { FB(); ~FB(); int x[40]; };
struct Sc : G { FB f1, f2; Sc(); ~Sc() { sclose(); } void sclose(); };
struct P : B { int pad[%d]; Sc s; P(); ~P() { close(); } };
''' % pad
V = []
R = """  use(v);
  if (g) {
    for (int i = 0; i < g2; ++i) { if (cond()) continue; if (lock(&v, 2, -1, -1, 0) != 0) return; act(v, i); }
    if (lock(&v, 2, -1, -1, 0) != 0) return;
    %s
    return;
  }
  if (cond()) { use(4); return; }
  if (g3) { use(5); return; }
  use(9);"""
def mk(n, body, pad=3200):
    V.append((n, pad, 'void t_%s(int v) {\n  P p;\n%s\n}\n' % (n, body)))
mk('base', R % 'if (g2) act(v, 1); else act(v, 2);')
mk('pad1', R % 'if (g2) act(v, 1); else act(v, 2);', pad=1)
mk('one', R % 'act(v, 1);')
if __name__ == '__main__':
    for k, pad, body in V:
        asm, out = ib.compile_asm(None, os.environ.get('FE_PFX_BASE', '33'), text=hdr(pad) + body)
        if not asm: print(out); sys.exit(1)
        f = [x for x in ib.functions(asm) if ('t_%s@@' % k) in x[0] or ('t_%s@@' % k) in x[1]][0]
        L = f[2]
        n = sum(1 for l in L if l.startswith('call') and '??1G@@' in l)
        r = sum(1 for l in L if re.match(r'mov\s+DWORD PTR __\$EHRec\$\[ebp\+8\], (edi|esi|ebx)', l))
        print('%-10s ~G copies %d  reg state stores %d' % (k, n, r))
        if k in sys.argv[1:]: print('\n'.join(L))
