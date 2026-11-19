#!/usr/bin/env python3
"""run.py [name ...]: which `return` statements get their own epilogue copy and which jump to a shared one?
(MATCHING_GUIDE 3.7, "Return-tail copies").  Each variant is one function; the report gives, per `ret` in
layout order, the value set just before it (K = `mov eax,K`/`xor eax,eax`, r = a register or other value),
how control reaches the copy (F = fallthrough from a conditional jump that skips it, J<n> = n jumps into it,
U = entered only by jumps, no fallthrough), and the total number of `ret`.  SHOW=1 prints the listing.
FE_PFX_BASE selects the compiler prefix (default 21)."""
import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))) + '/inline')
import ib
PFX = os.environ.get('FE_PFX_BASE', '21')
HDR = '''#include "probe.h"
struct P { int key; char pad[84]; };
struct W { int a; int b; int c; P ps[16]; int vis(); void redraw(); int ask(int); %s };
int pcv(const char *, const char *, int, int, int); int pc(const char *, const char *, int); void says(int, const char *, int, int); const char *nm(int); void cat(char *, const char *); void note(char *, int);
struct D { D(); ~D(); int v; };
'''
SRCH = 'int k = 0; while (k < 16 && i != ps[k].key) { ++k; } '
SW = 'switch (ask(1)) { case 0: g = 1; return 1; case 1: return 0; case 2: switch (ask(2)) { case 0: a = 5; return 0; case 1: a = 10; return 0; case 2: a = 20; return 0; case 3: a = 30; break; } return 0; } '
V = {
 # the Net::drop_player family: a guard `return 0` after a search loop, more `return 0` later
 'g_end0':      ('int', SRCH + 'if (k == 16) return 0; use(k); use(a); return 0;'),
 'g_end1':      ('int', SRCH + 'if (k == 16) return 0; use(k); use(a); return 1;'),
 'g_if0_end1':  ('int', SRCH + 'if (k == 16) return 0; use(k); if (b) { use(a); return 0; } use(2); return 1;'),
 'g_sw':        ('int', SRCH + 'if (k == 16) return 0; if (b == 0) { ' + SW + ' } use(k); return 1;'),
 'g_sw_wrap':   ('int', SRCH + 'if (k != 16) { if (b == 0) { ' + SW + ' } use(k); return 1; } return 0;'),
 'g_sw_noloop': ('int', 'int k = ga[i]; if (k == 16) return 0; if (b == 0) { ' + SW + ' } use(k); return 1;'),
 'g_sw_for':    ('int', 'int k; for (k = 0; k < 16; k++) { if (i == ps[k].key) break; } if (k == 16) return 0; if (b == 0) { ' + SW + ' } use(k); return 1;'),
 'g_sw_loopret': ('int', 'int k; for (k = 0; ; k++) { if (k >= 16) return 0; if (i == ps[k].key) break; } if (b == 0) { ' + SW + ' } use(k); return 1;'),
 'g_sw_ge':     ('int', SRCH + 'if (k >= 16) return 0; if (b == 0) { ' + SW + ' } use(k); return 1;'),
 'g_sw_else':   ('int', SRCH + 'if (k == 16) { return 0; } else { if (b == 0) { ' + SW + ' } use(k); return 1; }'),
 'g_sw_res':    ('int', SRCH + 'int r = 0; if (k == 16) return r; if (b == 0) { ' + SW + ' } use(k); return 1;'),
 # plain guards
 'two_guards':  ('int', 'if (ga[i] & 0x200) return 0; if (ga[i + 1] <= 3) return 0; return ga[i + 2] + 1;'),
 'or_guard':    ('int', 'if ((ga[i] & 0x200) || ga[i + 1] <= 3) return 0; return ga[i + 2] + 1;'),
 'guard_body0': ('int', 'if (!vis()) return 0; use(i); use(a); return 0;'),
 'guard_body1': ('int', 'if (!vis()) return 0; use(i); use(a); return 1;'),
 'guard2_body0': ('int', 'if (!vis()) return 0; use(i); if (!b) return 0; use(a); return 0;'),
 'guard2_body1': ('int', 'if (!vis()) return 0; use(i); if (!b) return 0; use(a); return 1;'),
 'guard3_body1': ('int', 'if (!vis()) return 0; use(i); if (!b) return 0; use(a); if (!c) return 0; use(3); return 1;'),
 'guard_call_ret': ('int', 'if (!vis()) return 0; use(i); if (!ask(i)) return 0; use(a); return 1;'),
 # the veh_init family: `je RET; jmp JOIN`
 'arm_ret_else': ('int', 'int k = ga[i]; if (k > 3) { use(k); if (b) return k; } else { use(k + 1); } use(a); return k + 2;'),
 'arm_ret_then': ('int', 'int k = ga[i]; if (k > 3) { use(k); if (b) return k; } use(a); return k + 2;'),
 'arm_ret_last': ('int', 'int k = ga[i]; if (k > 3) { use(k); if (b) return k; use(k + 5); } else { use(k + 1); } use(a); return k + 2;'),
 # the NetDaemon::init family: fall-off path
 'void_falloff': ('void', 'if (!vis()) return; use(i); if (b) { use(a); return; } use(2);'),
 'int_tail_call': ('int', 'if (!vis()) return 0; use(i); return ask(a);'),
 'int_tail_if':  ('int', 'if (vis()) { use(i); if (ask(a)) { use(1); return 1; } } return 0;'),
 'int_tail_if2': ('int', 'if (vis()) { use(i); if (ask(a)) { use(1); return 1; } use(2); } return 0;'),
 'int_tail_ifelse': ('int', 'if (vis()) { use(i); if (ask(a)) { use(1); return 1; } else { use(2); return 0; } } return 0;'),
 # returns in loops
 'loop_ret_in':  ('int', 'for (int k = 0; k < i; k++) { if (ga[k] == a) return 0; use(k); } use(b); return 0;'),
 'loop_ret_in1': ('int', 'for (int k = 0; k < i; k++) { if (ga[k] == a) return 1; use(k); } use(b); return 0;'),
 'loop_ret_after': ('int', 'int k; for (k = 0; k < i; k++) { if (ga[k] == a) break; use(k); } if (k == i) return 0; use(b); return 1;'),
 'loop_ret_after0': ('int', 'int k; for (k = 0; k < i; k++) { if (ga[k] == a) break; use(k); } if (k == i) return 0; use(b); return 0;'),
 # with a destructor
 'dtor_guard':  ('int', 'D d; if (!vis()) return 0; use(i); if (!b) return 0; use(a); return 1;'),
 'dtor_guard0': ('int', 'D d; if (!vis()) return 0; use(i); if (!b) return 0; use(a); return 0;'),
}

