#!/usr/bin/env python3
"""okc_deck.py: case-order variants of Console::on_key_click's final switch (0x005178C0),
measured with fe in the real unit. Usage: uv run python okc_deck.py [keys...]"""
import sys, os, re, itertools
R = os.path.abspath(os.path.dirname(os.path.abspath(__file__)) + '/../../../../..')
sys.path.insert(0, R + '/recovery/tools'); import perm
SRC = open(R + '/src/console.cpp', encoding='latin-1').read()
lo = SRC.index('        case 0x10026:\n        case 0x10068: {')
hi = SRC.index('        }\n    }\n\n    if (ConsoleGlobal.cursor_active_ != 0) {', lo)
pans = SRC[lo:hi]
blocks = re.split(r'(?=        case 0x1002[5-8]:\n)', pans)
blocks = [b for b in blocks if b]
P = {re.match(r'        case (0x\w+):', b).group(1): b for b in blocks}
dlo = SRC.index('        case 0x61:\n        case 0x23:')
dhi = SRC.index('        case 0x21:\n        case 0x69:')
head = SRC[dlo:dhi]          # case 0x61 + move_direction body
stubs = SRC[dhi:lo]
def build(order, head_pos='first'):
    p = ''.join(P[k] for k in order)
    if head_pos == 'first': body = head + stubs + p
    elif head_pos == 'last': body = stubs + p + head
    elif head_pos == 'afterstubs_last': body = stubs + head + p
    return SRC[:dlo] + body + SRC[hi:]
DECK = []
for order in itertools.permutations(['0x10026', '0x10028', '0x10025', '0x10027']):
    for hp in ('first', 'last'):
        DECK.append(('%s:%s' % (hp, '-'.join(o[-2:] for o in order)), build(order, hp)))
if __name__ == '__main__':
    os.environ.setdefault('FE_PFX_BASE', '14')
    res = perm.run_deck('src/console.cpp', '0x005178C0', DECK, jobs=int(os.environ.get('FE_JOBS', '5')))
    for w, d, ex, k in res[:20]: print(d, ex, k)
