#!/usr/bin/env python3
"""refscan.py [unit ...] [--addr A[,A]] [--claimed] [--unbound N] [--json OUT] [--jobs N]
Reference scan: the fe evaluator masks every relocated field, so a function that names the wrong
global, the wrong member offset of a global aggregate (`G.b` for `G.a`), the wrong constant index
(`A[3]` for `A[2]`), or the wrong string literal reads exact by fe while the strict comparator
(`recover measure`) calls it SHAPE_EXACT ("relocated field differs from its bound target").

For every marker that fe reads exact (relocations masked), this compiles the unit once (fe.py's raw
compile), takes the function COMDAT's DIR32/REL32 relocations and asks the package's own strict check,
`match_recovery.source.asm.reference_mismatches`, with the package's source-bound targets
(`measurement_reference_targets`: function mappings, RECOVERY_OBJECT bindings, component origins;
names bound to several addresses stay masked) and the image reader (string literal content). Prints,
per function, each field whose bound target plus addend is not the image's, with the image address's
bound symbol (and offset into it) when one source binding covers it.

  REF <addr> <unit> <sym> [claimed]
     +0x<off> <our symbol>+<addend>: source 0x<ours>, image 0x<img>  -> image names <symbol>+<k>

--unbound N lists the N symbols most often left masked (no single source binding: the scan's blind
spot; section-local $L labels excluded).
Default: non-BYTE_EXACT markers; --claimed adds the BYTE_EXACT ones (broken references in claims).
Screen only; `recover measure` is the authority. The main COMDAT only (an EH function's cold
cleanup span is not asked) and RECOVERY_FRAGMENT markers are skipped. A `// compile_context` marker
is measured in that unit's compile."""
import sys, os, re, json, argparse, bisect, pathlib
from types import SimpleNamespace
from concurrent.futures import ThreadPoolExecutor
T = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, T)
import fe  # noqa: E402
from match_recovery.source.asm import (CodeReference, reference_mismatches, image_reader,  # noqa: E402
                                       string_literal_bytes)
from match_recovery.source_selection import measurement_reference_targets  # noqa: E402
from match_recovery.measurement import Measurement  # noqa: E402

R = fe.R
EXE = fe.MAIN + '/.opensmacx/game/terranx_original.exe'
CHECKED = []   # fe-exact markers asked
UNBOUND = {}   # masked fields: symbols with no single source binding
MARK = re.compile(r'// ORIGINAL: (0x[0-9A-F]{8}) \S+ 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)')


def references(c, sym):
    """CodeReference rows of COMDAT `sym` in Coff `c` (as asm._subject_references reads them)."""
    s = c.function(sym)
    sec = c.sections[s.section - 1]
    rows = []
    for va, index, kind in sec.relocations:
        if kind not in (0x0006, 0x0014):
            continue
        target = c.symbols.get(index)
        if target is None:
            continue
        addend = int.from_bytes(sec.data[va:va + 4], 'little', signed=True)
        rows.append(CodeReference(va, kind == 0x0014, target.name, target.storage, addend,
                                  string_literal_bytes(c, target, addend)))
    return sec.data, tuple(rows)


class Namer:
    """Image address -> nearest source-bound symbol at or below it (for the report)."""
    def __init__(self, targets):
        pairs = sorted((a, n) for n, a in targets.external.items())
        self.addrs = [a for a, _ in pairs]
        self.names = [n for _, n in pairs]

    def __call__(self, address):
        i = bisect.bisect_right(self.addrs, address) - 1
        if i < 0 or address - self.addrs[i] > 0x10000:
            return '?'
        k = address - self.addrs[i]
        return self.names[i] + ('+0x%X' % k if k else '')


