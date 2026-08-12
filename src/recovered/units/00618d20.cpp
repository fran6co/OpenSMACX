// ORIGINAL: 0x00618D20 FILE
// name      ?close_class@Caviar@@QAAXXZ
// size      117 bytes
// spans     0x00618D20-0x00618D95
// prototype 
// callers   1   call targets   4
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D7470 0x00639380 0x00639630 0x00644EF2
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00618D20
// measured tier  NO_COMPILE
// refusal        u00618d20.cpp(22) : error C2653: 'Caviar' : is not a class or namespace name u00618d20.cpp(23) : error C2065: 'g_009bb478' : undeclared identifier u00618d20.cpp
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00618d20/unit.cpp
// and score it with tools/agent_brief.py.
// 0x00618D20  ?close_class@Caviar@@QAAXXZ  (117 bytes)
//
// `g_009bb478` is dereferenced FOUR separate times (0x618d58, 0x618d62,
// 0x618d6e, 0x618d7a), never cached in a register across the four uses -
// each `*g_009bb478` below is written fresh for the same reason, not
// factored into a local.
//
// `g_009b9108`/`g_009b96b0` are loaded as plain IMMEDIATES into `ecx`
// (`mov ecx, 0x9b9108`), not dereferenced - they ARE the `this` pointers of
// two fixed `Buffer` instances, so `reinterpret_cast<Buffer*>(g_...)` (no
// `*`) is what reproduces the literal `mov`. `g_009b9100` IS dereferenced
// first (`mov eax,[0x9b9100]`) before being passed to `free`.
//
// `sub_639630` is declared nullary by the scaffold (arity unrecorded); it is
// called here with one argument, so it needs the same fix as 0x0063DF90's
// callee - a second declaration with the real arity and NO `extern "C"`
// (the scaffold's own nullary declaration already establishes the C
// linkage; repeating `extern "C"` on a second, differently-shaped
// declaration is C2733).
int __cdecl sub_639630(int);

void __cdecl Caviar::close_class() {
    if (*g_009bb478 != 0) {
        reinterpret_cast<Buffer *>(g_009b9108)->close();
        reinterpret_cast<Buffer *>(g_009b96b0)->close();
        if (*g_009b9100 != 0) {
            free(reinterpret_cast<void *>(*g_009b9100));
            *g_009b9100 = 0;
        }
        *reinterpret_cast<int *>(*reinterpret_cast<int *>(*g_009bb478 + 4)) = 0;
        *reinterpret_cast<int *>(*reinterpret_cast<int *>(*g_009bb478 + 4) + 4) = 0;
        *reinterpret_cast<int *>(*reinterpret_cast<int *>(*g_009bb478 + 4) + 8) = 0;
        sub_639630(*g_009bb478);
        *g_009bb478 = 0;
    }
    nullsub_16();
}
