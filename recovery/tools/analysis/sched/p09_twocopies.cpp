#include "probe.h"
struct V3 { float v[3]; };
struct Ctx { char pad[0x90]; V3 light; V3 shadow; };
struct Big { char pad[0x10]; Ctx c; V3 &lv() { return c.light; } V3 &sv() { return c.shadow; } void f(); void g(); void h(); void i(); void j(); void k(); };
extern V3 gv;
void Big::f() { V3 d; d.v[0] = 0.0f; d.v[1] = 0.0f; d.v[2] = 1.0f; sv() = d; lv() = d; }
void Big::g() { V3 d; d.v[0] = 0.0f; d.v[1] = 0.0f; d.v[2] = 1.0f; f0(); sv() = d; lv() = d; }
void Big::h() { V3 d; d.v[0] = 0.0f; d.v[1] = 0.0f; d.v[2] = 1.0f; c.shadow = d; c.light = d; }
void Big::i() { sv() = gv; lv() = gv; }
void Big::j() { V3 d = gv; f0(); sv() = d; lv() = d; }
void Big::k() { V3 d; d.v[0] = 0.0f; d.v[1] = 0.0f; d.v[2] = 1.0f; f1((int)&d); sv() = d; lv() = d; }
