== base
void probe() { D(c); D(d); {N} U4(d); U4(d);  call(); U(c); U(d); }
== real2
void probe() { D(c); D(d); {N} U4(d); U4(d); sink_a = d; sink_b = d; call(); U(c); U(d); }
== void1
void probe() { D(c); D(d); {N} U4(d); U4(d); (void)d; call(); U(c); U(d); }
== void3
void probe() { D(c); D(d); {N} U4(d); U4(d); (void)d; (void)d; (void)d; call(); U(c); U(d); }
== dead1
void probe() { D(c); D(d); {N} U4(d); U4(d); { int z = d; } call(); U(c); U(d); }
== dead3
void probe() { D(c); D(d); {N} U4(d); U4(d); { int z = d; int y = d; int w = d; } call(); U(c); U(d); }
== selfas
void probe() { D(c); D(d); {N} U4(d); U4(d); d = d; call(); U(c); U(d); }
== if0
void probe() { D(c); D(d); {N} U4(d); U4(d); if (0) { use_a(d); use_a(d); use_a(d); } call(); U(c); U(d); }
== cse_mul_same
void probe() { D(c); D(d); {N} U4(d); U4(d); sink_a = d * 3; sink_b = d * 3; call(); U(c); U(d); }
== cse_mul_diff
void probe() { D(c); D(d); {N} U4(d); U4(d); sink_a = d * 3; sink_b = d * 5; call(); U(c); U(d); }
== cse_mul_same4
void probe() { D(c); D(d); {N} U4(d); U4(d); sink_a = d * 3; sink_b = d * 3; sink_c = d * 3; sink_e = d * 3; call(); U(c); U(d); }
== cse_mul_diff4
void probe() { D(c); D(d); {N} U4(d); U4(d); sink_a = d * 3; sink_b = d * 5; sink_c = d * 7; sink_e = d * 9; call(); U(c); U(d); }
== copy2
void probe() { D(c); D(d); {N} U4(d); U4(d); { int e = d; use_e(e); use_e(e); } call(); U(c); U(d); }
== direct2
void probe() { D(c); D(d); {N} U4(d); U4(d); use_e(d); use_e(d); call(); U(c); U(d); }
== deadglob
void probe() { D(c); D(d); {N} U4(d); U4(d); sink_a = d; sink_a = d; call(); U(c); U(d); }
