# sitedeck.py <unit> <addr> <rules> [--min N] [--write] : per-site toggles inside one function, greedy rounds.
# rules (comma list): lraw/lget = label_get(i) <-> StringTable.get(Labels.strings_ptr[i]); traw/tget = table_text(x) <-> StringTable.get(x).
# Each round fes every site of every rule alone, keeps the best when it gains --min rows (default 3), and repeats.
import sys,os,re
R=os.getcwd(); sys.path.insert(0,R+'/recovery/tools'); import perm,fe
# prefixes: FE_PFX_BASE .. FE_PFX_BASE+FE_JOBS-1 (perm.run_deck)
unit,addr=sys.argv[1],sys.argv[2]; rules=sys.argv[3].split(',')
mn=int(sys.argv[sys.argv.index('--min')+1]) if '--min' in sys.argv else 3
RX={
 'lraw':(r'\blabel_get\(([^()]+)\)', r'StringTable.get(Labels.strings_ptr[\1])'),
 'lget':(r'StringTable\.get\(Labels\.strings_ptr\[([^\]]+)\]\)', r'label_get(\1)'),
 'traw':(r'\btable_text\(([^()]+(?:\([^()]*\))?[^()]*)\)', r'StringTable.get(\1)'),
 'tget':(r'StringTable\.get\(((?!Labels)[^()]+(?:\([^()]*\))?[^()]*)\)', r'table_text(\1)'),
}
def span(t):
    m=re.search(r'// ORIGINAL: 0x%08X '%int(addr,16),t); s=m.start(); e=t.index('\n}\n',s); return s,e
cur=open(unit,encoding='latin-1').read()
def variants(t):
    s,e=span(t); out=[]
    for r in rules:
        rx,rep=RX[r]
        for m in re.finditer(rx,t[s:e]):
            a=s+m.start(); b=s+m.end()
            out.append(('%s@L%d'%(r,t.count('\n',0,a)+1), t[:a]+m.expand(rep)+t[b:]))
    return out
base=perm.run_deck(unit,addr,[('BASE',cur)])[0][1]; print('base',base)
done=set()
while True:
    deck=[v for v in variants(cur) if v[0] not in done]
    if not deck: break
    res=perm.run_deck(unit,addr,deck)
    res=sorted(res,key=lambda r:r[1])
    for w,d,ex,k in res[:12]: print('  ',k,d,d-base)
    if res[0][1]>base-mn: break
    best=res[0]; cur=dict(deck)[best[3]]; base=best[1]; print('ACCEPT',best[3],base); sys.stdout.flush()
    # toggled site would be offered in reverse by the opposite rule: block same line
    done.add(best[3].replace('lraw','lget') if 'lraw' in best[3] else best[3].replace('lget','lraw') if 'lget' in best[3] else best[3].replace('traw','tget') if 'traw' in best[3] else best[3].replace('tget','traw'))
if '--write' in sys.argv: open(unit,'w',encoding='latin-1').write(cur)
print('final',base)
