#ifndef OPENSMACX_LIFTED_LOADER_H
#define OPENSMACX_LIFTED_LOADER_H

#include <cstdint>

#include "lifted_runtime.h"

// Makes the guest address space REAL.
//
// The generated tree carries the image's shape and never its contents, so
// opensmacx_image[] ships zero-filled and the bytes arrive here at startup,
// read from the user's own copy of the executable. Nothing derived from the
// original is committed; the only thing this file knows about terranx.exe is
// its SHA-256, which is already pinned in tools/static_recompile_pilot.py.
//
// WHERE THE STACK LIVES
//
// The PE sections span 0x00400000..0x00a0c000 and that is all the flat array
// used to cover, so ESP had nowhere valid to point. Two ways to fix that:
//
//   (a) extend the one flat allocation upward and put the stack immediately
//       above the image, or
//   (b) range-dispatch inside opensmacx_at() between an image block and a
//       separate stack block.
//
// (b) costs an unpredictable branch on EVERY memory reference in 713,586
// lowered instructions - the hot path of the entire program - and buys only
// the freedom to put the stack at an arbitrary guest address. (a) costs one
// megabyte of extra BSS and constrains the stack to sit just above the image,
// which nothing observes: guest code reaches the stack through ESP/EBP, never
// through an absolute address, because the original's stack address was
// itself picked by the Windows loader at run time. So (a): opensmacx_at()
// stays a single subtraction with no test, exactly as generated.
//
// Layout of the one allocation, low to high:
//
//   0x00400000  image             0x0060c000 bytes  (every PE section)
//   0x00a0c000  stack guard band  0x00001000 bytes  (poisoned, never valid)
//   0x00a0d000  stack             0x00100000 bytes  (grows DOWN from the top)
//   0x00b0d000  one past the end
//
// The guard band sits BELOW the stack because the stack grows down, so an
// overflow walks into poison instead of silently into .reloc. It is checked
// by opensmacx_stack_guard_intact() rather than by hardware - there is no
// page protection inside a C array - which makes it a post-hoc assertion, not
// a trap. That is the honest limitation of (a).

// 0x00100000 is SizeOfStackReserve from the original's optional header, so
// the guest gets exactly the stack Windows would have reserved for it.
constexpr uint32_t OpensmacxStackSize = 0x00100000U;
constexpr uint32_t OpensmacxStackGuardSize = 0x00001000U;
constexpr uint32_t OpensmacxStackSpanSize =
    OpensmacxStackGuardSize + OpensmacxStackSize;

constexpr uint32_t OpensmacxStackGuardBase =
    OpensmacxImageBase + OpensmacxImageSize;
constexpr uint32_t OpensmacxStackLimit =
    OpensmacxStackGuardBase + OpensmacxStackGuardSize;
constexpr uint32_t OpensmacxStackTop =
    OpensmacxStackLimit + OpensmacxStackSize;

// Total size of the flat allocation. lifted_image.cpp must define
// opensmacx_image[] with exactly this many bytes.
constexpr uint32_t OpensmacxSpanSize =
    OpensmacxImageSize + OpensmacxStackSpanSize;

// Environment variable consulted first for the path to the user's own
// executable, so a test can point at its own copy without a path being baked
// into generated code.
constexpr const char *OpensmacxImagePathVariable = "OPENSMACX_IMAGE_PATH";

#ifndef OPENSMACX_DEFAULT_IMAGE_PATH
#define OPENSMACX_DEFAULT_IMAGE_PATH "terranx.exe"
#endif

// The path the loader will use: $OPENSMACX_IMAGE_PATH when it is set and not
// empty, otherwise OPENSMACX_DEFAULT_IMAGE_PATH.
const char *opensmacx_resolve_image_path();

// Read `path` (nullptr means opensmacx_resolve_image_path()), verify it, map
// every PE section to its canonical address inside opensmacx_image, apply
// base relocations if the load base differs from ImageBase, and poison the
// stack guard band.
//
// Returns nullptr on success or a human-readable reason on failure. Every
// failure leaves the image UNLOADED rather than half-written: nothing is
// copied until the header, machine type and SHA-256 have all been checked.
const char *opensmacx_load_image(const char *path = nullptr);

// opensmacx_load_image(), but prints the reason to stderr and exits 2. A
// silently wrong image would surface later as divergences that look exactly
// like lowering bugs, which is the one failure mode worth being loud about.
void opensmacx_load_image_or_die(const char *path = nullptr);

bool opensmacx_image_loaded();

// ImageBase + AddressOfEntryPoint, from the file that was actually loaded.
uint32_t opensmacx_entry_point();

// Number of base relocations applied. Expected to be 0 here: the image loads
// at its own ImageBase, so the delta is zero and every fixup is a no-op. The
// loader still walks the table when the delta is non-zero rather than
// assuming it never can be.
uint32_t opensmacx_relocations_applied();

// load base - ImageBase. Zero unless a caller relocates deliberately.
int32_t opensmacx_load_delta();

// Walk the base relocation table and add `delta` to every HIGHLOW fixup,
// storing the number applied through `applied`. opensmacx_load_image() calls
// this itself when the delta is non-zero.
//
// It is exposed because here the delta is always zero, so the loader's own
// call never runs a single fixup. Code that only executes on a machine you do
// not have is not code, it is a wish; exposing it lets the verifier relocate
// by +d and then by -d and demand the image come back bit-for-bit, against a
// fixup count computed independently from the same table by pefile.
const char *opensmacx_apply_relocations(int32_t delta, uint32_t *applied);

// Lowercase hex SHA-256 of the bytes actually read, or "" before a load.
const char *opensmacx_image_sha256();

// Point ESP at the top of the guest stack. Called once before entering lifted
// code; ESP grows down from here.
void opensmacx_init_stack(OpensmacxStaticRecompileState &state);

// False once anything has written below OpensmacxStackLimit, i.e. the guest
// stack overflowed. Cheap enough to call at scenario boundaries.
bool opensmacx_stack_guard_intact();

#endif
