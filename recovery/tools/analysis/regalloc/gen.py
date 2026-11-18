#!/usr/bin/env python3
"""gen.py: writes the generated probe families (g_*.cpp) next to this file.
Spec mini-language (one string per probe): space-separated tokens
  Da      int a = def_a();          Sa   a = def_a(); (redefinition; declare with Na first)
  Na      int a;                    a3   three use_a(a) calls        a    one use
  {  }    while (cond()) { }        [ ]  if (cond()) { }             [ | ]  if/else
  do{ }w  do { } while (cond());
  F{ }    for (int n = 0; n < 10; n++) { }      C   call();          Ga  sink_a = a;
"""
import os, re
HERE = os.path.dirname(os.path.abspath(__file__))


def body(spec):
    o = []
    for t in spec.split():
        m = re.fullmatch(r'([a-z])(\d*)', t)
        if m:
            o.append(' '.join('U(%s);' % m.group(1) for _ in range(int(m.group(2) or 1))))
        elif re.fullmatch(r'D[a-z]', t):
            o.append('D(%s);' % t[1])
        elif re.fullmatch(r'S[a-z]', t):
            o.append('S(%s);' % t[1])
        elif re.fullmatch(r'N[a-z]', t):
            o.append('int %s;' % t[1])
        elif re.fullmatch(r'G[a-z]', t):
            o.append('G(%s);' % t[1])
        elif t == '{':
            o.append('while (cond()) {')
        elif t == 'F{':
            o.append('for (int n = 0; n < 10; n++) {')
        elif t == '[':
            o.append('if (cond()) {')
        elif t == '|':
            o.append('} else {')
        elif t == 'do{':
            o.append('do {')
        elif t == '}w':
            o.append('} while (cond());')
        elif t in ('}', ']'):
            o.append('}')
        elif t == 'C':
            o.append('call();')
        else:
            o.append(t.replace('~', ' '))
    return ' '.join(o)


def write(name, probes, head=''):
    """probes: [(func, spec, signature or None)]"""
    o = ['#include "probe.h"', head]
    meth = [re.search(r'T::(\w+)(\(.*\))', sig) for fn, spec, sig in probes if sig and 'T::' in sig]
    if meth:
        o.append('struct T : Obj { %s };' % ' '.join('void %s%s;' % (m.group(1), m.group(2)) for m in meth))
    for fn, spec, sig in probes:
        o.append('// %s\n%s { %s }' % (spec, sig or 'void %s()' % fn, body(spec)))
    open('%s/g_%s.cpp' % (HERE, name), 'w').write('\n'.join(o) + '\n')


FAM = {}


def family(f):
    FAM[f.__name__] = f
    return f


@family
def overlap():
    """All four live over the whole function (defined first, one use each at the end).
    a,b,c get K straight uses; d gets one use inside a loop of depth L (or a for loop)."""
    P = []
    for L, op in ((1, '{'), (2, '{ {'), (3, '{ { {'), (1, 'F{'), (2, 'F{ F{')):
        for K in (1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 16, 17, 20, 32, 33, 64, 65, 66, 100):
            if L == 1 and K > 20 or L == 2 and K > 70:
                continue
            P.append(('ov_%s%d_k%d' % ('f' if 'F' in op else 'w', L, K),
                      'Da Db Dc Dd a%d b%d c%d %s d %s a b c d' % (K, K, K, op, '} ' * L), None))
    write('overlap', P)

def duel(name, mk, ks, doc=''):
    """p,q (30 uses) take esi/edi; c and d fight for ebx.  mk(K) -> (c_spec, d_spec); both orders of definition."""
    P = []
    for K in ks:
        cs, ds = mk(K)
        P.append(('%s_cd_k%d' % (name, K), 'Dp Dq Dc Dd p30 q30 %s %s p q c d' % (cs, ds), None))
        P.append(('%s_dc_k%d' % (name, K), 'Dp Dq Dd Dc p30 q30 %s %s p q c d' % (cs, ds), None))
    return P


