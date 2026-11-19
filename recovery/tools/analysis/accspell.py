#!/usr/bin/env python3
"""accspell.py: raw field tests against header accessors (MATCHING_GUIDE 2.8 "late free helpers", order_veh round 15).

  accspell.py list   [--rungs rungs.json] [--addr A,..]   per rung-1/2 function: accessor-able raw sites (rule, line,
                                                         index of the preceding `return`) and the inlinemap rows
                                                         that differ (the refused/expanded sites)
  accspell.py deck   <unit> <0xADDR> [--rules a,b] [--write VARIANT]
                                                         variants BASE, ALL, ALL:<rule>, AFTER_R<k> (every site after
                                                         the k-th return statement), ONLY_R<k> (sites between return
                                                         k and k+1); key per variant = (EH distance, named call
                                                         distance, inlinemap site distance, frame equal, fe rows)
  accspell.py census [--rungs rungs.json]               exact functions: raw against accessor spellings of the same
                                                         test, per unit and family

The rewritten sites are accessor-for-raw only (`*_fn` rules, the spelling the original's free inlines would expand
to exactly); `--rules` may add the reverse rules of spellsweep.py.  A local `b = 1 << f;` bit used against
human_mask_/alive_mask_ is rewritten to is_human(f)/is_alive(f) when `f` is a plain name.  Prefixes FE_PFX_BASE..
(FE_JOBS lanes).  Screen only: fe masks relocations; confirm with `recover measure`."""
import sys, os, re, json, collections, argparse
from concurrent.futures import ThreadPoolExecutor
import threading
LOCK = threading.Lock()
H = os.path.dirname(os.path.abspath(__file__))
T = os.path.dirname(H)
sys.path.insert(0, T)
sys.path.insert(0, H)
os.chdir(os.path.dirname(os.path.dirname(T)))
import fe, callees, framescan, ehcensus, inlinemap  # noqa: E402
import spellsweep  # noqa: E402
R = fe.R
M = 'GameDataStateStorage.fields_.lock_masks_.bytes_.'
BR = r'((?:[^\[\]]|\[(?:[^\[\]]|\[[^\[\]]*\])*\])+?)'          # subscript text, two nesting levels
AR = r'((?:[^(),]|\((?:[^()]|\([^()]*\))*\))+)'                  # one call argument
K = r'(\w+|\((?:[^()]|\([^()]*\))*\))'                           # mask operand
END = r'(?=\s*(?:\)|&&|\|\||\?|;|,|:|==|!=))'
VEH = r'Vehs\[' + BR + r'\]\.proto_id'


def _proto(field, table, fn, pfn):
    v = (table + r'\[VehPrototypes\[' + VEH + r'\]\.' + field + r'_id\]\.', fn)
    p = (table + r'\[VehPrototypes\[' + BR + r'\]\.' + field + r'_id\]\.', pfn)
    return v, p


RULES = collections.OrderedDict()
RULES['treaty_fn'] = (r'(?<![\w.>])PlayersData\[' + BR + r'\]\.diplo_treaties\[' + BR + r'\] & ' + K + END,
                      lambda m, b: 'has_treaty(%s, %s, %s)' % (m.group(1), m.group(2), m.group(3)))
RULES['agenda_fn'] = (r'(?<![\w.>])PlayersData\[' + BR + r'\]\.diplo_agenda\[' + BR + r'\] & ' + K + END,
                      lambda m, b: 'has_agenda(%s, %s, %s)' % (m.group(1), m.group(2), m.group(3)))
