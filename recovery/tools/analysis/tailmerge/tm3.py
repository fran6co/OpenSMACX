import tm, ib, sys
def src(name, tail, last='mk(9); call();'):
    s=''
    for k in (2,4,8):
        s += 'static void tg%d_%s() { int f = g ^ %d; g = f; if (f & %d) mk(%d); else mk2(%d); g3 = g; %s }\n' % (k,name,k,k,k,k,tail)
    s += 'void t_%s(int c) {\n  switch (c) {\n  case 1: mk(1); break;\n' % name
    for j,k in enumerate((2,4,8)):
        s += '  case %d: tg%d_%s(); break;\n' % (j+2, k, name)
    s += '  case 5: mk(5); use(7); break;\n  case 7: mk(6); use(8); break;\n  case 8: %s break;\n  }\n  if (g == 100) use(100);\n}\n' % last
    return s
TAILS={'T9':'tl(0); call(); call();','T10':'g4 = 1; g5 = 2; g6 = 3; g7 = 4; g8 = 5;','T11':'g4 = 1; g5 = 2; g6 = 3; g7 = 4;','T12':'call(); call(); call(); call();','T13':'call(); call(); call(); call(); call();','T14':'tl(0); tl(1);','T15':'call(); tl(0); call();','T1':'tl(0); use(1);','T2':'tl(0); call();','T3':'tl(0); use(1); call();','T4':'tl(0);','T5':'use(1); g4 = 1;','T6':'call(); call();','T7':'tl(0); g4 = 1;','T8':'g4 = 1; g5 = 2;'}
if __name__ == "__main__":
  V=[(k,src(k,t)) for k,t in TAILS.items()]
  res,asm=tm.run(V)
  for k,_ in res:
      f=[x for x in ib.functions(asm) if x[0].split()[0].endswith('t_'+k) or ('t_'+k+'@') in x[1]][0]
      jm=sum(1 for l in f[2] if l.startswith('jmp') and 'DWORD' not in l)
      print(k, TAILS[k].ljust(28), 'g3-stores', sum(1 for l in f[2] if 'g3@@' in l and l.startswith('mov') and l.split(',')[0].endswith('A')), 'mk2', sum(1 for l in f[2] if 'mk2@@' in l))
      if '-v' in sys.argv: print('\n'.join(f[2]))
