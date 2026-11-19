#!/usr/bin/env python3
"""tailscan.py <unit> <0xADDR[,0xADDR]>|--all [--src ALT.cpp] [--pfx N] [--min K] [--agree]
       tailscan.py --tree [unit ...] [--json OUT] [--min K] [--hosts]
Tail-merge / cross-jump scan (MATCHING_GUIDE 3.7, "Layout").

For each side (the image span and our COMDAT) it finds the copies VC6 kept unmerged: two code runs
of K or more identical instructions (calls and globals by name, local jumps by target) that end in
the same continuation -- `jmp T` and `jmp T`, `jmp T` and a fallthrough into T, or two returns
(`ret N` epilogues).  Cross-jumping would have kept one copy and turned the other into a `jmp` into
its middle, so a pair that one side keeps and the other lacks is a tail-merge disagreement:

  IMG-MERGES   ours keeps two copies; one of them has no aligned rows in the image, so the image
               jumps into the other copy instead (the source spells the tail twice, or once in a
               place the image does not use -- see MATCHING_GUIDE 3.7 for which copy survives)
  OURS-MERGES  the image keeps two copies; ours lacks one of them
  (--agree also prints pairs both sides keep)

`JJ` lists our jumper-into-jumper merges (a `jmp` into the middle of another statement's code whose
block then leaves by `jmp`): rare in exact functions, common where the image keeps the copies.
It also lists every unconditional `jmp` whose image and our targets disagree (fe's `!>` rows on
`jmp`), with the source lines at the jumper and at both targets: a `jmp` that lands K rows
earlier in the image than in ours is a K-instruction cross-jump the image made and ours did not.

Source lines come from our /FAsc listing (`L123`; `h:L45` for a line from an included header);
image positions are named through the fe alignment (`~L123` = the nearest aligned our row before).
`HOST` lines (MATCHING_GUIDE 3.7 "Which predecessor hosts a join"): for each join where the image and
ours place the join after different predecessors, both sides' predecessors in our source order as
`line[last instruction]` (`H:` host, `x:` cross-jumped into the host's tail, `/T` our tail ends the
then-arm of an inner if/else), what rules 1-2 predict for ours, and the source change they predict gives
the image's host: a pop-size difference (rule 3: move a call across an inner branch), an image host that
is not the first partnered arm (rule 2: its tail was in an inner then-arm), ours promoted by a then-arm
the image ignores (make it a lone if / else-arm / straight), or an arm order to change.
--all: every non-BYTE_EXACT marker of the unit.  --tree: every unit with non-exact markers, a
summary line per function with a disagreement, most disagreements first (FE_JOBS lanes from
FE_PFX_BASE).  Screen only; confirm with `recover measure`."""
import sys, os, re, json, struct, argparse, subprocess, collections
from concurrent.futures import ThreadPoolExecutor
T = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, T)
import fe

SENT = 0x7F000000


def compile_listing(unit, src, pfx):
    """-> (Coff, listing text) from one /FAsc compile"""
    cmd = fe.compile_cmd(unit)
    if not cmd:
        return None, None
    tag = 'ts_%d_%s' % (os.getpid(), pfx)
    obj, asm = fe.SCR + '/' + tag + '.obj', fe.SCR + '/' + tag + '.cod'
    out = []
    for t in cmd:
        if t.startswith('/Fo'):
            t = '/Fo' + obj
        elif t.startswith('/Fd') or t.startswith('--repro-root') or t == '/W4':
            continue
        elif t == fe.R + '/' + unit:
            t = os.path.abspath(src)
        out.append(t)
    out.insert(-1, '/FAsc')
    out.insert(-1, '/Fa' + asm)
    for p in (obj, asm):
        os.path.exists(p) and os.remove(p)
    env = dict(os.environ, VC6_PREFIX='%s/.opensmacx/wine/worker-%s' % (fe.MAIN, pfx))
    subprocess.run(out, cwd=fe.SCR, env=env, capture_output=True, text=True)
    if not os.path.exists(obj):
        return None, None
    c = fe.Coff(open(obj, 'rb').read())
    t = open(asm, errors='replace').read() if os.path.exists(asm) else ''
    for p in (obj, asm):
        os.path.exists(p) and os.remove(p)
    return c, t


