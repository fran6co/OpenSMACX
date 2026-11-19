== base
void probe() { D(c); D(d); {N} U4(d); U4(d);  call(); U(c); U(d); }
== tm_same
void probe() { D(c); D(d); {N} U4(d); U4(d); if (cond()) { call(); use_a(d); } else { use_a(d); } call(); U(c); U(d); }
== tm_diff
void probe() { D(c); D(d); {N} U4(d); U4(d); if (cond()) { call(); use_a(d); } else { use_b(d); } call(); U(c); U(d); }
== tm_same3
void probe() { D(c); D(d); {N} U4(d); U4(d); if (cond()) { call(); use_a(d); use_a(d); use_a(d); } else { use_a(d); use_a(d); use_a(d); } call(); U(c); U(d); }
== tm_diff3
void probe() { D(c); D(d); {N} U4(d); U4(d); if (cond()) { call(); use_a(d); use_a(d); use_a(d); } else { use_b(d); use_b(d); use_b(d); } call(); U(c); U(d); }
== tm_same_full
void probe() { D(c); D(d); {N} U4(d); U4(d); if (cond()) { use_a(d); } else { use_a(d); } call(); U(c); U(d); }
== tm_diff_full
void probe() { D(c); D(d); {N} U4(d); U4(d); if (cond()) { use_a(d); } else { use_b(d); } call(); U(c); U(d); }
== tm_single
void probe() { D(c); D(d); {N} U4(d); U4(d); cond(); use_a(d); call(); U(c); U(d); }
== tm_3arm_same
void probe() { D(c); D(d); {N} U4(d); U4(d); switch (cond()) { case 1: call(); use_a(d); break; case 2: sink_b = 1; use_a(d); break; default: use_a(d); } call(); U(c); U(d); }
== tm_3arm_diff
void probe() { D(c); D(d); {N} U4(d); U4(d); switch (cond()) { case 1: call(); use_a(d); break; case 2: sink_b = 1; use_b(d); break; default: use_e(d); } call(); U(c); U(d); }
== glob_cross_hoist_same
void probe() { D(c); D(d); {N} U4(d); U4(d); if (cond()) sink_a = d + 7; else sink_b = d + 7; call(); U(c); U(d); }
== glob_cross_hoist_diff
void probe() { D(c); D(d); {N} U4(d); U4(d); if (cond()) sink_a = d + 7; else sink_b = d + 9; call(); U(c); U(d); }
