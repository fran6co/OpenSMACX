%% MID=call();,Fo f; f.init();
== dgbox
struct R { int l, t, r, b; };
struct Sp { void kill(int); void add(int, int, int); };
struct Fo { int h, k, j; Fo(); ~Fo(); void init(); };
struct W { char pad[0x40c2c]; R g; char pad2[0x40eb4 - 0x40c2c - 16]; Sp s; void probe(int u); };
extern void setclip(R*); extern void tile(int, int, int, int); extern void upd(R*); extern void supd(R*); extern void box(R*);
extern int veh_at(); extern int nxt(int);
void W::probe(int u) {
    R a = g; setclip(&a);
    {MID}
    R b = g; tile(b.l, b.t, b.r - b.l, b.b - b.t);
    box(&b);
    b.l += 3; b.r -= 3; b.t += 3; b.b -= 3;
    s.kill(6);
    int v = veh_at();
    int col = b.l;
    while (v >= 0) { use_a(col); use_a(v); s.add(v, b.l, b.t); col += 5; v = nxt(v); }
    if (u) upd(&g); else supd(&g);
}