@family
def weights():
    """c: K straight uses.  d: one use in the named context.  Threshold K gives the weight of the context."""
    P = []
    ks = (0, 1, 2, 3, 4, 5, 6, 8, 9, 10)
    P += duel('w1', lambda K: ('c%d' % K, '{ d }'), ks)
    P += duel('w2', lambda K: ('c%d' % K, '{ { d } }'), ks)
    P += duel('w3', lambda K: ('c%d' % K, '{ { { d } } }'), ks)
    P += duel('f1', lambda K: ('c%d' % K, 'F{ d }'), ks)
    P += duel('if', lambda K: ('c%d' % K, '[ d ]'), ks)
    P += duel('ife', lambda K: ('c%d' % K, '[ d | d ]'), ks)
    P += duel('st', lambda K: ('c%d' % K, 'd'), ks)
    P += duel('cl', lambda K: ('{ c%d }' % K, '{ d }') if K else ('', '{ d }'), ks)
    write('weights', P)


@family
def weights2():
    """c: K straight uses.  d: M uses in a context.  The K at which c takes over is the context's weight times M."""
    P = []
    ks = range(0, 14)
    for tag, o, c in (('w', '{', '}'), ('f', 'F{', '}'), ('i', '[', ']'), ('o', 'do{', '}w'), ('ww', '{ {', '} }'), ('oo', 'do{ do{', '}w }w'),
                      ('wi', '{ [', '] }'), ('ii', '[ [', '] ]')):
        for M in (1, 2, 3, 4):
            P += duel('%s%d' % (tag, M), lambda K: ('c%d' % K, '%s d%d %s' % (o, M, c)), ks)
    write('weights2', P)


@family
def weights3():
    """Finer: d has M uses in an if / while; c has K straight uses.  Then the inverse (c in context, d straight)."""
    P = []
    ks = range(0, 14)
    for M in (5, 6, 7, 8, 9, 10, 12, 16):
        P += duel('i%d' % M, lambda K: ('c%d' % K, '[ d%d ]' % M), ks)
    for M in (5, 6, 7, 8, 10):
        P += duel('w%d' % M, lambda K: ('c%d' % K, '{ d%d }' % M), ks)
    for M in (1, 2, 3):
        P += duel('ri%d' % M, lambda K: ('[ c%d ]' % K if K else '', 'd%d' % M), ks)
        P += duel('rw%d' % M, lambda K: ('{ c%d }' % K if K else '', 'd%d' % M), ks)
    write('weights3', P)


@family
def depth():
    """d: M uses at while-depth 3, 4, 5; c: K straight uses."""
    P = []
    for L in (3, 4, 5):
        for M in (1, 2, 3):
            P += duel('L%dm%d' % (L, M), lambda K: ('c%d' % K, '{ ' * L + 'd%d ' % M + '} ' * L), range(0, 40))
    write('depth', P)


@family
def const():
    """Where does the half-use constant of a loop context come from?"""
    P = []
    ks = range(0, 12)
    P += duel('l4x1', lambda K: ('c%d' % K, '{ d } { d } { d } { d }'), ks)      # four loops, one use each
    P += duel('l1x4', lambda K: ('c%d' % K, '{ d4 }'), ks)                       # one loop, four uses
    P += duel('empty', lambda K: ('c%d' % K, '{ C } d2'), ks)                    # unrelated loop, d straight
    P += duel('both', lambda K: ('c%d { c }' % K, '{ d }'), ks)                  # each has one loop use in its own loop
    P += duel('same', lambda K: ('c%d' % K, '{ d c }'), ks)                      # both used once in the same loop
    P += duel('i4x1', lambda K: ('c%d' % K, '[ d ] [ d ] [ d ] [ d ]'), ks)      # four ifs
    P += duel('ret', lambda K: ('if~(cond())~return; c%d' % K, 'd2'), ks)        # straight uses after an early return
    P += duel('ret2', lambda K: ('c%d if~(cond())~return;' % K, 'd2'), ks)       # c before the return, d after
    P += duel('else', lambda K: ('c%d' % K, '[ d2 | d2 ]'), ks)                  # both arms
    P += duel('aft', lambda K: ('c%d' % K, '[ C ] d2'), ks)                      # after a join
    write('const', P)


