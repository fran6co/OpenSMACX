#!/usr/bin/env python3
"""run.py: vector-constructor-iterator probes (MATCHING_GUIDE 2.6 "Arrays of objects").
Each variant is a whole translation unit; we report, per probe function t_*, which array-construction
form our VC6 chose: H (call ??_H vector ctor iterator), L (??_L eh vector ctor iterator), loop (the
ctor called in an expanded loop), inl (ctor body expanded inline, no call), with the callee list.
Usage: uv run python run.py [variant ...]   (prefix from FE_PFX_BASE, default 21)"""
import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))) + '/inline')
import ib

PFX = os.environ.get('FE_PFX_BASE', '21')
HDR = '#include "probe.h"\n'


def classify(lines):
    calls = [re.match(r'call\s+(\S+)', l).group(1) for l in lines if re.match(r'call\s+\S+', l)]
    tags = []
    if any(c.startswith('??_H') for c in calls): tags.append('H')
    if any(c.startswith('??_L') for c in calls): tags.append('L')
    if any(c.startswith('??_I') for c in calls): tags.append('I')
    if any(c.startswith('??_M') for c in calls): tags.append('M')
    if any(c.startswith('??0') for c in calls): tags.append('ctor-call')
    if not tags: tags.append('none')
    return '+'.join(tags), calls


def run(variants, show=False):
    out = []
    for key, text in variants:
        asm, _ = ib.compile_asm(None, PFX, text=HDR + text)
        for name, sym, lines in ib.functions(asm):
            if not re.search(r'\bt_\w+', name):
                continue
            tag, calls = classify(lines)
            out.append((key, name, tag, len(lines)))
            print('%-28s %-22s %-14s n=%d %s' % (key, name.split('(')[0], tag, len(lines),
                                                 ' '.join(c for c in calls if not c.startswith('?use'))))
            if show:
                print('\n'.join('    ' + l for l in lines))
    return out


# element type spellings
OOL_AFTER = ('struct R { int a[95]; R(); };\n', 'R::R() { a[0] = 1; a[94] = 2; }\n')       # defined after the user
OOL_BEFORE = ('struct R { int a[95]; R(); };\nR::R() { a[0] = 1; a[94] = 2; }\n', '')       # defined before
EXTERN = ('struct R { int a[95]; R(); };\n', '')                                             # not in this TU
INLINE = ('struct R { int a[95]; R() { a[0] = 1; a[94] = 2; } };\n', '')                    # in-class inline
MEMBER_CTOR_GEN = ('struct S { S(); int s; };\nstruct R { int a[94]; S m; };\n', '')         # compiler-generated
WITH_DTOR = ('struct R { int a[95]; R(); ~R(); };\n', '')
WITH_INL_DTOR = ('struct R { int a[95]; R(); ~R() { use(a[0]); } };\n', '')


def users(n=7, where='local'):
    if where == 'local':
        return 'void t_local() { R r[%d]; sink(r); }\n' % n
    if where == 'member':
        return ('struct P { int h[4]; R rows[%d]; };\nvoid t_member() { P p; sink(&p); }\n' % n)
    if where == 'member_ctor':
        return ('struct Q { int h[4]; R rows[%d]; Q(); };\nQ::Q() { h[0] = 0; }\nvoid t_mctor() { Q p; sink(&p); }\n' % n)
    if where == 'new':
        return 'void t_new() { R *r = new R[%d]; sink(r); }\n' % n
    if where == 'global':
        return 'R gr[%d];\nvoid t_global() { sink(gr); }\n' % n


PRE = 'extern void sink(void *);\nvoid *operator new(unsigned int);\n'


def deck(elem, n=7, wheres=('local', 'member', 'member_ctor')):
    head, tail = elem
    return PRE + head + ''.join(users(n, w) for w in wheres) + tail


if __name__ == '__main__':
    V = []
    for k, e in (('ool_after', OOL_AFTER), ('ool_before', OOL_BEFORE), ('extern', EXTERN), ('inline', INLINE),
                 ('generated', MEMBER_CTOR_GEN), ('dtor', WITH_DTOR), ('inl_dtor', WITH_INL_DTOR)):
        for n in (1, 2, 3, 7, 16):
            V.append(('%s n=%d' % (k, n), deck(e, n)))
    sel = sys.argv[1:]
    if sel:
        V = [v for v in V if any(s in v[0] for s in sel)]
    run(V, show='--show' in sys.argv)