# how many `return 0` statements before the guard's own copy is folded into a shared one?
for _n in range(1, 9):
    V['g_ret%d' % _n] = ('int', SRCH + 'if (k == 16) return 0; switch (ask(1)) { ' + ' '.join('case %d: a = %d; return 0;' % (c, c * 5) for c in range(_n)) + ' } use(k); return 1;')
    V['g_brk%d' % _n] = ('int', SRCH + 'if (k == 16) return 0; switch (ask(1)) { ' + ' '.join('case %d: a = %d; break;' % (c, c * 5) for c in range(_n)) + ' default: use(k); return 1; } return 0;')
    V['g_if%d' % _n] = ('int', SRCH + 'if (k == 16) return 0; ' + ' '.join('if (ask(%d)) { a = %d; return 0; }' % (c, c * 5) for c in range(_n)) + ' use(k); return 1;')
    V['g_bare%d' % _n] = ('int', SRCH + 'if (k == 16) return 0; ' + ' '.join('if (ask(%d)) return 0;' % c for c in range(_n)) + ' use(k); return 1;')

# the drop_player shape twice: two nested answer switches, `return 0` or `break` in the inner cases
def _sw(inner):
    return ('switch (ask(1)) { case 0: g = 1; return 1; case 1: return 0; case 2: switch (ask(2)) { case 0: a = 5; %s case 1: a = 10; %s case 2: a = 20; %s case 3: a = 30; break; } return 0; } ' % (inner, inner, inner))
for _k, _inner in (('ret', 'return 0;'), ('brk', 'break;')):
    V['g_sw2_' + _k] = ('int', SRCH + 'if (k == 16) return 0; if (b == 0) { if (c) { ' + _sw(_inner) + ' return 0; } ' + _sw(_inner) + ' } use(k); return 1;')
    V['g_sw1_' + _k] = ('int', SRCH + 'if (k == 16) return 0; if (b == 0) { ' + _sw(_inner) + ' } use(k); return 1;')
    V['g_sw3_' + _k] = ('int', SRCH + 'if (k == 16) return 0; if (b == 0) { if (c) { ' + _sw(_inner) + ' return 0; } if (c == 7) { ' + _sw(_inner) + ' return 0; } ' + _sw(_inner) + ' } use(k); return 1;')