@family
def shapes():
    """Which top-level blocks are full weight and which are 'conditional'?  d has 4 uses in the shape; c K straight uses.
    full weight -> tie at K=4; if-weight -> d wins K<=1 only."""
    P = []
    ks = range(0, 20)
    for tag, sh in (('else', '[ C | d4 ]'), ('goto', 'if~(cond())~goto~L; d4 L:'), ('retblk', '[ C return; ] d4'),
                    ('inret', '[ d4 return; ]'), ('w2if', '{ { [ d4 ] } }'), ('w1if', '{ [ d4 ] }'), ('w1ifif', '{ [ [ d4 ] ] }'),
                    ('sw', 'switch~(cond())~{ case~1: d4 break; case~2: C break; }'),
                    ('and', 'if~(cond()~&&~cond())~{ d4 }'), ('w1tail', '{ C } d4'), ('w1ret', '{ if~(cond())~return; d4 }'),
                    ('w1brk', '{ if~(cond())~break; d4 }'), ('w1cont', '{ if~(cond())~continue; d4 }'), ('ifw', '[ { d4 } ]'),
                    ('ifww', '[ { { d4 } } ]'), ('do1', 'do{ d4 }w'), ('inf', 'for~(;;)~{ d4 if~(cond())~break; }')):
        P += duel(tag, lambda K: ('c%d' % K, sh), ks)
    write('shapes', P)


SHAPES = (('top', '%s'), ('if', '[ %s ]'), ('else', '[ C | %s ]'), ('ifif', '[ [ %s ] ]'), ('ififif', '[ [ [ %s ] ] ]'),
          ('sw', 'switch~(cond())~{ case~1: %s break; case~2: C break; }'),
          ('sw5', 'switch~(cond())~{ case~1: %s break; case~2: C break; case~3: C C break; case~4: C C C break; case~5: C C C C break; }'),
          ('swdef', 'switch~(cond())~{ case~1: C break; default: %s break; }'),
          ('ifsw', '[ switch~(cond())~{ case~1: %s break; case~2: C break; } ]'),
          ('w1', '{ %s }'), ('w1if', '{ [ %s ] }'), ('w1sw', '{ switch~(cond())~{ case~1: %s break; case~2: C break; } }'),
          ('do1', 'do{ %s }w'), ('inf', 'for~(;;)~{ %s if~(cond())~break; }'), ('inf2', 'for~(;;)~{ if~(cond())~break; %s }'),
          ('wtrue', 'while~(1)~{ %s if~(cond())~break; }'), ('for', 'F{ %s }'), ('w2', '{ { %s } }'), ('w2if', '{ { [ %s ] } }'),
          ('ifw1', '[ { %s } ]'), ('tern', 'use_a(cond()~?~d~:~0);'), ('andc', 'if~(cond()~&&~d)~{ C }'), ('cond', 'if~(d)~{ C }'),
          ('wcond', 'while~(d~!=~cond())~{ C }'))


@family
def calib():
    """Score of d's M uses in each syntactic context (c: K straight top-level uses)."""
    P = []
    for tag, sh in SHAPES:
        for M in (1, 2, 4, 8):
            if '%s' not in sh and M > 1:
                continue
            P += duel('%s_m%d' % (tag, M), lambda K: ('c%d' % K, sh % ('d%d' % M) if '%s' in sh else sh), range(0, 2 + 3 * M + (16 if tag in ('w2', 'w2if', 'inf', 'wtrue') else 0)))
    write('calib', P)


def duel2(name, ks, mk, sig=None):
    """mk(K) -> spec; one probe per K, function <name>_cd_k<K> (thresh.py reads c's first win as the score of d)."""
    return [('%s_cd_k%d' % (name, K), mk(K), (sig % ('%s_cd_k%d' % (name, K))) if sig else None) for K in ks]