def lines_of(cod, sym, unitbase):
    """/FAsc listing -> {offset: 'L123' or 'file.h:L45'}"""
    i = cod.find(sym + '\tPROC')
    if i < 0:
        i = cod.find(sym + ' PROC')
    e = cod.find(sym + '\tENDP', i)
    if e < 0:
        e = cod.find(sym + ' ENDP', i)
    if i < 0:
        return {}
    res, line, fname = {}, 0, unitbase
    for l in cod[i:e].split('\n'):
        l = l.rstrip('\r')
        m = re.match(r'; File (.*)$', l)
        if m:
            fname = re.split(r'[\\/]', m.group(1).strip())[-1].lower()
            continue
        m = re.match(r'; (\d+)\s+:', l)
        if m:
            line = int(m.group(1))
            continue
        m = re.match(r'  ([0-9a-f]{5})\t', l)
        if m:
            res[int(m.group(1), 16)] = ('L%d' % line) if fname == unitbase else '%s:L%d' % (fname, line)
    return res


def named(data, org, relocs, syms, struct_rows):
    """Re-disassemble with relocated operands written as symbol names (ours) or absolute addresses
    (image); local jumps keep fe's targets.  -> list of (off, text, size, tgt, masked_text)"""
    data = bytearray(data)
    rel = {}
    for k, r in enumerate(relocs):
        off, si = r[0], r[1]
        sy = syms.get(si)
        add = struct.unpack_from('<I', data, off)[0]
        rel[off] = (sy.name if sy else '?', add)
        struct.pack_into('<I', data, off, SENT + (k << 4))
    names = {SENT + (k << 4): rel[r[0]] for k, r in enumerate(relocs)}
    byoff = {}
    for ins in fe.md.disasm(bytes(data), org):
        byoff[ins.address - org] = ins
    out = []
    for off, mtxt, size, tgt in struct_rows:
        ins = byoff.get(off)
        if ins is None:
            out.append((off, mtxt, size, tgt, mtxt))
            continue
        mn, op = ins.mnemonic, ins.op_str
        if tgt is not None:
            txt = mn + ' L'
        elif (mn == 'call' or mn == 'jmp') and (off + 1) in rel and size == 5:
            txt = mn + ' ' + rel[off + 1][0]
        else:
            def sub(m):
                v = int(m.group(0), 16)
                if v in names:
                    n, a = names[v]
                    return n + ('+%x' % a if a else '')
                return m.group(0)
            txt = mn + ' ' + re.sub(r'0x[0-9a-f]+', sub, op)
        out.append((off, txt.strip(), size, tgt, mtxt))
    return out


def image_named(lo, hi, struct_rows):
    byoff = {i.address - lo: i for i in fe.md.disasm(fe.imgbytes(lo, hi), lo)}
    out = []
    for off, mtxt, size, tgt in struct_rows:
        ins = byoff[off]
        txt = ins.mnemonic + (' L' if tgt is not None else ' ' + ins.op_str)
        out.append((off, txt.strip(), size, tgt, mtxt))
    return out


UNCOND = ('jmp', 'ret')


def is_uncond(t):
    return t.startswith('jmp') or t.startswith('ret')


def dups(I, kmin):
    """unmerged identical tails, clustered: [(k, cont, [(start_idx, end_idx), ...])] -- every member
    of a cluster ends in the same continuation and shares its last k instructions"""
    idx = {x[0]: i for i, x in enumerate(I)}
    labels = {x[3] for x in I if x[3] is not None}

    def follow(t):
        seen = set()
        while t in idx and I[idx[t]][1] == 'jmp L' and t not in seen:
            seen.add(t)
            t = I[idx[t]][3]
        return t

    def key(p):
        x = I[p]
        return (x[1], follow(x[3]) if x[3] is not None else None)
    ends = []  # (index after the run's last instruction, continuation)
    for e, x in enumerate(I):
        if x[1] == 'jmp L':
            ends.append((e, ('J', follow(x[3]))))
        elif x[1].startswith('ret'):
            ends.append((e, ('R', x[1])))
        if x[0] in labels and e > 0 and not is_uncond(I[e - 1][1]):
            ends.append((e, ('J', follow(x[0]))))

    def suffix(e, n):
        out = []
        for i in range(n):
            p = e - 1 - i
            if p < 0 or is_uncond(I[p][1]):
                return None
            out.append(key(p))
        return tuple(out)
    by = collections.defaultdict(set)
    for e, c in ends:
        sk = suffix(e, kmin)
        if sk is not None:
            by[(c, sk)].add(e)
    res = []
    for (c, sk), es in by.items():
        es = sorted(es)
        if len(es) < 2:
            continue
        k = kmin
        while True:
            ks = [suffix(e, k + 1) for e in es]
            if any(x is None for x in ks) or len(set(ks)) != 1 or any(
                    es[i] - k - 1 < es[i - 1] for i in range(1, len(es))):
                break
            k += 1
        res.append((k, c, [(e - k, e) for e in es]))
    # a longer cluster supersedes the shorter clusters inside it
    res.sort(key=lambda r: -r[0])
    seen, out = set(), []
    for r in res:
        ends_ = frozenset(x[1] for x in r[2])
        if ends_ in seen:
            continue
        seen.add(ends_)
        out.append(r)
    return out

