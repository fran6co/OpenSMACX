import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import okc_deck as D, okc_layout as L
R = D.R; d = R + '/build/recovery/fe/switchtree'; os.makedirs(d, exist_ok=True)
for order in sys.argv[1:]:
    o = ['0x100' + x for x in order.split('-')]
    p = '%s/ord.cpp' % d; open(p, 'w', encoding='latin-1').write(D.build(o, 'first'))
    print('%-12s' % order, ' '.join(L.marks(L.ours(p))))
print('img         ', ' '.join(L.marks(L.image())))
