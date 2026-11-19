%% N=2,3,4,5,6,8
== shl_N
extern unsigned gm, gm2[];
void probe(int f, int g) {
    D(a); D(b); D(c);
    for (int k = 0; k < {N}; k++) { }
    if (gm & (1 << f)) call();
    U(a); U(b); U(c);
    if (gm2[0] & (1 << f)) call();
    if (gm2[1] & (1 << g)) call();
    if (gm2[2] & (1 << f)) call();
    if (gm2[3] & (1 << g)) call();
    if (gm2[4] & (1 << f)) call();
    U(a); U(b); U(c); use_d(f); use_d(g);
}
%% N=2,3,4,5,6,8
== shl_inl_N
extern unsigned gm, gm2[];
inline unsigned bit(int f) { return 1 << f; }
void probe(int f, int g) {
    D(a); D(b); D(c);
    if (gm & bit(f)) call();
    U(a); U(b); U(c);
    if (gm2[0] & bit(f)) call();
    if (gm2[1] & bit(g)) call();
    if (gm2[2] & bit(f)) call();
    if (gm2[3] & bit(g)) call();
    if (gm2[4] & bit(f)) call();
    U(a); U(b); U(c); use_d(f); use_d(g);
}
