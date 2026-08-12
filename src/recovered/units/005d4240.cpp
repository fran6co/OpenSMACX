// ORIGINAL: 0x005D4240 FILE
// name      sub_5d4240
// size      60 bytes
// spans     0x005D4240-0x005D427C
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005D4240
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005d4240/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5d4240  at 0x005D4240  (60 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.

typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
// `char`, NOT `signed char`. They are distinct MSVC types and mangle
// differently - D against C - and the catalogue's `int8` means the first:
// counted over every catalogued mangled name, `PAD` appears 508 times and
// `PAC` once. Spelling it `signed char` made 150 derived prototypes emit a
// symbol no target object holds. `int8_t` keeps its C meaning below; neither
// catalogue ever uses it.
typedef char int8;
typedef unsigned char uint8;

// WALL: the original is a hand-written VGA DAC loader -
// `push esi; push edi; pushal; cli; ...out dx,al...; sti; popal; pop edi;
// pop esi; leave; ret` - i.e. the ORIGINAL function body is itself inline
// assembly (`pushad`/`popad` have no C-level spelling; no compiler
// intrinsic emits them). `_disable()`/`_outp()`/`_enable()` (declared in
// <dos.h>/<conio.h>, real VC6 intrinsics, not __asm) reproduce the
// cli/out/sti instructions exactly, but the surrounding pushad/popad frame
// and the resulting register choices (ebx/ecx counter, edx-vs-dx port,
// eax-vs-al clear) are unreachable without __asm, which is forbidden.
// Closest reachable candidate; ruled out reaching BYTE_EXACT for the above
// reason.
#include <conio.h>
#include <dos.h>

extern "C" void __cdecl sub_5d4240(unsigned char *a1) {
    _disable();
    _outp(0x3c8, 0);
    int n = 0x100;
    do {
        _outp(0x3c9, a1[0] >> 2);
        _outp(0x3c9, a1[1] >> 2);
        _outp(0x3c9, a1[2] >> 2);
        a1 += 4;
        n--;
    } while (n != 0);
    _enable();
}
