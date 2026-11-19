#include "probe.h"
// add_menu shape: strcat(entries_[slot].text, text) with text a parameter
extern "C" char *strcat(char *, const char *);
struct E { int id; char *text; int m; };
struct MN { int count; E e[15]; void add(int slot, char *text); void add2(int slot, char *text);
            void add3(int slot, char *text); void add4(int slot, char *text); };
void MN::add(int slot, char *text) {
    f1(ga + 1);
    strcat(e[slot].text, text);
    f1(e[slot].m);
}
void MN::add2(int slot, char *text) {
    f1(ga + 1);
    { char *d = e[slot].text; strcat(d, text); }
    f1(e[slot].m);
}
void P__expr(E *p, char *text) { f1(ga + 1); strcat(p->text, text); f1(gb); }
void P__named(E *p, char *text) { f1(ga + 1); char *d = p->text; strcat(d, text); f1(gb); }
void G__expr(E *p, char *text) { f1(ga + 1); f2(p->id, ga); f1(gb); }
void G__named(E *p, char *text) { f1(ga + 1); int d = p->id; f2(d, ga); f1(gb); }
void Q__expr(E *p, int x) { f1(ga + 1); f2(p->id, x); f1(gb); }
void Q__named(E *p, int x) { f1(ga + 1); int d = p->id; f2(d, x); f1(gb); }
void Q2__expr(E *p, int x) { f1(ga + 1); f2(gb, p->id); f1(gb); }
void Q2__named(E *p, int x) { f1(ga + 1); int d = p->id; f2(gb, d); f1(gb); }