@family
def factors():
    """One factor at a time; p,q hold esi/edi; c (K straight uses after one def, one final use) fights d for ebx.
    thresh.py prints d's score minus the common def + final use (2 for a local with one def and the final use)."""
    P = []
    ks = range(0, 12)
    for M in (1, 2, 4):
        # parameter d, no definition: M uses + final use
        P += duel2('par%d' % M, ks, lambda K: 'Dp Dq Dc p30 q30 c%d d%d p q c d' % (K, M), 'void %s(int d)')
        # c is the parameter, d the local
        P += duel2('rpar%d' % M, ks, lambda K: 'Dp Dq Dd p30 q30 c%d d%d p q c d' % (K, M), 'void %s(int c)')
        # both parameters, c first / d first
        P += duel2('pp%d' % M, ks, lambda K: 'Dp Dq p30 q30 c%d d%d p q c d' % (K, M), 'void %s(int c, int d)')
        P += duel2('ppr%d' % M, ks, lambda K: 'Dp Dq p30 q30 c%d d%d p q c d' % (K, M), 'void %s(int d, int c)')
        # d redefined M times (each definition followed by nothing): definitions as references
        P += duel2('def%d' % M, ks, lambda K: 'Dp Dq Dc Dd p30 q30 c%d %s p q c d' % (K, 'Sd ' * M))
        # d read through global stores instead of call arguments
        P += duel2('sink%d' % M, ks, lambda K: 'Dp Dq Dc Dd p30 q30 c%d %s p q c d' % (K, 'Gd C ' * M))
        # d used in arithmetic: sink_a = d + 1
        P += duel2('arith%d' % M, ks, lambda K: 'Dp Dq Dc Dd p30 q30 c%d %s p q c d' % (K, 'sink_a~=~d~+~garr[1]; C ' * M))
        # d used as an index
        P += duel2('index%d' % M, ks, lambda K: 'Dp Dq Dc Dd p30 q30 c%d %s p q c d' % (K, 'sink_a~=~garr[d]; C ' * M))
        # d used twice in one expression
        P += duel2('twice%d' % M, ks, lambda K: 'Dp Dq Dc Dd p30 q30 c%d %s p q c d' % (K, 'sink_a~=~garr[d]~+~d; C ' * M))
        # d compared
        P += duel2('cmp%d' % M, ks, lambda K: 'Dp Dq Dc Dd p30 q30 c%d %s p q c d' % (K, 'if~(d~==~7)~call(); C ' * M))
        # d incremented
        P += duel2('inc%d' % M, ks, lambda K: 'Dp Dq Dc Dd p30 q30 c%d %s p q c d' % (K, 'd++; C ' * M))
        P += duel2('addeq%d' % M, ks, lambda K: 'Dp Dq Dc Dd p30 q30 c%d %s p q c d' % (K, 'd~+=~garr[2]; C ' * M))
    write('factors', P)


@family
def factors2():
    """Tie-breaks, redefinitions, parameters used early."""
    P = []
    ks = range(0, 14)
    for M in (1, 2, 4):
        # parameter d first used before c is defined (compare with par<M> in factors: d first used after c's definition)
        P += duel2('early%d' % M, ks, lambda K: 'd Dp Dq Dc p30 q30 c%d d%d p q c d' % (K, M - 1), 'void %s(int d)')
        # both parameters, d used first
        P += duel2('ppd%d' % M, ks, lambda K: 'Dp Dq d p30 q30 c%d d%d p q c d' % (K, M - 1), 'void %s(int c, int d)')
        # redefinition followed by a use
        P += duel2('redef%d' % M, ks, lambda K: 'Dp Dq Dc Dd p30 q30 c%d %s p q c d' % (K, 'Sd d ' * M))
        # increment followed by a use
        P += duel2('incuse%d' % M, ks, lambda K: 'Dp Dq Dc Dd p30 q30 c%d %s p q c d' % (K, 'd++; d ' * M))
        # d declared before c but defined after
        P += duel2('decl%d' % M, ks, lambda K: 'Dp Dq Nd Dc Sd p30 q30 c%d d%d p q c d' % (K, M))
        # d defined first but c used first
        P += duel2('dfirst%d' % M, ks, lambda K: 'Dp Dq Dd Dc p30 q30 c%d d%d p q c d' % (K, M))
        # d defined first, constant initialiser
        P += duel2('const%d' % M, ks, lambda K: 'Dp Dq int~d~=~5; Dc p30 q30 c%d d%d p q c d' % (K, M))
        # char and short d
        P += duel2('char%d' % M, ks, lambda K: 'Dp Dq Dc char~d~=~(char)def_d(); p30 q30 c%d d%d p q c d' % (K, M))
        P += duel2('short%d' % M, ks, lambda K: 'Dp Dq Dc short~d~=~(short)def_d(); p30 q30 c%d d%d p q c d' % (K, M))
        # d's address taken once (escapes)
        P += duel2('addr%d' % M, ks, lambda K: 'Dp Dq Dc Dd p30 q30 c%d d%d use_a((int)&d); p q c d' % (K, M))
    write('factors2', P)


