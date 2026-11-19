%% THIS=m();,m(); m();,m(); m(); m();,m(); m(); m(); m();,m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m(); m(); m(); m();
== c1
struct R { int l, t, r, b; };
extern void ur(R*);
struct W : Obj { char pad[0x40000]; R g; void probe(); };
void W::probe() { { R a = g; ur(&a); } {THIS} m(); }
%% THIS=m();,m(); m();,m(); m(); m();,m(); m(); m(); m();,m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m(); m(); m(); m();
== c1a1
struct R { int l, t, r, b; };
extern void ur(R*);
struct W : Obj { char pad[0x40000]; R g; void probe(); };
void W::probe() { { R a = g; ur(&a); } ur(&g); {THIS} m(); }
%% THIS=m();,m(); m();,m(); m(); m();,m(); m(); m(); m();,m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m(); m(); m(); m();
== c1a2
struct R { int l, t, r, b; };
extern void ur(R*);
struct W : Obj { char pad[0x40000]; R g; void probe(); };
void W::probe() { { R a = g; ur(&a); } ur(&g); ur(&g); {THIS} m(); }
%% THIS=m();,m(); m();,m(); m(); m();,m(); m(); m(); m();,m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m(); m(); m(); m();
== a1c1
struct R { int l, t, r, b; };
extern void ur(R*);
struct W : Obj { char pad[0x40000]; R g; void probe(); };
void W::probe() { ur(&g); { R a = g; ur(&a); } {THIS} m(); }
%% THIS=m();,m(); m();,m(); m(); m();,m(); m(); m(); m();,m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m(); m(); m(); m();
== c2a2
struct R { int l, t, r, b; };
extern void ur(R*);
struct W : Obj { char pad[0x40000]; R g; void probe(); };
void W::probe() { { R a = g; ur(&a); } { R b = g; ur(&b); } ur(&g); ur(&g); {THIS} m(); }
%% THIS=m();,m(); m();,m(); m(); m();,m(); m(); m(); m();,m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m(); m(); m(); m();
== copyonly2
struct R { int l, t, r, b; };
extern void ur(R*);
struct W : Obj { char pad[0x40000]; R g; void probe(); };
void W::probe() { R a = g; R b = g; ur(&a); ur(&b); {THIS} m(); }
%% THIS=m();,m(); m();,m(); m(); m();,m(); m(); m(); m();,m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m(); m(); m();,m(); m(); m(); m(); m(); m(); m(); m(); m(); m(); m();
== assign2
struct R { int l, t, r, b; };
extern void ur(R*);
struct W : Obj { char pad[0x40000]; R g; void probe(); };
void W::probe() { R a; a = g; ur(&a); a = g; ur(&a); {THIS} m(); }
