%% N=1,2,3,4,6
== alone_N
extern int gz[8];
void probe() { call(); for (int k = 0; k < {N}; k++) gz[k] = 0; call(); }
== pair_push/0
extern int gz0, gz1;
void probe() { use_p(0);  call(); gz0 = 0; gz1 = 0; call();  }
== pair_push/1
extern int gz0, gz1;
void probe() { use_p(0); D(a); call(); gz0 = 0; gz1 = 0; call(); U4(a); }
== pair_push/2
extern int gz0, gz1;
void probe() { use_p(0); D(a); D(b); call(); gz0 = 0; gz1 = 0; call(); U4(a); U4(b); }
== pair_push/3
extern int gz0, gz1;
void probe() { use_p(0); D(a); D(b); D(c); call(); gz0 = 0; gz1 = 0; call(); U4(a); U4(b); U4(c); }
== pair_alone/0
extern int gz0, gz1;
void probe() {  call(); gz0 = 0; gz1 = 0; call();  }
== pair_alone/1
extern int gz0, gz1;
void probe() { D(a); call(); gz0 = 0; gz1 = 0; call(); U4(a); }
== pair_alone/2
extern int gz0, gz1;
void probe() { D(a); D(b); call(); gz0 = 0; gz1 = 0; call(); U4(a); U4(b); }
== pair_alone/3
extern int gz0, gz1;
void probe() { D(a); D(b); D(c); call(); gz0 = 0; gz1 = 0; call(); U4(a); U4(b); U4(c); }
== pair_memcmp
extern int gz0, gz1, gl[4];
void probe() { if (gl[0] != 0) call(); gz0 = 0; gz1 = 0; call(); }
== pair_callcmp
extern int gz0, gz1;
void probe() { if (cond() != 0) call(); gz0 = 0; gz1 = 0; call(); }
== pair_born
extern int gz0, gz1;
void probe() { use_p(0); call(); gz0 = 0; gz1 = 0; call(); D(a); call(); U4(a); }
== pair_int_char
extern int gz0; extern char gc0; extern int gz1, gz2;
void probe() { D(a); D(b); D(c); use_p(0); call(); gz0 = 0; gc0 = 0; call(); U4(a); U4(b); U4(c); }
== pair_int_float
extern int gz0; extern float gf0;
void probe() { D(a); D(b); D(c); use_p(0); call(); gz0 = 0; gf0 = 0; call(); U4(a); U4(b); U4(c); }
== slot_cmp_none
extern int gm, gz0, gz1; extern int *tab[8]; extern void gp(int *, int *); extern int inb(int, int, int, int);
void probe(int wi) { int s = 0; int x, y;  if (wi < 0) { gp(&x, &y); if (inb(x, y, 0, 0) && x >= 0) { gz0 = x;  } else { do { if (tab[s] == 0) continue; use_a(s); } while (++s < gm); } wi = s; } for (s = 0; s < gm; s++) use_a(s); use_b(wi); }
== slot_cmp_store_top
extern int gm, gz0, gz1; extern int *tab[8]; extern void gp(int *, int *); extern int inb(int, int, int, int);
void probe(int wi) { int s = 0; int x, y; gz1 = 0; if (wi < 0) { gp(&x, &y); if (inb(x, y, 0, 0) && x >= 0) { gz0 = x;  } else { do { if (tab[s] == 0) continue; use_a(s); } while (++s < gm); } wi = s; } for (s = 0; s < gm; s++) use_a(s); use_b(wi); }
== slot_cmp_store_arm
extern int gm, gz0, gz1; extern int *tab[8]; extern void gp(int *, int *); extern int inb(int, int, int, int);
void probe(int wi) { int s = 0; int x, y;  if (wi < 0) { gp(&x, &y); if (inb(x, y, 0, 0) && x >= 0) { gz0 = x; gz1 = 0; } else { do { if (tab[s] == 0) continue; use_a(s); } while (++s < gm); } wi = s; } for (s = 0; s < gm; s++) use_a(s); use_b(wi); }
== slot_cmp_dead
extern int gm, gz0, gz1; extern int *tab[8]; extern void gp(int *, int *); extern int inb(int, int, int, int);
void probe(int wi) { int s = 0; int x, y; gz1 = 0; gz1 = 5; if (wi < 0) { gp(&x, &y); if (inb(x, y, 0, 0) && x >= 0) { gz0 = x;  } else { do { if (tab[s] == 0) continue; use_a(s); } while (++s < gm); } wi = s; } for (s = 0; s < gm; s++) use_a(s); use_b(wi); }
== slot_cmp_param
extern int gm, gz0, gz1; extern int *tab[8]; extern void gp(int *, int *); extern int inb(int, int, int, int);
void probe(int wi) { int s = 0; int x, y;  if (wi < 0) { gp(&x, &y); if (inb(x, y, 0, 0) && x >= 0) { gz0 = x; wi = 0; } else { do { if (tab[s] == 0) continue; use_a(s); } while (++s < gm); } wi = s; } for (s = 0; s < gm; s++) use_a(s); use_b(wi); }
== slot2_cmp_none
extern int gm, gz0, gz1; extern int *tab[8]; extern void gp(int *, int *); extern int inb(int, int, int, int);
struct MW { int z; }; extern MW *mw[8];
void probe(int wi) { int s = 0; int x, y;  if (wi < 0) { gp(&x, &y); if (inb(x, y, 0, 0) && x >= 0) { gz0 = x;  } else { do { if (tab[s] == 0) continue; use_a(s); } while (++s < gm); } wi = s; } for (s = 0; s < gm; s++) { if (mw[s] == 0) continue; use_a(s); } use_b(wi); }
== slot2_cmp_store_top
extern int gm, gz0, gz1; extern int *tab[8]; extern void gp(int *, int *); extern int inb(int, int, int, int);
struct MW { int z; }; extern MW *mw[8];
void probe(int wi) { int s = 0; int x, y; gz1 = 0; if (wi < 0) { gp(&x, &y); if (inb(x, y, 0, 0) && x >= 0) { gz0 = x;  } else { do { if (tab[s] == 0) continue; use_a(s); } while (++s < gm); } wi = s; } for (s = 0; s < gm; s++) { if (mw[s] == 0) continue; use_a(s); } use_b(wi); }
== slot2_cmp_store_arm
extern int gm, gz0, gz1; extern int *tab[8]; extern void gp(int *, int *); extern int inb(int, int, int, int);
struct MW { int z; }; extern MW *mw[8];
void probe(int wi) { int s = 0; int x, y;  if (wi < 0) { gp(&x, &y); if (inb(x, y, 0, 0) && x >= 0) { gz0 = x; gz1 = 0; } else { do { if (tab[s] == 0) continue; use_a(s); } while (++s < gm); } wi = s; } for (s = 0; s < gm; s++) { if (mw[s] == 0) continue; use_a(s); } use_b(wi); }
== slot2_cmp_dead
extern int gm, gz0, gz1; extern int *tab[8]; extern void gp(int *, int *); extern int inb(int, int, int, int);
struct MW { int z; }; extern MW *mw[8];
void probe(int wi) { int s = 0; int x, y; gz1 = 0; gz1 = 5; if (wi < 0) { gp(&x, &y); if (inb(x, y, 0, 0) && x >= 0) { gz0 = x;  } else { do { if (tab[s] == 0) continue; use_a(s); } while (++s < gm); } wi = s; } for (s = 0; s < gm; s++) { if (mw[s] == 0) continue; use_a(s); } use_b(wi); }
== slot2_cmp_param
extern int gm, gz0, gz1; extern int *tab[8]; extern void gp(int *, int *); extern int inb(int, int, int, int);
struct MW { int z; }; extern MW *mw[8];
void probe(int wi) { int s = 0; int x, y;  if (wi < 0) { gp(&x, &y); if (inb(x, y, 0, 0) && x >= 0) { gz0 = x; wi = 0; } else { do { if (tab[s] == 0) continue; use_a(s); } while (++s < gm); } wi = s; } for (s = 0; s < gm; s++) { if (mw[s] == 0) continue; use_a(s); } use_b(wi); }