# closer to Net::drop_player 0x00632350: cdecl prompts with pending stack cleanup, a 256-byte frame after
def _swc(inner, first):
    return ('switch (pcv("jackal", %s, 1, 0, 0)) { %s case 1: return 0; case 2: switch (pc("jackal", "TOL", 0)) { case 0: a = 5000; %s case 1: a = 10000; %s case 2: a = 20000; %s case 3: a = 30000; break; } return 0; } ' % (first[0], first[1], inner, inner, inner))
_TAIL = ' char name[256]; name[0] = 0; for (k = 0; k < 16; ++k) { if (ps[k].key == i) { cat(name, ps[k].pad); break; } } --c; note(name, i); return 1;'
for _k, _inner in (('ret', 'return 0;'), ('brk', 'break;')):
    V['dp_' + _k] = ('int', SRCH + 'if (k == 16) return 0; if (b == 0) { if (c != a && c != ps[1].key) { says(0, nm(ps[0].key), -1, -1); says(1, nm(ps[1].key), -1, -1); '
                     + _swc(_inner, ('"HOSTS"', 'case 0: g = 1; return 1;')) + ' return 0; } says(0, nm(i), -1, -1); ' + _swc(_inner, ('c == a ? "CLIENT" : "HOST"', '')) + ' }' + _TAIL)
    V['dp1_' + _k] = ('int', SRCH + 'if (k == 16) return 0; if (b == 0) { says(0, nm(i), -1, -1); ' + _swc(_inner, ('c == a ? "CLIENT" : "HOST"', '')) + ' }' + _TAIL)


def analyse(L):
    L = [l for l in L if not l.startswith(';')]
    labels = {l[:-1]: i for i, l in enumerate(L) if l.endswith(':')}
    jumps = {}
    for i, l in enumerate(L):
        m = re.match(r'(j\w+)\s+(?:SHORT\s+)?(\$\w+)', l)
        if m:
            jumps.setdefault(m.group(2), []).append((i, m.group(1)))
    out = []
    for i, l in enumerate(L):
        if not l.startswith('ret'):
            continue
        # walk back to the start of this tail: pops, add esp, mov esp, value set, labels
        j = i - 1
        val = 'r'
        nj = 0
        while j >= 0:
            t = L[j]
            if t.endswith(':'):
                nj += len(jumps.get(t[:-1], []))
            elif re.match(r'(pop|leave|add\s+esp|mov\s+esp)', t) or re.match(r'mov\s+ecx, DWORD PTR __\$EH|mov\s+DWORD PTR fs', t):
                pass
            elif re.match(r'xor\s+eax, eax', t):
                val = '0'
            elif re.match(r'mov\s+eax, (-?\d+)$', t):
                val = re.match(r'mov\s+eax, (-?\d+)$', t).group(1)
            elif re.match(r'or\s+eax, -1', t):
                val = '-1'
            else:
                break
            j -= 1
        prev = L[j] if j >= 0 else ''
        ft = 'F' if re.match(r'j(?!mp)\w+\s', prev) else ('U' if re.match(r'(jmp|ret)', prev) else 'S')
        out.append('%s:%s%s' % (val, ft, ('J%d' % nj) if nj else ''))
    return out


if __name__ == '__main__':
    sel = sys.argv[1:] or list(V)
    decl = ' '.join('%s %s(int i);' % (V[n][0], n) for n in V)
    src = HDR % decl + ''.join('%s W::%s(int i) {\n%s\n}\n' % (V[n][0], n, V[n][1]) for n in sel)
    asm, out = ib.compile_asm(None, PFX, text=src)
    F = {f[1]: f[2] for f in ib.functions(asm)}
    for n in sel:
        L = next((l for k, l in F.items() if ('?%s@W@@' % n) in k), None)
        if L is None:
            print(n, 'MISSING', out[-300:] if out else '')
            continue
        r = analyse(L)
        print('%-16s ret=%d  %s' % (n, len(r), ' '.join(r)))
        if os.environ.get('SHOW'):
            print('   ' + ' ; '.join(re.sub(r'\s+', ' ', l) for l in L))