@family
def split():
    """Live-range pieces: who gets a register where."""
    P = [
        # d (loser) used four times in the middle while a,b,c are live across but idle there
        ('hole_straight', 'Da Db Dc Dd a9 b9 c9 d4 a b c d', None),
        ('hole_loop', 'Da Db Dc Dd a9 b9 c9 { d4 } a b c d', None),
        ('hole_loop2', 'Da Db Dc Dd a9 b9 c9 { { d8 } } a b c', None),
        # c dead before the middle
        ('dead_before', 'Da Db Dc Dd a9 b9 c9 { d4 } a b d', None),
        # c is two webs with d's region between them
        ('two_webs', 'Da Db Dc Dd a9 b9 c9 { d4 } Sc c9 a b d', None),
        # loser with one use per region
        ('one_use', 'Da Db Dc Dd a9 b9 c9 { d } a b d', None),
        ('one_use_top', 'Da Db Dc Dd a9 b9 c9 d a b', None),
        ('two_use_top', 'Da Db Dc Dd a9 b9 c9 d d a b', None),
        ('def_use', 'Da Db Dc a9 b9 c9 Dd d a b', None),
        ('def_use2', 'Da Db Dc a9 b9 c9 Dd d d a b', None),
        ('def_2use_c_live', 'Da Db Dc a9 b9 c9 Dd d d a b c', None),
        # disjoint ranges: priority order vs position
        ('disjoint3', 'Da a2 Db b5 Dc c3', None),
        ('disjoint_overlap', 'Da a5 Db a b3 Dc b c4 Dd c d2 d', None),
        # staggered: a ends, then d begins; b, c live throughout
        ('stagger', 'Da Db Dc a3 b9 c9 Dd d2 b c d', None),
        ('stagger_hi', 'Da Db Dc a3 b2 c2 Dd d9 b c d', None),
        # a param loser reloaded
        ('param_lose', 'Da Db Dc a9 b9 c9 d d a b c d', 'void param_lose(int d)'),
        ('param_lose_loop', 'Da Db Dc a9 b9 c9 { d d } a b c d', 'void param_lose_loop(int d)'),
        ('param_free', 'Da Db Dc a9 b9 c9 a b c d d', 'void param_free(int d)'),
        # five: two losers sharing a freed register
        ('two_losers', 'Da Db Dc Dd De a9 b9 c9 a b c d3 e3', None),
        ('two_losers_mix', 'Da Db Dc Dd De a9 b9 c9 a b c d e d e d e', None),
        ('two_losers_2free', 'Da Db Dc Dd De a9 b9 c9 a b d e d e d e c', None),
        # loser across a join
        ('if_region', 'Da Db Dc Dd a9 b9 c9 a b c [ d2 ] d', None),
        ('if_region2', 'Da Db Dc Dd a9 b9 c9 a b c [ d2 ] [ d2 ]', None),
        ('if_else_region', 'Da Db Dc Dd a9 b9 c9 a b c [ d2 | d2 ] d', None),
    ]
    write('split', P)


@family
def rangelen():
    """Is priority normalised by live-range length?  c lives over the whole function, d only over its own uses."""
    P = []
    ks = range(0, 12)
    for M in (2, 4, 8):
        P += duel2('short%d' % M, ks, lambda K: 'Dp Dq Dc p30 q30 c%d Dd d%d p q c' % (K, M))            # pure count: c wins from K=M-1
        P += duel2('shortlp%d' % M, ks, lambda K: 'Dp Dq Dc p30 q30 c%d { C } { C } Dd d%d { C } p q c' % (K, M))
        P += duel2('early%d' % M, ks, lambda K: 'Dp Dq Dd d%d Dc p30 q30 c%d p q c' % (M, K))             # d dead before c is defined: no fight
    write('rangelen', P)


