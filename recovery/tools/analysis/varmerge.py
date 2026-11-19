# varmerge.py <unit> <addr> <start-text> v>w [v>w ...] : fe rows of the function with local v merged into w (declaration
# `int v;` dropped, `int v = ` becomes `w = `, every use renamed) between <start-text> and the next // ORIGINAL marker.
# One variant per pair; text rewrite only, so read the winner for meaning (overlapping live ranges break it).
# Prefixes FE_PFX_BASE .. +FE_JOBS-1.
import re,os,sys
sys.path.insert(0,'recovery/tools')
import perm
unit,addr,start=sys.argv[1],sys.argv[2],sys.argv[3]
t=open(unit,encoding='latin-1').read()
a=t.index(start); b=t.index('\n// ORIGINAL:',a)
def merge(v,w):
    s=t[a:b]
    s=re.sub(r'\n[ \t]*int %s;'%v,'',s)
    s=re.sub(r'\bint %s = '%v,'%s = '%w,s)
    s=re.sub(r'\b%s\b'%v,w,s)
    return t[:a]+s+t[b:]
pairs=[tuple(p.split('>')) for p in sys.argv[4:]]
deck=[('BASE',t)]+[('%s>%s'%p,merge(*p)) for p in pairs]
for r in perm.run_deck(unit,addr,deck): print(r[1],r[3])