def scan_unit(unit, addrs, claimed, pfx, targets, namer, image):
    text = open(R + '/' + unit, encoding='latin-1').read()
    c, err = fe.compile_unit(unit, R + '/' + unit, pfx)
    if c is None:
        return [dict(unit=unit, addr='?', error='compile failed: ' + (err or '')[:200])]
    out, contexts = [], {}
    host = pathlib.Path(R + '/' + unit).resolve()
    for addr in addrs:
        if re.search(r'// ORIGINAL: 0x%08X [^\n]*\n// RECOVERY_FRAGMENT' % addr, text):
            continue
        m = re.search(r'// ORIGINAL: 0x%08X [^\n]*\n((?://(?! ORIGINAL)[^\n]*\n)*)' % addr, text)
        cc_ = c
        mc = m and re.search(r'// compile_context (\S+)', m.group(1))
        if mc and mc.group(1) != unit:   # measured in that unit's compile, as fe and recover measure do
            if mc.group(1) not in contexts:
                contexts[mc.group(1)] = fe.compile_unit(mc.group(1), R + '/' + mc.group(1), pfx)[0]
            cc_ = contexts[mc.group(1)]
            if cc_ is None:
                continue
        r = fe.evaluate(cc_, text, addr)
        if 'error' in r or not r['exact']:
            continue
        try:
            code, refs = references(cc_, r['sym'])
        except Exception:
            continue
        CHECKED.append(addr)
        lo = r['lo']
        original = SimpleNamespace(code=fe.imgbytes(lo, lo + len(code)), base=lo)
        compiled = SimpleNamespace(references=refs)
        wrong = reference_mismatches(original, compiled, targets, host, image)
        for ref in refs:
            if ref.offset + 4 <= len(original.code) and ref.literal is None \
                    and targets.address(ref, host) is None and not ref.symbol.startswith('$L'):
                UNBOUND[ref.symbol] = UNBOUND.get(ref.symbol, 0) + 1
        if not wrong:
            continue
        rows = []
        for w in wrong:
            mi = re.search(r'image 0x([0-9A-F]{8})$', w)
            rows.append(w + ('  -> image names ' + namer(int(mi.group(1), 16)) if mi else ''))
        out.append(dict(unit=unit, addr='%08X' % addr, sym=r['sym'], claimed=addr in claimed, sites=rows))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('units', nargs='*'); ap.add_argument('--addr', default='')
    ap.add_argument('--claimed', action='store_true'); ap.add_argument('--unbound', type=int, default=0); ap.add_argument('--json')
    ap.add_argument('--jobs', type=int, default=int(os.environ.get('FE_JOBS', '5')))
    a = ap.parse_args()
    base = int(os.environ.get('FE_PFX_BASE', '21'))
    want = {int(x, 16) for x in a.addr.split(',') if x}
    cc = json.load(open(fe.MAIN + '/build/compile_commands.json'))
    units = [e['file'][len(fe.MAIN) + 1:] for e in cc if e['file'].startswith(fe.MAIN + '/src/')]
    if a.units:
        units = [u for u in units if u in a.units or os.path.basename(u) in a.units]
    work = []
    for u in units:
        try:
            t = open(R + '/' + u, encoding='latin-1').read()
        except OSError:
            continue
        claimed = {int(m.group(1), 16) for m in MARK.finditer(t) if 'BYTE_EXACT' in m.group(2)}
        addrs = [int(m.group(1), 16) for m in MARK.finditer(t)
                 if (a.claimed or a.addr or 'BYTE_EXACT' not in m.group(2))
                 and (not want or int(m.group(1), 16) in want)]
        if addrs:
            work.append((u, addrs, claimed))
    root = pathlib.Path(R).resolve()
    targets = measurement_reference_targets(root, Measurement.read(root))
    namer, image = Namer(targets), image_reader(EXE)
    fe.compile_cmd(work[0][0])   # fill fe's command cache before the lanes share it

    def lane(k):
        res = []
        for i in range(k, len(work), a.jobs):
            u, addrs, claimed = work[i]
            res += scan_unit(u, addrs, claimed, base + k, targets, namer, image)
        return res
    found = []
    with ThreadPoolExecutor(a.jobs) as ex:
        for part in ex.map(lane, range(a.jobs)):
            found += part
    found.sort(key=lambda x: (x['unit'], x['addr']))
    for f in found:
        if 'error' in f:
            print('ERR', f['unit'], f['error']); continue
        print('REF', f['addr'], f['unit'], f['sym'][:60], 'claimed' if f['claimed'] else '')
        for s in f['sites']:
            print('    ', s)
    if a.unbound:
        for name, n in sorted(UNBOUND.items(), key=lambda kv: -kv[1])[:a.unbound]:
            print('UNBOUND %5d %s' % (n, name))
    print('masked (unbound) fields', sum(UNBOUND.values()), 'in', len(UNBOUND), 'symbols')
    print('fe-exact markers checked', len(CHECKED))
    print('units', len(work), 'functions with wrong references', sum('sites' in f for f in found))
    if a.json:
        json.dump(found, open(a.json, 'w'), indent=1)


if __name__ == '__main__':
    main()
