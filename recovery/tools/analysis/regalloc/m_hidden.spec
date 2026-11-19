== base
void probe() { D(c); D(d); {N} U4(d); U4(d);  call(); U(c); U(d); }
== direct1
void probe() { D(c); D(d); {N} U4(d); U4(d); use_e(d); call(); U(c); U(d); }
== dse_local
void probe() { D(c); D(d); {N} U4(d); U4(d); { int t = d; t = def_e(); use_e(t); } call(); U(c); U(d); }
== dse_local_nouse
void probe() { D(c); D(d); {N} U4(d); U4(d); { int t = def_e(); use_e(t); t = d; } call(); U(c); U(d); }
== dse_arm
void probe() { D(c); D(d); {N} U4(d); U4(d); { int t; if (cond()) t = d; else t = 5; } call(); U(c); U(d); }
== alg_sub
void probe() { D(c); D(d); {N} U4(d); U4(d); sink_a = d - d; call(); U(c); U(d); }
== alg_mul0
void probe() { D(c); D(d); {N} U4(d); U4(d); sink_a = d * 0; call(); U(c); U(d); }
== fold_and
void probe() { D(c); D(d); {N} U4(d); U4(d); if (d && 0) use_a(d); call(); U(c); U(d); }
== unreach_ret
void probe() { D(c); D(d); {N} U4(d); U4(d); if (cond()) { return; use_a(d); use_a(d); } call(); U(c); U(d); }
== unreach_ret_base
void probe() { D(c); D(d); {N} U4(d); U4(d); if (cond()) { return; } call(); U(c); U(d); }
== inl_ign
inline void ign(int x) { }
void probe() { D(c); D(d); {N} U4(d); U4(d); ign(d); call(); U(c); U(d); }
== inl_ign3
inline void ign(int x) { }
void probe() { D(c); D(d); {N} U4(d); U4(d); ign(d); ign(d); ign(d); call(); U(c); U(d); }
== inl_pick0
inline int pick(int x, int k) { return k ? x : 7; }
void probe() { D(c); D(d); {N} U4(d); U4(d); sink_a = pick(d, 0); call(); U(c); U(d); }
== inl_use2
inline void u2(int x) { use_e(x); use_e(x); }
void probe() { D(c); D(d); {N} U4(d); U4(d); u2(d); call(); U(c); U(d); }
== direct2
void probe() { D(c); D(d); {N} U4(d); U4(d); use_e(d); use_e(d); call(); U(c); U(d); }
== inl_copy2
inline void u2(int x) { int y = x; use_e(y); use_e(y); }
void probe() { D(c); D(d); {N} U4(d); U4(d); u2(d); call(); U(c); U(d); }
== copy_reassign
void probe() { D(c); D(d); {N} U4(d); U4(d); { int t = d; use_e(t); t = d; use_e(t); } call(); U(c); U(d); }
== cmp_self
void probe() { D(c); D(d); {N} U4(d); U4(d); if (d == d) sink_b = 1; call(); U(c); U(d); }
== dead_after_loop
void probe() { D(c); D(d); {N} U4(d); U4(d); { int t = d; for (int i = 0; i < cond(); i++) t = i; } call(); U(c); U(d); }
