# p2eh.py FEALL.json OUT.json : FuncInfo comparison (maxState, unwind parents, try blocks) image vs ours
import sys,os,re,json,struct,queue
from concurrent.futures import ThreadPoolExecutor
sys.path.insert(0,os.getcwd()+'/recovery/tools'); import fe
res=json.load(open(sys.argv[1]))
TEXT_LO,TEXT_HI=0x401000,0x700000
def img_fi(lo):
    ins=list(fe.md.disasm(fe.imgbytes(lo,lo+40),lo))[:8]
    for x in ins:
        if x.mnemonic=='push' and x.op_str.startswith('0x'):
            h=int(x.op_str,16)
            if TEXT_LO<=h<TEXT_HI:
                b=fe.imgbytes(h,h+10)
                if b[0]==0xB8 and b[5]==0xE9:
                    fi=struct.unpack('<I',b[1:5])[0]
                    magic,ms,pu,nt=struct.unpack('<IIII',fe.imgbytes(fi,fi+16))
                    if magic!=0x19930520: return None
                    return dict(ms=ms,par=[struct.unpack('<i',fe.imgbytes(pu+8*i,pu+8*i+4))[0] for i in range(ms)],
                                act=[1 if struct.unpack('<I',fe.imgbytes(pu+8*i+4,pu+8*i+8))[0] else 0 for i in range(ms)],nt=nt)
    return None
def our_fi(c,sym):
    try: s=c.function(sym)
    except Exception: return 'NOSYM'
    sec=c.sections[s.section-1]
    rel={off:si for off,si,ty in sec.relocations}
    for x in fe.md.disasm(bytes(sec.data[:40]),0):
        if x.mnemonic=='push' and x.size==5 and (x.address+1) in rel:
            hs=c.symbols[rel[x.address+1]]
            if hs.section<=0: continue
            hsec=c.sections[hs.section-1]
            if hs.value>=len(hsec.data) or hsec.data[hs.value]!=0xB8: continue
            fis=None
            for off,si,ty in hsec.relocations:
                if off==hs.value+1: fis=c.symbols[si]
            if fis is None: continue
            d=c.sections[fis.section-1]
            magic,ms=struct.unpack_from('<II',d.data,fis.value)
            nt=struct.unpack_from('<I',d.data,fis.value+12)[0]
            us=None
            for off,si,ty in d.relocations:
                if off==fis.value+8: us=c.symbols[si]
            if us is None: return dict(ms=ms,par=[],act=[],nt=nt)
            ud=c.sections[us.section-1]; base=us.value+struct.unpack_from('<I',d.data,fis.value+8)[0]
            urel={off for off,si,ty in ud.relocations}
            return dict(ms=ms,par=[struct.unpack_from('<i',ud.data,base+8*i)[0] for i in range(ms)],
                        act=[1 if (base+8*i+4) in urel else 0 for i in range(ms)],nt=nt)
    return None
work={}
for r in res:
    if r.get('diff',-1)<=0 or r['addr']=='?': continue
    work.setdefault(r['unit'],[]).append(r)
units=list(work)
Q=queue.Queue()
for i in range(5): Q.put(i)
def unit_job(u):
    L=Q.get()
    try:
        out=[]; cache={}
        text=open(fe.R+'/'+u,encoding='latin-1').read()
        for r in work[u]:
            a=int(r['addr'],16)
            m=re.search(r'// ORIGINAL: 0x%08X [^\n]*\n((?://(?! ORIGINAL)[^\n]*\n)*)'%a,text)
            mc=m and re.search(r'// compile_context (\S+)',m.group(1))
            host=mc.group(1) if mc else u
            if host not in cache: cache[host]=fe.compile_unit(host,fe.R+'/'+host,str(8+L))[0]
            c=cache[host]
            idn=fe.ident(text,a)
            if not idn or c is None: continue
            sym,lo,hi=idn
            I=img_fi(lo); O=our_fi(c,sym)
            out.append(dict(unit=u,addr=r['addr'],sym=sym,diff=r['diff'],img=I,ours=O))
        return out
    finally: Q.put(L)
allr=[]
with ThreadPoolExecutor(5) as ex:
    for o in ex.map(unit_job,units): allr+=o
json.dump(allr,open(sys.argv[2],'w'))
def key(f): return None if not isinstance(f,dict) else (f['ms'],tuple(f['par']),tuple(f['act']),f['nt'])
n=0
for r in allr:
    I,O=r['img'],r['ours']
    if key(I)!=key(O):
        n+=1
        fi=lambda f: '-' if f is None else (f if isinstance(f,str) else 'ms=%d nt=%d'%(f['ms'],f['nt']))
        print('%s %-22s %-44s d=%4d IMG %-12s OURS %-12s'%(r['addr'],r['unit'][4:26],r['sym'][:44],r['diff'],fi(I),fi(O)))
print('differ',n,'of',len(allr))
