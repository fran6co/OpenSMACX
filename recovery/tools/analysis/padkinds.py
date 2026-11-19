"""padkinds.py <unit> <0xADDR> <signature text> <lo-hi> [kind,kind...]: what counts for a function-local tie (MATCHING_GUIDE 3.6).
Adds k = 0..9 code-free pads of each kind (dead locals, parameters stand-ins, aggregates, statics, externs, literals, labels,
inline expansions, block declarations) at the start or end of the function and prints `window rows/total rows` per k for the
image address window lo-hi (hex, the tie rows). A kind that counts shows a period (8 for one symbol, 4 for two); a kind that
only grows the /Ob2 caller budget moves once and stays. RULE_BASE=ALT.cpp reads the function from another source text.
FE_PFX_BASE.., FE_JOBS lanes. Screen only."""
import sys,os,re
sys.path.insert(0,os.getcwd()+'/recovery/tools'); import perm
u,addr,sig,win=sys.argv[1:5]; kinds=sys.argv[5].split(',') if len(sys.argv)>5 else None
s=open(os.environ.get('RULE_BASE',u),encoding='latin-1').read()
a=s.index(sig); b=s.index('{',a)+1; e=s.index('\n}\n',a)+1
K={
 'end_local': lambda k:('', '', ''.join('    { int fq%d = 0; }\n'%i for i in range(k))),
 'start_local': lambda k:('', ''.join('\n    int fq%d = 0;'%i for i in range(k)), ''),
 'end_unref_local': lambda k:('', '', ''.join('    { int fq%d; }\n'%i for i in range(k))),
 'end_static': lambda k:('', '', ''.join('    { static int fq%d; fq%d = 0; }\n'%(i,i) for i in range(k))),
 'end_extern_store': lambda k:(''.join('extern int FeX%d;\n'%i for i in range(k)), '', ''.join('    FeX%d = 0;\n'%i for i in range(k))),
 'end_extern_addr': lambda k:(''.join('extern int FeX%d;\n'%i for i in range(k)), '', ''.join('    (void)&FeX%d;\n'%i for i in range(k))),
 'end_same_extern_store': lambda k:('extern int FeX0;\n', '', ''.join('    FeX0 = %d;\n'%i for i in range(k))),
 'end_literal': lambda k:('extern const char *FeP;\n', '', ''.join('    FeP = "q%d";\n'%i for i in range(k))),
 'end_label': lambda k:('', '', ''.join('    goto fl%d;\nfl%d:;\n'%(i,i) for i in range(k))),
 'end_inline1': lambda k:('static inline int fe_id(int x) { return x; }\nextern int FeX0;\n', '', ''.join('    FeX0 = fe_id(FeX0);\n' for i in range(k))),
 'end_array': lambda k:('', '', ''.join('    { char fq%d[4]; fq%d[0] = 0; }\n'%(i,i) for i in range(k))),
 'end_call': lambda k:('void FeCall(int);\n', '', ''.join('    FeCall(%d);\n'%i for i in range(k))),
 'end_calls_distinct': lambda k:(''.join('void FeCall%d();\n'%i for i in range(k)), '', ''.join('    FeCall%d();\n'%i for i in range(k))),
 'end_block': lambda k:('', '', ''.join('    { }\n' for i in range(k))),
 'end_local_in_one_scope': lambda k:('', '', '    {\n'+''.join('    int fq%d = 0;\n'%i for i in range(k))+'    }\n'),
 'end_double': lambda k:('', '', ''.join('    { double fq%d = 0; }\n'%i for i in range(k))),
 'end_ptr': lambda k:('', '', ''.join('    { char *fq%d = 0; }\n'%i for i in range(k))),
 'end_struct': lambda k:('', '', ''.join('    { RECT fq%d; fq%d.left = 0; }\n'%(i,i) for i in range(k))),
 'end_inl_local1': lambda k:('static inline int fe_id(int x) { int y = x + 1; return y * y; }\nextern int FeX0;\n', '', ''.join('    FeX0 = fe_id(FeX0);\n' for i in range(k))),
 'end_inl_local2': lambda k:('static inline int fe_id(int x) { int y = x + 1; int z = y * x; return y * z; }\nextern int FeX0;\n', '', ''.join('    FeX0 = fe_id(FeX0);\n' for i in range(k))),
 'end_inl_param_reassigned': lambda k:('static inline int fe_id(int x) { x += 2; return x * x; }\nextern int FeX0;\n', '', ''.join('    FeX0 = fe_id(FeX0);\n' for i in range(k))),
 'end_inl_2params': lambda k:('static inline int fe_id(int x, int w) { return x * w + x; }\nextern int FeX0;\n', '', ''.join('    FeX0 = fe_id(FeX0 + 1, FeX0 * 3);\n' for i in range(k))),
 'end_array_passed': lambda k:('void FeCallS(const char *);\n', '', ''.join('    { char fq%d[4]; FeCallS(fq%d); }\n'%(i,i) for i in range(k))),
 'end_array_idx': lambda k:('extern int FeX0;\n', '', ''.join('    { char fq%d[4]; fq%d[FeX0] = 0; }\n'%(i,i) for i in range(k))),
 'end_struct_passed': lambda k:('void FeCallR(RECT *);\n', '', ''.join('    { RECT fq%d; FeCallR(&fq%d); }\n'%(i,i) for i in range(k))),
 'end_local_addr': lambda k:('void FeCallI(int *);\n', '', ''.join('    { int fq%d; FeCallI(&fq%d); }\n'%(i,i) for i in range(k))),
 'end_local_reg': lambda k:('extern int FeX0;\n', '', ''.join('    { int fq%d = FeX0; FeX0 = fq%d * fq%d; }\n'%(i,i,i) for i in range(k))),
 'end_class_obj': lambda k:('', '', ''.join('    { Font fq%d; }\n'%(i) for i in range(k))),
 'end_struct_2f': lambda k:('', '', ''.join('    { RECT fq%d; fq%d.left = 0; fq%d.top = 1; }\n'%(i,i,i) for i in range(k))),
 'end_struct_3f': lambda k:('', '', ''.join('    { RECT fq%d; fq%d.left = 0; fq%d.top = 1; fq%d.right = 2; }\n'%(i,i,i,i) for i in range(k))),
 'end_struct_1f_passed': lambda k:('void FeCallR(RECT *);\n', '', ''.join('    { RECT fq%d; fq%d.left = 0; FeCallR(&fq%d); }\n'%(i,i,i) for i in range(k))),
 'end_struct_copy': lambda k:('', '', ''.join('    { RECT fq%d; fq%d = field_20_; }\n'%(i,i) for i in range(k))),
 'end_arr_2e': lambda k:('', '', ''.join('    { int fq%d[4]; fq%d[0] = 0; fq%d[1] = 1; }\n'%(i,i,i) for i in range(k))),
 'end_local_two_stores': lambda k:('', '', ''.join('    { int fq%d = 0; fq%d = 1; }\n'%(i,i) for i in range(k))),
 'end_local_addr_and_use': lambda k:('void FeCallI(int *);\nextern int FeX0;\n', '', ''.join('    { int fq%d = 0; FeCallI(&fq%d); FeX0 = fq%d; }\n'%(i,i,i) for i in range(k))),
 'end_block_extern': lambda k:('', '', ''.join('    { extern int FeBX%d; (void)&FeBX%d; }\n'%(i,i) for i in range(k))),
 'end_block_extern_unref': lambda k:('', '', ''.join('    { extern int FeBX%d; }\n'%(i) for i in range(k))),
 'end_block_fn_decl': lambda k:('', '', ''.join('    { void FeBF%d(); }\n'%(i) for i in range(k))),
 'end_const_local': lambda k:('', '', ''.join('    { const int fq%d = 5; }\n'%(i) for i in range(k))),
 'end_typedef': lambda k:('', '', ''.join('    { typedef int fqt%d; }\n'%(i) for i in range(k))),
 'end_enum': lambda k:('', '', ''.join('    { enum { fqe%d = 1 }; }\n'%(i) for i in range(k))),
 'end_register': lambda k:('', '', ''.join('    { register int fq%d = 0; }\n'%(i) for i in range(k))),
 'file_static_fn': lambda k:(''.join('static int FeS%d() { return %d; }\n'%(i,i) for i in range(k)), '', ''),
}
deck=[]
for name,gen in K.items():
    if kinds and name not in kinds: continue
    for k in range(0,10):
        pre,st,en=gen(k)
        deck.append(('%s:%d'%(name,k), s[:a]+pre+s[a:b]+st+s[b:e]+en+s[e:]))
res=perm.run_deck(u,addr,deck,win=win)
d={k:(w,dd) for w,dd,exa,k in res}
for name in K:
    if kinds and name not in kinds: continue
    print('%-24s'%name,' '.join('%s/%s'%(d['%s:%d'%(name,k)][0][0],d['%s:%d'%(name,k)][1]) for k in range(10)))
