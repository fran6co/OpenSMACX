#include "probe.h"
struct V3 { float v[3]; };
struct Cav { char pad[0x60]; V3 light; V3 shadow; V3 &lv() { return light; } V3 &sv() { return shadow; } void set(V3 *); void sc(float); };
struct SW { char pad[0x30]; Cav cav; void t0(); void t1(); void t2(); void t3(); void t4(); void t5(); void t6(); void t7(); void t8(); void t9(); };
void SW::t0() { V3 d; d.v[0] = 1.0f; d.v[1] = 2.0f; d.v[2] = 3.0f; cav.set(&d); d.v[0] = 0.0f; d.v[1] = 0.0f; d.v[2] = 1.0f; cav.sv() = d; cav.lv() = d; }
void SW::t1() { V3 d; d.v[0] = 1.0f; d.v[1] = 2.0f; d.v[2] = 3.0f; cav.set(&d); d.v[0] = 0.0f; d.v[1] = 0.0f; d.v[2] = 1.0f; cav.lv() = d; cav.sv() = d; }
void SW::t2() { V3 d; d.v[0] = 1.0f; d.v[1] = 2.0f; d.v[2] = 3.0f; cav.set(&d); d.v[0] = 0.0f; d.v[1] = 0.0f; d.v[2] = 1.0f; cav.sv() = cav.lv() = d; }
void SW::t3() { V3 d; d.v[0] = 1.0f; d.v[1] = 2.0f; d.v[2] = 3.0f; cav.set(&d); d.v[0] = 0.0f; d.v[1] = 0.0f; d.v[2] = 1.0f; cav.lv() = cav.sv() = d; }
void SW::t4() { V3 d; d.v[0] = 1.0f; d.v[1] = 2.0f; d.v[2] = 3.0f; cav.set(&d); d.v[0] = 0.0f; d.v[1] = 0.0f; d.v[2] = 1.0f; cav.shadow = d; cav.light = d; }
void SW::t5() { V3 d; d.v[0] = 1.0f; d.v[1] = 2.0f; d.v[2] = 3.0f; cav.set(&d); d.v[0] = 0.0f; d.v[1] = 0.0f; d.v[2] = 1.0f; V3 e = d; cav.sv() = e; cav.lv() = e; }
void SW::t6() { V3 d; d.v[0] = 1.0f; d.v[1] = 2.0f; d.v[2] = 3.0f; cav.set(&d); d.v[2] = 1.0f; d.v[1] = 0.0f; d.v[0] = 0.0f; cav.sv() = d; cav.lv() = d; }
void SW::t7() { V3 d; d.v[0] = 1.0f; d.v[1] = 2.0f; d.v[2] = 3.0f; cav.set(&d); d.v[0] = d.v[1] = 0.0f; d.v[2] = 1.0f; cav.sv() = d; cav.lv() = d; }
void SW::t8() { V3 d; d.v[0] = 1.0f; d.v[1] = 2.0f; d.v[2] = 3.0f; cav.set(&d); d.v[0] = 0.0f; d.v[1] = 0.0f; d.v[2] = 1.0f; f0(); cav.sv() = d; cav.lv() = d; }
void SW::t9() { V3 d; d.v[0] = 1.0f; d.v[1] = 2.0f; d.v[2] = 3.0f; cav.set(&d); d.v[0] = 0.0f; d.v[1] = 0.0f; d.v[2] = 1.0f; V3 &sv = cav.sv(); V3 &lv = cav.lv(); sv = d; lv = d; }
