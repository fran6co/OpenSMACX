%% THIS=m();,m(); m();,m(); m(); m();,m(); m(); m(); m();,m(); m(); m(); m(); m(); m();
== duel_M2
struct R { int l, t, r, b; };
extern void ur(R*);
struct W : Obj { char pad[0x40000]; R g; void probe(); };
void W::probe() { ur(&g); for (int k = 0; k < 0; k++); {THIS} ur(&g); m(); }
%% THIS=m();,m(); m();,m(); m(); m();,m(); m(); m(); m();,m(); m(); m(); m(); m(); m();
== duel_M3
struct R { int l, t, r, b; };
extern void ur(R*);
struct W : Obj { char pad[0x40000]; R g; void probe(); };
void W::probe() { ur(&g); for (int k = 0; k < 0; k++); {THIS} ur(&g); ur(&g); m(); }
%% THIS=m();,m(); m();,m(); m(); m();,m(); m(); m(); m();,m(); m(); m(); m(); m(); m();
== duel_M4
struct R { int l, t, r, b; };
extern void ur(R*);
struct W : Obj { char pad[0x40000]; R g; void probe(); };
void W::probe() { ur(&g); for (int k = 0; k < 0; k++); {THIS} ur(&g); ur(&g); ur(&g); m(); }
%% THIS=m();,m(); m();,m(); m(); m();,m(); m(); m(); m();,m(); m(); m(); m(); m(); m();
== duel_M6
struct R { int l, t, r, b; };
extern void ur(R*);
struct W : Obj { char pad[0x40000]; R g; void probe(); };
void W::probe() { ur(&g); for (int k = 0; k < 0; k++); {THIS} ur(&g); ur(&g); ur(&g); ur(&g); ur(&g); m(); }