@family
def cache():
    """Per-block caching of memory-homed variables (a,b,c hold the registers until their last use)."""
    P = [
        ('blk2', 'Da Db Dc Dd a9 b9 c9 a b c d d', None),                       # 2 uses, one block -> cached
        ('blk1_1', 'Da Db Dc Dd a9 b9 c9 a b c d [ C ] d', None),               # 1 use per block
        ('blk2_1', 'Da Db Dc Dd a9 b9 c9 a b c d d [ C ] d', None),
        ('blk2_2', 'Da Db Dc Dd a9 b9 c9 a b c d d [ C ] d d', None),
        ('loop_multi', 'Da Db Dc Dd a9 b9 c9 a b c { d [ C ] d [ C ] d }', None),  # loop of three blocks, 1 use each
        ('loop_multi2', 'Da Db Dc Dd a9 b9 c9 a b c { d d [ C ] d d [ C ] d d }', None),
        ('loop_w', 'Da Db Dc Dd a9 b9 c9 a b c { d d } d d', None),
        ('wr_blk', 'Da Db Dc Dd a9 b9 c9 a b c Sd d d [ C ] d d', None),         # def + uses in one block
        ('wr_loop', 'Da Db Dc Dd a9 b9 c9 a b c { Sd d d [ C ] d d }', None),
        ('inc_loop', 'Da Db Dc Dd a9 b9 c9 a b c { d++; d [ C ] d++; d }', None),
        # still occupied registers: cached only while one is free
        ('occupied', 'Da Db Dc Dd a9 b9 c9 d d a b c d d', None),
        # a scratch register is enough when no call intervenes
        ('nocall', 'Da Db Dc Dd a9 b9 c9 Gd sink_a~=~d~+~1; sink_b~=~d~+~2; C a b c', None),
        ('nocall_free', 'Da Db Dc Dd a9 b9 c9 a b c Gd sink_a~=~d~+~1; sink_b~=~d~+~2; C', None),
    ]
    write('cache', P)


@family
def temps():
    """Memory-homed variables' cached loads, `this`, and CSE temporaries against named variables."""
    M = 'void T::%s()'
    P = [
        # d lost at its definition (a,b,c live); later its cached load (9 uses) meets f,g (9 uses) and e (3 uses)
        ('cache_vs_web', 'Da Db Dc Dd a9 b9 c9 Df Dg De f9 g9 e3 d9', None),
        ('cache_vs_web2', 'Da Db Dc Dd a9 b9 c9 Df Dg De d9 f9 g9 e3', None),
        ('cache_vs_web3', 'Da Db Dc Dd a9 b9 c9 Df Dg De e d f9 g9 e2 d8', None),
        ('cache_vs_web4', 'Da Db Dc Dd a9 b9 c9 Df Dg De e d f9 g9 e12 d8', None),
        # this: K member calls against three locals
        ('this1', 'Da Db Dc a3 b3 c3 m(); a b c', M % 'this1'),
        ('this3', 'Da Db Dc a3 b3 c3 m(); m(); m(); a b c', M % 'this3'),
        ('this4', 'Da Db Dc a3 b3 c3 m(); m(); m(); m(); a b c', M % 'this4'),
        ('this5', 'Da Db Dc a3 b3 c3 m(); m(); m(); m(); m(); a b c', M % 'this5'),
        ('this6', 'Da Db Dc a3 b3 c3 m(); m(); m(); m(); m(); m(); a b c', M % 'this6'),
        ('this_first', 'm(); Da Db Dc a3 b3 c3 m(); m(); m(); m(); a b c', M % 'this_first'),
        ('this_field', 'Da Db Dc a3 b3 c3 use_d(f0); use_d(f1); use_d(f2); use_d(f3); use_d(f0); a b c', M % 'this_field'),
        # index used only through one address: the address is the candidate, not the index
        ('cse_addr', 'Da Db Dc Dd a3 b3 c3 use_e(gobj[d].f0); use_e(gobj[d].f1); use_e(gobj[d].f2); use_e(gobj[d].f3); use_e(gobj[d].f0); use_e(gobj[d].f1); a b c', None),
        ('cse_addr_d', 'Da Db Dc Dd a3 b3 c3 use_e(gobj[d].f0); use_e(gobj[d].f1); use_e(gobj[d].f2); use_e(gobj[d].f3); use_e(gobj[d].f0); use_e(gobj[d].f1); a b c d', None),
        ('cse_addr_2', 'Da Db Dd a3 b3 use_e(gobj[d].f0); use_e(gobj[d].f1); use_e(gobj[d].f2); d a b d', None),
        # global read repeatedly across calls: not cached
        ('global', 'Da Db a3 b3 use_e(sink_c); use_e(sink_c); use_e(sink_c); use_e(sink_c); a b', None),
        ('global_nocall', 'Da Db a3 b3 sink_d~=~sink_c; sink_e~=~sink_c; sink_f~=~sink_c; a b', None),
        ('member_nocall', 'Da Db a3 b3 sink_d~=~f1; sink_e~=~f1; sink_f~=~f1; a b', M % 'member_nocall'),
    ]
    write('temps', P)


