# Probe family for MATCHING_GUIDE 3.3 "RECT packs with a later address-taken array".
# A non-address-taken RECT (strip) shares its slot with a later char[16] whose
# address is taken, when their live ranges do not overlap. Only a common loop
# around both (whole_loop) or an out-of-line call taking the RECT's address
# (late_taker) keeps them apart. Prints each variant's frame and slot map;
# `run.py NAME` also prints that variant's listing.
import sys, os, re
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'inline'))
import ib
PFX = os.environ.get('FE_PFX_BASE', '27')
HDR = '''#include "probe.h"
struct RECT { long left, top, right, bottom; };
static inline void inset_rect(RECT *dst, const RECT *src, int h, int v) {
  dst->left = src->left + h; dst->top = src->top + v; dst->right = src->right - h; dst->bottom = src->bottom - v; }
extern void takec(char *); extern void taker(RECT *);
struct W { RECT a, b, c, d; RECT out; void f(int t); };
'''
BODY = {
 'base': '''  RECT rect; RECT strip; char number[16];
  inset_rect(&rect, &a, 12, 12);
  strip = rect; strip.right -= 3; strip.bottom -= 3; strip.top = strip.bottom - 20;
  b.left = strip.left + 3; b.top = strip.top; b.right = strip.right; b.bottom = strip.bottom;
  rect.bottom = strip.top - 7; c = rect;
  call(); call();
  takec(number); use(number[0]); call(); takec(number);
''',
 'ptr': '''  RECT rect; RECT strip; char number[16];
  inset_rect(&rect, &a, 12, 12);
  RECT *s = &strip;
  *s = rect; s->right -= 3; s->bottom -= 3; s->top = s->bottom - 20;
  b.left = s->left + 3; b.top = s->top; b.right = s->right; b.bottom = s->bottom;
  rect.bottom = s->top - 7; c = rect;
  call(); call();
  takec(number); use(number[0]); call(); takec(number);
''',
 'inset_strip': '''  RECT rect; RECT strip; char number[16];
  inset_rect(&rect, &a, 12, 12);
  inset_rect(&strip, &rect, 0, 0); strip.right -= 3; strip.bottom -= 3; strip.top = strip.bottom - 20;
  b.left = strip.left + 3; b.top = strip.top; b.right = strip.right; b.bottom = strip.bottom;
  rect.bottom = strip.top - 7; c = rect;
  call(); call();
  takec(number); use(number[0]); call(); takec(number);
''',
 'copyout': '''  RECT rect; RECT strip; char number[16];
  inset_rect(&rect, &a, 12, 12);
  strip = rect; strip.right -= 3; strip.bottom -= 3; strip.top = strip.bottom - 20;
  b = strip; b.left = strip.left + 3;
  rect.bottom = strip.top - 7; c = rect;
  call(); call();
  takec(number); use(number[0]); call(); takec(number);
''',
 'late_taker': '''  RECT rect; RECT strip; char number[16];
  inset_rect(&rect, &a, 12, 12);
  strip = rect; strip.right -= 3; strip.bottom -= 3; strip.top = strip.bottom - 20;
  b.left = strip.left + 3; b.top = strip.top; b.right = strip.right; b.bottom = strip.bottom;
  rect.bottom = strip.top - 7; c = rect; taker(&strip);
  call(); call();
  takec(number); use(number[0]); call(); takec(number);
''',
}
B0 = BODY['base']
BODY.update({
 'num_block': B0.replace(' char number[16];','').replace('  takec(number);', '  { char number[16]; takec(number);',1).replace('takec(number);\n','takec(number); }\n'),
 'strip_block': B0.replace(' RECT strip;','').replace('  strip = rect;','  { RECT strip; strip = rect;').replace('c = rect;\n','c = rect; }\n'),
 'num_use_in_loop': B0.replace('  takec(number); use(number[0]); call(); takec(number);','  for (int i = 0; i < g; ++i) { takec(number); use(number[0]); } call(); takec(number);'),
 'whole_loop': B0.replace('  inset_rect(&rect, &a, 12, 12);','  while (cond()) { inset_rect(&rect, &a, 12, 12);').replace('takec(number);\n','takec(number); }\n'),
})

def slots(asm, name):
    i = asm.find(name); p = asm.find(' PROC NEAR', i); j = asm.rfind('_TEXT\tSEGMENT', 0, p)
    out = {}
    for m in re.finditer(r'^_(\w+)\$(\d*) = (-?\d+)', asm[j:p], re.M):
        out.setdefault(int(m.group(3)), []).append(m.group(1))
    return out
show = sys.argv[1] if len(sys.argv) > 1 else None
for k, b in BODY.items():
    src = HDR + 'void W::f(int t) {\n' + b + '}\n'
    asm, _ = ib.compile_asm(None, PFX, text=src)
    fr = re.search(r'sub\s+esp, (\d+)', asm[asm.find('?f@W'):])
    print('%-12s frame=%s %s' % (k, fr.group(1) if fr else '?', ' '.join('%s:%d' % ('/'.join(v), o) for o, v in sorted(slots(asm, '?f@W').items()))))
    if show == k: print(asm[asm.find('?f@W@@QAEXH@Z PROC'):][:2500])
