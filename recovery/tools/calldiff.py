#!/usr/bin/env python3
"""calldiff.py [pfx] : for every non-exact function, compare the callee multiset of the image with ours
(by symbol) and print the callees that most often differ across the tree.
'image calls, ours does not' = something we wrote out by hand; the reverse = a call the image has inline."""
import sys,os,re,json,struct,glob,collections
S=os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0,S); import fe
R=fe.R; pfx=sys.argv[1] if len(sys.argv)>1 else '20'
addr2sym={}
for p in glob.glob(R+'/src/**/*.cpp',recursive=True):
    t=open(p,encoding='latin-1').read()
    for m in re.finditer(r'// ORIGINAL: 0x([0-9A-F]{8}) (\S+) 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+[^\n]*\n((?://[^\n]*\n)*)',t):
        sym=m.group(2); m2=re.search(r'// symbol\s+(\S+)',m.group(3))
        addr2sym[int(m.group(1),16)]=m2.group(1) if m2 else sym
cc=json.load(open(R+'/build/compile_commands.json'))
units=[e['file'][len(R)+1:] for e in cc if e['file'].startswith(R+'/src/')]
more=collections.Counter(); less=collections.Counter(); per=[]
for u in units:
    t=open(R+'/'+u,encoding='latin-1').read()
    todo=[(int(m.group(1),16),int(m.group(2),16),int(m.group(3),16)) for m in re.finditer(r'// ORIGINAL: 0x([0-9A-F]{8}) \S+ 0x([0-9A-Fa-f]+)-0x([0-9A-Fa-f]+)([^\n]*)',t) if 'BYTE_EXACT' not in m.group(4)]
    if not todo: continue
    c,err=fe.compile_unit(u,R+'/'+u,pfx)
    if c is None: continue
    for addr,lo,hi in todo:
        idn=fe.ident(t,addr)
        try: s=c.function(idn[0])
        except Exception: continue
        sec=c.sections[s.section-1]
        ours=collections.Counter()
        rm={r[0]:r for r in sec.relocations}
        for ins in fe.md.disasm(sec.data,0):
            if ins.mnemonic=='call' and ins.size==5 and (ins.address+1) in rm:
                sy=c.symbols.get(rm[ins.address+1][1]); ours[sy.name if sy else '?']+=1
        img=collections.Counter()
        data=fe.imgbytes(lo,hi)
        for ins in fe.md.disasm(data,lo):
            if ins.mnemonic=='call' and ins.size==5:
                try: tgt=int(ins.op_str,16)
                except ValueError: continue
                img[addr2sym.get(tgt,'sub_%x'%tgt)]+=1
        d_more={k:img[k]-ours.get(k,0) for k in img if img[k]>ours.get(k,0)}
        d_less={k:ours[k]-img.get(k,0) for k in ours if ours[k]>img.get(k,0)}
        for k,v in d_more.items(): more[k]+=v
        for k,v in d_less.items(): less[k]+=v
        if d_more or d_less: per.append(dict(unit=u,addr='%08X'%addr,sym=idn[0],image_more=d_more,ours_more=d_less))
json.dump(per,open(R+'/build/recovery/fe/calldiff.json','w'),indent=0)
print('functions with call differences',len(per))
print('--- image calls it more often than ours (we wrote it out / inlined it):')
for k,v in more.most_common(30): print(' ',v,k)
print('--- ours calls it more often than the image (image has it inline):')
for k,v in less.most_common(30): print(' ',v,k)
