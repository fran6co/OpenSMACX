import sys; from cj import *
import cj
cj.HDR += 'extern Mn m;\n'
T=[('gmp','imp',0x10000,0x209),('gp','ip',0x100,0x20a),('gp','ip',0x200,0x20b)]
INL=''.join('static inline void tg%d_%%s() %s\n'%(i,toggle(i,*t)) for i,t in enumerate(T))
INLBN='static inline void bn_%s() '+BN+'\n'
def fn(name, inl_t, inl_bn, end='break;', after='', order='TDB', extra=''):
    s=''
    if inl_t: s+=INL.replace('%s',name)
    if inl_bn: s+=INLBN%name
    s+='void t_%s(int c) {\n  switch (c) {\n  case 1: f0(); break;\n'%name
    cases=[]
    for i,t in enumerate(T):
        body=('tg%d_%s();'%(i,name)) if inl_t else toggle(i,*t)
        cases.append(body)
    tb=('bn_%s();'%name) if inl_bn else BN
    k=10
    seq=[]
    for ch in order:
        if ch=='T': seq+=cases
        if ch=='D': seq.append(DEST)
        if ch=='B': seq.append(tb)
    for b in seq:
        s+='  case %d: %s %s\n'%(k,b,end); k+=1
    s+=extra
    s+='  case 30: use(30); break;\n  case 31: use(31); break;\n  }\n'
    s+=after or '  if (gx & 0x40) { switch (c) { case 40: f0(); return; case 41: use(41); return; } }\n'
    s+='}\n'
    return s
if __name__=="__main__":
 V=[]
 V.append(('host_all', fn('ha',0,0)))
 V.append(('inl_all', fn('ia',1,1)))
 V.append(('inl_toggles_host_bn', fn('ib',1,0)))
 V.append(('host_toggles_inl_bn', fn('ic',0,1)))
 V.append(('ret_host', fn('rh',0,0,end='return;')))
 V.append(('ret_inl', fn('ri',1,1,end='return;')))
 V.append(('bn_last', fn('bl',1,1,order='DTB')))
 V.append(('bn_first', fn('bf',1,1,order='BDT')))
 V.append(('nosecond', fn('ns',1,1,after='  use(99);\n')))
 V.append(('noafter', fn('na',1,1,after=' ')))
 run(V, show=sys.argv[1:])
