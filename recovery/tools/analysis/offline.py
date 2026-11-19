# offline.py <unit> <addr> [src] -> prints the 'offset line' map of our function (from /FAsc); prefix FE_PFX_BASE
import sys,os,json
R=os.getcwd(); sys.path.insert(0,R+'/recovery/tools'); sys.path.insert(0,R+'/recovery/tools/analysis')
import fe,tailscan
unit,addr=sys.argv[1],int(sys.argv[2],16); src=sys.argv[3] if len(sys.argv)>3 else unit
t=open(src,encoding='latin-1').read(); sym,lo,hi=fe.ident(t,addr)
c,cod=tailscan.compile_listing(unit,src,os.environ.get('FE_PFX_BASE','27'))
m=tailscan.lines_of(cod,sym,os.path.basename(unit).lower())
for k in sorted(m): print('%x %s'%(k,m[k]))
