#include "probe.h"
extern "C" char *strcat(char *, const char *);
template <class T> inline T nv(T v) { return v; }
struct E { int id; char *text; int m; };
// the destination read through a pointer kept in a callee-saved register
void H1__expr(E *p, char *text) { f1(ga + 1); strcat(p->text, text); f1(p->m); strcat(p->text, "x"); f1(p->id); }
void H1__nv(E *p, char *text) { f1(ga + 1); strcat(nv(p->text), text); f1(p->m); strcat(p->text, "x"); f1(p->id); }
void H2__expr(E *p, char *text) { f1(ga + 1); f1(p->m); strcat(p->text, text); f1(p->id); f1(p->m); }
void H2__nv(E *p, char *text) { f1(ga + 1); f1(p->m); strcat(nv(p->text), text); f1(p->id); f1(p->m); }
void H3__expr(E *p, char *text) { f1(ga + 1); f1(p->m); strcat(p->text, "x"); f1(p->id); f1(p->m); }
void H3__nv(E *p, char *text) { f1(ga + 1); f1(p->m); strcat(nv(p->text), "x"); f1(p->id); f1(p->m); }
void H4__expr(E *p, char *text) { f1(ga + 1); f1(p->m); f2(p->id, gb); f1(p->id); f1(p->m); }
void H4__nv(E *p, char *text) { f1(ga + 1); f1(p->m); f2(nv(p->id), gb); f1(p->id); f1(p->m); }
