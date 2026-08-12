// ORIGINAL: 0x00451A10 FILE
// name      ?load_palette@@YAXXZ
// size      747 bytes
// spans     0x00451A10-0x00451C44;0x00655042-0x006550F9
// prototype 
// callers   1   call targets   18
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0059DB40 0x005C8DA0 0x005C8E60 0x005C9330 0x005D4DD0 0x005D7210 0x005D7410 0x005D7DE0 0x005FE2E0 0x005FE2F0 0x00605370 0x00607040 0x00607DA0 0x00614F30 0x00616200 0x00618EE0 0x0062C010 0x00645550
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00451A10
// measured tier  NO_COMPILE
// refusal        u00451a10.cpp(5) : error C2146: syntax error : missing ';' before identifier 'default_palette' u00451a10.cpp(5) : fatal error C1004: unexpected end of file foun
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00451a10/unit.cpp
// and score it with tools/agent_brief.py.
extern int prefs_get_str[];
extern float gamma_scale_factor;
extern float gamma_output;
extern int palette_pcx_filename[];
extern Palette default_palette;
extern Palette console_palette;

void __cdecl load_palette() {
    Buffer buffer;
    Gamma gamma;

    int gamma_val = prefs_get((char *)prefs_get_str, 100, 0);
    float gamma_float = (float)gamma_val * gamma_scale_factor;
    gamma_output = gamma_float;

    buffer.load_pcx((const char *)palette_pcx_filename, &default_palette, 0xa, 0xec);
    console_palette.__as(&default_palette);
    gamma.exec(&default_palette, gamma_float, 1);
}
