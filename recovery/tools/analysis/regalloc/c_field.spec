%% N=1,2,3,4,5,6,8
== named3
struct V { short x, y; int state; short flags; short proto; unsigned char fac; char pad[39]; };
extern V Vs[];
#define R1 use_a(Vs[id].x); call();
#define R2 R1 use_b(Vs[id].y); call();
#define R3 R2 use_c(Vs[id].state); call();
#define R4 R3 use_a(Vs[id].flags); call();
#define R5 R4 use_b(Vs[id].fac); call();
#define R6 R5 use_c(Vs[id].x); call();
#define R8 R6 use_a(Vs[id].y); call(); use_b(Vs[id].state); call();
int probe(int id) {
  D(p); D(q); U8(p); U8(q); U8(p); U8(q); call();
  int pr = Vs[id].proto;
  R{N}
  if (pr == 16 || pr == 17 || pr == 18) call();
  U(p); U(q); return 0;
}
%% N=1,2,3,4,5,6,8
== inplace3
struct V { short x, y; int state; short flags; short proto; unsigned char fac; char pad[39]; };
extern V Vs[];
#define R1 use_a(Vs[id].x); call();
#define R2 R1 use_b(Vs[id].y); call();
#define R3 R2 use_c(Vs[id].state); call();
#define R4 R3 use_a(Vs[id].flags); call();
#define R5 R4 use_b(Vs[id].fac); call();
#define R6 R5 use_c(Vs[id].x); call();
#define R8 R6 use_a(Vs[id].y); call(); use_b(Vs[id].state); call();
int probe(int id) {
  D(p); D(q); U8(p); U8(q); U8(p); U8(q); call();
  R{N}
  if (Vs[id].proto == 16 || Vs[id].proto == 17 || Vs[id].proto == 18) call();
  U(p); U(q); return 0;
}
%% N=1,2,4,8
== losernamed
struct V { short x, y; int state; short flags; short proto; unsigned char fac; char pad[39]; };
extern V Vs[];
#define R1 use_a(Vs[id].x); call();
#define R2 R1 use_b(Vs[id].y); call();
#define R3 R2 use_c(Vs[id].state); call();
#define R4 R3 use_a(Vs[id].flags); call();
#define R5 R4 use_b(Vs[id].fac); call();
#define R6 R5 use_c(Vs[id].x); call();
#define R8 R6 use_a(Vs[id].y); call(); use_b(Vs[id].state); call();
int probe(int id) {
  D(p); D(q); U8(p); U8(q); U8(p); U8(q); call();
  int pr = Vs[id].proto;
  R{N}
  if (pr == 16 || pr == 17 || pr == 18) call();
  D(r); U8(r); U8(r); U8(r); U8(r); U(p); U(q); use_a(Vs[id].x); call(); use_a(Vs[id].y);
  U(p); U(q); return 0;
}
%% N=1,2,4,8
== loserinplace
struct V { short x, y; int state; short flags; short proto; unsigned char fac; char pad[39]; };
extern V Vs[];
#define R1 use_a(Vs[id].x); call();
#define R2 R1 use_b(Vs[id].y); call();
#define R3 R2 use_c(Vs[id].state); call();
#define R4 R3 use_a(Vs[id].flags); call();
#define R5 R4 use_b(Vs[id].fac); call();
#define R6 R5 use_c(Vs[id].x); call();
#define R8 R6 use_a(Vs[id].y); call(); use_b(Vs[id].state); call();
int probe(int id) {
  D(p); D(q); U8(p); U8(q); U8(p); U8(q); call();
  R{N}
  if (Vs[id].proto == 16 || Vs[id].proto == 17 || Vs[id].proto == 18) call();
  D(r); U8(r); U8(r); U8(r); U8(r); U(p); U(q); use_a(Vs[id].x); call(); use_a(Vs[id].y);
  U(p); U(q); return 0;
}
%% M=,use_d(pr);,use_d(pr);use_e(pr);,use_d(pr);use_e(pr);use_f(pr);,use_d(pr);use_e(pr);use_f(pr);use_g(pr);use_h(pr);,use_d(pr);use_e(pr);use_f(pr);use_g(pr);use_h(pr);use_d(pr);use_e(pr);use_f(pr);use_g(pr);use_h(pr);
== useM_N2
struct V { short x, y; int state; short flags; short proto; unsigned char fac; char pad[39]; };
extern V Vs[];
#define R1 use_a(Vs[id].x); call();
#define R2 R1 use_b(Vs[id].y); call();
#define R3 R2 use_c(Vs[id].state); call();
#define R4 R3 use_a(Vs[id].flags); call();
#define R5 R4 use_b(Vs[id].fac); call();
#define R6 R5 use_c(Vs[id].x); call();
#define R8 R6 use_a(Vs[id].y); call(); use_b(Vs[id].state); call();
int probe(int id) { D(p); D(q); U8(p); U8(q); U8(p); U8(q); call(); int pr = Vs[id].proto; {M} R2 if (pr == 16 || pr == 17 || pr == 18) call(); D(r); U8(r); U8(r); U8(r); U8(r); U(p); U(q); use_a(Vs[id].x); call(); use_a(Vs[id].y); U(p); U(q); return 0; }
%% M=use_d(garr[Vs[id].proto]);,use_d(garr[Vs[id].proto]);use_e(gobj[Vs[id].proto].f1);,if(cond()) use_d(Vs[id].proto);
== cseM_N2
struct V { short x, y; int state; short flags; short proto; unsigned char fac; char pad[39]; };
extern V Vs[];
#define R1 use_a(Vs[id].x); call();
#define R2 R1 use_b(Vs[id].y); call();
#define R3 R2 use_c(Vs[id].state); call();
#define R4 R3 use_a(Vs[id].flags); call();
#define R5 R4 use_b(Vs[id].fac); call();
#define R6 R5 use_c(Vs[id].x); call();
#define R8 R6 use_a(Vs[id].y); call(); use_b(Vs[id].state); call();
int probe(int id) { D(p); D(q); U8(p); U8(q); U8(p); U8(q); call(); int pr = Vs[id].proto; {M} R2 if (pr == 16 || pr == 17 || pr == 18) call(); D(r); U8(r); U8(r); U8(r); U8(r); U(p); U(q); use_a(Vs[id].x); call(); use_a(Vs[id].y); U(p); U(q); return 0; }
%% M=use_d(pr);,use_d(pr);use_e(pr);,use_d(pr);use_e(pr);use_f(pr);,use_d(pr);use_e(pr);use_f(pr);use_g(pr);use_h(pr);
== lateM_N2
struct V { short x, y; int state; short flags; short proto; unsigned char fac; char pad[39]; };
extern V Vs[];
#define R1 use_a(Vs[id].x); call();
#define R2 R1 use_b(Vs[id].y); call();
#define R3 R2 use_c(Vs[id].state); call();
#define R4 R3 use_a(Vs[id].flags); call();
#define R5 R4 use_b(Vs[id].fac); call();
#define R6 R5 use_c(Vs[id].x); call();
#define R8 R6 use_a(Vs[id].y); call(); use_b(Vs[id].state); call();
int probe(int id) { D(p); D(q); U8(p); U8(q); U8(p); U8(q); call(); int pr = Vs[id].proto; R2 {M} if (pr == 16 || pr == 17 || pr == 18) call(); D(r); U8(r); U8(r); U8(r); U8(r); U(p); U(q); use_a(Vs[id].x); call(); use_a(Vs[id].y); U(p); U(q); return 0; }
%% M=,use_d(pr);,use_d(pr);use_e(pr);,use_d(pr);use_e(pr);use_f(pr);,use_d(pr);use_e(pr);use_f(pr);use_g(pr);use_h(pr);,use_d(pr);use_e(pr);use_f(pr);use_g(pr);use_h(pr);use_d(pr);use_e(pr);use_f(pr);use_g(pr);use_h(pr);
== useM_N4
struct V { short x, y; int state; short flags; short proto; unsigned char fac; char pad[39]; };
extern V Vs[];
#define R1 use_a(Vs[id].x); call();
#define R2 R1 use_b(Vs[id].y); call();
#define R3 R2 use_c(Vs[id].state); call();
#define R4 R3 use_a(Vs[id].flags); call();
#define R5 R4 use_b(Vs[id].fac); call();
#define R6 R5 use_c(Vs[id].x); call();
#define R8 R6 use_a(Vs[id].y); call(); use_b(Vs[id].state); call();
int probe(int id) { D(p); D(q); U8(p); U8(q); U8(p); U8(q); call(); int pr = Vs[id].proto; {M} R4 if (pr == 16 || pr == 17 || pr == 18) call(); D(r); U8(r); U8(r); U8(r); U8(r); U(p); U(q); use_a(Vs[id].x); call(); use_a(Vs[id].y); U(p); U(q); return 0; }
%% M=use_d(garr[Vs[id].proto]);,use_d(garr[Vs[id].proto]);use_e(gobj[Vs[id].proto].f1);,if(cond()) use_d(Vs[id].proto);
== cseM_N4
struct V { short x, y; int state; short flags; short proto; unsigned char fac; char pad[39]; };
extern V Vs[];
#define R1 use_a(Vs[id].x); call();
#define R2 R1 use_b(Vs[id].y); call();
#define R3 R2 use_c(Vs[id].state); call();
#define R4 R3 use_a(Vs[id].flags); call();
#define R5 R4 use_b(Vs[id].fac); call();
#define R6 R5 use_c(Vs[id].x); call();
#define R8 R6 use_a(Vs[id].y); call(); use_b(Vs[id].state); call();
int probe(int id) { D(p); D(q); U8(p); U8(q); U8(p); U8(q); call(); int pr = Vs[id].proto; {M} R4 if (pr == 16 || pr == 17 || pr == 18) call(); D(r); U8(r); U8(r); U8(r); U8(r); U(p); U(q); use_a(Vs[id].x); call(); use_a(Vs[id].y); U(p); U(q); return 0; }
%% M=use_d(pr);,use_d(pr);use_e(pr);,use_d(pr);use_e(pr);use_f(pr);,use_d(pr);use_e(pr);use_f(pr);use_g(pr);use_h(pr);
== lateM_N4
struct V { short x, y; int state; short flags; short proto; unsigned char fac; char pad[39]; };
extern V Vs[];
#define R1 use_a(Vs[id].x); call();
#define R2 R1 use_b(Vs[id].y); call();
#define R3 R2 use_c(Vs[id].state); call();
#define R4 R3 use_a(Vs[id].flags); call();
#define R5 R4 use_b(Vs[id].fac); call();
#define R6 R5 use_c(Vs[id].x); call();
#define R8 R6 use_a(Vs[id].y); call(); use_b(Vs[id].state); call();
int probe(int id) { D(p); D(q); U8(p); U8(q); U8(p); U8(q); call(); int pr = Vs[id].proto; R4 {M} if (pr == 16 || pr == 17 || pr == 18) call(); D(r); U8(r); U8(r); U8(r); U8(r); U(p); U(q); use_a(Vs[id].x); call(); use_a(Vs[id].y); U(p); U(q); return 0; }
%% M=,use_d(pr);,use_d(pr);use_e(pr);,use_d(pr);use_e(pr);use_f(pr);,use_d(pr);use_e(pr);use_f(pr);use_g(pr);use_h(pr);,use_d(pr);use_e(pr);use_f(pr);use_g(pr);use_h(pr);use_d(pr);use_e(pr);use_f(pr);use_g(pr);use_h(pr);
== useM_N8
struct V { short x, y; int state; short flags; short proto; unsigned char fac; char pad[39]; };
extern V Vs[];
#define R1 use_a(Vs[id].x); call();
#define R2 R1 use_b(Vs[id].y); call();
#define R3 R2 use_c(Vs[id].state); call();
#define R4 R3 use_a(Vs[id].flags); call();
#define R5 R4 use_b(Vs[id].fac); call();
#define R6 R5 use_c(Vs[id].x); call();
#define R8 R6 use_a(Vs[id].y); call(); use_b(Vs[id].state); call();
int probe(int id) { D(p); D(q); U8(p); U8(q); U8(p); U8(q); call(); int pr = Vs[id].proto; {M} R8 if (pr == 16 || pr == 17 || pr == 18) call(); D(r); U8(r); U8(r); U8(r); U8(r); U(p); U(q); use_a(Vs[id].x); call(); use_a(Vs[id].y); U(p); U(q); return 0; }
%% M=use_d(garr[Vs[id].proto]);,use_d(garr[Vs[id].proto]);use_e(gobj[Vs[id].proto].f1);,if(cond()) use_d(Vs[id].proto);
== cseM_N8
struct V { short x, y; int state; short flags; short proto; unsigned char fac; char pad[39]; };
extern V Vs[];
#define R1 use_a(Vs[id].x); call();
#define R2 R1 use_b(Vs[id].y); call();
#define R3 R2 use_c(Vs[id].state); call();
#define R4 R3 use_a(Vs[id].flags); call();
#define R5 R4 use_b(Vs[id].fac); call();
#define R6 R5 use_c(Vs[id].x); call();
#define R8 R6 use_a(Vs[id].y); call(); use_b(Vs[id].state); call();
int probe(int id) { D(p); D(q); U8(p); U8(q); U8(p); U8(q); call(); int pr = Vs[id].proto; {M} R8 if (pr == 16 || pr == 17 || pr == 18) call(); D(r); U8(r); U8(r); U8(r); U8(r); U(p); U(q); use_a(Vs[id].x); call(); use_a(Vs[id].y); U(p); U(q); return 0; }
%% M=use_d(pr);,use_d(pr);use_e(pr);,use_d(pr);use_e(pr);use_f(pr);,use_d(pr);use_e(pr);use_f(pr);use_g(pr);use_h(pr);
== lateM_N8
struct V { short x, y; int state; short flags; short proto; unsigned char fac; char pad[39]; };
extern V Vs[];
#define R1 use_a(Vs[id].x); call();
#define R2 R1 use_b(Vs[id].y); call();
#define R3 R2 use_c(Vs[id].state); call();
#define R4 R3 use_a(Vs[id].flags); call();
#define R5 R4 use_b(Vs[id].fac); call();
#define R6 R5 use_c(Vs[id].x); call();
#define R8 R6 use_a(Vs[id].y); call(); use_b(Vs[id].state); call();
int probe(int id) { D(p); D(q); U8(p); U8(q); U8(p); U8(q); call(); int pr = Vs[id].proto; R8 {M} if (pr == 16 || pr == 17 || pr == 18) call(); D(r); U8(r); U8(r); U8(r); U8(r); U(p); U(q); use_a(Vs[id].x); call(); use_a(Vs[id].y); U(p); U(q); return 0; }
%% J=0,1,2,4,8,16
== latepr
struct V { short x, y; int state; short flags; short proto; unsigned char fac; char pad[39]; };
extern V Vs[];
#define R1 use_a(Vs[id].x); call();
#define R2 R1 use_b(Vs[id].y); call();
#define R3 R2 use_c(Vs[id].state); call();
#define R4 R3 use_a(Vs[id].flags); call();
#define R5 R4 use_b(Vs[id].fac); call();
#define R6 R5 use_c(Vs[id].x); call();
#define R8 R6 use_a(Vs[id].y); call(); use_b(Vs[id].state); call();
#define J1 call(); use_d(pr);
#define J2 J1 J1
#define J4 J2 J2
#define J8 J4 J4
#define J16 J8 J8
#define J0
#define T1 call(); use_a(Vs[id].x);
#define T2 T1 T1
#define T4 T2 T2
#define T8 T4 T4
#define T16 T8 T8
#define T0
int probe(int id) { D(p); D(q); U8(p); U8(q); U8(p); U8(q); call(); int pr = Vs[id].proto; R4 if (pr == 16 || pr == 17 || pr == 18) call(); D(r); U8(r); U8(r); U8(r); U8(r); U(p); U(q); J{J} U8(r); U8(r); use_a(Vs[id].x); call(); use_a(Vs[id].y); U(p); U(q); return 0; }
%% J=0,1,2,4,8,16
== latetmp
struct V { short x, y; int state; short flags; short proto; unsigned char fac; char pad[39]; };
extern V Vs[];
#define R1 use_a(Vs[id].x); call();
#define R2 R1 use_b(Vs[id].y); call();
#define R3 R2 use_c(Vs[id].state); call();
#define R4 R3 use_a(Vs[id].flags); call();
#define R5 R4 use_b(Vs[id].fac); call();
#define R6 R5 use_c(Vs[id].x); call();
#define R8 R6 use_a(Vs[id].y); call(); use_b(Vs[id].state); call();
#define J1 call(); use_d(pr);
#define J2 J1 J1
#define J4 J2 J2
#define J8 J4 J4
#define J16 J8 J8
#define J0
#define T1 call(); use_a(Vs[id].x);
#define T2 T1 T1
#define T4 T2 T2
#define T8 T4 T4
#define T16 T8 T8
#define T0
int probe(int id) { D(p); D(q); U8(p); U8(q); U8(p); U8(q); call(); int pr = Vs[id].proto; use_d(pr);use_e(pr);use_f(pr);use_g(pr);use_h(pr); R2 if (pr == 16 || pr == 17 || pr == 18) call(); D(r); U8(r); U8(r); U8(r); U8(r); U(p); U(q); T{J} U8(r); U8(r); U(p); U(q); return 0; }
%% M=garr[pr],garr[pr]+gobj[pr].f1,garr[pr]+gobj[pr].f1+gobj[pr].f2+gobj[pr].f3,garr[pr]+gobj[pr].f1+gobj[pr].f2+gobj[pr].f3+gobj[pr].arr[1]+gobj[pr].arr[2]+pr*7+(pr>>1)
== spell_pr
extern int cond2(int,int);
struct V { short x, y; int state; short flags; short proto; unsigned char fac; char pad[39]; };
extern V Vs[];
#define R1 use_a(Vs[id].x); call();
#define R2 R1 use_b(Vs[id].y); call();
#define R3 R2 use_c(Vs[id].state); call();
#define R4 R3 use_a(Vs[id].flags); call();
#define R5 R4 use_b(Vs[id].fac); call();
#define R6 R5 use_c(Vs[id].x); call();
#define R8 R6 use_a(Vs[id].y); call(); use_b(Vs[id].state); call();
int probe(int id) { D(p); D(q); U8(p); U8(q); U8(p); U8(q); call(); int pr = Vs[id].proto; int s = {M}; use_d(s); R2 if (pr == 16 || pr == 17 || pr == 18) call(); D(r); U8(r); U8(r); U8(r); U8(r); U(p); U(q); use_a(Vs[id].x); call(); use_a(Vs[id].y); U(p); U(q); return 0; }
%% M=0
== push_pr
extern int cond2(int,int);
struct V { short x, y; int state; short flags; short proto; unsigned char fac; char pad[39]; };
extern V Vs[];
#define R1 use_a(Vs[id].x); call();
#define R2 R1 use_b(Vs[id].y); call();
#define R3 R2 use_c(Vs[id].state); call();
#define R4 R3 use_a(Vs[id].flags); call();
#define R5 R4 use_b(Vs[id].fac); call();
#define R6 R5 use_c(Vs[id].x); call();
#define R8 R6 use_a(Vs[id].y); call(); use_b(Vs[id].state); call();
int probe(int id) { D(p); D(q); U8(p); U8(q); U8(p); U8(q); call(); int pr = Vs[id].proto; if (garr[pr] == 3 && cond2(pr, 5)) use_e(1); R2 if (pr == 16 || pr == 17 || pr == 18) call(); D(r); U8(r); U8(r); U8(r); U8(r); U(p); U(q); use_a(Vs[id].x); call(); use_a(Vs[id].y); U(p); U(q); return 0; }
%% M=0
== push2_pr
extern int cond2(int,int);
struct V { short x, y; int state; short flags; short proto; unsigned char fac; char pad[39]; };
extern V Vs[];
#define R1 use_a(Vs[id].x); call();
#define R2 R1 use_b(Vs[id].y); call();
#define R3 R2 use_c(Vs[id].state); call();
#define R4 R3 use_a(Vs[id].flags); call();
#define R5 R4 use_b(Vs[id].fac); call();
#define R6 R5 use_c(Vs[id].x); call();
#define R8 R6 use_a(Vs[id].y); call(); use_b(Vs[id].state); call();
int probe(int id) { D(p); D(q); U8(p); U8(q); U8(p); U8(q); call(); int pr = Vs[id].proto; if (garr[pr] == 3 && cond2(pr, 5)) use_e(1); R2 call(); use_f(pr); call(); use_f(pr); D(r); U8(r); U8(r); U8(r); U8(r); U(p); U(q); use_a(Vs[id].x); call(); use_a(Vs[id].y); U(p); U(q); return 0; }
%% M=garr[Vs[id].proto],garr[Vs[id].proto]+gobj[Vs[id].proto].f1,garr[Vs[id].proto]+gobj[Vs[id].proto].f1+gobj[Vs[id].proto].f2+gobj[Vs[id].proto].f3,garr[Vs[id].proto]+gobj[Vs[id].proto].f1+gobj[Vs[id].proto].f2+gobj[Vs[id].proto].f3+gobj[Vs[id].proto].arr[1]+gobj[Vs[id].proto].arr[2]+Vs[id].proto*7+(Vs[id].proto>>1)
== spell_sidproto
extern int cond2(int,int);
struct V { short x, y; int state; short flags; short proto; unsigned char fac; char pad[39]; };
extern V Vs[];
#define R1 use_a(Vs[id].x); call();
#define R2 R1 use_b(Vs[id].y); call();
#define R3 R2 use_c(Vs[id].state); call();
#define R4 R3 use_a(Vs[id].flags); call();
#define R5 R4 use_b(Vs[id].fac); call();
#define R6 R5 use_c(Vs[id].x); call();
#define R8 R6 use_a(Vs[id].y); call(); use_b(Vs[id].state); call();
int probe(int id) { D(p); D(q); U8(p); U8(q); U8(p); U8(q); call(); int pr = Vs[id].proto; int s = {M}; use_d(s); R2 if (pr == 16 || pr == 17 || pr == 18) call(); D(r); U8(r); U8(r); U8(r); U8(r); U(p); U(q); use_a(Vs[id].x); call(); use_a(Vs[id].y); U(p); U(q); return 0; }
%% M=0
== push_sidproto
extern int cond2(int,int);
struct V { short x, y; int state; short flags; short proto; unsigned char fac; char pad[39]; };
extern V Vs[];
#define R1 use_a(Vs[id].x); call();
#define R2 R1 use_b(Vs[id].y); call();
#define R3 R2 use_c(Vs[id].state); call();
#define R4 R3 use_a(Vs[id].flags); call();
#define R5 R4 use_b(Vs[id].fac); call();
#define R6 R5 use_c(Vs[id].x); call();
#define R8 R6 use_a(Vs[id].y); call(); use_b(Vs[id].state); call();
int probe(int id) { D(p); D(q); U8(p); U8(q); U8(p); U8(q); call(); int pr = Vs[id].proto; if (garr[Vs[id].proto] == 3 && cond2(Vs[id].proto, 5)) use_e(1); R2 if (pr == 16 || pr == 17 || pr == 18) call(); D(r); U8(r); U8(r); U8(r); U8(r); U(p); U(q); use_a(Vs[id].x); call(); use_a(Vs[id].y); U(p); U(q); return 0; }
%% M=0
== push2_sidproto
extern int cond2(int,int);
struct V { short x, y; int state; short flags; short proto; unsigned char fac; char pad[39]; };
extern V Vs[];
#define R1 use_a(Vs[id].x); call();
#define R2 R1 use_b(Vs[id].y); call();
#define R3 R2 use_c(Vs[id].state); call();
#define R4 R3 use_a(Vs[id].flags); call();
#define R5 R4 use_b(Vs[id].fac); call();
#define R6 R5 use_c(Vs[id].x); call();
#define R8 R6 use_a(Vs[id].y); call(); use_b(Vs[id].state); call();
int probe(int id) { D(p); D(q); U8(p); U8(q); U8(p); U8(q); call(); int pr = Vs[id].proto; if (garr[Vs[id].proto] == 3 && cond2(Vs[id].proto, 5)) use_e(1); R2 call(); use_f(pr); call(); use_f(pr); D(r); U8(r); U8(r); U8(r); U8(r); U(p); U(q); use_a(Vs[id].x); call(); use_a(Vs[id].y); U(p); U(q); return 0; }
%% M=,use_d(pr);
== ret
struct V { short x, y; int state; short flags; short proto; unsigned char fac; char pad[39]; };
extern V Vs[];
#define R1 use_a(Vs[id].x); call();
#define R2 R1 use_b(Vs[id].y); call();
#define R3 R2 use_c(Vs[id].state); call();
#define R4 R3 use_a(Vs[id].flags); call();
#define R5 R4 use_b(Vs[id].fac); call();
#define R6 R5 use_c(Vs[id].x); call();
#define R8 R6 use_a(Vs[id].y); call(); use_b(Vs[id].state); call();
extern int alien(int); extern int rng(int,int,int); extern int pb(int); extern int gflag; extern unsigned char tab[];
int probe(int id) { D(p); D(q); U8(p); U8(q); U8(p); U8(q); call(); if (!Vs[id].fac && !gflag) return alien(id); int pr = Vs[id].proto; {M} R2 if (pr == 16 || pr == 17 || pr == 18) call(); D(r); U8(r); U8(r); U8(r); U8(r); U(p); U(q); use_a(Vs[id].x); call(); use_a(Vs[id].y); U(p); U(q); return 0; }
%% M=,use_d(pr);
== chain
struct V { short x, y; int state; short flags; short proto; unsigned char fac; char pad[39]; };
extern V Vs[];
#define R1 use_a(Vs[id].x); call();
#define R2 R1 use_b(Vs[id].y); call();
#define R3 R2 use_c(Vs[id].state); call();
#define R4 R3 use_a(Vs[id].flags); call();
#define R5 R4 use_b(Vs[id].fac); call();
#define R6 R5 use_c(Vs[id].x); call();
#define R8 R6 use_a(Vs[id].y); call(); use_b(Vs[id].state); call();
extern int alien(int); extern int rng(int,int,int); extern int pb(int); extern int gflag; extern unsigned char tab[];
int probe(int id) { D(p); D(q); U8(p); U8(q); U8(p); U8(q); call(); if (!Vs[id].fac && !gflag) return alien(id); int pr = Vs[id].proto; {M} use_a(Vs[id].x); use_b(Vs[id].y); { int dmg = Vs[id].fac; int damage; if (dmg >= rng(tab[Vs[id].proto], 1, 100) / 2) damage = 4; else if (dmg > rng(tab[Vs[id].proto], 1, 100) / 3) damage = 3; else if (dmg > tab[Vs[id].proto] * 2) damage = 2; else damage = dmg != 0; if (pb(id)) damage = 0; if (pr == 16 || pr == 17 || pr == 18) damage = 0; use_e(damage); } D(r); U8(r); U8(r); U8(r); U8(r); U(p); U(q); use_a(Vs[id].x); call(); use_a(Vs[id].y); U(p); U(q); return 0; }
%% M=,use_d(pr);
== chain_inpl
struct V { short x, y; int state; short flags; short proto; unsigned char fac; char pad[39]; };
extern V Vs[];
#define R1 use_a(Vs[id].x); call();
#define R2 R1 use_b(Vs[id].y); call();
#define R3 R2 use_c(Vs[id].state); call();
#define R4 R3 use_a(Vs[id].flags); call();
#define R5 R4 use_b(Vs[id].fac); call();
#define R6 R5 use_c(Vs[id].x); call();
#define R8 R6 use_a(Vs[id].y); call(); use_b(Vs[id].state); call();
extern int alien(int); extern int rng(int,int,int); extern int pb(int); extern int gflag; extern unsigned char tab[];
int probe(int id) { D(p); D(q); U8(p); U8(q); U8(p); U8(q); call(); if (!Vs[id].fac && !gflag) return alien(id); int pr = Vs[id].proto; {M} use_a(Vs[id].x); use_b(Vs[id].y); { int dmg = Vs[id].fac; int damage; if (dmg >= rng(tab[Vs[id].proto], 1, 100) / 2) damage = 4; else if (dmg > rng(tab[Vs[id].proto], 1, 100) / 3) damage = 3; else if (dmg > tab[Vs[id].proto] * 2) damage = 2; else damage = dmg != 0; if (pb(id)) damage = 0; if (Vs[id].proto == 16 || Vs[id].proto == 17 || Vs[id].proto == 18) damage = 0; use_e(damage); } use_f(pr); D(r); U8(r); U8(r); U8(r); U8(r); U(p); U(q); use_a(Vs[id].x); call(); use_a(Vs[id].y); U(p); U(q); return 0; }
%% K=B0,B1,B4,B16,B64
== holes
struct V { short x, y; int state; short flags; short proto; unsigned char fac; char pad[39]; };
extern V Vs[];
#define R1 use_a(Vs[id].x); call();
#define R2 R1 use_b(Vs[id].y); call();
#define R3 R2 use_c(Vs[id].state); call();
#define R4 R3 use_a(Vs[id].flags); call();
#define R5 R4 use_b(Vs[id].fac); call();
#define R6 R5 use_c(Vs[id].x); call();
#define R8 R6 use_a(Vs[id].y); call(); use_b(Vs[id].state); call();
#define B0
#define B1 if (cond()) { U(r); U(p); }
#define B4 B1 B1 B1 B1
#define B16 B4 B4 B4 B4
#define B64 B16 B16 B16 B16
int probe(int id) { D(p); D(q); U8(p); U8(q); U8(p); U8(q); call(); int pr = Vs[id].proto; R4 if (pr == 16 || pr == 17 || pr == 18) call(); D(r); U8(r); U8(r); U8(r); U8(r); U(p); U(q); {K} use_a(Vs[id].x); call(); use_a(Vs[id].y); U(p); U(q); return 0; }
%% K=B0,B1,B4,B16,B64
== holes_prlate
struct V { short x, y; int state; short flags; short proto; unsigned char fac; char pad[39]; };
extern V Vs[];
#define R1 use_a(Vs[id].x); call();
#define R2 R1 use_b(Vs[id].y); call();
#define R3 R2 use_c(Vs[id].state); call();
#define R4 R3 use_a(Vs[id].flags); call();
#define R5 R4 use_b(Vs[id].fac); call();
#define R6 R5 use_c(Vs[id].x); call();
#define R8 R6 use_a(Vs[id].y); call(); use_b(Vs[id].state); call();
#define B0
#define B1 if (cond()) { U(r); U(p); }
#define B4 B1 B1 B1 B1
#define B16 B4 B4 B4 B4
#define B64 B16 B16 B16 B16
int probe(int id) { D(p); D(q); U8(p); U8(q); U8(p); U8(q); call(); int pr = Vs[id].proto; R4 if (pr == 16 || pr == 17 || pr == 18) call(); D(r); U8(r); U8(r); U8(r); U8(r); U(p); U(q); {K} use_a(Vs[id].x); call(); use_a(Vs[id].y); use_d(pr); U(p); U(q); return 0; }