for nm, mask in (('human', 'human_mask_'), ('alive', 'alive_mask_')):
    fn = 'is_' + nm
    RULES[nm + '_fn'] = (re.escape(M) + mask + r' & \(1u? << ' + AR + r'\)',
                         (lambda f: lambda m, b: '%s(%s)' % (f, m.group(1)))(fn))
    RULES[nm + '_fn2'] = (r'\(1u? << ' + AR + r'\) & ' + re.escape(M) + mask + r'\b',
                          (lambda f: lambda m, b: '%s(%s)' % (f, m.group(1)))(fn))
    # local bit: b & mask / mask & b with `b = 1 << f` in the function
    RULES[nm + '_bit'] = (r'(?<![\w.>])(?:(\w+) & ' + re.escape(M) + mask + r'\b|' + re.escape(M) + mask + r' & (\w+)\b(?!\s*[\[(.]))',
                          (lambda f: lambda m, b: (f + '(%s)' % b[m.group(1) or m.group(2)]) if (m.group(1) or m.group(2)) in b else None)(fn))
RULES['triad_fn'] = (r'(?<![\w.>])Chassis\[VehPrototypes\[' + VEH + r'\]\.chassis_id\]\.triad\b(?!\s*=[^=])',
                     lambda m, b: 'get_triad(%s)' % m.group(1))
RULES['ptriad_fn'] = (r'(?<![\w.>])Chassis\[VehPrototypes\[(?!Vehs\[)' + BR + r'\]\.chassis_id\]\.triad\b(?!\s*=[^=])',
                      lambda m, b: 'get_proto_triad(%s)' % m.group(1))
RULES['missile_fn'] = (r'(?<![\w.>])Chassis\[VehPrototypes\[' + VEH + r'\]\.chassis_id\]\.missile\b(?!\s*=[^=])',
                       lambda m, b: 'is_missile(%s)' % m.group(1))
RULES['off_fn'] = (r'(?<![\w.>])Weapon\[VehPrototypes\[' + VEH + r'\]\.weapon_id\]\.offense_rating\b(?!\s*=[^=])',
                   lambda m, b: 'get_offense_rating(%s)' % m.group(1))
RULES['def_fn'] = (r'(?<![\w.>])Armor\[VehPrototypes\[' + VEH + r'\]\.armor_id\]\.defense_rating\b(?!\s*=[^=])',
                   lambda m, b: 'get_defense_rating(%s)' % m.group(1))
ML = r'(?<![\w.>])map_loc\(' + AR + r', ' + AR + r'\)->'
RULES['temp_fn'] = (ML + r'climate & 7\b', lambda m, b: 'temp_at(%s, %s)' % (m.group(1), m.group(2)))
RULES['alt_fn'] = (ML + r'climate & 0xE0\b', lambda m, b: 'altitude_at(%s, %s)' % (m.group(1), m.group(2)))
RULES['owner_fn'] = (ML + r'val2 & 0xF\b', lambda m, b: 'owner_at(%s, %s)' % (m.group(1), m.group(2)))
RULES['code_fn'] = (ML + r'bit2 >> 24\b', lambda m, b: 'code_at(%s, %s)' % (m.group(1), m.group(2)))
RULES['ocean_fn'] = (r'\(' + ML + r'climate & ~0x1F\) < ALT_BIT_SHORE_LINE', lambda m, b: 'is_ocean(%s, %s)' % (m.group(1), m.group(2)))
FAMILY = lambda r: re.sub(r'_(fn2?|bit)$', '', r)
DEFAULT = list(RULES)
# accessor spellings, for the census
ACC = collections.OrderedDict([('treaty', r'\bhas_treaty\('), ('agenda', r'\bhas_agenda\('), ('human', r'\bis_human\('),
                               ('alive', r'\bis_alive\('), ('triad', r'\bget_triad\('), ('ptriad', r'\bget_proto_triad\('),
                               ('missile', r'\bis_missile\('), ('off', r'\bget_offense_rating\('),
                               ('def', r'\bget_defense_rating\('), ('temp', r'\btemp_at\('), ('alt', r'\baltitude_at\('),
                               ('owner', r'\bowner_at\('), ('code', r'\bcode_at\('), ('ocean', r'\bis_ocean\(')])


