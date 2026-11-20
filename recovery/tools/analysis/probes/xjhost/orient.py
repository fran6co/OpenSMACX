#!/usr/bin/env python3
"""orient.py [name ...]: cross-jump host ORIENTATION (MATCHING_GUIDE 3.7 "Host orientation", 2026-11-20 par-5).
Three copies of the monolith pop tail `mp.pop(label, K, 0, pcx); gs |= 4;` (H1, S1, last F) ending in returns; prints
FIRST when the first copy (H1) hosts and the others jmp into it, LAST when F hosts. FE_PFX_BASE selects the prefix."""
import sys, os
H = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(H) + '/crossjump')
sys.path.insert(0, os.path.dirname(os.path.dirname(H)) + '/inline')
import ib
import run as cj
PFX = os.environ.get('FE_PFX_BASE', '21')
HDR = cj.HDR + '''extern int g2; extern int gs; extern char gstr[]; extern int ga[];
struct M { void pop(const char *l, int d, int c, const char *t); };
extern M mp;
inline void vb(const char *l, const char *t) { mp.pop(l, 5000, 0, t); }
inline void np(const char *l, const char *t) { mp.pop(l, -5000, 0, t); }
'''
TAIL = '''
 if (b || r1(c) >= 6) { if (c) np("NO", "p"); return; }
 use(5); if (c) { if (g2) vb(gstr, "p"); else np(gstr, "p"); gs |= 4; }
'''
N5 = ' if (!a) { if (b) { use(1); if (c) { vb("H1", "p"); gs |= 4; } return; } if (g2) { use(2); if (c) { np("S1", "p"); gs |= 4; } } return; }\n'
N1 = ' if (!a) { if (b) { use(1); if (c) { vb("H1", "p"); gs |= 4; } } else if (g2) { use(2); if (c) { np("S1", "p"); gs |= 4; } } return; }\n'
O = lambda k: ' if (g2 == %d || g2 == %d) return;\n' % (k, k + 1)
V = {
 'base': N5 + TAIL,                                   # LAST
 'elseif': N1 + TAIL,                                 # FIRST: H1/S1 as then/else of one if/else
 'or_ret': O(90) + N5 + TAIL,                         # FIRST: a bare `if (A || B) return;`
 'or_ret2': O(90) + O(80) + N5 + TAIL,                # LAST: two of them cancel
 'or_ret3': O(90) + O(80) + O(70) + N5 + TAIL,        # FIRST
 'or_ret_late': N5 + O(90) + TAIL,                    # FIRST: position does not matter
 'sep_ret': ' if (g2 == 90) return;\n if (g2 == 91) return;\n' + N5 + TAIL,   # LAST
 'and_ret': ' if (g2 == 1 && gs == 2) return;\n' + N5 + TAIL,                 # LAST
 'or_ret_stmt': ' if (g2 == 1 || g2 == 2) { gs = 0; return; }\n' + N5 + TAIL, # LAST
 'nested_or': ' if (c == 3) { if (g2 == 1 || g2 == 2) return; }\n' + N5 + TAIL,  # LAST
 'or_call_ret': ' if (!r1(5) || !r1(6)) return;\n' + N5 + TAIL,              # FIRST
 'ifelse_tail': N5 + ' if (b || r1(c) >= 6) { if (c) np("NO", "p"); } else { use(5); if (c) { if (g2) vb(gstr, "p"); else np(gstr, "p"); gs |= 4; } }\n',  # S1 hosts (prints LAST: H1 jmps first): F is no longer the fallthrough end
}
if __name__ == '__main__':
    sel = sys.argv[1:] or list(V)
    src = HDR + ''.join('void %s(int a, int b, int c) {\n%s\n}\n' % (n, V[n]) for n in sel)
    asm, out = ib.compile_asm(None, PFX, text=src)
    F = {f[1]: f[2] for f in ib.functions(asm)}
    for n in sel:
        L = next((l for k, l in F.items() if ('?%s@@' % n) in k), None)
        if L is None:
            print(n, 'MISSING', out[-400:] if out else ''); continue
        t = cj.short(cj.compact(L))
        o = [i for i, x in enumerate(t) if 'or al' in x]
        j = [i for i, x in enumerate(t) if 'jmp' in x]
        print('%-12s %s' % (n, ('FIRST' if o[0] < j[0] else 'LAST') if o and j else '?'))
