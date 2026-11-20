#!/usr/bin/env python3
"""dup.py [--show NAME,..] [--only SUBSTR]: when does VC6 give a nested `return v` its own epilogue copy
(`jncc JOIN; <value; pops; ret>`) and when does it branch to the final return's block (`jcc RET; jmp JOIN`)?
(MATCHING_GUIDE 3.7, "Epilogue copy or branch".)  A matrix over the inner return's context:

  shape  T4 top level `if (B) return V; J; return V;`
         T3 nested, no else `if (A) { if (B) return V; } J; return V;`
         T1 nested, else arm `if (A) { if (B) return V; } else { X; } J; return V;`
         T2 T1 with `B || C`
         T5 veh_init: `if (A) { if (B || C) return V; } else if (D) { X; return V; } J; return V;`
         T6 T1 with a statement after the inner if, T7 T1 with the if/else written inverted
  val    k constant, r register (live across calls), m address-taken local, e expression r+1
  sv     callee-saved registers in use (0..3), frame f = a 64-byte local array (ebp frame) or none

Per variant: number of `ret`, and for the inner return whether it got a COPY or a branch (BR) to the shared
final block (the inner test is `cmp [esi+4], 7` in every shape).  FE_PFX_BASE selects the prefix."""
import sys, os, re, itertools
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))) + '/inline')
import ib
PFX = os.environ.get('FE_PFX_BASE', '21')
HDR = '''struct W { int a; int b; int c; int d; int ask(int); int f(int); %s };
extern int g; void use(int); void usep(int *); void usebuf(char *);
'''
SHAPES = {
    'T4': 'if (b == 7) return V; J return V;',
    'T3': 'if (A) { if (b == 7) return V; } J return V;',
    'T1': 'if (A) { if (b == 7) return V; } else { g |= 2; } J return V;',
    'T2': 'if (A) { if ((c & 1) || b == 7) return V; } else { g |= 2; } J return V;',
    'T5': 'if (A) { if ((c & 1) || b == 7) return V; } else if (d != 7 && !ask(3)) { g |= 2; return V; } J return V;',
    'T6': 'if (A) { if (b == 7) return V; use(5); } else { g |= 2; } J return V;',
    'T7': 'if (!(A)) { g |= 2; } else { if (b == 7) return V; } J return V;',
    'T8': 'if (A) { if (b != 7) goto join; return V; } else { g |= 2; } join: J return V;',
    'T9': 'if (A) { if (b == 7) goto out; } else { g |= 2; } J out: return V;',
    'T10': 'if (A) { if ((c & 1) || b == 7) goto out; } else if (d != 7 && !ask(3)) { g |= 2; return V; } J out: return V;',
    'T11': 'if (A) { if ((c & 1) || b == 7) goto out; } else if (d != 7 && !ask(3)) { g |= 2; goto out; } J out: return V;',
    'T12': 'if (A) { if (b == 7) goto out; } J out: return V;',
    'T14': 'if (A) { if ((c & 1) || b == 7) goto out; } else { g |= 2; } J out: return V;',
    'T15': 'if (A) { if (b == 7) goto out; } else if (d != 7 && !ask(3)) { g |= 2; } J out: return V;',
    'T16': 'if (A) { if (b == 7) goto out; } else if (d != 7) { g |= 2; return V; } J out: return V;',
    'T17': 'if (A) { if (b == 7) goto out; } else { g |= 2; return V; } J out: return V;',
    'T18': 'if (A) { if (b == 7) return V; } else { g |= 2; return V; } J return V;',
    'T19': 'if (A) { if (c & 1) goto out; if (b == 7) goto out; } else { g |= 2; } J out: return V;',
    'T20': 'if (A) { if (c & 1) return V; if (b == 7) return V; } else { g |= 2; } J return V;',
    'T21': 'if (A) { if (c & 1) goto out; if (b == 7) goto out; } else if (d != 7 && !ask(3)) { g |= 2; return V; } J out: return V;',
    'T22': 'if (A) { if (c & 1) goto out; if (b == 7) goto out; } else if (d != 7 && !ask(3)) { g |= 2; goto out; } J out: return V;',
    'V_or_r_eir': 'if (A) { if ((c & 1) || b == 7) return V; } else if (d != 7 && !ask(3)) { g |= 2; return V; } join: J out: return V;',
    'V_or_r_ebr': 'if (A) { if ((c & 1) || b == 7) return V; } else { if (d != 7 && !ask(3)) { g |= 2; return V; } } join: J out: return V;',
    'V_or_r_eig': 'if (A) { if ((c & 1) || b == 7) return V; } else if (d != 7 && !ask(3)) { g |= 2; goto out; } join: J out: return V;',
    'V_or_r_ebg': 'if (A) { if ((c & 1) || b == 7) return V; } else { if (d != 7 && !ask(3)) { g |= 2; goto out; } } join: J out: return V;',
    'V_or_r_ebn': 'if (A) { if ((c & 1) || b == 7) return V; } else { if (d == 7 || ask(3)) goto join; g |= 2; return V; } join: J out: return V;',
    'V_or_r_e2': 'if (A) { if ((c & 1) || b == 7) return V; } else { if (d == 7) goto join; if (ask(3)) goto join; g |= 2; return V; } join: J out: return V;',
    'V_or_g_eir': 'if (A) { if ((c & 1) || b == 7) goto out; } else if (d != 7 && !ask(3)) { g |= 2; return V; } join: J out: return V;',
    'V_or_g_ebr': 'if (A) { if ((c & 1) || b == 7) goto out; } else { if (d != 7 && !ask(3)) { g |= 2; return V; } } join: J out: return V;',
    'V_or_g_eig': 'if (A) { if ((c & 1) || b == 7) goto out; } else if (d != 7 && !ask(3)) { g |= 2; goto out; } join: J out: return V;',
    'V_or_g_ebg': 'if (A) { if ((c & 1) || b == 7) goto out; } else { if (d != 7 && !ask(3)) { g |= 2; goto out; } } join: J out: return V;',
    'V_or_g_ebn': 'if (A) { if ((c & 1) || b == 7) goto out; } else { if (d == 7 || ask(3)) goto join; g |= 2; return V; } join: J out: return V;',
    'V_or_g_e2': 'if (A) { if ((c & 1) || b == 7) goto out; } else { if (d == 7) goto join; if (ask(3)) goto join; g |= 2; return V; } join: J out: return V;',
    'V_two_r_eir': 'if (A) { if (c & 1) return V; if (b == 7) return V; } else if (d != 7 && !ask(3)) { g |= 2; return V; } join: J out: return V;',
    'V_two_r_ebr': 'if (A) { if (c & 1) return V; if (b == 7) return V; } else { if (d != 7 && !ask(3)) { g |= 2; return V; } } join: J out: return V;',
    'V_two_r_eig': 'if (A) { if (c & 1) return V; if (b == 7) return V; } else if (d != 7 && !ask(3)) { g |= 2; goto out; } join: J out: return V;',
    'V_two_r_ebg': 'if (A) { if (c & 1) return V; if (b == 7) return V; } else { if (d != 7 && !ask(3)) { g |= 2; goto out; } } join: J out: return V;',
    'V_two_r_ebn': 'if (A) { if (c & 1) return V; if (b == 7) return V; } else { if (d == 7 || ask(3)) goto join; g |= 2; return V; } join: J out: return V;',
    'V_two_r_e2': 'if (A) { if (c & 1) return V; if (b == 7) return V; } else { if (d == 7) goto join; if (ask(3)) goto join; g |= 2; return V; } join: J out: return V;',
    'V_two_g_eir': 'if (A) { if (c & 1) goto out; if (b == 7) goto out; } else if (d != 7 && !ask(3)) { g |= 2; return V; } join: J out: return V;',
    'V_two_g_ebr': 'if (A) { if (c & 1) goto out; if (b == 7) goto out; } else { if (d != 7 && !ask(3)) { g |= 2; return V; } } join: J out: return V;',
    'V_two_g_eig': 'if (A) { if (c & 1) goto out; if (b == 7) goto out; } else if (d != 7 && !ask(3)) { g |= 2; goto out; } join: J out: return V;',
    'V_two_g_ebg': 'if (A) { if (c & 1) goto out; if (b == 7) goto out; } else { if (d != 7 && !ask(3)) { g |= 2; goto out; } } join: J out: return V;',
    'V_two_g_ebn': 'if (A) { if (c & 1) goto out; if (b == 7) goto out; } else { if (d == 7 || ask(3)) goto join; g |= 2; return V; } join: J out: return V;',
    'V_two_g_e2': 'if (A) { if (c & 1) goto out; if (b == 7) goto out; } else { if (d == 7) goto join; if (ask(3)) goto join; g |= 2; return V; } join: J out: return V;',
    'V_mix_eir': 'if (A) { if (c & 1) goto out; if (b == 7) return V; } else if (d != 7 && !ask(3)) { g |= 2; return V; } join: J out: return V;',
    'V_mix_ebr': 'if (A) { if (c & 1) goto out; if (b == 7) return V; } else { if (d != 7 && !ask(3)) { g |= 2; return V; } } join: J out: return V;',
    'V_mix_eig': 'if (A) { if (c & 1) goto out; if (b == 7) return V; } else if (d != 7 && !ask(3)) { g |= 2; goto out; } join: J out: return V;',
    'V_mix_ebg': 'if (A) { if (c & 1) goto out; if (b == 7) return V; } else { if (d != 7 && !ask(3)) { g |= 2; goto out; } } join: J out: return V;',
    'V_mix_ebn': 'if (A) { if (c & 1) goto out; if (b == 7) return V; } else { if (d == 7 || ask(3)) goto join; g |= 2; return V; } join: J out: return V;',
    'V_mix_e2': 'if (A) { if (c & 1) goto out; if (b == 7) return V; } else { if (d == 7) goto join; if (ask(3)) goto join; g |= 2; return V; } join: J out: return V;',
    'V_mix2_eir': 'if (A) { if (c & 1) return V; if (b == 7) goto out; } else if (d != 7 && !ask(3)) { g |= 2; return V; } join: J out: return V;',
    'V_mix2_ebr': 'if (A) { if (c & 1) return V; if (b == 7) goto out; } else { if (d != 7 && !ask(3)) { g |= 2; return V; } } join: J out: return V;',
    'V_mix2_eig': 'if (A) { if (c & 1) return V; if (b == 7) goto out; } else if (d != 7 && !ask(3)) { g |= 2; goto out; } join: J out: return V;',
    'V_mix2_ebg': 'if (A) { if (c & 1) return V; if (b == 7) goto out; } else { if (d != 7 && !ask(3)) { g |= 2; goto out; } } join: J out: return V;',
    'V_mix2_ebn': 'if (A) { if (c & 1) return V; if (b == 7) goto out; } else { if (d == 7 || ask(3)) goto join; g |= 2; return V; } join: J out: return V;',
    'V_mix2_e2': 'if (A) { if (c & 1) return V; if (b == 7) goto out; } else { if (d == 7) goto join; if (ask(3)) goto join; g |= 2; return V; } join: J out: return V;',
    'W_andj_g_eir': 'if (A) { if (!(c & 1) && b != 7) goto join; goto out; } else if (d != 7 && !ask(3)) { g |= 2; return V; } join: J out: return V;',
    'W_andj_g_ebr': 'if (A) { if (!(c & 1) && b != 7) goto join; goto out; } else { if (d != 7 && !ask(3)) { g |= 2; return V; } } join: J out: return V;',
    'W_andj_g_eirj': 'if (A) { if (!(c & 1) && b != 7) goto join; goto out; } else if (d != 7 && !ask(3)) { g |= 2; return V; } else { goto join; } join: J out: return V;',
    'W_andj_r_eir': 'if (A) { if (!(c & 1) && b != 7) goto join; return V; } else if (d != 7 && !ask(3)) { g |= 2; return V; } join: J out: return V;',
    'W_andj_r_ebr': 'if (A) { if (!(c & 1) && b != 7) goto join; return V; } else { if (d != 7 && !ask(3)) { g |= 2; return V; } } join: J out: return V;',
    'W_andj_r_eirj': 'if (A) { if (!(c & 1) && b != 7) goto join; return V; } else if (d != 7 && !ask(3)) { g |= 2; return V; } else { goto join; } join: J out: return V;',
    'W_or_gj_eir': 'if (A) { if ((c & 1) || b == 7) goto out; goto join; } else if (d != 7 && !ask(3)) { g |= 2; return V; } join: J out: return V;',
    'W_or_gj_ebr': 'if (A) { if ((c & 1) || b == 7) goto out; goto join; } else { if (d != 7 && !ask(3)) { g |= 2; return V; } } join: J out: return V;',
    'W_or_gj_eirj': 'if (A) { if ((c & 1) || b == 7) goto out; goto join; } else if (d != 7 && !ask(3)) { g |= 2; return V; } else { goto join; } join: J out: return V;',
    'W_or_rj_eir': 'if (A) { if ((c & 1) || b == 7) return V; goto join; } else if (d != 7 && !ask(3)) { g |= 2; return V; } join: J out: return V;',
    'W_or_rj_ebr': 'if (A) { if ((c & 1) || b == 7) return V; goto join; } else { if (d != 7 && !ask(3)) { g |= 2; return V; } } join: J out: return V;',
    'W_or_rj_eirj': 'if (A) { if ((c & 1) || b == 7) return V; goto join; } else if (d != 7 && !ask(3)) { g |= 2; return V; } else { goto join; } join: J out: return V;',
    'W_two_gj_eir': 'if (A) { if (c & 1) goto out; if (b == 7) goto out; goto join; } else if (d != 7 && !ask(3)) { g |= 2; return V; } join: J out: return V;',
    'W_two_gj_ebr': 'if (A) { if (c & 1) goto out; if (b == 7) goto out; goto join; } else { if (d != 7 && !ask(3)) { g |= 2; return V; } } join: J out: return V;',
    'W_two_gj_eirj': 'if (A) { if (c & 1) goto out; if (b == 7) goto out; goto join; } else if (d != 7 && !ask(3)) { g |= 2; return V; } else { goto join; } join: J out: return V;',
    'W_two_rj_eir': 'if (A) { if (c & 1) return V; if (b == 7) return V; goto join; } else if (d != 7 && !ask(3)) { g |= 2; return V; } join: J out: return V;',
    'W_two_rj_ebr': 'if (A) { if (c & 1) return V; if (b == 7) return V; goto join; } else { if (d != 7 && !ask(3)) { g |= 2; return V; } } join: J out: return V;',
    'W_two_rj_eirj': 'if (A) { if (c & 1) return V; if (b == 7) return V; goto join; } else if (d != 7 && !ask(3)) { g |= 2; return V; } else { goto join; } join: J out: return V;',
    'W_nest_r_eir': 'if (A) { if (!(c & 1)) { if (b != 7) goto join; } return V; } else if (d != 7 && !ask(3)) { g |= 2; return V; } join: J out: return V;',
    'W_nest_r_ebr': 'if (A) { if (!(c & 1)) { if (b != 7) goto join; } return V; } else { if (d != 7 && !ask(3)) { g |= 2; return V; } } join: J out: return V;',
    'W_nest_r_eirj': 'if (A) { if (!(c & 1)) { if (b != 7) goto join; } return V; } else if (d != 7 && !ask(3)) { g |= 2; return V; } else { goto join; } join: J out: return V;',
    'W_nest_g_eir': 'if (A) { if (!(c & 1)) { if (b != 7) goto join; } goto out; } else if (d != 7 && !ask(3)) { g |= 2; return V; } join: J out: return V;',
    'W_nest_g_ebr': 'if (A) { if (!(c & 1)) { if (b != 7) goto join; } goto out; } else { if (d != 7 && !ask(3)) { g |= 2; return V; } } join: J out: return V;',
    'W_nest_g_eirj': 'if (A) { if (!(c & 1)) { if (b != 7) goto join; } goto out; } else if (d != 7 && !ask(3)) { g |= 2; return V; } else { goto join; } join: J out: return V;',
    'W_ifel_r_eir': 'if (A) { if ((c & 1) || b == 7) { return V; } else { goto join; } } else if (d != 7 && !ask(3)) { g |= 2; return V; } join: J out: return V;',
    'W_ifel_r_ebr': 'if (A) { if ((c & 1) || b == 7) { return V; } else { goto join; } } else { if (d != 7 && !ask(3)) { g |= 2; return V; } } join: J out: return V;',
    'W_ifel_r_eirj': 'if (A) { if ((c & 1) || b == 7) { return V; } else { goto join; } } else if (d != 7 && !ask(3)) { g |= 2; return V; } else { goto join; } join: J out: return V;',
    'T13': 'if (A) { if ((c & 1) || b == 7) return V; } else if (d != 7 && !ask(3)) { g |= 2; return V; } J return V;',
}
J = 'g |= 4; c++; if (A) { c |= 1; } '


