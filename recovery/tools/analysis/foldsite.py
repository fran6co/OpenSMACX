"""foldsite.py unit addr : replace two-way treaty/agenda write pairs with treaty_off/treaty_on/agenda_on/agenda_off calls (each site alone + all; the all-sites text goes to build/recovery/fe/fold_all.cpp).  Prefixes FE_PFX_BASE.., FE_JOBS."""
import sys,os,re
S=os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0,S)
import accsweep as A
import cdeck
unit,addr=sys.argv[1],int(sys.argv[2],16)
t=open(A.R+'/'+unit,encoding='latin-1').read(); a,b=A.span(t,addr); g=t[a:b]
pat=re.compile(r'([ \t]*)PlayersData\[([^\[\]]+)\]\.diplo_(treaties|agenda)\[([^\[\]]+)\] (&=|\|=) (~?)(\w+);\n\s*PlayersData\[\4\]\.diplo_\3\[\2\] \5 \6\7;\n')
sites=list(pat.finditer(g))
def rep(m):
    ind,x,kind,y,op,neg,k=m.groups()
    if op=='&=' and not neg: return None
    if op=='|=' and neg: return None
    k=k.rstrip('uU')
    fn={('treaties','&='):'treaty_off',('treaties','|='):'treaty_on',('agenda','&='):'agenda_off',('agenda','|='):'agenda_on'}[(kind,op)]
    return '%s%s(%s, %s, %s);\n'%(ind,fn,x,y,k)
deck=[('base',t)]; allg=g
for i,m in reversed(list(enumerate(sites))):
    r=rep(m)
    if not r: continue
    if 'SKIP' in os.environ and str(i) in os.environ['SKIP'].split(','): continue
    deck.append(('s%d:%s'%(i,r.strip()[:40]),t[:a]+g[:m.start()]+r+g[m.end():]+t[b:]))
    allg=allg[:m.start()]+r+allg[m.end():]
deck.append(('all',t[:a]+allg+t[b:]))
open(A.R+'/build/recovery/fe/fold_all.cpp','w',encoding='latin-1').write(deck[-1][1])
cdeck.run(unit,addr,deck[:1]+deck[-1:] if 'SKIP' in os.environ else deck,focus=('treaty_o','agenda_o'))
