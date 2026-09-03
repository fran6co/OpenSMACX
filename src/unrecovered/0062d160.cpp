// ORIGINAL: 0x0062D160 sub_62d160 0x0062D160-0x0062D191 FILE
// size      49 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0062D188
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0062D160
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0062d160/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_62d160  at 0x0062D160  (49 bytes)
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

// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b3ab0 = (int *)0x009B3AB0;
static int *const g_009bc48c = (int *)0x009BC48C;
static int *const g_009bc498 = (int *)0x009BC498;

// The vtable entry is a plain callback taking the object itself and an
// output pointer as explicit stdcall args - the object pointer is pushed
// on the stack rather than passed implicitly in ecx, so a C++ virtual
// call (which never pushes `this`) cannot reproduce it; a function
// pointer of the observed signature does.
typedef int(__stdcall *Handler17)(int, int *);

extern "C" int __cdecl sub_62d160() {
    // Register allocation is swapped from the original throughout: the
    // original keeps the counter in ecx (mov ecx,[...]; the general
    // ModRM form) and the flag in eax (the short A1-encoded
    // mov-eax-from-memory form); this compile puts the counter in eax
    // and the flag in ecx - the opposite pairing. Every attempt at
    // reordering the three loads/declarations (counter-first, flag-first,
    // obj-first, a named counter local, `register`, and nested ifs in
    // place of `&&`) reproduced this same swap, never the original's
    // pairing, so it is recorded as a register-allocator choice rather
    // than a fixable source-form difference. Same instruction count (15),
    // same control flow and same call site; only the eax/ecx roles for
    // the counter and the flag differ, which costs 2 bytes (47 vs 49)
    // because MSVC's A1 short encoding for eax-with-memory is one byte
    // shorter per access than the general ModRM form ecx needs.
    int flag = *g_009bc48c;
    ++*g_009b3ab0;
    int obj = *g_009bc498;
    if (obj != 0 && flag == 0) {
        Handler17 fn = reinterpret_cast<Handler17 *>(*reinterpret_cast<int **>(obj))[17];
        fn(obj, g_009bc48c);
    }
    return *g_009bc48c;
}

