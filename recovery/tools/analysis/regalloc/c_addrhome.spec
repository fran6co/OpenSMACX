# Homed address temporaries (MATCHING_GUIDE 2.7 "Homed addresses"; python3 cand.py c_addrhome.spec --asm NAME).
# mid/N    N other buttons between the first use of &arr[0] / &font and the loop that reuses them:
#          the `this+K` temporaries are always formed at the first use; with a register free they keep it,
#          otherwise `lea ecx,[this+K]; mov [slot],ecx` at the definition and a reload at the reuse, and
#          `this` ends in a destructive `add edi,K`.
# fold/W   the call after the loop on a base at +0x444 of the member: W=direct (`one.set_font(..)`, the
#          base folds into this+K+0x444, `this` stays live through the loop, the loop counter goes to the
#          frame and both addresses are re-formed) against W=inline (a header inline member: its `this` is
#          the member's address, base = [reg+0x444], `this` dies before the loop).
# idx/W    the same question for an index into a global struct array: W=fields (direct `gobj[i].f`), W=member
#          (an expanded inline member), W=ptr (a named pointer local), W=outofline (a called member).
%% N=0,1,2,3,4,8
== mid
struct Fo { int h, k, j; void init(int); };
struct Bt { virtual void hide(); virtual void redraw(); char pad[0xb44]; void set_font(Fo*, int, int, int); void init(int); void icons(int); };
struct W { char pad[0x100]; Bt one; Bt arr[22]; char pad2[0x1000]; Fo font; int z; void probe(int u); };
#define B(k) if ({N} >= k) { arr[k].init(k); arr[k].icons(3); arr[k].icons(4); }
void W::probe(int u) {
    font.init(14);
    call();
    arr[0].init(1);
    call();
    B(1) B(2) B(3) B(4) B(5) B(6) B(7) B(8)
    one.init(2); one.icons(3);
    for (int i = 0; i < 22; ++i) {
        arr[i].hide();
        arr[i].set_font(&font, 0, 0, 0);
        arr[i].redraw();
        arr[i].redraw();
    }
    one.hide();
    one.set_font(&font, 0, 0, 0);
    one.redraw();
    one.redraw();
}
%% W=direct,inline_,inlineloop,inlineboth
== fold
struct Fo { int h, k, j; void init(int); };
struct Wn { virtual void hide(); virtual void redraw(); char pad[0x440]; };
struct Bf { int q[0x1c2]; void set_font(Fo*, int, int, int); };
struct Bt : Wn, Bf { void init(int); void icons(int); void sfr(Fo *f) { Bf::set_font(f, 0, 0, 0); redraw(); } };
struct W { char pad[0x100]; Bt one; Bt arr[22]; char pad2[0x1000]; Fo font; int z; void probe(int u); };
#define direct 1
#define inline_ 2
#define inlineloop 3
#define inlineboth 4
#define B(k) { arr[k].init(k); arr[k].icons(3); arr[k].icons(4); }
void W::probe(int u) {
    font.init(14);
    call();
    arr[0].init(1);
    call();
    B(1) B(2) B(3) B(4)
    one.init(2); one.icons(3);
    for (int i = 0; i < 22; ++i) {
        arr[i].hide();
#if {W} == inlineloop || {W} == inlineboth
        arr[i].sfr(&font);
#else
        arr[i].set_font(&font, 0, 0, 0);
        arr[i].redraw();
#endif
        arr[i].redraw();
    }
    one.hide();
#if {W} == direct || {W} == inlineloop
    one.set_font(&font, 0, 0, 0);
    one.redraw();
#else
    one.sfr(&font);
#endif
    one.redraw();
}
%% W=fields,member,ptr,outofline
== idx
struct Ob { int f0, f1, f2, f3; int arr[79]; void set(int a, int b, int c) { f0 = a; f1 = b; f2 = c; } void oset(int a, int b, int c); int get() { return f1 + f3; } };
extern Ob gob[];
extern int gsrc[];
#define fields 1
#define member 2
#define ptr 3
#define outofline 4
int probe(int *slots) {
    int total = 0;
    for (int k = 0; k < 6; k++) {
        int i = slots[k];
        if (!i) continue;
#if {W} == fields
        gob[i].f3 = gsrc[i];
#elif {W} == ptr
        Ob *o = &gob[i];
        o->f3 = gsrc[i];
#else
        gob[i].f3 = gsrc[i];
#endif
        if (cond()) continue;
        call(); call(); use_a(k); call(); use_b(k); call();
        int v = def_c();
#if {W} == fields
        gob[i].f0 = v; gob[i].f1 = 2; gob[i].f2 = k;
        total += gob[i].f1 + gob[i].f3;
#elif {W} == member
        gob[i].set(v, 2, k);
        total += gob[i].get();
#elif {W} == ptr
        o->f0 = v; o->f1 = 2; o->f2 = k;
        total += o->f1 + o->f3;
#else
        gob[i].oset(v, 2, k);
        total += gob[i].get();
#endif
    }
    return total;
}
