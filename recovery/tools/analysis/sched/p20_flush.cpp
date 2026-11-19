#include "probe.h"
extern "C" char *strcat(char *, const char *);
template <class T> inline T nv(T v) { return v; }
struct E { int id; char *text; int m; };
// pops flushed by a branch between calls
void F1__expr(E *p, char *text) { f1(ga + 1); if (gc) return; strcat(p->text, text); if (gd) return; f1(gb); }
void F1__nv(E *p, char *text) { f1(ga + 1); if (gc) return; strcat(nv(p->text), text); if (gd) return; f1(gb); }
void F2__expr(E *p, char *text) { f1(ga + 1); strcat(p->text, text); f1(gb); }
void F2__nv(E *p, char *text) { f1(ga + 1); strcat(nv(p->text), text); f1(gb); }
void F3__expr(E *p, char *text) { f1(ga + 1); f1(gb + 1); if (gc) return; strcat(p->text, text); if (gd) return; f1(gb); }
void F3__nv(E *p, char *text) { f1(ga + 1); f1(gb + 1); if (gc) return; strcat(nv(p->text), text); if (gd) return; f1(gb); }
void G1__expr(char *text) { f1(ga + 1); if (gc) return; f2(gb, gd); if (gd) return; f1(gb); }
void G1__nv(char *text) { f1(ga + 1); if (gc) return; f2(nv(gb), gd); if (gd) return; f1(gb); }
void G2__expr(char *text) { f1(ga + 1); if (gc) return; f2(gd, gb); if (gd) return; f1(gb); }
void G2__nv(char *text) { f1(ga + 1); if (gc) return; f2(gd, nv(gb)); if (gd) return; f1(gb); }
void G3__expr(int *p) { f1(ga + 1); if (gc) return; f2(p[1], gd); if (gd) return; f1(gb); }
void G3__nv(int *p) { f1(ga + 1); if (gc) return; f2(nv(p[1]), gd); if (gd) return; f1(gb); }
void G4__expr(int *p) { f1(ga + 1); f2(p[1], gd); f1(gb); }
void G4__nv(int *p) { f1(ga + 1); f2(nv(p[1]), gd); f1(gb); }
