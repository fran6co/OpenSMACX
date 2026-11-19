import tm, ib, sys
def mk_src(name, distinct, zero_reg, member):
    s=''
    pre = 'S::' if member else ''
    for k in (2,4,8):
        fn = 'tg%d_%s' % (k, name) if distinct else 'tg_%s' % name
        if distinct or k==2:
            if member:
                s += 'struct S%s; ' % ''  if False else ''
            arg = 'int k' if not distinct else ''
            kk = 'k' if not distinct else str(k)
            body = '{ int f = g ^ %s; g = f; if (f & %s) mk(%s); else mk2(%s); g3 = g; tl(z); use(1); }' % (kk,kk,kk,kk)
            s += 'static void %s(%s) %s\n' % (fn, arg, body)
    s += 'void t_%s(int c) {\n  int z = 0;\n' % name if zero_reg else 'void t_%s(int c) {\n' % name
    if not zero_reg:
        s = s.replace('tl(z)', 'tl(0)')
    else:
        s = s.replace('void tg', 'void tg').replace('tl(z)', 'tl(gz)')
    s += '  switch (c) {\n  case 1: mk(1); break;\n'
    for j,k in enumerate((2,4,8)):
        fn = 'tg%d_%s()' % (k, name) if distinct else 'tg_%s(%d)' % (name, k)
        s += '  case %d: %s; break;\n' % (j+2, fn)
    s += '  case 5: mk(5); use(7); break;\n  case 7: mk(6); use(8); break;\n  case 8: mk(9); call(); break;\n  }\n  if (g == 100) use(100);\n}\n'
    return s
V=[]
for d in (0,1):
    for z in (0,1):
        n='d%dz%d'%(d,z)
        src=mk_src(n,d,z,0)
        if z: src='extern int gz;\n'+src
        V.append((n,src))
res,asm=tm.run(V)
for k,n in res:
    f=[x for x in ib.functions(asm) if 't_'+k in x[0]][0]
    print(k, 'use-calls', sum(1 for l in f[2] if l.startswith('call') and 'use@@' in l), 'tl', n)