def bits(code):
    """local `b = 1 << f;` (f a plain name, b not reassigned otherwise)"""
    d = {}
    for m in re.finditer(r'\b(\w+) = 1u? << (\w+);', code):
        d[m.group(1)] = m.group(2)
    for k in list(d):
        if len(re.findall(r'\b%s\s*(?:=[^=]|\+=|-=|\|=|&=|<<=|>>=|\+\+|--)' % k, code)) > 1:
            del d[k]
    return d


def sites(t, addr, rules):
    s, e = spellsweep.span(t, addr)
    body = t[s:e]
    code = re.sub(r'//[^\n]*', lambda m: ' ' * len(m.group(0)), body)
    code = re.sub(r'"(?:[^"\\]|\\.)*"', lambda m: '"' + ' ' * (len(m.group(0)) - 2) + '"', code)
    b = bits(code)
    out = []
    for r in rules:
        rx, rep = RULES[r] if r in RULES else spellsweep.RULES[r]
        for m in re.finditer(rx, code):
            new = rep(m, b) if r in RULES else rep(m)
            if new is not None:
                out.append((s + m.start(), s + m.end(), new, r))
    out.sort(key=lambda x: (x[0], -x[1]))
    keep, last = [], -1
    for x in out:
        if x[0] >= last:
            keep.append(x)
            last = x[1]
    return keep


def returns(t, addr):
    s, e = spellsweep.span(t, addr)
    code = re.sub(r'//[^\n]*', lambda m: ' ' * len(m.group(0)), t[s:e])
    return [s + m.start() for m in re.finditer(r'\breturn\b', code)]


def apply(t, sel):
    for a, b, rep, r in sorted(sel, reverse=True):
        t = t[:a] + rep + t[b:]
    if any(FAMILY(x[3]) in ('treaty', 'agenda') for x in sel) and 'faction_inline.h' not in t:
        # faction.h declares has_treaty out of line; the inline lives in faction_inline.h
        i = [m.end() for m in re.finditer(r'#include [^\n]*\n', t)][-1]
        t = t[:i] + '#include "faction_inline.h"\n' + t[i:]
    return t


def ehdist(a, b):
    if a == b:
        return 0
    if not isinstance(a, dict) or not isinstance(b, dict):
        return 99
    ga, gb = ehgroups(a['par']), ehgroups(b['par'])
    n = max(len(ga), len(gb))
    return abs(a['ms'] - b['ms']) + sum(1 for i in range(n) if (ga[i] if i < len(ga) else 0) != (gb[i] if i < len(gb) else 0)) + \
        (a['nt'] != b['nt']) + (1 if (ga == gb and a != b) else 0)


def ehgroups(par):
    g = []
    for v in par:
        if v == -1 or not g:
            g.append(1)
        else:
            g[-1] += 1
    return g


def measure(c, text, addr, im=True):
    sym, lo, hi = fe.ident(text, addr)
    r = fe.evaluate(c, text, addr)
    if 'error' in r:
        return None
    ei, eo = ehcensus.img_fi(lo), ehcensus.our_fi(c, sym)
    d = callees.diff(c, text, addr)
    cd = {}
    if d:
        ci, co = d
        cd = {k: (ci[k], co[k]) for k in set(ci) | set(co) if ci[k] != co[k]}
    s = c.function(sym)
    sec = c.sections[s.section - 1]
    fi = framescan.frame(framescan.insns(fe.imgbytes(lo, hi), lo))[:3]
    fo = framescan.frame(framescan.insns(bytes(sec.data), 0))[:3]
    sd = 0
    if im:
        try:
            with LOCK:  # analyse() keeps module-global signature caches
                per = inlinemap.analyse(c, sym, lo, hi)[3]
            sd = sum(1 for n, ss in per.items() for q in ss if (q[0] and q[0][2]) != (q[1] and q[1][2]))
        except Exception:
            sd = -1
    return dict(eh=ehdist(ei, eo), ehs=[ehgroups(ei['par']) if isinstance(ei, dict) else ei,
                                        ehgroups(eo['par']) if isinstance(eo, dict) else eo],
                calls=sum(abs(a - b) for a, b in cd.values()), cd=cd, sites=sd, frame=fi == fo,
                fr=[fi[0], fo[0]], rows=0 if r['exact'] else r['diff'])


