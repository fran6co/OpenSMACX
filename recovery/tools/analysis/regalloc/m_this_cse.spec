== base
// duel this d
struct W : Obj { void probe(); };
void W::probe() { D(d); {NT} U4(d); U4(d);  call(); m(); U(d); }
== read1
// duel this d
struct W : Obj { void probe(); };
void W::probe() { D(d); {NT} U4(d); U4(d); sink_a = f0; call(); m(); U(d); }
== read_cached_if
// duel this d
struct W : Obj { void probe(); };
void W::probe() { D(d); {NT} U4(d); U4(d); { int v = f0; if (v > 0) use_a(v); } call(); m(); U(d); }
== read_direct_if
// duel this d
struct W : Obj { void probe(); };
void W::probe() { D(d); {NT} U4(d); U4(d); if (f0 > 0) use_a(f0); call(); m(); U(d); }
== read_cached_2
// duel this d
struct W : Obj { void probe(); };
void W::probe() { D(d); {NT} U4(d); U4(d); { int v = f0; sink_a = v + 1; sink_b = v + 2; } call(); m(); U(d); }
== read_direct_2
// duel this d
struct W : Obj { void probe(); };
void W::probe() { D(d); {NT} U4(d); U4(d); sink_a = f0 + 1; sink_b = f0 + 2; call(); m(); U(d); }
== read_cached_mul
// duel this d
struct W : Obj { void probe(); };
void W::probe() { D(d); {NT} U4(d); U4(d); { int v = f0; sink_a = v * f1 + v; } call(); m(); U(d); }
== read_direct_mul
// duel this d
struct W : Obj { void probe(); };
void W::probe() { D(d); {NT} U4(d); U4(d); sink_a = f0 * f1 + f0; call(); m(); U(d); }
== read_direct_mul_d
// duel this d
struct W : Obj { void probe(); };
void W::probe() { D(d); {NT} U4(d); U4(d); sink_a = f0 * f1 + f2; call(); m(); U(d); }
== read_cached_3
// duel this d
struct W : Obj { void probe(); };
void W::probe() { D(d); {NT} U4(d); U4(d); { int v = f0; use_a(v + v * f1 + v * f2); } call(); m(); U(d); }
== read_direct_3
// duel this d
struct W : Obj { void probe(); };
void W::probe() { D(d); {NT} U4(d); U4(d); use_a(f0 + f0 * f1 + f0 * f2); call(); m(); U(d); }
