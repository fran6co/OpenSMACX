// Write-through pieces (MATCHING_GUIDE 2.7 "Write-through pieces", 2026-11-19 par-5). Read with:
//   python3 cand.py c_wthru.spec --grep '_o\$|_c\$|_d\$'
// prefix/K*_M*: rule-9 prefix (c loses, held in ebx with a store until d starts; lateinit = d's constant
//   initialiser early: d's web starts there, no prefix). nd/*: a memory-homed o with constant (k), call-result
//   (c) or unconditional (u) defs in a free stretch: reads cached (d4_r8), defs never written through.
// at/*: address-taken and slot-sharing locals. dr/*: def and read counts in a free stretch.
== prefix/lateinit/K1_M2
void probe() { D(p); D(q); int d = 0; call(); D(c); U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U2(p); U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U2(q); U(c); call(); if (cond()) d = 7; if (cond()) d = 10; U(d); U(p); U(q); U(c); U(d); }
== prefix/nolate/K1_M2
void probe() { D(p); D(q); call(); D(c); U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U2(p); U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U2(q); U(c); call(); int d = 0; if (cond()) d = 7; if (cond()) d = 10; U(d); U(p); U(q); U(c); U(d); }
== prefix/lateinit/K1_M4
void probe() { D(p); D(q); int d = 0; call(); D(c); U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U2(p); U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U2(q); U(c); call(); if (cond()) d = 7; if (cond()) d = 10; U(d); if (cond()) d = 11; U(d); U(p); U(q); U(c); U(d); }
== prefix/nolate/K1_M4
void probe() { D(p); D(q); call(); D(c); U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U2(p); U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U2(q); U(c); call(); int d = 0; if (cond()) d = 7; if (cond()) d = 10; U(d); if (cond()) d = 11; U(d); U(p); U(q); U(c); U(d); }
== prefix/lateinit/K1_M6
void probe() { D(p); D(q); int d = 0; call(); D(c); U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U2(p); U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U2(q); U(c); call(); if (cond()) d = 7; if (cond()) d = 10; U(d); if (cond()) d = 11; U(d); if (cond()) d = 12; U(d); U(p); U(q); U(c); U(d); }
== prefix/nolate/K1_M6
void probe() { D(p); D(q); call(); D(c); U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U2(p); U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U2(q); U(c); call(); int d = 0; if (cond()) d = 7; if (cond()) d = 10; U(d); if (cond()) d = 11; U(d); if (cond()) d = 12; U(d); U(p); U(q); U(c); U(d); }
== prefix/lateinit/K1_M8
void probe() { D(p); D(q); int d = 0; call(); D(c); U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U2(p); U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U2(q); U(c); call(); if (cond()) d = 7; if (cond()) d = 10; U(d); if (cond()) d = 11; U(d); if (cond()) d = 12; U(d); if (cond()) d = 13; U(d); U(p); U(q); U(c); U(d); }
== prefix/nolate/K1_M8
void probe() { D(p); D(q); call(); D(c); U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U2(p); U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U2(q); U(c); call(); int d = 0; if (cond()) d = 7; if (cond()) d = 10; U(d); if (cond()) d = 11; U(d); if (cond()) d = 12; U(d); if (cond()) d = 13; U(d); U(p); U(q); U(c); U(d); }
== prefix/lateinit/K2_M2
void probe() { D(p); D(q); int d = 0; call(); D(c); U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U2(p); U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U2(q); U(c); U(c); call(); if (cond()) d = 7; if (cond()) d = 10; U(d); U(p); U(q); U(c); U(d); }
== prefix/nolate/K2_M2
void probe() { D(p); D(q); call(); D(c); U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U2(p); U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U2(q); U(c); U(c); call(); int d = 0; if (cond()) d = 7; if (cond()) d = 10; U(d); U(p); U(q); U(c); U(d); }
== prefix/lateinit/K2_M4
void probe() { D(p); D(q); int d = 0; call(); D(c); U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U2(p); U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U2(q); U(c); U(c); call(); if (cond()) d = 7; if (cond()) d = 10; U(d); if (cond()) d = 11; U(d); U(p); U(q); U(c); U(d); }
== prefix/nolate/K2_M4
void probe() { D(p); D(q); call(); D(c); U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U2(p); U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U2(q); U(c); U(c); call(); int d = 0; if (cond()) d = 7; if (cond()) d = 10; U(d); if (cond()) d = 11; U(d); U(p); U(q); U(c); U(d); }
== prefix/lateinit/K2_M6
void probe() { D(p); D(q); int d = 0; call(); D(c); U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U2(p); U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U2(q); U(c); U(c); call(); if (cond()) d = 7; if (cond()) d = 10; U(d); if (cond()) d = 11; U(d); if (cond()) d = 12; U(d); U(p); U(q); U(c); U(d); }
== prefix/nolate/K2_M6
void probe() { D(p); D(q); call(); D(c); U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U2(p); U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U2(q); U(c); U(c); call(); int d = 0; if (cond()) d = 7; if (cond()) d = 10; U(d); if (cond()) d = 11; U(d); if (cond()) d = 12; U(d); U(p); U(q); U(c); U(d); }
== prefix/lateinit/K2_M8
void probe() { D(p); D(q); int d = 0; call(); D(c); U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U2(p); U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U2(q); U(c); U(c); call(); if (cond()) d = 7; if (cond()) d = 10; U(d); if (cond()) d = 11; U(d); if (cond()) d = 12; U(d); if (cond()) d = 13; U(d); U(p); U(q); U(c); U(d); }
== prefix/nolate/K2_M8
void probe() { D(p); D(q); call(); D(c); U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U2(p); U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U2(q); U(c); U(c); call(); int d = 0; if (cond()) d = 7; if (cond()) d = 10; U(d); if (cond()) d = 11; U(d); if (cond()) d = 12; U(d); if (cond()) d = 13; U(d); U(p); U(q); U(c); U(d); }
== prefix/lateinit/K4_M2
void probe() { D(p); D(q); int d = 0; call(); D(c); U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U2(p); U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U2(q); U(c); U(c); U(c); U(c); call(); if (cond()) d = 7; if (cond()) d = 10; U(d); U(p); U(q); U(c); U(d); }
== prefix/nolate/K4_M2
void probe() { D(p); D(q); call(); D(c); U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U2(p); U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U2(q); U(c); U(c); U(c); U(c); call(); int d = 0; if (cond()) d = 7; if (cond()) d = 10; U(d); U(p); U(q); U(c); U(d); }
== prefix/lateinit/K4_M4
void probe() { D(p); D(q); int d = 0; call(); D(c); U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U2(p); U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U2(q); U(c); U(c); U(c); U(c); call(); if (cond()) d = 7; if (cond()) d = 10; U(d); if (cond()) d = 11; U(d); U(p); U(q); U(c); U(d); }
== prefix/nolate/K4_M4
void probe() { D(p); D(q); call(); D(c); U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U2(p); U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U2(q); U(c); U(c); U(c); U(c); call(); int d = 0; if (cond()) d = 7; if (cond()) d = 10; U(d); if (cond()) d = 11; U(d); U(p); U(q); U(c); U(d); }
== prefix/lateinit/K4_M6
void probe() { D(p); D(q); int d = 0; call(); D(c); U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U2(p); U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U2(q); U(c); U(c); U(c); U(c); call(); if (cond()) d = 7; if (cond()) d = 10; U(d); if (cond()) d = 11; U(d); if (cond()) d = 12; U(d); U(p); U(q); U(c); U(d); }
== prefix/nolate/K4_M6
void probe() { D(p); D(q); call(); D(c); U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U2(p); U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U2(q); U(c); U(c); U(c); U(c); call(); int d = 0; if (cond()) d = 7; if (cond()) d = 10; U(d); if (cond()) d = 11; U(d); if (cond()) d = 12; U(d); U(p); U(q); U(c); U(d); }
== prefix/lateinit/K4_M8
void probe() { D(p); D(q); int d = 0; call(); D(c); U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U2(p); U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U2(q); U(c); U(c); U(c); U(c); call(); if (cond()) d = 7; if (cond()) d = 10; U(d); if (cond()) d = 11; U(d); if (cond()) d = 12; U(d); if (cond()) d = 13; U(d); U(p); U(q); U(c); U(d); }
== prefix/nolate/K4_M8
void probe() { D(p); D(q); call(); D(c); U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U4(p);U2(p); U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U4(q);U2(q); U(c); U(c); U(c); U(c); call(); int d = 0; if (cond()) d = 7; if (cond()) d = 10; U(d); if (cond()) d = 11; U(d); if (cond()) d = 12; U(d); if (cond()) d = 13; U(d); U(p); U(q); U(c); U(d); }
== nd/k_d2_r2
int probe() {
    D(p); D(q); int o = 0;
    D(t); U4(t); call(); U(t);
    if (cond()) o = 10;
    if (o == 40) call();
    if (cond()) o = 11;
    if (o == 41) call();
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
== nd/k_d2_r4
int probe() {
    D(p); D(q); int o = 0;
    D(t); U4(t); call(); U(t);
    if (cond()) o = 10;
    if (o == 40) call();
    if (cond()) o = 11;
    if (o == 41) call();
    if (o == 42) call();
    if (o == 43) call();
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
== nd/k_d2_r8
int probe() {
    D(p); D(q); int o = 0;
    D(t); U4(t); call(); U(t);
    if (cond()) o = 10;
    if (o == 40) call();
    if (cond()) o = 11;
    if (o == 41) call();
    if (o == 42) call();
    if (o == 43) call();
    if (o == 44) call();
    if (o == 45) call();
    if (o == 46) call();
    if (o == 47) call();
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
== nd/k_d4_r2
int probe() {
    D(p); D(q); int o = 0;
    D(t); U4(t); call(); U(t);
    if (cond()) o = 10;
    if (o == 40) call();
    if (cond()) o = 11;
    if (o == 41) call();
    if (cond()) o = 12;
    if (cond()) o = 13;
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
== nd/k_d4_r4
int probe() {
    D(p); D(q); int o = 0;
    D(t); U4(t); call(); U(t);
    if (cond()) o = 10;
    if (o == 40) call();
    if (cond()) o = 11;
    if (o == 41) call();
    if (cond()) o = 12;
    if (o == 42) call();
    if (cond()) o = 13;
    if (o == 43) call();
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
== nd/k_d4_r8
int probe() {
    D(p); D(q); int o = 0;
    D(t); U4(t); call(); U(t);
    if (cond()) o = 10;
    if (o == 40) call();
    if (cond()) o = 11;
    if (o == 41) call();
    if (cond()) o = 12;
    if (o == 42) call();
    if (cond()) o = 13;
    if (o == 43) call();
    if (o == 44) call();
    if (o == 45) call();
    if (o == 46) call();
    if (o == 47) call();
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
== nd/c_d2_r2
int probe() {
    D(p); D(q); int o = 0;
    D(t); U4(t); call(); U(t);
    if (cond()) o = def_c();
    if (o == 40) call();
    if (cond()) o = def_c();
    if (o == 41) call();
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
== nd/c_d2_r4
int probe() {
    D(p); D(q); int o = 0;
    D(t); U4(t); call(); U(t);
    if (cond()) o = def_c();
    if (o == 40) call();
    if (cond()) o = def_c();
    if (o == 41) call();
    if (o == 42) call();
    if (o == 43) call();
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
== nd/c_d2_r8
int probe() {
    D(p); D(q); int o = 0;
    D(t); U4(t); call(); U(t);
    if (cond()) o = def_c();
    if (o == 40) call();
    if (cond()) o = def_c();
    if (o == 41) call();
    if (o == 42) call();
    if (o == 43) call();
    if (o == 44) call();
    if (o == 45) call();
    if (o == 46) call();
    if (o == 47) call();
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
== nd/c_d4_r2
int probe() {
    D(p); D(q); int o = 0;
    D(t); U4(t); call(); U(t);
    if (cond()) o = def_c();
    if (o == 40) call();
    if (cond()) o = def_c();
    if (o == 41) call();
    if (cond()) o = def_c();
    if (cond()) o = def_c();
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
== nd/c_d4_r4
int probe() {
    D(p); D(q); int o = 0;
    D(t); U4(t); call(); U(t);
    if (cond()) o = def_c();
    if (o == 40) call();
    if (cond()) o = def_c();
    if (o == 41) call();
    if (cond()) o = def_c();
    if (o == 42) call();
    if (cond()) o = def_c();
    if (o == 43) call();
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
== nd/c_d4_r8
int probe() {
    D(p); D(q); int o = 0;
    D(t); U4(t); call(); U(t);
    if (cond()) o = def_c();
    if (o == 40) call();
    if (cond()) o = def_c();
    if (o == 41) call();
    if (cond()) o = def_c();
    if (o == 42) call();
    if (cond()) o = def_c();
    if (o == 43) call();
    if (o == 44) call();
    if (o == 45) call();
    if (o == 46) call();
    if (o == 47) call();
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
== nd/u_d2_r2
int probe() {
    D(p); D(q); int o = 0;
    D(t); U4(t); call(); U(t);
    o = def_c();
    if (o == 40) call();
    o = def_c();
    if (o == 41) call();
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
== nd/u_d2_r4
int probe() {
    D(p); D(q); int o = 0;
    D(t); U4(t); call(); U(t);
    o = def_c();
    if (o == 40) call();
    o = def_c();
    if (o == 41) call();
    if (o == 42) call();
    if (o == 43) call();
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
== nd/u_d2_r8
int probe() {
    D(p); D(q); int o = 0;
    D(t); U4(t); call(); U(t);
    o = def_c();
    if (o == 40) call();
    o = def_c();
    if (o == 41) call();
    if (o == 42) call();
    if (o == 43) call();
    if (o == 44) call();
    if (o == 45) call();
    if (o == 46) call();
    if (o == 47) call();
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
== nd/u_d4_r2
int probe() {
    D(p); D(q); int o = 0;
    D(t); U4(t); call(); U(t);
    o = def_c();
    if (o == 40) call();
    o = def_c();
    if (o == 41) call();
    o = def_c();
    o = def_c();
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
== nd/u_d4_r4
int probe() {
    D(p); D(q); int o = 0;
    D(t); U4(t); call(); U(t);
    o = def_c();
    if (o == 40) call();
    o = def_c();
    if (o == 41) call();
    o = def_c();
    if (o == 42) call();
    o = def_c();
    if (o == 43) call();
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
== nd/u_d4_r8
int probe() {
    D(p); D(q); int o = 0;
    D(t); U4(t); call(); U(t);
    o = def_c();
    if (o == 40) call();
    o = def_c();
    if (o == 41) call();
    o = def_c();
    if (o == 42) call();
    o = def_c();
    if (o == 43) call();
    if (o == 44) call();
    if (o == 45) call();
    if (o == 46) call();
    if (o == 47) call();
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
== at/self
extern void outp(int *);
int probe() {
    int o = 0;
    if (cond()) o = 7;
    if (cond() && !o) o = 6;
    outp(&o);
    if (o == 7) call();
    return o;
}
== at/noat
extern void outp(int *);
int probe() {
    int t; outp(&t); use_a(t);
    int o = 0;
    if (cond()) o = 7;
    if (cond() && !o) o = 6;
    if (o == 7) call();
    if (o == 5) call();
    return o;
}
== at/noat_nopair
extern void outp(int *);
int probe() {
    D(a); use_a(a); call();
    int o = 0;
    if (cond()) o = 7;
    if (cond() && !o) o = 6;
    if (o == 7) call();
    if (o == 5) call();
    return o;
}
== at/press
extern void outp(int *);
int probe() {
    D(p); D(q); D(r);
    int t; outp(&t); use_a(t);
    int o = 0;
    if (cond()) o = 7;
    if (cond() && !o) o = 6;
    if (o == 7) call();
    if (o == 5) call();
    U4(p); U4(q); U4(r);
    return o;
}
== at/noat_press
extern void outp(int *);
int probe() {
    D(p); D(q); D(r);
    D(a); use_a(a); call();
    int o = 0;
    if (cond()) o = 7;
    if (cond() && !o) o = 6;
    if (o == 7) call();
    if (o == 5) call();
    U4(p); U4(q); U4(r);
    return o;
}
== dr/d2_r1
int probe() {
    D(p); D(q); int o = 0;
    D(c); U4(c); call(); U(c);
    if (cond()) o = 10;
    if (o == 40) call();
    if (cond()) o = 11;
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
== dr/d2_r2
int probe() {
    D(p); D(q); int o = 0;
    D(c); U4(c); call(); U(c);
    if (cond()) o = 10;
    if (o == 40) call();
    if (cond()) o = 11;
    if (o == 41) call();
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
== dr/d2_r4
int probe() {
    D(p); D(q); int o = 0;
    D(c); U4(c); call(); U(c);
    if (cond()) o = 10;
    if (o == 40) call();
    if (cond()) o = 11;
    if (o == 41) call();
    if (o == 42) call();
    if (o == 43) call();
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
== dr/d2_r8
int probe() {
    D(p); D(q); int o = 0;
    D(c); U4(c); call(); U(c);
    if (cond()) o = 10;
    if (o == 40) call();
    if (cond()) o = 11;
    if (o == 41) call();
    if (o == 42) call();
    if (o == 43) call();
    if (o == 44) call();
    if (o == 45) call();
    if (o == 46) call();
    if (o == 47) call();
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
== dr/d4_r1
int probe() {
    D(p); D(q); int o = 0;
    D(c); U4(c); call(); U(c);
    if (cond()) o = 10;
    if (o == 40) call();
    if (cond()) o = 11;
    if (cond()) o = 12;
    if (cond()) o = 13;
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
== dr/d4_r2
int probe() {
    D(p); D(q); int o = 0;
    D(c); U4(c); call(); U(c);
    if (cond()) o = 10;
    if (o == 40) call();
    if (cond()) o = 11;
    if (o == 41) call();
    if (cond()) o = 12;
    if (cond()) o = 13;
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
== dr/d4_r4
int probe() {
    D(p); D(q); int o = 0;
    D(c); U4(c); call(); U(c);
    if (cond()) o = 10;
    if (o == 40) call();
    if (cond()) o = 11;
    if (o == 41) call();
    if (cond()) o = 12;
    if (o == 42) call();
    if (cond()) o = 13;
    if (o == 43) call();
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
== dr/d4_r8
int probe() {
    D(p); D(q); int o = 0;
    D(c); U4(c); call(); U(c);
    if (cond()) o = 10;
    if (o == 40) call();
    if (cond()) o = 11;
    if (o == 41) call();
    if (cond()) o = 12;
    if (o == 42) call();
    if (cond()) o = 13;
    if (o == 43) call();
    if (o == 44) call();
    if (o == 45) call();
    if (o == 46) call();
    if (o == 47) call();
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
== dr/d8_r1
int probe() {
    D(p); D(q); int o = 0;
    D(c); U4(c); call(); U(c);
    if (cond()) o = 10;
    if (o == 40) call();
    if (cond()) o = 11;
    if (cond()) o = 12;
    if (cond()) o = 13;
    if (cond()) o = 14;
    if (cond()) o = 15;
    if (cond()) o = 16;
    if (cond()) o = 17;
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
== dr/d8_r2
int probe() {
    D(p); D(q); int o = 0;
    D(c); U4(c); call(); U(c);
    if (cond()) o = 10;
    if (o == 40) call();
    if (cond()) o = 11;
    if (o == 41) call();
    if (cond()) o = 12;
    if (cond()) o = 13;
    if (cond()) o = 14;
    if (cond()) o = 15;
    if (cond()) o = 16;
    if (cond()) o = 17;
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
== dr/d8_r4
int probe() {
    D(p); D(q); int o = 0;
    D(c); U4(c); call(); U(c);
    if (cond()) o = 10;
    if (o == 40) call();
    if (cond()) o = 11;
    if (o == 41) call();
    if (cond()) o = 12;
    if (o == 42) call();
    if (cond()) o = 13;
    if (o == 43) call();
    if (cond()) o = 14;
    if (cond()) o = 15;
    if (cond()) o = 16;
    if (cond()) o = 17;
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
== dr/d8_r8
int probe() {
    D(p); D(q); int o = 0;
    D(c); U4(c); call(); U(c);
    if (cond()) o = 10;
    if (o == 40) call();
    if (cond()) o = 11;
    if (o == 41) call();
    if (cond()) o = 12;
    if (o == 42) call();
    if (cond()) o = 13;
    if (o == 43) call();
    if (cond()) o = 14;
    if (o == 44) call();
    if (cond()) o = 15;
    if (o == 45) call();
    if (cond()) o = 16;
    if (o == 46) call();
    if (cond()) o = 17;
    if (o == 47) call();
    D(r);
    U4(p); U4(q); U4(r); U4(p); U4(q); U4(r); U4(p); U4(q); U4(r);
    use_b(o); U4(p); U4(q); U4(r);
    return o;
}