def key(v):
    return (9e9,) if v is None else (v['eh'], v['calls'], v['sites'], not v['frame'], v['rows'])


def evaluate(unit, addr, deck, im=True):
    base = int(os.environ.get('FE_PFX_BASE', '2'))
    jobs = int(os.environ.get('FE_JOBS', '5'))
    fe.compile_cmd(unit)
    callees.addr2sym()  # lazy global caches: fill them before the lanes start (a race names image calls sub_*)
    callees.iat()
    res = {}

    def one(k):
        for i in range(k, len(deck), jobs):
            name, text = deck[i]
            path = '%s/build/recovery/fe/acc%d.cpp' % (R, k)
            open(path, 'w', encoding='latin-1').write(text)
            c, err = fe.compile_unit(unit, path, str(base + k))
            try:
                res[name] = None if c is None else measure(c, text, addr, im)
            except Exception:
                res[name] = None
    with ThreadPoolExecutor(jobs) as ex:
        list(ex.map(one, range(jobs)))
    return res


def build_deck(src, addr, rules):
    S = sites(src, addr, rules)
    rets = returns(src, addr)
    deck = [('BASE', src, [])]
    if S:
        deck.append(('ALL', apply(src, S), S))
        for r in sorted({FAMILY(x[3]) for x in S}):
            g = [x for x in S if FAMILY(x[3]) == r]
            if len(g) < len(S):
                deck.append(('ALL:' + r, apply(src, g), g))
        for k, p in enumerate(rets):
            g = [x for x in S if x[0] > p]
            if g and len(g) < len(S) and not any(d[2] == g for d in deck):
                deck.append(('AFTER_R%d' % (k + 1), apply(src, g), g))
            nxt = rets[k + 1] if k + 1 < len(rets) else 1 << 60
            g = [x for x in S if p < x[0] < nxt]
            if g and not any(d[2] == g for d in deck):
                deck.append(('ONLY_R%d' % (k + 1), apply(src, g), g))
        for k in range(1, len(S)):  # suffixes: every site from the k-th on (after a refused site)
            g = S[k:]
            if not any(d[2] == g for d in deck):
                deck.append(('FROM_S%d' % (k + 1), apply(src, g), g))
    return S, rets, deck


def fmt(v):
    if v is None:
        return 'FAIL'
    cd = ' '.join('%s:%d/%d' % (re.sub(r'^\?+', '', k)[:28], a, b) for k, (a, b) in sorted(v['cd'].items()))
    return 'eh=%d calls=%d sites=%d frame=%s rows=%d  %s' % (v['eh'], v['calls'], v['sites'], 'ok' if v['frame'] else '%x/%x' % tuple(v['fr']), v['rows'], cd)


def cmd_deck(a):
    unit, addr = a.unit, int(a.addr, 16)
    rules = a.rules.split(',') if a.rules else DEFAULT
    src = open(R + '/' + unit, encoding='latin-1').read()
    S, rets, deck = build_deck(src, addr, rules)
    print('%s %s: %d sites, %d returns' % (unit, a.addr, len(S), len(rets)))
    for x in S:
        print('  L%-5d %-10s after R%-3d %s' % (src.count('\n', 0, x[0]) + 1, x[3], sum(1 for p in rets if p < x[0]), x[2]))
    if a.write:
        sel = [d for d in deck if d[0] == a.write][0]
        open(R + '/' + unit, 'w', encoding='latin-1').write(sel[1])
        print('wrote', a.write)
        return
    res = evaluate(unit, addr, [(n, t) for n, t, _ in deck], not a.no_im)
    b = res['BASE']
    for n, t, g in deck:
        v = res.get(n)
        print('%-12s n=%-3d %s%s' % (n, len(g), fmt(v), '  <' if v and b and key(v) < key(b) else ''))
    if a.json:
        json.dump({n: res.get(n) for n, t, g in deck}, open(a.json, 'w'), default=str)


