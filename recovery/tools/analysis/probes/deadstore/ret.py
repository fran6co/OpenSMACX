#!/usr/bin/env python3
"""ret.py: a dead store of the result into a parameter's home just before `return` (Buffer::text_width
0x005DC9F6: `mov [ebp+8],ecx; mov eax,ecx; ... ret`).  Each variant accumulates a width across a loop
under register pressure and returns it; prints the rows before `ret` and where the accumulator lives.
Usage: uv run python ret.py [name ...]   (FE_PFX_BASE, default 21)"""
import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))) + '/inline')
import ib
PFX = os.environ.get('FE_PFX_BASE', '21')
HDR = '''#include "probe.h"
int wd(char *, int);
char *scan(char *, int *);
void scanp(char **, int *);
struct B { int slot; int on; int pend; int f[4]; int tw(char *text, int len); int tw2(char *text, int len); int tw3(char *text, int len); int tw4(char *text, int len); int tw5(char *text, int len); int tw6(char *text, int len); int tw7(char *text, int len); int tw8(char *text, int len); int tw9(char *text, int len); int twa(char *text, int len); int xa1(char *text, int len); int xa2(char *text, int len); int xa3(char *text, int len); int pa1(char *text, int len); int pa2(int text, int len); int ir1(char *text, int len); int ir2(char *text, int len); int ir3(char *text, int len); int eh1(char *text, int len); int st1(char *text, int len); };
struct D { D(); ~D(); };
extern int GW;
inline void addw(int &w, char *s, int n) { w += wd(s, n); }
inline void addp(int *w, char *s, int n) { *w += wd(s, n); }
inline int addr_(int &w, char *s, int n) { return w += wd(s, n); }
'''
LOOP = '''  for (;;) {
    char *cur = scan(seg, &len);
    if (len == 0) break;
    W += f[slot] ? wd(seg, cur - seg) : wd(seg, cur - seg + 1);
    if (pend) { slot = *cur; pend = 0; }
    seg = cur + 1; len--;
  }
  W += wd(seg, len);
  on = 0;
  return W;'''
V = {
 'eh1': 'D d; char *seg = text; int width = 0; if (!on) return wd(seg, len);\n' + LOOP.replace('W', 'width'),
 'st1': 'char *seg = text; static int width; width = 0; if (!on) return wd(seg, len);\n' + LOOP.replace('W', 'width'),

 'ir1': 'char *seg = text; int width = 0; if (!on) return wd(seg, len);\n' + LOOP.replace('  W += wd(seg, len);', '  addw(W, seg, len);').replace('W', 'width'),
 'ir2': 'char *seg = text; int width = 0; if (!on) return wd(seg, len);\n' + LOOP.replace('  W += wd(seg, len);', '  addp(&W, seg, len);').replace('W', 'width'),
 'ir3': 'char *seg = text; int width = 0; if (!on) return wd(seg, len);\n' + LOOP.replace('  W += wd(seg, len);\n  on = 0;\n  return W;', '  on = 0;\n  return addr_(W, seg, len);').replace('W', 'width'),

 'pa1': 'char *seg = text; text = 0; if (!on) return wd(seg, len);\n' + LOOP.replace('W += ', 'text += ').replace('return W;', 'return (int)text;').replace('W', 'text'),

 'xa1': 'char *seg = text; int width = 0; if (!on) return wd(seg, len); __asm {\n mov eax, len\n }\n' + LOOP.replace('W', 'width'),
 'xa2': 'char *seg = text; int width = 0; if (!on) return wd(seg, len); __asm {\n mov eax, width\n }\n' + LOOP.replace('W', 'width'),
 'xa3': 'char *seg = text; int width = 0; if (!on) return wd(seg, len);\n' + LOOP.replace('W', 'width').replace('char *cur = scan(seg, &len);', 'char *cur;\n __asm {\n mov eax, len\n mov cur, eax\n }'),

 'tw8': 'char *seg = text; int width = 0; if (!on) return wd(seg, len); if (len < -100) scan(seg, &width);\n' + LOOP.replace('  W += wd(seg, len);\n', '').replace('return W;', 'return W += wd(seg, len);').replace('W', 'width'),
 'tw9': 'char *seg = text; int &width = *(int *)&text; width = 0; if (!on) return wd(seg, len);\n' + LOOP.replace('  W += wd(seg, len);\n', '').replace('return W;', 'return W += wd(seg, len);').replace('W', 'width'),
 'twa': 'char *seg = text; int width = 0; if (!on) return wd(seg, len);\n' + LOOP.replace('  W += wd(seg, len);\n  on = 0;\n', '').replace('return W;', 'on = 0; return W += wd(seg, len);').replace('W', 'width'),

 'tw': 'char *seg = text; int width = 0; if (!on) return wd(seg, len);\n' + LOOP.replace('W', 'width'),
 'tw2': 'char *seg = text; int width = 0; if (!on) return wd(seg, len); (void)&width;\n' + LOOP.replace('W', 'width'),
 'tw3': 'char *seg = text; int &width = *(int *)&text; width = 0; if (!on) return wd(seg, len);\n' + LOOP.replace('W', 'width'),
 'tw4': 'char *seg = text; int width = 0; if (!on) return wd(seg, len); if (len < -100) scan(seg, &width);\n' + LOOP.replace('W', 'width'),
 'tw5': 'char *seg = text; int width; width = 0; if (!on) return wd(seg, len); int r;\n' + LOOP.replace('W', 'width').replace('return width;', 'r = width; return r;'),
 'tw6': 'char *seg = text; int width = 0; if (!on) return wd(seg, len);\n' + LOOP.replace('W', 'width').replace('  W += wd(seg, len);', '').replace('W', 'width').replace('return width;', 'return width += wd(seg, len);'),
 'tw7': 'char *seg = text; int width = 0; if (!on) return wd(seg, len);\n' + LOOP.replace('W', 'width').replace('  on = 0;\n  return width;', '  on = 0;\n  int result = width;\n  return result;'),
}
if __name__ == '__main__':
    sel = sys.argv[1:] or list(V)
    src = HDR + ''.join('int B::%s(char *text, int len) {\n%s\n}\n' % (n, V[n]) for n in sel)
    asm, _ = ib.compile_asm(None, PFX, text=src)
    F = {f[0].split('(')[0].strip(): f[2] for f in ib.functions(asm)}
    for n in sel:
        L = [l for k, l in F.items() if k.startswith(n + '@B') or ('?%s@B@@' % n) in k or k.split('(')[0].endswith('::' + n)]
        if not L: print(n, list(F)[:3]); continue
        L = L[0]
        i = max(k for k, l in enumerate(L) if 'wd@' in l)
        print('%-5s | %s' % (n, ' ; '.join(L[i:i + 12])))