# ---- join host analysis (MATCHING_GUIDE 3.7 "Which predecessor hosts a join") ----
WIN = 8


def is_cf(t):
    return t.startswith('j') or t.startswith('ret') or t.startswith('loop')


def joins(I):
    """-> {J_index: dict(host=idx_of_last_host_instr, preds=[(kind, jmp_idx or None, last_idx)])}
    A join J is a label with a fallthrough predecessor (the host) that one or more `jmp`s reach, either
    directly or through a cross-jump into the host's tail (a label at most WIN straight-line instructions
    before J).  A cross-jump label inside a later join's host tail is folded into that join."""
    idx = {x[0]: i for i, x in enumerate(I)}
    labels = {x[3] for x in I if x[3] is not None and x[3] in idx}
    lab_i = {idx[t] for t in labels}

    def chain(i):
        out = [i]
        j = i
        for _ in range(WIN):
            if is_cf(I[j][1]):
                break
            j += 1
            if j >= len(I):
                break
            if j in lab_i:
                out.append(j)
        return out
    reach = collections.defaultdict(list)  # label index -> jumpers reaching it
    for j, x in enumerate(I):
        if x[1] == 'jmp L' and x[3] in idx:
            for L in chain(idx[x[3]]):
                reach[L].append(j)
    cand = {L for L in reach if L > 0 and not is_cf(I[L - 1][1])}
    res = {}
    for L in sorted(cand):
        ch = chain(L)[1:]
        # folded into a later join whose own jumpers come directly
        if any(L2 in cand and set(reach[L]) <= set(reach[L2]) for L2 in ch):
            continue
        preds = [('host', None, L - 1)]
        for j in reach[L]:
            t = idx[I[j][3]]
            if t == L:
                preds.append(('jmp', j, j - 1))
            else:
                preds.append(('xjmp', j, L - 1))  # cross-jumped: its tail is the host's
        res[L] = dict(host=L - 1, preds=preds)
    return res


def tail_key(I, k, masked=False):
    """the pred's last instruction: named (calls and globals by symbol / address) for partnership within
    a side, masked (relocated operands as <r>) to compare the two sides"""
    t = I[k][4 if masked else 1]
    return None if is_cf(I[k][4]) else t


def lnum(s):
    m = re.search(r'L(\d+)$', s or '')
    return int(m.group(1)) if m and ':' not in (s or '') else None


def indent(srclines, n):
    if n is None or n > len(srclines):
        return None
    l = srclines[n - 1]
    return len(l) - len(l.lstrip())


def then_arm(srclines, n, arm_indent=None):
    """our source line n ends the then-arm of an INNER if/else: the next code line is `} else` and the
    tail sits deeper than the join's arm level (a `} else` after an arm-level tail is the arm separator)"""
    if n is None:
        return False
    if arm_indent is not None and indent(srclines, n) is not None and indent(srclines, n) <= arm_indent:
        return False
    for l in srclines[n:n + 4]:
        s = l.strip()
        if not s or s.startswith('//'):
            continue
        return bool(re.match(r'\}\s*else\b', s))
    return False


