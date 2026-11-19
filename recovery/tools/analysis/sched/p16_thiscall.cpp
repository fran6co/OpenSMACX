#include "probe.h"
extern int ge, gf, gg, gh;
extern char **gpp;
struct ST { char *get(char *); };
extern ST gst;
inline char *lg(int k) { char *v = gpp[k]; return gst.get(v); }
inline char *lg2(int k) { return gst.get(gpp[k]); }
struct M { void add(int, char *); void show(int, int, int); };
void t__expr(M *m, int x, int y) { f1(ga + 1); m->add(2, gst.get(gpp[5])); m->show(x, y, 0); f1(gf); }
void t__lg(M *m, int x, int y) { f1(ga + 1); m->add(2, lg(5)); m->show(x, y, 0); f1(gf); }
void t__lg2(M *m, int x, int y) { f1(ga + 1); m->add(2, lg2(5)); m->show(x, y, 0); f1(gf); }
void t__named(M *m, int x, int y) { f1(ga + 1); char *v = gpp[5]; m->add(2, gst.get(v)); m->show(x, y, 0); f1(gf); }
void u__expr(M *m, int x, int y) { f1(ga + 1); f1(gb + 1); m->add(2, gst.get(gpp[5])); m->show(x, y, 0); f1(gf); }
void u__lg(M *m, int x, int y) { f1(ga + 1); f1(gb + 1); m->add(2, lg(5)); m->show(x, y, 0); f1(gf); }
void v__expr(M *m, int x, int y) { f2(ga, gb); m->add(2, gst.get(gpp[5])); m->show(x, y, 0); f1(gf); }
void v__lg(M *m, int x, int y) { f2(ga, gb); m->add(2, lg(5)); m->show(x, y, 0); f1(gf); }
