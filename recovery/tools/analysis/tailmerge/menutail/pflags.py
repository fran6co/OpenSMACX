"""Cross-jump vs compiler switches (par-6 r53): p21 base (merged) and labels (kept) under extra flags."""
import sys; import cj
import ib
orig = ib.compile_asm
X = tuple(a for a in sys.argv[1:])
ib.compile_asm = lambda src, pfx, oy=False, extra=(), text=None: orig(src, pfx, oy, extra=X, text=text)
sys.argv = ['p21']
src = open(__file__.replace('pflags.py', 'p21.py')).read().replace('run(V, show=sys.argv[1:])', 'V = [v for v in V if v[0] in ("base", "labels", "ifchain")]; run(V)')
exec(compile(src, 'p21.py', 'exec'))