def srcfile(unit):
    return open(R + '/' + unit, encoding='latin-1').read()


def cmd_list(a):
    rs = json.load(open(a.rungs))
    sel = [r for r in rs if not r.get('exact') and r.get('rung') in (1, 2) and 'error' not in r]
    if a.addr:
        want = {x.upper().replace('0X', '').zfill(8) for x in a.addr.split(',')}
        sel = [r for r in sel if r['addr'] in want]
    for r in sel:
        src = srcfile(r['unit'])
        addr = int(r['addr'], 16)
        S = sites(src, addr, DEFAULT)
        rets = returns(src, addr)
        fam = collections.Counter(FAMILY(x[3]) for x in S)
        print('%s %s %s rung %d rows %d: %d sites %s, %d returns' % (r['unit'], r['addr'], r['sym'][:40], r['rung'], r['diff'],
                                                                     len(S), dict(fam), len(rets)))
        for x in S:
            print('    L%-5d %-10s after R%d' % (src.count('\n', 0, x[0]) + 1, x[3], sum(1 for p in rets if p < x[0])))


def cmd_census(a):
    rs = json.load(open(a.rungs))
    ex = collections.defaultdict(list)
    for r in rs:
        if r.get('exact'):
            ex[r['unit']].append(int(r['addr'], 16))
    tot = collections.defaultdict(lambda: [0, 0])
    per = collections.defaultdict(lambda: collections.defaultdict(lambda: [0, 0, 0]))
    for unit, addrs in sorted(ex.items()):
        try:
            src = srcfile(unit)
        except Exception:
            continue
        for addr in addrs:
            try:
                s, e = spellsweep.span(src, addr)
            except Exception:
                continue
            code = re.sub(r'//[^\n]*', '', src[s:e])
            raw = collections.Counter(FAMILY(x[3]) for x in sites(src, addr, DEFAULT))
            for f, rx in ACC.items():
                n_acc = len(re.findall(rx, code))
                n_raw = raw.get(f, 0)
                if n_acc or n_raw:
                    tot[f][0] += n_raw
                    tot[f][1] += n_acc
                    p = per[f][unit]
                    p[0] += n_raw
                    p[1] += n_acc
                    p[2] += 1 if (n_raw and n_acc) else 0
    for f in ACC:
        if f not in tot:
            continue
        print('%-8s raw %4d accessor %4d' % (f, tot[f][0], tot[f][1]))
        for unit, (nr, na, mixed) in sorted(per[f].items(), key=lambda kv: -(kv[1][0] + kv[1][1])):
            print('    %-34s raw %3d acc %3d%s' % (unit, nr, na, '  mixed in %d fn' % mixed if mixed else ''))


def main():
    ap = argparse.ArgumentParser()
    sp = ap.add_subparsers(dest='cmd')
    p = sp.add_parser('list')
    p.add_argument('--rungs', default=R + '/build/recovery/rungs/rungs.json')
    p.add_argument('--addr')
    p = sp.add_parser('deck')
    p.add_argument('unit')
    p.add_argument('addr')
    p.add_argument('--rules')
    p.add_argument('--write')
    p.add_argument('--json')
    p.add_argument('--no-im', action='store_true', help='skip the inlinemap site distance (giants)')
    p = sp.add_parser('census')
    p.add_argument('--rungs', default=R + '/build/recovery/rungs/rungs.json')
    a = ap.parse_args()
    {'list': cmd_list, 'deck': cmd_deck, 'census': cmd_census}[a.cmd](a)


if __name__ == '__main__':
    main()
