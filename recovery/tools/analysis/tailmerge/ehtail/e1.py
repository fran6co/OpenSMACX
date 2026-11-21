"""par-6 r54: use_ability's 14-byte teardown pair (0x4D8420/0x4D863C: `lea ecx,[a]; mov byte [ebp-4],K; call ~M;
mov [ebp-4],edi(-1); jmp ~Base`).  P : B with members a, b (non-inline dtors) and an inline ~P body, so every
return expands its own teardown with its own EH states.  Prints the copies of `call ~Ma` (a's dtor) left and
whether the last state store before each jump is edi or an immediate."""
import sys, os, re
T = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, T + '/inline'); import ib
HDR = '''extern void f0(); extern int cond(); extern void use(int); extern int lock(int *, int, int, int, int); extern void act(int, int);
extern int g, g2;
struct B { B(); ~B(); int x[40]; };
struct Ma { Ma(); ~Ma(); int x[400]; };
struct Mb { Mb(); ~Mb(); int x[400]; };
struct P : B { Ma a; Mb b; P(); ~P() { f0(); } };
'''
V = []
def mk(n, body):
    V.append((n, 'void t_%s(int v) {\n  P p;\n%s\n}\n' % (n, body)))
# R1 = lock fail return, R2 = return after the action, inside `if (g)`; the final return (host) has no -1 live
R = """  use(v);
  if (g) {
    for (int i = 0; i < g2; ++i) { if (cond()) continue; if (lock(&v, 2, -1, -1, 0) != 0) return; act(v, i); }
    if (lock(&v, 2, -1, -1, 0) != 0) return;
    %s
    return;
  }
  if (cond()) { use(4); return; }
  use(9);"""
mk('base', R % 'if (g2) act(v, 1); else act(v, 2);')
mk('one', R % 'act(v, 1);')
mk('noloop', R.replace('for (int i = 0; i < g2; ++i) { if (cond()) continue; if (lock(&v, 2, -1, -1, 0) != 0) return; act(v, i); }', '') % 'act(v, 1);')
mk('imm7', R.replace('-1, -1', '7, 7') % 'act(v, 1);')
mk('three', R % 'if (cond()) return; act(v, 1);')
mk('m1', R % 'act(v, -1);')
if __name__ == '__main__':
    src = HDR + ''.join(v for _, v in V)
    asm, out = ib.compile_asm(None, os.environ.get('FE_PFX_BASE', '33'), text=src)
    if not asm: print(out); sys.exit(1)
    for k, _ in V:
        f = [x for x in ib.functions(asm) if ('t_%s@@' % k) in x[0] or ('t_%s@@' % k) in x[1]][0]
        L = f[2]
        n = sum(1 for l in L if l.startswith('call') and '??1Ma@@' in l)
        edi = sum(1 for l in L if re.match(r'mov\s+DWORD PTR __\$EHRec\$\[ebp\+8\], (edi|esi|ebx)', l))
        print('%-10s ~Ma copies %d  reg state stores %d' % (k, n, edi))
        if k in sys.argv[1:]: print('\n'.join(L))
