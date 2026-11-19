"""hv.py: hvar.run(unit, addrs, variants=[(key, {hdr: (lineidx, text) or full_text}, cpp_text_or_None)]) in mirrored trees."""
import sys,os,re,shutil,subprocess,threading
from concurrent.futures import ThreadPoolExecutor
R='/home/fran6co/code/OpenSMACX/.opensmacx/worktrees/par-5'
sys.path.insert(0,R+'/recovery/tools'); import fe
from match_recovery.pe_coff import Coff
def strip_hp(txt):
    m=re.search(r'// HEADER-PADS: function \w+\n((?:void \w+\(\);\n)*)',txt)
    return txt[:m.start(1)]+txt[m.end(1):] if m else txt
def run(unit,addrs,variants,jobs=5,pb=27):
    cmd0=fe.compile_cmd(unit); lk=threading.Lock(); ctr=[0]; tls=threading.local()
    def job(v):
        key,hd,cpp=v
        if not hasattr(tls,'p'):
            with lk: tls.p=pb+ctr[0]; ctr[0]+=1
            tls.D='%s/build/recovery/fe/hm%d'%(R,tls.p); tls.touched=set()
            shutil.rmtree(tls.D,ignore_errors=True); shutil.copytree(R+'/src',tls.D+'/src')
        D=tls.D
        for h in tls.touched|set(hd): shutil.copy(R+'/src/'+h,D+'/src/'+h)
        tls.touched|=set(hd)
        for h,ed in hd.items():
            if isinstance(ed,str): open(D+'/src/'+h,'w',encoding='latin-1').write(ed); continue
            H=open(R+'/src/'+h,encoding='latin-1').read().split('\n')
            for i,tx in sorted(ed,reverse=True): H.insert(i,tx)
            open(D+'/src/'+h,'w',encoding='latin-1').write('\n'.join(H))
        txt=cpp if cpp is not None else open(R+'/'+unit,encoding='latin-1').read()
        src=D+'/'+unit; open(src,'w',encoding='latin-1').write(txt)
        obj=D+'/o.obj'; out=[]
        for t in cmd0:
            if t.startswith('/Fo'): t='/Fo'+obj
            elif t.startswith('/Fd') or t.startswith('--repro-root') or t=='/W4': continue
            elif t==R+'/'+unit: t=src
            else: t=t.replace(R+'/src',D+'/src')
            out.append(t)
        if os.path.exists(obj): os.remove(obj)
        p=subprocess.run(out,cwd=D,env=dict(os.environ,VC6_PREFIX=f'{fe.MAIN}/.opensmacx/wine/worker-{tls.p}'),capture_output=True,text=True)
        if not os.path.exists(obj): return key,'ERR '+'\n'.join(l for l in (p.stdout+p.stderr).split('\n') if 'error' in l)[:300]
        c=Coff(open(obj,'rb').read())
        return key,[fe.evaluate(c,txt,a).get('diff') for a in addrs]
    return list(ThreadPoolExecutor(jobs).map(job,variants))
