// ORIGINAL: 0x004C7D80 FILE
// name      ?load@Midi@@QAEHPBD@Z
// size      77 bytes
// spans     0x004C7D80-0x004C7DCD
// prototype int (__thiscall ?load@Midi@@QAEHPBD@Z)(Midi* this, int8*)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C6280
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004C7D80
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004c7d80/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?load@Midi@@QAEHPBD@Z  at 0x004C7D80  (77 bytes)
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

class Midi;
class Sound;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Sound { public:
    int load(const char *);
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0090db24 = (int *)0x0090DB24;
static int *const g_0090db7c = (int *)0x0090DB7C;

class Midi { public:
    uint8_t pad_0_[0x3C];
    uint32_t field_3c_;

    int load(const char *);
};

// NOT BYTE_EXACT - MISMATCH, closest reached (mnemonic_similarity 0.9275,
// 32/34 mnemonics in common). The control-flow shape matches the original
// exactly (same branches, same early return, same shared tail calling
// Sound::load), and the ONLY divergence is which callee-saved register
// holds which value: the original assigns esi=this, edi=a1; every source
// form tried here compiles to edi=this, esi=a1 instead - a straight swap,
// not a logic difference. Ruled out (all six produced the identical
// swap): if/else vs. nested-if vs. goto-threaded control flow, naming the
// Sound* cast in its own local, naming a1 in its own local, and addressing
// field_3c_ through a named member vs. a raw reinterpret_cast<char*>(this)
// + 0x3c pointer. This reads as a fixed backend register-allocation
// preference for this shape (which stack param gets which saved register)
// rather than anything source-form can steer - the same class of thing the
// scheduler notes above describe for `lea`/`push` ordering.
int Midi::load(const char * a1) {
    int result;
    if (field_3c_ == 0) {
        if (*g_0090db7c == 0) {
            return 1;
        }
        typedef int (__cdecl *LoadDeviceFn)(uint32_t *, const char *, int);
        result = reinterpret_cast<LoadDeviceFn>(*g_0090db24)(&field_3c_, a1, 2);
    } else {
        result = reinterpret_cast<int>(a1);
    }
    if (result == 0) {
        result = reinterpret_cast<Sound *>(this)->load(a1);
    }
    return result;
}