def predict_host(ps, then):
    """rules 1-2 of MATCHING_GUIDE 3.7 over preds in source order: a partnered tail that ends an inner
    then-arm promotes its pair (host = the pair's first member); else the first partnered arm; else the
    first arm.  Cross-jumped preds count as partners of the host's tail."""
    keys = collections.Counter(p['key'] for p in ps if p['kind'] != 'xjmp' and p['key'])
    h = next(q for q in ps if q['kind'] == 'host')
    for p in ps:
        if p['kind'] == 'xjmp':
            keys[h['key']] += 1
    arms = [p for p in ps if p['kind'] != 'xjmp']
    for p in arms:
        if p['key'] and keys[p['key']] >= 2 and then(p):
            return next(q for q in arms if q['key'] == p['key']), 'rule 2 (then-arm %s)' % p['line']
    for p in arms:
        if p['key'] and keys[p['key']] >= 2:
            return p, 'rule 1'
    return (arms[0] if arms else None), 'no partner'


def host_report(C, O, lo, src, i2o, o2i, iline, srclines):
    """-> report lines for joins whose host differs between the image and ours, with the source change
    that the host rules (MATCHING_GUIDE 3.7) predict gives the image's host"""
    Jc, Jo = joins(C), joins(O)
    cidx = {x[0]: i for i, x in enumerate(C)}

    def oline(i):
        return src.get(C[i][0], '?')

    def mapped_line(i):  # image index -> our line through the alignment
        off = O[i][0]
        if off in i2o:
            return src.get(i2o[off], '?')
        return iline(off)
    cj = {}
    for J, d in Jc.items():
        for kind, j, last in d['preds']:
            if j is not None:
                cj[j] = J
    best = {}
    for Ji, di in Jo.items():
        votes = collections.Counter()
        for kind, j, last in di['preds']:
            if j is not None and O[j][0] in i2o and i2o[O[j][0]] in cidx:
                J = cj.get(cidx[i2o[O[j][0]]])
                if J is not None:
                    votes[J] += 1
        if votes:
            Jc_ = votes.most_common(1)[0][0]
            if Jc_ not in best or len(di['preds']) > len(Jo[best[Jc_]]['preds']):
                best[Jc_] = Ji

    def describe(d, I, ours):
        ps = []
        idx_ = {x[0]: n for n, x in enumerate(I)}
        xt = [idx_[I[j][3]] for kind, j, last in d['preds'] if kind == 'xjmp']
        for kind, j, last in d['preds']:
            pos = last if kind != 'xjmp' else j - 1
            if kind == 'host' and xt:
                pos = min(xt) - 1  # the host's own code before the shared (cross-jumped) part
            if pos < 0:
                continue
            ln = oline(pos) if ours else mapped_line(pos)
            if lnum(ln.lstrip('~')) is None and j is not None:
                ln = oline(j) if ours else mapped_line(j)
            ps.append(dict(kind=kind, line=ln, n=lnum(ln.lstrip('~')), key=tail_key(I, last),
                           mkey=tail_key(I, last, True),
                           at=(I[pos][0] if ours else lo + I[pos][0])))
        ps.sort(key=lambda p: (p['n'] is None, p['n'] or 0))
        return ps
    out = []
    for Jc_, Ji in sorted(best.items()):
        dc, di = Jc[Jc_], Jo[Ji]
        pc, pi = describe(dc, C, True), describe(di, O, False)
        if max(len(pc), len(pi)) < 3 and not any(
                (p['key'] or '').startswith(('add esp', 'call')) for p in pc + pi):
            continue
        hc = next(p for p in pc if p['kind'] == 'host')
        hi_ = next(p for p in pi if p['kind'] == 'host')
        if hc['n'] == hi_['n'] or (hi_['line'].startswith('~') and hc['n'] is not None and hi_['n'] is not None
                                   and abs(hc['n'] - hi_['n']) <= 2 and hc['mkey'] == hi_['mkey']):
            continue
        ind = [indent(srclines, p['n']) for p in pc if p['n'] is not None]
        ind = [x for x in ind if x is not None]
        arm_ind = min(ind) if ind else None

        def then(p):
            return then_arm(srclines, p['n'], arm_ind)

        def fmt(ps):
            return ' '.join('%s%s[%s]%s' % ('H:' if p['kind'] == 'host' else ('x:' if p['kind'] == 'xjmp' else ''),
                                            p['line'], (p['key'] or 'cf')[:14], '/T' if then(p) else '')
                            for p in ps)
        out.append('  HOST join img@%x ours@%x (%s): img host %s [%s]  ours host %s [%s]' % (
            lo + O[Ji][0], C[Jc_][0], oline(Jc_), hi_['line'], hi_['key'], hc['line'], hc['key']))
        out.append('       img  preds: ' + fmt(pi))
        out.append('       ours preds: ' + fmt(pc))
        adv = []
        pr_c, why_c = predict_host(pc, then)
        pr_i1, _ = predict_host(pi, lambda p: False)
        adv.append('rules on ours predict %s (%s): %s' % (pr_c['line'] if pr_c else '?', why_c,
                                                           'agrees' if pr_c and pr_c['n'] == hc['n'] else 'DISAGREES'))
        bym = {}
        for p in pc:
            if p['n'] is not None and p['kind'] != 'xjmp':
                bym.setdefault(p['n'], p)
        diffk = []
        for p in pi:
            q = bym.get(p['n'])
            if q and p['kind'] != 'xjmp' and p['mkey'] != q['mkey']:
                diffk.append((p, q))
        for p, q in diffk:
            m1 = re.match(r'add esp, (\w+)', p['key'] or '')
            m2 = re.match(r'add esp, (\w+)', q['key'] or '')
            if m1 and m2:
                a_, b_ = int(m1.group(1), 0), int(m2.group(1), 0)
                adv.append('%s pops %#x in the image, %#x in ours (rule 3): %s' % (
                    p['line'], a_, b_, 'the image deferred an earlier call\'s pop into the tail: put that call and '
                    'the tail straight-line, no inner branch between' if a_ > b_ else 'the image flushed an earlier '
                    'pop at a branch: an inner if/else between the earlier call and the tail'))
            else:
                adv.append('%s tail [%s] in the image, [%s] in ours: partnership differs' % (
                    p['line'], p['mkey'], q['mkey']))
        if pr_i1 is not None and pr_i1['n'] != hi_['n']:
            adv.append('image host %s is not the first partnered arm (%s): rule-2 priority, the original ended %s '
                       'in the then-arm of an inner if/else whose else does not end in it' % (
                           hi_['line'], pr_i1['line'], hi_['line']))
        elif why_c.startswith('rule 2') and pr_i1 is not None and pr_i1['n'] == hi_['n']:
            adv.append('the image follows rule 1 (%s); ours is promoted by %s: make that tail a lone if, an '
                       'else-arm or straight in its arm (or give its pair different pops)' % (hi_['line'], why_c))
        elif pr_c and hi_['n'] is not None and pr_c['n'] is not None and hi_['n'] > pr_c['n'] and not diffk:
            adv.append('the image passes over %s: unpartner it (a distinct last instruction) or move %s after %s' % (
                pr_c['line'], pr_c['line'], hi_['line']))
        elif pr_c and hi_['n'] is not None and pr_c['n'] is not None and hi_['n'] < pr_c['n'] and not diffk:
            adv.append('the image hosts the earlier %s: give it a partner, or promote it (inner then-arm tail)' %
                       hi_['line'])
        for a_ in adv:
            out.append('       -> ' + a_)
    return out