@family
def position():
    """Does the weight of a top-level use depend on what precedes it?  d: 8 uses placed after 0-3 loops / ifs;
    defined at the top (dtop) or just before its uses (dlate).  Score 8 = full weight (thresh.py prints d's uses)."""
    P = []
    ks = range(0, 12)
    for n in (0, 1, 2, 3):
        P += duel2('dtop_w%d' % n, ks, lambda K: 'Dp Dq Dc Dd p30 q30 c%d %s d8 p q c d' % (K, '{ C } ' * n))
        P += duel2('dlate_w%d' % n, ks, lambda K: 'Dp Dq Dc p30 q30 c%d %s Dd d8 p q c d' % (K, '{ C } ' * n))
        P += duel2('dlate_i%d' % n, ks, lambda K: 'Dp Dq Dc p30 q30 c%d %s Dd d8 p q c d' % (K, '[ C ] ' * n))
        P += duel2('dlast_w%d' % n, ks, lambda K: 'Dp Dq Dc p30 q30 c%d %s Dd d8 p q c' % (K, '{ C } ' * n))
        P += duel2('dlast_wt%d' % n, ks, lambda K: 'Dp Dq Dc p30 q30 c%d %s Dd d8 { C } p q c' % (K, '{ C } ' * n))
        P += duel2('clate_w%d' % n, ks, lambda K: 'Dp Dq Dc Dd p30 q30 %s c%d d8 p q c d' % ('{ C } ' * n, K))
    write('position', P)


@family
def span():
    """c is live through blocks it is not referenced in; d lives in one block.  thresh.py: first K at which c wins."""
    P = []
    ks = range(0, 40)
    for M in (2, 4, 8, 12):
        P += duel2('w1_m%d' % M, ks, lambda K: 'Dp Dq Dc p30 q30 c%d { C } Dd d%d p q c' % (K, M))
        P += duel2('w1f_m%d' % M, ks, lambda K: 'Dp Dq Dc p30 q30 c%d { C } Dd d%d p q c d' % (K, M))
        P += duel2('i1_m%d' % M, ks, lambda K: 'Dp Dq Dc p30 q30 c%d [ C ] Dd d%d p q c' % (K, M))
        P += duel2('w1t_m%d' % M, ks, lambda K: 'Dp Dq Dc p30 q30 c%d { C } Dd d%d { C } p q c' % (K, M))
        P += duel2('t_m%d' % M, ks, lambda K: 'Dp Dq Dc p30 q30 c%d Dd d%d { C } p q c' % (K, M))
        P += duel2('s_m%d' % M, ks, lambda K: 'Dp Dq Dc p30 q30 c%d Dd d%d p q c' % (K, M))
    write('span', P)