def body(shape, val, sv, frame):
    pre = []
    if frame:
        pre.append('char buf[64]; usebuf(buf);')
    regs = ['p', 'q', 's'][:sv]
    for r in regs:
        pre.append('int %s = ask(%d);' % (r, len(pre) + 1))
    # A uses the registers so they stay live across the whole body
    A = ' && '.join(['%s < %d' % (r, k + 10) for k, r in enumerate(regs)]) or 'a < 9'
    if val == 'k':
        V = '0'
    elif val == 'r':
        pre.append('int v = ask(9); use(v);')
        V = 'v'
    elif val == 'm':
        pre.append('int v = ask(9); usep(&v);')
        V = 'v'
    else:
        pre.append('int v = ask(9); use(v);')
        V = 'v + 1'
    s = SHAPES[shape].replace('J ', J).replace('V', V).replace('A', A)
    return ' '.join(pre) + ' ' + s


TAIL = re.compile(r'(mov\s+eax, |xor\s+eax, eax|or\s+eax, -1|lea\s+eax, |pop\s|mov\s+esp, ebp|ret)')


def is_tail(seq):
    """seq starts a straight run of value set + pops ending in ret"""
    for t in seq:
        if t.startswith('ret'):
            return True
        if not TAIL.match(t):
            return False
    return False


