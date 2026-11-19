%% N=1,2,3,4
== small_off_fields
struct W { int pad[4]; int f[4]; void probe(); };
void W::probe() { use_a(f[0]); call(); use_a(f[1]); call(); use_a(f[2]); call(); use_a(f[3]); call(); for (int i = 0; i < {N}; i++) call(); }
%% K=0x10,0x80,0x100,0x1000,0x40000
== off_K_fields_calls
struct W { char pad[{K}]; int f[4]; void probe(); };
void W::probe() { use_a(f[0]); call(); use_a(f[1]); call(); use_a(f[2]); call(); use_a(f[3]); }
%% K=0x10,0x80,0x100,0x1000,0x40000
== off_K_addr2
struct R { int l, t, r, b; };
extern void ur(R*);
struct W { char pad[{K}]; R g; void probe(); };
void W::probe() { ur(&g); call(); ur(&g); }
%% K=0x10,0x80,0x100,0x1000,0x40000
== off_K_addr3
struct R { int l, t, r, b; };
extern void ur(R*);
struct W { char pad[{K}]; R g; void probe(); };
void W::probe() { ur(&g); call(); ur(&g); call(); ur(&g); }
%% K=0x10,0x80,0x100,0x1000,0x40000
== off_K_copy2
struct R { int l, t, r, b; };
extern void ur(R*);
struct W { char pad[{K}]; R g; void probe(); };
void W::probe() { R a = g; ur(&a); R b = g; ur(&b); }
