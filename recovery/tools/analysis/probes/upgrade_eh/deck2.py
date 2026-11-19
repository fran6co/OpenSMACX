import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import deck as D
S = D.SRC; F = D.FN
head, body = S[:F], S[F:]
end = body.index('\n}\n') + 3
fb, tail = body[:end], body[end:]
def popp3(t, which=(0, 1, 2, 3)):
    ms = list(re.finditer(r'popp\(ScriptPopupFile,\n\s*(\(VehPrototypes\[choice\]\.flags & 4\) \? "\w+" : "\w+"), 0,', t))
    for k in reversed(range(len(ms))):
        if k in which:
            m = ms[k]; t = t[:m.start()] + 'popp(' + m.group(1) + ',' + t[m.end():]
    return t
V = {'base': S}
for w in [(0, 1, 2, 3), (0, 1), (2, 3), (0,), (1,)]:
    V['popp3_' + ''.join(map(str, w))] = head + popp3(fb, w) + tail
D.run(V, sys.argv[1:] or None)
