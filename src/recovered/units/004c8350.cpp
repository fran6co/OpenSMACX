// ORIGINAL: 0x004C8350 FILE
// name      sub_4c8350
// size      70 bytes
// spans     0x004C8350-0x004C8396
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C836D 0x004C8384
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004C8350
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004c8350/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_4c8350  at 0x004C8350  (70 bytes)
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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669368 = (int *)0x00669368;

typedef unsigned int(__stdcall *TimeGetTimeFn)();

class Subject { public:
    int roll();
};

int Subject::roll() {
    // Close but not exact: the original computes `range` (sub/inc) BEFORE
    // calling the RNG, so only `range` needs to survive the call and one
    // callee-saved register (esi) covers it, on top of `this` (edi) -
    // matching "push edi ... push esi ... pop esi ... pop edi". This
    // compile schedules the `call` before the subtraction instead (both
    // ways produce the same value, so the backend is free to reorder
    // them), which forces BOTH `lo` and `hi` to survive the call and
    // costs a third callee-saved register (push ebx/esi/edi vs the
    // original's edi/esi). A `volatile` on `range` stops the reorder but
    // pushes the value to a stack slot behind a frame pointer instead,
    // which the original also does not use - worse on structure even
    // though the mnemonic-similarity score looks better. Tried both;
    // neither reaches the original's register plan, so this keeps the
    // frameless version, which is the smaller structural difference (one
    // extra push/pop pair rather than a whole frame).
    char *self = reinterpret_cast<char *>(this);
    if (!(self[4] & 1)) {
        return 0;
    }
    int lo = *reinterpret_cast<int *>(self + 8);
    int hi = *reinterpret_cast<int *>(self + 0xc);
    if (lo <= hi) {
        unsigned int range = hi - lo + 1;
        unsigned int t = (*reinterpret_cast<TimeGetTimeFn *>(g_00669368))();
        return *reinterpret_cast<int *>(self + 0xc) + t % range;
    }
    unsigned int range = lo - hi + 1;
    unsigned int t = (*reinterpret_cast<TimeGetTimeFn *>(g_00669368))();
    return *reinterpret_cast<int *>(self + 0xc) + t % range;
}