def analyse(L):
    L = [l for l in L if not l.startswith(';')]
    rets = sum(1 for l in L if l.startswith('ret'))
    lab = {l[:-1]: k for k, l in enumerate(L) if l.endswith(':')}
    for k, l in enumerate(L):
        if re.match(r'cmp\s+DWORD PTR \[\w+\+4\], 7', l) or (re.match(r'cmp\s+eax, 7', l) and 'esi+4' in L[k - 1]):
            jcc = L[k + 1].split()
            tgt = jcc[-1]
            if is_tail(L[k + 2:]):
                return rets, 'COPY', jcc[0]
            if tgt in lab and is_tail(L[lab[tgt] + 1:]):
                return rets, 'BRJ' if L[k + 2].startswith('jmp') else 'BR', jcc[0]
            nxt = L[k + 2]
            if nxt.startswith('jmp') and nxt.split()[-1] in lab and is_tail(L[lab[nxt.split()[-1]] + 1:]):
                return rets, 'JMP', jcc[0]
            return rets, 'OTHER', jcc[0]
    return rets, 'NOTEST', ''


if __name__ == '__main__':
    show = set()
    only = None
    args = sys.argv[1:]
    if '--show' in args:
        show = set(args[args.index('--show') + 1].split(','))
    if '--only' in args:
        only = args[args.index('--only') + 1]
    V = {}
    for shape, val, sv, frame in itertools.product(SHAPES, 'krme', range(4), (0, 1)):
        n = '%s_%s%d%s' % (shape, val, sv, 'f' if frame else '')
        if only and only not in n:
            continue
        V[n] = body(shape, val, sv, frame)
    decl = ' '.join('int %s(int i);' % n for n in V)
    src = HDR % decl + ''.join('int W::%s(int i) {\n%s\n}\n' % (n, V[n]) for n in V)
    asm, out = ib.compile_asm(None, PFX, text=src)
    F = {f[1]: f[2] for f in ib.functions(asm)}
    rows = {}
    for n in V:
        L = next((l for k, l in F.items() if ('?%s@W@@' % n) in k), None)
        if L is None:
            print(n, 'MISSING', (out or '')[-300:])
            continue
        rows[n] = analyse(L)
        if n in show:
            print(n, ' ; '.join(re.sub(r'\s+', ' ', l) for l in L if not l.startswith(';')))
    # one line per shape: kind per (val, sv, frame)
    for shape in SHAPES:
        cells = []
        for val in 'krme':
            for frame in (0, 1):
                ks = []
                for sv in range(4):
                    n = '%s_%s%d%s' % (shape, val, sv, 'f' if frame else '')
                    if n in rows:
                        r, kind, j = rows[n]
                        ks.append({'COPY': 'C', 'BR': 'b', 'BRJ': 'B', 'JMP': 'j', 'OTHER': 'o'}.get(kind, '?'))
                if ks:
                    cells.append('%s%s:%s' % (val, 'f' if frame else '', ''.join(ks)))
        print('%-3s %s' % (shape, '  '.join(cells)))