def scan(unit, addr, text, c, cod, kmin=2, agree=False, quiet=False):
    """-> (report lines, counts dict)"""
    r = fe.evaluate(c, text, addr)
    if 'error' in r:
        return ['TS %08X %s' % (addr, r['error'])], None
    sym, lo = r['sym'], r['lo']
    _, lo_, hi = fe.ident(text, addr)
    s = c.function(sym)
    sec = c.sections[s.section - 1]
    Cs = fe.dis(sec.data, 0, sec.relocations, selfsec=s.section, syms=c.symbols)
    Os = fe.imgdis(lo_, hi)
    C = named(sec.data, 0, sec.relocations, c.symbols, Cs)
    O = image_named(lo_, hi, Os)
    base = os.path.basename(unit).lower()
    src = lines_of(cod, sym, base)
    # alignment maps, from fe's rows
    i2o, o2i = {}, {}
    for k_, o, c_ in r['rows']:
        if k_ != '!!' and o and c_:
            i2o[o[0]] = c_[0]
            o2i[c_[0]] = o[0]
    co = sorted(o2i)

    def oline(off):
        return src.get(off, '?')

    def iline(off):
        # nearest aligned image row at or before off -> our line
        best = None
        for io in sorted(i2o):
            if io <= off:
                best = io
            else:
                break
        return ('~' + src.get(i2o[best], '?')) if best is not None else '~?'

    def span_lines(I, a, b, ours):
        ls = []
        for x in I[a:b]:
            l = oline(x[0]) if ours else iline(x[0])
            if l not in ls:
                ls.append(l)
        return ','.join(ls[:4]) + (',..' if len(ls) > 4 else '')

    def frac(I, a, b, amap):
        n = sum(1 for x in I[a:b] if x[0] in amap)
        return n / max(1, b - a)
    out = []
    cnt = dict(img_merges=0, ours_merges=0, jt_jmp=0, img_dups=0, ours_dups=0)
    D_o, D_i = dups(C, kmin), dups(O, kmin)
    cnt['ours_dups'], cnt['img_dups'] = len(D_o), len(D_i)

    def cont_s(cn, I, ours):
        if cn[0] == 'R':
            return cn[1]
        return 'jmp->%s' % ('%x' % cn[1] if ours else '%x' % (lo + cn[1])) + '(%s)' % (oline(cn[1]) if ours else iline(cn[1]))
    for side, D, I, amap, ours in (('ours', D_o, C, o2i, True), ('img', D_i, O, i2o, False)):
        for k, cn, copies in D:
            fr = [frac(I, a1, b1, amap) for a1, b1 in copies]
            miss = [c_ for c_, f in zip(copies, fr) if f < 0.5]
            if miss:
                cnt['img_merges' if ours else 'ours_merges'] += 1
            if not miss and not agree:
                continue
            tag = ('IMG-MERGES ' if ours else 'OURS-MERGES') if miss else 'both-keep  '
            org = 0 if ours else lo
            out.append('  %s %s keeps %d copies (other side %d) k=%d [%s .. %s] -> %s' % (
                tag, side, len(copies), len(copies) - len(miss), k, I[copies[0][0]][1][:30],
                I[copies[0][1] - 1][1][:30], cont_s(cn, I, ours)))
            for (a1, b1), f in zip(copies, fr):
                out.append('        %s @%x %-14s %s' % ('MISSING ' if f < 0.5 else 'present ', org + I[a1][0],
                                                       span_lines(I, a1, b1, ours), ''))
    # unconditional jumps whose targets disagree
    grp = collections.OrderedDict()
    ci = {x[0]: n for n, x in enumerate(C)}
    for k_, o, c_ in r['rows']:
        if k_ == '!>' and o[1].startswith('jmp'):
            cnt['jt_jmp'] += 1
            it, ot = o[3], c_[3]
            mapped = i2o.get(it)
            if mapped is not None and mapped in ci and ot in ci:
                rel = 'img lands %+d instr from ours' % (ci[mapped] - ci[ot])
            else:
                rel = 'img target unaligned'
            grp.setdefault((it, ot, rel), []).append((o[0], c_[0]))
    for (it, ot, rel), js in grp.items():
        out.append('  JMP x%d img ->%x (%s)  ours ->%x (%s)  %s; from %s' % (
            len(js), lo + it, iline(it), ot, oline(ot), rel,
            ' '.join('%x(%s)' % (lo + a_, oline(b_)) for a_, b_ in js[:6]) + (' ..' if len(js) > 6 else '')))
    # jumper-into-jumper merges in ours (MATCHING_GUIDE 3.7): a jmp into the middle of another
    # statement's code whose block then leaves by jmp, not by falling into a label
    cidx = {x[0]: n for n, x in enumerate(C)}
    clabels = {x[3] for x in C if x[3] is not None}
    jj = []
    for x in C:
        if x[1] != 'jmp L' or x[3] not in cidx or cidx[x[3]] == 0:
            continue
        k = cidx[x[3]]
        p = C[k - 1]
        if p[1].startswith('j') or p[1].startswith('ret') or src.get(p[0]) != src.get(x[3]) \
                or src.get(x[0]) == src.get(x[3]) or x[3] not in src:
            continue
        for y in C[k:]:
            if y[0] != x[3] and y[0] in clabels:
                break
            if y[1].startswith('jmp') or y[1].startswith('ret'):
                if y[1].startswith('jmp'):
                    jj.append((x[0], x[3]))
                break
    cnt['jj'] = len(jj)
    hosts = host_report(C, O, lo, src, i2o, o2i, iline, text.split('\n'))
    cnt['host'] = sum(1 for l in hosts if l.startswith('  HOST'))
    out += hosts
    if jj:
        out.append('  JJ   ours jumper-into-jumper merges: ' + ' '.join(
            '%x(%s)->%x(%s)' % (a_, oline(a_), b_, oline(b_)) for a_, b_ in jj[:8]))
    head = 'TS %08X %-34s diff=%d jt=%d img-merges=%d ours-merges=%d jmp-target=%d jj=%d dups img/ours=%d/%d host=%d' % (
        addr, sym[:34], r['diff'], r['jt'], cnt['img_merges'], cnt['ours_merges'], cnt['jt_jmp'], cnt['jj'],
        cnt['img_dups'], cnt['ours_dups'], cnt['host'])
    cnt.update(diff=r['diff'], sym=sym)
    return [head] + out, cnt


