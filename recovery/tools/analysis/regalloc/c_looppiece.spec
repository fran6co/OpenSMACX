// Loop-piece probes (MATCHING_GUIDE 2.7 "A loser's loop reference"): a memory-homed parameter read once
// per iteration in two loops. Read with: python3 cand.py c_looppiece.spec --grep '_reload|sp1@|sp2@|clr@'
== clr
extern unsigned char gmask; extern int g1, g2, g3, g4, gT[8];
extern int sp1(int, int, int); extern int sp2(int, int, int); extern void clr(); extern void lab(); extern void mid(); extern int tst(int);
void probe(int reload) {
    int res[8];
    g1 = 0; g2 = 0; g3 = 0; g4 = 0;
    if (g1 == 0 && reload == 0) call();
    g1 = 0; g2 = 0; g3 = 0; g4 = 0;
    if (!g2 && reload != 0) call();
    for (int m = 1; m < 8; m++) {
        D(a); D(b);
        U(a); U(b); U(a); U(b); use_c(m); use_c(m); U(a); U(b);
    }
    for (int i = 0; i < 8; i++) {
        if (gmask & (1 << i)) { sp1(i, -1 - reload, 0); res[i] = 0; }
    }
    clr();
    for (int j = 0; j < 8; j++) {
        if (!(gmask & (1 << j))) { sp2(j, -1 - reload, 0); res[j] = 0; }
    }
    lab(); lab(); lab();
    if (reload == 0) { for (int k = 1; k < 8; k++) if (res[k] == 0) res[k]++; use_a(res[3]); }
}

== none
extern unsigned char gmask; extern int g1, g2, g3, g4, gT[8];
extern int sp1(int, int, int); extern int sp2(int, int, int); extern void clr(); extern void lab(); extern void mid(); extern int tst(int);
void probe(int reload) {
    int res[8];
    g1 = 0; g2 = 0; g3 = 0; g4 = 0;
    if (g1 == 0 && reload == 0) call();
    g1 = 0; g2 = 0; g3 = 0; g4 = 0;
    if (!g2 && reload != 0) call();
    for (int m = 1; m < 8; m++) {
        D(a); D(b);
        U(a); U(b); U(a); U(b); use_c(m); use_c(m); U(a); U(b);
    }
    for (int i = 0; i < 8; i++) {
        if (gmask & (1 << i)) { sp1(i, -1 - reload, 0); res[i] = 0; }
    }

    for (int j = 0; j < 8; j++) {
        if (!(gmask & (1 << j))) { sp2(j, -1 - reload, 0); res[j] = 0; }
    }
    lab(); lab(); lab();
    if (reload == 0) { for (int k = 1; k < 8; k++) if (res[k] == 0) res[k]++; use_a(res[3]); }
}

== gT
extern unsigned char gmask; extern int g1, g2, g3, g4, gT[8];
extern int sp1(int, int, int); extern int sp2(int, int, int); extern void clr(); extern void lab(); extern void mid(); extern int tst(int);
void probe(int reload) {
    int res[8];
    g1 = 0; g2 = 0; g3 = 0; g4 = 0;
    if (g1 == 0 && reload == 0) call();
    g1 = 0; g2 = 0; g3 = 0; g4 = 0;
    if (!g2 && reload != 0) call();
    for (int m = 1; m < 8; m++) {
        D(a); D(b);
        U(a); U(b); U(a); U(b); use_c(m); use_c(m); U(a); U(b);
    }
    for (int i = 0; i < 8; i++) {
        if (gmask & (1 << i)) { sp1(i, -1 - reload, 0); res[i] = 0; }
    }
    for (int t = 0; t < 8; t++) gT[t] = 0;
    for (int j = 0; j < 8; j++) {
        if (!(gmask & (1 << j))) { sp2(j, -1 - reload, 0); res[j] = 0; }
    }
    lab(); lab(); lab();
    if (reload == 0) { for (int k = 1; k < 8; k++) if (res[k] == 0) res[k]++; use_a(res[3]); }
}

== clr_noafter
extern unsigned char gmask; extern int g1, g2, g3, g4, gT[8];
extern int sp1(int, int, int); extern int sp2(int, int, int); extern void clr(); extern void lab(); extern void mid(); extern int tst(int);
void probe(int reload) {
    int res[8];
    g1 = 0; g2 = 0; g3 = 0; g4 = 0;
    if (g1 == 0 && reload == 0) call();
    g1 = 0; g2 = 0; g3 = 0; g4 = 0;
    if (!g2 && reload != 0) call();
    for (int m = 1; m < 8; m++) {
        D(a); D(b);
        U(a); U(b); U(a); U(b); use_c(m); use_c(m); U(a); U(b);
    }
    for (int i = 0; i < 8; i++) {
        if (gmask & (1 << i)) { sp1(i, -1 - reload, 0); res[i] = 0; }
    }
    clr();
    for (int j = 0; j < 8; j++) {
        if (!(gmask & (1 << j))) { sp2(j, -1 - reload, 0); res[j] = 0; }
    }

    if (reload == 0) { for (int k = 1; k < 8; k++) if (res[k] == 0) res[k]++; use_a(res[3]); }
}

== clr_p0
extern unsigned char gmask; extern int g1, g2, g3, g4, gT[8];
extern int sp1(int, int, int); extern int sp2(int, int, int); extern void clr(); extern void lab(); extern void mid(); extern int tst(int);
void probe(int reload) {
    int res[8];
    g1 = 0; g2 = 0; g3 = 0; g4 = 0;
    if (g1 == 0 && reload == 0) call();
    g1 = 0; g2 = 0; g3 = 0; g4 = 0;
    if (!g2 && reload != 0) call();

    for (int i = 0; i < 8; i++) {
        if (gmask & (1 << i)) { sp1(i, -1 - reload, 0); res[i] = 0; }
    }
    clr();
    for (int j = 0; j < 8; j++) {
        if (!(gmask & (1 << j))) { sp2(j, -1 - reload, 0); res[j] = 0; }
    }
    lab(); lab(); lab();
    if (reload == 0) { for (int k = 1; k < 8; k++) if (res[k] == 0) res[k]++; use_a(res[3]); }
}

