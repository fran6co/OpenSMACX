"""sharedlits.py: every .data string referenced by functions of >=2 units (a named object, the image is unpooled):
the single-unit literal before it (A) and after it (B), and whether A / B reference it."""
import sys
sys.path.insert(0,'recovery/tools'); sys.path.insert(0,'recovery/tools/analysis')
import tumap,fe
fs=tumap.load()
L=sorted(tumap.literal_owners(fs))
def u1(us): 
    us=[u for u in us if not u.endswith('.h')]; return us
n=len(L)
i=0
while i<n:
    a,s,us,ns=L[i]; us=u1(us)
    if len(set(us))<2: i+=1; continue
    # group of consecutive shared
    j=i
    while j<n and len(set(u1(L[j][2])))>=2: j+=1
    A=u1(L[i-1][2])[0] if i>0 and u1(L[i-1][2]) else '-'
    B=u1(L[j][2])[0] if j<n and u1(L[j][2]) else '-'
    for k in range(i,j):
        a,s,us,ns=L[k]; us=sorted(set(u1(us)))
        print('%08X %-22r A=%-22s %s B=%-22s %s  units=%d %s'%(a,s[:20],A.replace('src/',''),'Y' if A in us else 'n',B.replace('src/',''),'Y' if B in us else 'n',len(us),' '.join(x.replace('src/','') for x in us[:5])))
    print()
    i=j
