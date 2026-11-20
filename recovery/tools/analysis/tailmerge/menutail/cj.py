"""menu_proc toggle-tail probe: count copies of `call psave` (one per toggle = unmerged, as in the image)."""
import sys, os, re
W=os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, W+'/inline'); import ib
HDR='''extern void chk(int, int); extern void unchk(int, int); extern void psave(int); extern void dmap(int);
extern int gp, gmp, ip, imp, gx; extern void use(int); extern void f0(); extern int cond();
struct Mn { void check(int, int); void uncheck(int, int); };
'''
def toggle(i, var, ivar, bit, menu, tail='psave(0); dmap(1);'):
    return ('{ int f = %s ^ 0x%x; %s = f; if (f & 0x%x) m.check(2, 0x%x); else m.uncheck(2, 0x%x); %s = %s; %s }'
            % (var, bit, var, bit, menu, menu, ivar, var, tail))
BN='{ int f = gmp; int n; if (f & 0x1000) n = f & 0xFFFFCFFF; else if (f & 0x2000) n = f | 0x1000; else n = f | 0x2000; gmp = n; imp = n; psave(0); dmap(1); }'
DEST='{ gx ^= 0x40000; dmap(1); }'
def run(V, show=()):
    src = HDR + ''.join(v for _, v in V)
    asm, out = ib.compile_asm(None, os.environ.get('FE_PFX_BASE','33'), text=src)
    for k, v in V:
        name = re.findall(r'(t_\w+)\(', v)[-1]
        f = [x for x in ib.functions(asm) if re.search(r'\b'+name+r'\b', x[0])][0]
        L = f[2]
        n = sum(1 for l in L if l.startswith('call') and '?psave@@' in l)
        st = sum(1 for l in L if l.startswith('mov') and ('?imp@@' in l.split(',')[0] or '?ip@@' in l.split(',')[0]))
        print('%-28s psave-calls %d imp/ip-stores %d' % (k, n, st))
        if k in show: print('\n'.join(L))
