#include "probe.h"
// Console::set_ai: image `mov cl,[g]; movsx eax,cl; and eax,1; ...; movsx eax,cl; and eax,2; shr eax,1` (byte loaded once, movsx per use)
extern int gint;
struct PD { int a, b, c, d; };
extern PD pd[];
void A__intcast(int f) { if (g0() >= 0) { pd[f].a = (char)gint & 1u; pd[f].b = ((char)gint & 2u) >> 1; pd[f].c = ((char)gint & 4u) >> 2; pd[f].d = ((char)gint & 8u) >> 3; f1(f); } }
void A__charg(int f) { if (g0() >= 0) { pd[f].a = gch & 1u; pd[f].b = (gch & 2u) >> 1; pd[f].c = (gch & 4u) >> 2; pd[f].d = (gch & 8u) >> 3; f1(f); } }
void A__scharg(int f) { if (g0() >= 0) { pd[f].a = gsc & 1u; pd[f].b = (gsc & 2u) >> 1; pd[f].c = (gsc & 4u) >> 2; pd[f].d = (gsc & 8u) >> 3; f1(f); } }
void A__charl(int f) { if (g0() >= 0) { char s = gch; pd[f].a = s & 1u; pd[f].b = (s & 2u) >> 1; pd[f].c = (s & 4u) >> 2; pd[f].d = (s & 8u) >> 3; f1(f); } }
void A__charl_int(int f) { if (g0() >= 0) { char s = (char)gint; pd[f].a = s & 1u; pd[f].b = (s & 2u) >> 1; pd[f].c = (s & 4u) >> 2; pd[f].d = (s & 8u) >> 3; f1(f); } }
void A__charl_signed(int f) { if (g0() >= 0) { char s = gch; pd[f].a = s & 1; pd[f].b = (s & 2) >> 1; pd[f].c = (s & 4) >> 2; pd[f].d = (s & 8) >> 3; f1(f); } }
void A__charl_ne(int f) { if (g0() >= 0) { char s = gch; pd[f].a = (s & 1) != 0; pd[f].b = (s & 2) != 0; pd[f].c = (s & 4) != 0; pd[f].d = (s & 8) != 0; f1(f); } }
void A__charl_shift(int f) { if (g0() >= 0) { char s = gch; pd[f].a = s & 1; pd[f].b = s >> 1 & 1; pd[f].c = s >> 2 & 1; pd[f].d = s >> 3 & 1; f1(f); } }
void A__charl_uint(int f) { if (g0() >= 0) { char s = gch; pd[f].a = (unsigned)s & 1; pd[f].b = ((unsigned)s & 2) >> 1; pd[f].c = ((unsigned)s & 4) >> 2; pd[f].d = ((unsigned)s & 8) >> 3; f1(f); } }
void A__ptrs(int f) { int *a = &pd[f].a, *b = &pd[f].b, *c = &pd[f].c, *d = &pd[f].d; if (g0() >= 0) { char s = gch; *a = s & 1u; *b = (s & 2u) >> 1; *c = (s & 4u) >> 2; *d = (s & 8u) >> 3; f1(f); } }
