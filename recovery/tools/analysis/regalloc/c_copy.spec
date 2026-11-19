%% THIS=m();,m(); m();,m(); m(); m();,m(); m(); m(); m();,m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m(); m();
== copy_M2
struct R { int l, t, r, b; };
extern void ur(R*); extern void ur2(R*, R*);
struct W : Obj { char pad[0x40000]; R g; void probe(); };
void W::probe() { { R a0 = g; ur(&a0); } { R a1 = g; ur(&a1); } {THIS} m(); }
%% THIS=m();,m(); m();,m(); m(); m();,m(); m(); m(); m();,m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m(); m();
== copy_M3
struct R { int l, t, r, b; };
extern void ur(R*); extern void ur2(R*, R*);
struct W : Obj { char pad[0x40000]; R g; void probe(); };
void W::probe() { { R a0 = g; ur(&a0); } { R a1 = g; ur(&a1); } { R a2 = g; ur(&a2); } {THIS} m(); }
%% THIS=m();,m(); m();,m(); m(); m();,m(); m(); m(); m();,m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m(); m();
== copy_M4
struct R { int l, t, r, b; };
extern void ur(R*); extern void ur2(R*, R*);
struct W : Obj { char pad[0x40000]; R g; void probe(); };
void W::probe() { { R a0 = g; ur(&a0); } { R a1 = g; ur(&a1); } { R a2 = g; ur(&a2); } { R a3 = g; ur(&a3); } {THIS} m(); }
%% THIS=m();,m(); m();,m(); m(); m();,m(); m(); m(); m();,m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m(); m();
== addr_M2
struct R { int l, t, r, b; };
extern void ur(R*); extern void ur2(R*, R*);
struct W : Obj { char pad[0x40000]; R g; void probe(); };
void W::probe() { ur(&g); ur(&g); {THIS} m(); }
%% THIS=m();,m(); m();,m(); m(); m();,m(); m(); m(); m();,m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m(); m();
== addr_M3
struct R { int l, t, r, b; };
extern void ur(R*); extern void ur2(R*, R*);
struct W : Obj { char pad[0x40000]; R g; void probe(); };
void W::probe() { ur(&g); ur(&g); ur(&g); {THIS} m(); }
%% THIS=m();,m(); m();,m(); m(); m();,m(); m(); m(); m();,m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m(); m();
== addr_M4
struct R { int l, t, r, b; };
extern void ur(R*); extern void ur2(R*, R*);
struct W : Obj { char pad[0x40000]; R g; void probe(); };
void W::probe() { ur(&g); ur(&g); ur(&g); ur(&g); {THIS} m(); }
