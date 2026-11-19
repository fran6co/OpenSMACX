== base
void probe() { D(c); D(d); {N} U4(d); U4(d);  call(); U(c); U(d); }
== test1
void probe() { D(c); D(d); {N} U4(d); U4(d); if (d > 0) sink_a = 1; call(); U(c); U(d); }
== test2_seq
void probe() { D(c); D(d); {N} U4(d); U4(d); if (d > 0) sink_a = 1; if (d > 0) sink_b = 1; call(); U(c); U(d); }
== test2_one
void probe() { D(c); D(d); {N} U4(d); U4(d); if (d > 0) { sink_a = 1; sink_b = 1; } call(); U(c); U(d); }
== test_nested_same
void probe() { D(c); D(d); {N} U4(d); U4(d); if (d > 0) { sink_a = 1; if (d > 0) sink_b = 1; } call(); U(c); U(d); }
== test_nested_weaker
void probe() { D(c); D(d); {N} U4(d); U4(d); if (d > 5) { sink_a = 1; if (d > 0) sink_b = 1; } call(); U(c); U(d); }
== test_else_same
void probe() { D(c); D(d); {N} U4(d); U4(d); if (d > 0) sink_a = 1; else if (d > 0) sink_b = 1; call(); U(c); U(d); }
== andand_dup
void probe() { D(c); D(d); {N} U4(d); U4(d); if (d > 0 && cond() && d > 0) sink_a = 1; call(); U(c); U(d); }
== while_test
void probe() { D(c); D(d); {N} U4(d); U4(d); while (sink_a < d) sink_a++; call(); U(c); U(d); }
== for_test
void probe() { D(c); D(d); {N} U4(d); U4(d); for (; sink_a < d; ) sink_a++; call(); U(c); U(d); }
== do_test
void probe() { D(c); D(d); {N} U4(d); U4(d); if (sink_a < d) do sink_a++; while (sink_a < d); call(); U(c); U(d); }