def markers(text, all_=False):
    return [int(m.group(1), 16) for m in re.finditer(
        r'// ORIGINAL: 0x([0-9A-F]{8}) \S+ 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)', text)
        if all_ or 'BYTE_EXACT' not in m.group(2)]


def run_unit(unit, addrs, src, pfx, kmin, agree, quiet):
    text = open(src, encoding='latin-1').read()
    c, cod = compile_listing(unit, src, pfx)
    if c is None:
        return [('COMPILE FAIL ' + unit, None)]
    res = []
    for a in addrs:
        try:
            res.append(scan(unit, a, text, c, cod, kmin, agree, quiet))
        except Exception as e:  # one bad marker must not stop a tree pass
            res.append((['TS %08X error %s' % (a, e)], None))
    return res


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('unit', nargs='?')
    ap.add_argument('addr', nargs='?')
    ap.add_argument('--all', action='store_true')
    ap.add_argument('--tree', nargs='*')
    ap.add_argument('--src')
    ap.add_argument('--pfx', default=os.environ.get('FE_PFX_BASE', '21'))
    ap.add_argument('--min', type=int, default=2)
    ap.add_argument('--agree', action='store_true')
    ap.add_argument('--json')
    ap.add_argument('--hosts', action='store_true', help='--tree: print each HOST block')
    a = ap.parse_args()
    if a.tree is not None:
        cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
        units = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/')]
        if a.tree:
            units = [u for u in units if u in a.tree or os.path.basename(u) in a.tree]
        work = []
        for u in units:
            try:
                t = open(fe.R + '/' + u, encoding='latin-1').read()
            except OSError:
                continue
            ad = markers(t)
            if ad and '// compile_context' not in t:
                work.append((u, ad))
        jobs = int(os.environ.get('FE_JOBS', '5'))
        fe.compile_cmd(units[0])  # load compile_commands before the lanes start (lazy global)
        base = int(a.pfx)

        def lane(k):
            r = []
            for i in range(k, len(work), jobs):
                u, ad = work[i]
                for lines, cnt in run_unit(u, ad, fe.R + '/' + u, str(base + k), a.min, False, True):
                    if cnt:
                        cnt.update(unit=u, addr='%s' % lines[0][3:11], lines=lines)
                        r.append(cnt)
            return r
        allr = []
        with ThreadPoolExecutor(jobs) as ex:
            for o in ex.map(lane, range(jobs)):
                allr += o
        hit = [x for x in allr if x['img_merges'] or x['ours_merges'] or x['jt_jmp'] or x['jj'] or x['host']]
        hit.sort(key=lambda x: (-x['host'], -(x['img_merges'] + x['ours_merges'] + x['jt_jmp'])))
        for x in hit:
            print('%s %-28s %-30s diff=%-5d img-merges=%d ours-merges=%d jmp-target=%d jj=%d host=%d' % (
                x['addr'], x['unit'][4:], x['sym'][:30], x['diff'], x['img_merges'], x['ours_merges'], x['jt_jmp'],
                x['jj'], x['host']))
            if a.hosts:
                for l in x['lines']:
                    if l.startswith(('  HOST', '       ')):
                        print(l)
        print('functions', len(allr), 'with tail/jump disagreements', len(hit),
              'with a join-host disagreement', sum(1 for x in hit if x['host']))
        if a.json:
            json.dump(hit, open(a.json, 'w'), indent=0)
        return
    unit = a.unit if a.unit.startswith('src/') else 'src/' + a.unit
    src = os.path.abspath(a.src) if a.src else fe.R + '/' + unit
    text = open(src, encoding='latin-1').read()
    addrs = markers(text) if a.all else [int(x, 16) for x in a.addr.split(',')]
    for lines, cnt in run_unit(unit, addrs, src, a.pfx, a.min, a.agree, False):
        print('\n'.join(lines) if isinstance(lines, list) else lines)


if __name__ == '__main__':
    main()
