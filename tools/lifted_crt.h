#ifndef OPENSMACX_LIFTED_CRT_H
#define OPENSMACX_LIFTED_CRT_H

// The CRT the lift calls and never translates.
//
// `lift_whole_image.py` drops every `recovery_state == external_library` row -
// 327 functions, 45,155 bytes of CRT and Windows library code - and its comment
// says they "are supplied by the host toolchain". Nothing supplied them. A
// direct call to one lands in the dispatcher's no-body trap, which is why
// booting the image stops at `__itoa` two calls into WinMain.
//
// This supplies them, and the reason it is worth doing separately from every
// other kind of recovery is that these functions are NOT game logic. `_strlen`
// is `strlen`. Its behaviour is specified by the C standard rather than
// recovered from disassembly, so a shim here is a transcription with a known
// answer, not a hypothesis needing a differential oracle to confirm. That makes
// this the one part of the image that can be finished by writing it down.
//
// WHAT A SHIM MUST DO, because getting it wrong corrupts the caller rather than
// failing locally:
//
//   * Arguments are cdecl on the GUEST stack: [esp+4] is the first. The caller
//     cleans them, so a shim pops only the return address - `s.esp += 4` -
//     exactly as `ret` lowers (see x86_lower.py: `ret` emits `s.esp += 4+pop`).
//   * The result goes in EAX.
//   * Every guest pointer is translated with `opensmacx_at`, which is a
//     SUBTRACTION WITH NO BOUNDS CHECK. A wild guest pointer therefore reads
//     the harness's own memory and looks exactly like a lowering bug - this
//     project has lost days to precisely that. So pointers go through
//     `opensmacx_crt_checked`, which traps instead.
//
// Coverage is reported rather than assumed: `opensmacx_crt_coverage()` returns
// how many of the catalogued external_library addresses have a shim, so "the
// CRT is done" is a number and not an impression.

#include "lifted_runtime.h"

#include <cstdint>

// Resolves a guest address to a CRT shim, or nullptr when there is none. The
// generated dispatcher consults this before trapping, so an unimplemented CRT
// routine still reports itself by name instead of silently doing nothing.
OpensmacxLiftedFunction opensmacx_crt_dispatch(uint32_t address);

// The catalogued name for an external_library address, or nullptr. Used by the
// dispatcher's trap text so the work item is "__itoa" and not just an address.
const char *opensmacx_crt_name(uint32_t address);

// (implemented, catalogued) - the honest completion figure for this layer.
void opensmacx_crt_coverage(unsigned *implemented, unsigned *catalogued);

#endif  // OPENSMACX_LIFTED_CRT_H
