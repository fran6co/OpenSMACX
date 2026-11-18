import sys,re; sys.path.insert(0,'/tmp/claude-1000/-home-fran6co-code-OpenSMACX/be235c8c-71b6-49f3-abea-875f51ac16da/scratchpad/p5'); import symx
def norm(e):
    # parse into tree, sort commutative '+'
    e=e.replace('<<8','')
    pos=0
    def parse():
        nonlocal pos
        if e[pos]=='-':
            pos+=1; return ('neg',parse())
        if e[pos]=='(':
            pos+=1; a=parse(); op=e[pos]; pos+=1; b=parse(); assert e[pos]==')',e; pos+=1
            if op=='+': a,b=sorted([a,b],key=str)
            return (op,a,b)
        m=re.match(r'[XYZ]\d|\?\w*|\[[^\]]*\]',e[pos:]); pos+=m.end(); return m.group(0)
    return parse()
def trees(out):
    return {c:norm(e) for c,e in out if c.startswith('c')}
def cmp(src,pfx='32',verbose=True):
    io,islot,iesp,_=symx.image(); oo,oslot,oesp,_=symx.ours(src,pfx)
    ti=trees(io); to=trees(oo); bad=[c for c in ti if ti[c]!=to.get(c)]
    if verbose:
        for c in bad: print(c,'IMG',dict(io)[c],' OURS',dict(oo).get(c))
        print('bad',len(bad),'slots img',len(islot),'ours',len(oslot))
    return len(bad),len(oslot)
if __name__=='__main__':
    cmp(sys.argv[1] if len(sys.argv)>1 else symx.W+'/src/caviar/caviar_raster.cpp')
