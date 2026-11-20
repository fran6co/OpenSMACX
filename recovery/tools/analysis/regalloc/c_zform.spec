%% K=0,5,-1
== none/K
extern int gz0, gz1, gz2, gl[4]; extern char gc0; extern short gs0; extern int *gp0;
void probe() { call(); gz0 = {K}; gz1 = {K}; call(); }
%% K=0,5,-1
== memcmp_ne/K
extern int gz0, gz1, gz2, gl[4];
void probe() { if (gl[0] != {K}) call(); gz0 = {K}; gz1 = {K}; call(); }
== memcmp_bool
extern int gz0, gz1, gz2, gl[4];
void probe() { if (gl[0]) call(); gz0 = 0; gz1 = 0; call(); }
== memcmp_not
extern int gz0, gz1, gz2, gl[4];
void probe() { if (!gl[0]) call(); gz0 = 0; gz1 = 0; call(); }
== memcmp_lt
extern int gz0, gz1, gz2, gl[4];
void probe() { if (gl[0] < 0) call(); gz0 = 0; gz1 = 0; call(); }
== memcmp_gt
extern int gz0, gz1, gz2, gl[4];
void probe() { if (gl[0] > 0) call(); gz0 = 0; gz1 = 0; call(); }
== memcmp_char
extern int gz0, gz1, gz2, gl[4]; extern char gc0;
void probe() { if (gc0 != 0) call(); gz0 = 0; gz1 = 0; call(); }
== memcmp_ptr
extern int gz0, gz1, gz2, gl[4]; extern int *gp0;
void probe() { if (gp0 != 0) call(); gz0 = 0; gz1 = 0; call(); }
== memcmp_field
extern int gz0, gz1, gz2, gl[4];
void probe(Obj *o) { if (o->f1 != 0) call(); gz0 = 0; gz1 = 0; call(); }
%% K=0,5,-1
== push/K
extern int gz0, gz1, gz2, gl[4];
void probe() { use_p({K}); call(); gz0 = {K}; gz1 = {K}; call(); }
== push_after
extern int gz0, gz1, gz2, gl[4];
void probe() { call(); gz0 = 0; gz1 = 0; call(); use_p(0); }
== regcmp
extern int gz0, gz1, gz2, gl[4];
void probe() { D(a); if (a != 0) call(); gz0 = 0; gz1 = 0; call(); U4(a); }
== regcmp_lt
extern int gz0, gz1, gz2, gl[4];
void probe() { D(a); if (a < 0) call(); gz0 = 0; gz1 = 0; call(); U4(a); }
== callcmp_lt
extern int gz0, gz1, gz2, gl[4];
void probe() { if (cond() < 0) call(); gz0 = 0; gz1 = 0; call(); }
== localinit_prop
extern int gz0, gz1, gz2, gl[4];
void probe() { int x = 0; call(); gz0 = 0; gz1 = 0; call(); use_s(x); }
== localinit_var
extern int gz0, gz1, gz2, gl[4];
void probe() { int x = 0; if (cond()) x = def_s(); call(); gz0 = 0; gz1 = 0; call(); use_s(x); }
== loopctr
extern int gz0, gz1, gz2, gl[4]; extern int gm;
void probe() { for (int i = 0; i < gm; i++) use_i(i); gz0 = 0; gz1 = 0; call(); }
== ret0
extern int gz0, gz1, gz2, gl[4];
int probe() { if (cond()) return 0; gz0 = 0; gz1 = 0; call(); return 1; }
== charstore
extern int gz0, gz1, gz2, gl[4]; extern char gc0;
void probe() { gc0 = 0; call(); gz0 = 0; gz1 = 0; call(); }
== shortstore
extern int gz0, gz1, gz2, gl[4]; extern short gs0;
void probe() { gs0 = 0; call(); gz0 = 0; gz1 = 0; call(); }
== ptrstore
extern int gz0, gz1, gz2, gl[4]; extern int *gp0;
void probe() { gp0 = 0; call(); gz0 = 0; gz1 = 0; call(); }
== otherblock
extern int gz0, gz1, gz2, gl[4];
void probe() { if (cond()) gz2 = 0; call(); gz0 = 0; gz1 = 0; call(); }
== sameblock_store
extern int gz0, gz1, gz2, gl[4];
void probe() { gz2 = 0; call(); gz0 = 0; gz1 = 0; call(); }
== fieldstore
extern int gz0, gz1, gz2, gl[4];
void probe(Obj *o) { o->f2 = 0; call(); gz0 = 0; gz1 = 0; call(); }
== inl_store
extern int gz0, gz1, gz2, gl[4];
inline void clr() { gz2 = 0; }
void probe() { clr(); call(); gz0 = 0; gz1 = 0; call(); }
== inl_cmp
extern int gz0, gz1, gz2, gl[4];
inline int nz() { return gl[1] != 0; }
void probe() { if (nz()) call(); gz0 = 0; gz1 = 0; call(); }
== inl_push
extern int gz0, gz1, gz2, gl[4];
inline void up() { use_p(0); }
void probe() { up(); call(); gz0 = 0; gz1 = 0; call(); }
== inl_param0
extern int gz0, gz1, gz2, gl[4];
inline void st(int v) { gz2 = v; }
void probe() { st(0); call(); gz0 = 0; gz1 = 0; call(); }
== paramstore
extern int gz0, gz1, gz2, gl[4];
void probe(int w) { w = 0; call(); gz0 = 0; gz1 = 0; call(); use_p(w); }
== arith_add0
extern int gz0, gz1, gz2, gl[4];
void probe() { D(a); call(); gz0 = 0; gz1 = 0; call(); use_a(a > 0 ? a : 0); }
