"""pick.py: which dead slot does a spilled loop counter take when both an inlined callee's address-taken
locals (offset/mask) and an earlier spilled caller counter are free?  (draw_intel: the image packs the
`faction` counter with the earlier loop counters' slot; ours packs it with has_fac's `mask`.)"""
import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import run as R
PRESS = '{ int a = ga[%s]; int b = ga[%s + 8]; int c = ga[%s + 16]; int d = ga[%s + 24]; use(a); use(b); use(c); use(d); use(a + b + c + d + %s); }'
def loop(v):
    return '  for (int %s = 1; %s < 8; ++%s) %s\n' % (v, v, v, PRESS % ((v,) * 5))
CALL = '''  int hq = -1;
  for (int b = 0; b < g; ++b) { if (Bs[b].owner == s && hf(5, b)) { hq = b; break; } }
  use(hq);
'''
V = {
 'early_then_call_then_fac': loop('early') + CALL + loop('faction'),
 'call_then_early_then_fac': CALL + loop('early') + loop('faction'),
 'call_then_fac': CALL + loop('faction'),
 'early_call_fac_late': loop('early') + CALL + loop('faction') + loop('late'),
}
for callee in ('addr', 'addr_guard'):
    for k, body in V.items():
        src = R.HDR + R.CALLEE[callee] + 'void t_f(int s) {\n' + body + '}\n'
        asm, _ = R.ib.compile_asm(None, R.PFX, text=src)
        sl = R.slots(asm, 't_f')
        print('%-11s %-26s %s' % (callee, k, ' '.join('%s:%d' % ('/'.join(re.sub(r'\$\d+', '', x) for x in v), kk) for kk, v in sorted(sl.items()))))
