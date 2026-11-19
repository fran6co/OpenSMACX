#!/usr/bin/env python3
"""tm7.py: which properties make VC6 refuse a tail merge it would otherwise make.
Each variant is one function: an if/else-if chain over `a` whose arms end in identical
`f2(g4, g5);` tails (push; push; call f2; add esp,8) plus two `f3(1,2,3)` arms (add esp,0xc),
then a join `use(g); ...`.  Reported: copies of `call f2`, copies of `call f3`, and the call the join's
fallthrough predecessor ends with (who hosts the join).  Knobs vary one factor at a time."""
import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + '/inline')
import ib

HDR = ('#include "probe.h"\nextern void f2(int, int); extern void f3(int, int, int); extern int f2r(int, int);\n'
       'struct D { D(); ~D(); int v; };\nextern void f2x(int, int);\n'
       'struct M { static void pop(const char*, const char*, int = 0); static void pop2(const char *a, const char *b) { pop(a, b, 0); } };\n')


def arms(n=6, tail=lambda j: 'f2(%d, %d);' % (j, j + 100), other='f3(1, 2, 3);', pre=lambda j: 'mk(%d); g3 = %d;' % (j * 7 + 3, j), wrap=None, last_other=True):
    out = []
    for j in range(n):
        body = '%s if (cond()) { %s }' % (pre(j + 1), tail if not callable(tail) else tail(j))
        if wrap:
            body = wrap(j, body)
        out.append(body)
    o = ['mk(%d); use(%d); %s' % (50 + j, 50 + j, other) for j in range(2)]
    return (out[:3] + o[:1] + out[3:] + o[1:]) if last_other else (o + out)


def fn(name, A, pro='', join='use(g); call();', sig='int a', epi=''):
    s = 'void %s(%s) {\n%s' % (name, sig, pro)
    for j, body in enumerate(A):
        s += '  %sif (a == %d) { %s }\n' % ('else ' if j else '', j + 1, body)
    s += '  %s\n%s}\n' % (join, epi)
    return s


def analyse(asm, name):
    f = [x for x in ib.functions(asm) if re.search(r'\b' + name + r'\b', x[0])]
    if not f:
        return None
    L = f[0][2]
    n2 = sum(1 for l in L if l.startswith('call') and ('?f2@@' in l or '?f2s@@' in l or '?pop@M@@' in l or '?f2x@@' in l or '?f2r@@' in l))
    n3 = sum(1 for l in L if l.startswith('call') and '?f3@@' in l)
    # join = first `call ?use@@` with push g (mov eax,g; push eax) — find the label before 'call ?call@@' last
    host = '?'
    for i, l in enumerate(L):
        if l.startswith('call') and '?call@@' in l:
            k = i
            while k > 0 and not L[k].endswith(':'):
                k -= 1
            # the arm that falls into the join: the last `push K` before a `call ?mk@@` above it
            p = k - 1
            while p > 0:
                if L[p].startswith('call') and '?mk@@' in L[p]:
                    q = p - 1
                    while q > 0 and not L[q].startswith('push'):
                        q -= 1
                    host = 'mk' + L[q].split()[-1]
                    break
                if L[p].startswith('jmp') or L[p].startswith('ret'):
                    host = 'none'
                    break
                p -= 1
            break
    return n2, n3, host


def run(V, pfx=os.environ.get('FE_PFX_BASE', '21'), extra=(), show=()):
    src = HDR + ''.join(v for _, v in V)
    asm, out = ib.compile_asm(None, pfx, text=src, extra=extra, oy=('/Oy' in extra))
    res = []
    for key, v in V:
        name = re.findall(r'void (t_\w+)\(', v)[-1]
        res.append((key, analyse(asm, name)))
        if key in show:
            f = [x for x in ib.functions(asm) if name in x[0]][0]
            print('\n'.join(f[2]))
    return res


def report(res, V):
    for (k, r), (_, v) in zip(res, V):
        print('%-28s f2-copies=%s f3-copies=%s host=%s' % ((k,) + tuple(r if r else ('?', '?', '?'))))
