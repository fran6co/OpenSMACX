// Shared declarations for the register-allocation probes. Every variable `x` is defined by
// def_x() and consumed by use_x(x); run.py reads the home of x at each site from the listing.
#define V(x) extern int def_##x(); extern void use_##x(int); extern int sink_##x;
V(a) V(b) V(c) V(d) V(e) V(f) V(g) V(h) V(i) V(j) V(m) V(p) V(q) V(r) V(s) V(t)
#undef V
#define D(x) int x = def_##x()
#define S(x) x = def_##x()
#define U(x) use_##x(x)
#define U2(x) U(x); U(x)
#define U3(x) U(x); U(x); U(x)
#define U4(x) U2(x); U2(x)
#define U5(x) U4(x); U(x)
#define U6(x) U4(x); U2(x)
#define U8(x) U4(x); U4(x)
#define G(x) sink_##x = x
extern int cond();
extern void call();
extern int garr[];
struct Obj { int f0, f1, f2, f3; int arr[16]; void m(); void m1(int); int get(); };
extern Obj gobj[];
