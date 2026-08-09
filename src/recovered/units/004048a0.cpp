// ORIGINAL: 0x004048A0 FILE
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004048A0
// name           ??0Popup@@QAE@XZ
// size           105 bytes
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004048a0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??0Popup@@QAE@XZ  at 0x004048A0  (105 bytes)
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

class BasePop;
class Popup;
class Scroll;

// ---- callees, declared and never defined (a definition would be inlined) ----
class BasePop { public:
    BasePop();
    // Added: the EH funclet outside the primary span (0x00650760, borrowing
    // this constructor's frame) does `jmp ??1BasePop@@QAE@XZ` to unwind the
    // base subobject if Scroll's constructor throws. The callee-walk that
    // built this scaffold only follows calls inside the primary span, so it
    // never saw this destructor; without declaring it, the base subobject
    // has no non-trivial destructor and the compiler never emits the
    // unwind protection at all.
    ~BasePop();
    // Agreed size (0x3230, docs/recovery/agreed-class-sizes.csv) given as
    // plain padding so `Scroll scroll_` on Popup below lands at the same
    // offset the original computes with `lea ecx, [esi + 0x3230]` - an
    // empty base class here would put scroll_ at offset 0 instead.
    char pad_[0x3230];
};
class Scroll { public:
    Scroll();
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00650768 = (int *)0x00650768;
static int *const g_006695c0 = (int *)0x006695C0;
static int *const g_006695c8 = (int *)0x006695C8;
static int *const g_00671a58 = (int *)0x00671A58;

// Signature change: single non-virtual inheritance from BasePop (agreed
// size 0x3230, docs/recovery/agreed-class-sizes.csv) with a real `Scroll`
// member landing at that same offset - `lea ecx, [esi + 0x3230]` before the
// Scroll ctor call is exactly `offsetof(Popup, scroll_)` once BasePop is the
// first base. Written as true inheritance/composition (not raw offsets) so
// the compiler generates the SEH frame and the base-class unwind funclet on
// its own; it does not need to be written by hand.
class Popup : public BasePop { public:
    Popup();
    Scroll scroll_;
};

Popup::Popup() {
    *reinterpret_cast<void **>(this) = g_006695c8;
    *reinterpret_cast<void **>(reinterpret_cast<char *>(this) + 0x444) = g_006695c0;
}

