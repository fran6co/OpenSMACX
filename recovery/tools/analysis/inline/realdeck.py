"""realdeck.py: evaluate whole-file variants of a real unit and report, per variant, the function's
call sequence differences against the image (callseq-style) plus fe's diff count.
  run(unit, addr, [(key, text)], pfx_base=21, jobs=5) -> [(key, diff, exact, [(callee, image_count, our_count)...])]"""
import os, sys, re, collections
from concurrent.futures import ThreadPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
sys.path.insert(0, os.path.dirname(os.path.dirname(HERE)))
import fe
import inlinemap as im


def evaluate(unit, addr, key, text, pfx):
    scr = os.environ.get('FE_SCRATCH', fe.R + '/build/recovery/fe') + '/inline'
    os.makedirs(scr, exist_ok=True)
    path = '%s/rd_%s.cpp' % (scr, pfx)
    open(path, 'w', encoding='latin-1').write(text)
    c, err = fe.compile_unit(unit, path, pfx)
    if c is None:
        return key, None, False, err
    r = fe.evaluate(c, text, addr)
    sym, lo, hi = fe.ident(text, addr)
    img = collections.Counter(n for _, n in im.img_stream(lo, hi))
    ours = collections.Counter(n for _, n in im.our_stream(c, sym))
    d = sorted((n, img[n], ours[n]) for n in set(img) | set(ours) if img[n] != ours[n])
    return key, r.get('diff'), r.get('exact'), d


def run(unit, addr, deck, pfx_base=int(os.environ.get('FE_PFX_BASE', '21')), jobs=int(os.environ.get('FE_JOBS', '5'))):
    fe.compile_cmd(unit)  # fill the command cache before the lanes share it
    lanes = [deck[i::jobs] for i in range(jobs)]

    def lane(k):
        return [evaluate(unit, addr, key, text, pfx_base + k) for key, text in lanes[k]]
    out = []
    with ThreadPoolExecutor(jobs) as ex:
        for o in ex.map(lane, range(jobs)):
            out += o
    order = {k: i for i, (k, _) in enumerate(deck)}
    out.sort(key=lambda r: order[r[0]])
    return out
