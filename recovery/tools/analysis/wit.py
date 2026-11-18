import sys; sys.path.insert(0,'/tmp/claude-1000/-home-fran6co-code-OpenSMACX/be235c8c-71b6-49f3-abea-875f51ac16da/scratchpad/r4'); import live
src=sys.argv[1]; a=int(sys.argv[2],16); slot=int(sys.argv[3],16)
O=live.ours('src/enemy.cpp',src,0x56B5B0,sys.argv[4] if len(sys.argv)>4 else '2') if src!='img' else live.image(0x56B5B0,0x579208)
p=O.witness(O.idx[a],slot)
if not p: print('no witness')
else:
    prev=None; out=[]
    for n in p:
        if prev is None or n!=prev+1: out.append('%x'%O.ins[n]['a'])
        prev=n
    print(' -> '.join(out),'READ at %x: %s'%(O.ins[p[-1]]['a'],O.ins[p[-1]]['txt']))
