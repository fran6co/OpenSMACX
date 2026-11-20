"""Cross-jump blocker screen (par-6 r53): the JJ-merging menutail base (p1 `fn(.., 1, 1)`, inline toggles
and base_names) plus ONE whole-function property per variant.  psave-calls 1 = merged (ours), 4 = kept
(the image's menu_proc).  Usage: uv run python p20.py [variant ...]  (names print their listing)."""
import sys; from cj import *
import cj
from p1 import fn
cj.HDR += '''extern volatile int vg; extern double gd; extern float gf; struct D { D(); ~D(); int v; };
struct T { int v; ~T() {} }; extern void __stdcall sc(int); extern void __fastcall fc(int);
__declspec(noreturn) extern void die(int); extern int (*fp)(int); extern "C" int _setjmp(void *); extern int jb[16];
extern "C" void *_alloca(unsigned); extern "C" void exit(int);
'''
V = []
def add(k, s):
    V.append((k, s))
def case1(name, body, **kw):
    s = fn(name, 1, 1, **kw)
    return s.replace('  case 1: f0(); break;\n', '  case 1: f0(); %s break;\n' % body)
def top(name, body, **kw):
    s = fn(name, 1, 1, **kw)
    return s.replace('{\n  switch (c) {', '{\n  %s\n  switch (c) {' % body, 1)
add('base', fn('base', 1, 1))
add('vol_read', case1('vr', 'use(vg);'))
add('vol_write', case1('vw', 'vg = 1;'))
add('vol_top', top('vt', 'vg = c;'))
add('eh_top', top('et', 'D d;'))
add('eh_case', case1('ec', '{ D d; use(d.v); }'))
add('trivial_dtor_case', case1('tc', '{ T t; t.v = c; use(t.v); }'))
add('seh_case', case1('sc', '__try { use(3); } __except (1) { use(4); }'))
add('x87_live', top('xl', 'double x = gd * c;', after='  gd = x;\n'))
add('x87_case', case1('xc', 'gd = gd * c;'))
add('noreturn_case', case1('nr', 'die(1);'))
add('exit_case', case1('ex', 'exit(1);'))
add('stdcall_case', case1('st', 'sc(1);'))
add('fastcall_case', case1('fa', 'fc(1);'))
add('fnptr_case', case1('fn', 'use(fp(1));'))
add('setjmp_top', top('sj', 'if (_setjmp(jb)) return;'))
add('asm_case', case1('as', '__asm { nop }'))
add('alloca_case', case1('al', 'use(*(int *)_alloca(c));'))
add('recurse_case', case1('rc', 't_rc(c + 1);'))
run(V, show=sys.argv[1:])