@family
def middle():
    """Weight of top-level blocks by position, both variables live over the whole function (d: 8 uses there)."""
    P = []
    ks = range(0, 14)
    for tag, sh in (('first', 'd8 { C } { C }'), ('mid', '{ C } d8 { C }'), ('last', '{ C } { C } d8'), ('mid_if', '[ C ] d8 [ C ]'),
                    ('mid3', '{ C } { C } d8 { C } { C }'), ('before_do', 'do{ C }w d8 do{ C }w'), ('before_ret', '{ C } d8 if~(cond())~return; { C }')):
        P += duel('%s' % tag, lambda K: ('c%d' % K, sh), ks)
    write('middle', P)


@family
def middle2():
    """More positions (both variables live over the whole function).  x4 = entry/exit weight, x1 = middle weight."""
    P = []
    ks = range(0, 14)
    for tag, sh in (('test_mid', '[ C ] if~(d)~{ C } [ C ]'),              # one test in a middle block
                    ('test_mid4', '[ C ] if~(d)~{ C } if~(d~>~1)~{ C } if~(d~>~2)~{ C } if~(d~>~3)~{ C } [ C ]'),
                    ('test_after_clamp', 'if~(sink_a~<~1)~{ sink_a~=~1; } if~(d~<~0)~{ C } [ C ]'),
                    ('test2_after_clamp', 'if~(sink_a~<~1)~{ sink_a~=~1; } if~(d~<~0)~{ C } if~(d~>~5)~{ C } [ C ]'),
                    ('def_if', '[ Sd ]'), ('def_if2', '[ Sd ] [ Sd ]'), ('def_if4', '[ Sd ] [ Sd ] [ Sd ] [ Sd ]'),
                    ('def_const4', '[ d~=~1; ] [ d~=~2; ] [ d~=~3; ] [ d~=~4; ]'),
                    ('def_top4', 'C Sd C Sd C Sd C Sd'),
                    ('use_if_ret', '[ d4 return; ]'), ('use_ret_last', '[ C return; ] d4'),
                    ('first_ifelse', '[ C | C ] d4 [ C ]')):
        P += duel('%s' % tag, lambda K: ('c%d' % K, sh), ks)
    write('middle2', P)


@family
def parpos():
    """A parameter's references by position (c: local, K entry-block uses + definition + final use).
    Compare with the same shapes for a local d in `middle` / `calib`."""
    P = []
    ks = range(0, 14)
    S = 'void %s(int d)'
    for M in (4, 8):
        P += duel2('p_entry%d' % M, ks, lambda K: 'Dp Dq Dc p30 q30 c%d d%d [ C ] [ C ] p q c' % (K, M), S)
        P += duel2('p_mid%d' % M, ks, lambda K: 'Dp Dq Dc p30 q30 c%d [ C ] d%d [ C ] p q c' % (K, M), S)
        P += duel2('p_last%d' % M, ks, lambda K: 'Dp Dq Dc p30 q30 c%d [ C ] [ C ] d%d p q c' % (K, M), S)
        P += duel2('p_if%d' % M, ks, lambda K: 'Dp Dq Dc p30 q30 c%d [ d%d ] [ C ] p q c' % (K, M), S)
        P += duel2('p_loop%d' % M, ks, lambda K: 'Dp Dq Dc p30 q30 c%d { d%d } [ C ] p q c' % (K, M), S)
        P += duel2('p_entry_mid%d' % M, ks, lambda K: 'Dp Dq Dc p30 q30 c%d d [ C ] d%d [ C ] p q c' % (K, M - 1), S)
        P += duel2('l_mid%d' % M, ks, lambda K: 'Dp Dq Dc Dd p30 q30 c%d [ C ] d%d [ C ] p q c' % (K, M))
        P += duel2('l_late_mid%d' % M, ks, lambda K: 'Dp Dq Dc p30 q30 c%d [ C ] Dd d%d [ C ] p q c' % (K, M))
    write('parpos', P)


if __name__ == '__main__':
    import sys
    for n, f in FAM.items():
        if len(sys.argv) < 2 or n in sys.argv[1:]:
            f()
