== base
// duel this d
struct W : Obj { void probe(); };
void W::probe() { D(d); {NT} U4(d); U4(d);  call(); m(); U(d); }
== st_reread_mem
// duel this d
struct W : Obj { void probe(); };
void W::probe() { D(d); {NT} U4(d); U4(d); f0 = def_a(); use_a(f0); call(); m(); U(d); }
== st_reread_loc
// duel this d
struct W : Obj { void probe(); };
void W::probe() { D(d); {NT} U4(d); U4(d); { int v = def_a(); f0 = v; use_a(v); } call(); m(); U(d); }
== tm_this_same
// duel this d
struct W : Obj { void probe(); };
void W::probe() { D(d); {NT} U4(d); U4(d); if (cond()) { call(); m1(5); } else { m1(5); } call(); m(); U(d); }
== tm_this_diff
// duel this d
struct W : Obj { void probe(); };
void W::probe() { D(d); {NT} U4(d); U4(d); if (cond()) { call(); m1(5); } else { m1(6); } call(); m(); U(d); }
== tm_this_one
// duel this d
struct W : Obj { void probe(); };
void W::probe() { D(d); {NT} U4(d); U4(d); if (cond()) { call(); } m1(5); call(); m(); U(d); }
== tm_this_same3
// duel this d
struct W : Obj { void probe(); };
void W::probe() { D(d); {NT} U4(d); U4(d); if (cond()) { call(); m1(5); m1(6); m1(7); } else { sink_b = 2; m1(5); m1(6); m1(7); } call(); m(); U(d); }
== tm_this_one3
// duel this d
struct W : Obj { void probe(); };
void W::probe() { D(d); {NT} U4(d); U4(d); if (cond()) { call(); } else { sink_b = 2; } m1(5); m1(6); m1(7); call(); m(); U(d); }
== xblock_cse
// duel this d
struct W : Obj { void probe(); };
void W::probe() { D(d); {NT} U4(d); U4(d); if (f0 > 0) { sink_a = f0; } call(); m(); U(d); }
== xblock_cse2
// duel this d
struct W : Obj { void probe(); };
void W::probe() { D(d); {NT} U4(d); U4(d); if (f0 > 0) { sink_a = f0; sink_b = f1; } call(); m(); U(d); }
