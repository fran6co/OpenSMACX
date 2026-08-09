// ORIGINAL: 0x0041A8B0 FILE
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0041A8B0
// name           ?check_veh@BaseWin@@QAEXH@Z
// size           109 bytes
// measured tier  NO_COMPILE
// refusal        u0041a8b0.cpp(44) : error C2653: 'BaseWin' : is not a class or namespace name u0041a8b0.cpp(45) : error C2061: syntax error : identifier 'Win' u0041a8b0.cpp(45)
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0041a8b0/unit.cpp
// and score it with tools/agent_brief.py.
// 0x0041A8B0  ?check_veh@BaseWin@@QAEXH@Z  (109 bytes)
//
// Two data tables, neither auto-declared by the emitter (they never showed
// up in "fixed globals this body references"), so both are declared here:
// g_00952828, stride 0x34 bytes, indexed by the parameter; g_0097d040,
// stride 0x134 bytes, indexed by `this->field_40b0c` (the field itself is
// inside BaseWin's opaque 0x40b10-byte pad, reached by raw offset - it is
// NOT the class's own recovered `field_40b10_`, which sits 4 bytes later).
//
// The two globals are declared `static char *const`, not `extern`: this
// looked at first like the "indexed table base" case the general lever
// warns about, but it is not a WALK (no register is advanced across
// iterations) - it is a single indexed access, and folding the known
// address into the LOAD's own displacement is exactly what reproduces
// `mov di, word ptr [ecx + 0x952828]`. The real requirement was narrower:
// EVERY field access must recompute `a1 * 0x34` (or `idx * 0x134`) and add
// its OWN literal field offset in the SAME expression, rather than
// materializing a shared `char *e1 = g_00952828 + a1*0x34;` pointer once and
// indexing off of it. The shared-pointer form compiles the multiply into an
// INDEX register but then needs one extra `add reg, 0x952828` to turn it
// into a real pointer before the first dereference - the original never
// builds that intermediate pointer at all, it keeps `a1*0x34` in a register
// and lets each access fold ITS OWN displacement (0x952828, 0x95282a,
// 0x952856) directly into the instruction. Repeating the multiply
// expression at each access site (trusting CSE to keep it in one register)
// reproduces this exactly; a shared pointer variable does not.
//
// Separately (and unrelated to the above): the original's final `idx*0x134`
// scale step is `shl edx,2` (a pure *4, since 77*4=308 exactly, no leftover
// `+idx` term). Every source spelling tried here - a single `idx*0x134`
// multiply, a manually unrolled 9/19/77/308 chain, array-of-struct
// indexing - compiled that LAST step to `lea edx,[eax*4]` (scale-only LEA,
// 7 bytes) instead of `shl` (2-3 bytes). This is what the byte-count gap
// was before the field-access rewrite above; it turned out to be masked
// entirely once the redundant pointer materialization was removed (the
// working body below has no separate scale-by-4 step left exposed - it is
// folded into the `[eax*4 + 0x97d040]` addressing of the load itself, which
// this compiler DOES reproduce byte-for-byte). Recorded because the naive
// "shl vs lea" framing was a red herring: it wasn't a fixed backend choice,
// it was downstream of the same shared-pointer problem above.
static char *const g_00952828 = (char *)0x00952828;
static char *const g_0097d040 = (char *)0x0097D040;

void BaseWin::check_veh(int a1) {
    if (reinterpret_cast<Win *>(this)->is_visible()) {
        int idx = *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x40b0c);
        if ((*reinterpret_cast<short *>(g_00952828 + a1 * 0x34) ==
             *reinterpret_cast<short *>(g_0097d040 + idx * 0x134) &&
             *reinterpret_cast<short *>(g_00952828 + a1 * 0x34 + 2) ==
             *reinterpret_cast<short *>(g_0097d040 + idx * 0x134 + 2)) ||
            *reinterpret_cast<short *>(g_00952828 + a1 * 0x34 + 0x2e) == idx) {
            base_compute(1);
            reinterpret_cast<VCall *>(this)->slot062();
        }
    }
}
