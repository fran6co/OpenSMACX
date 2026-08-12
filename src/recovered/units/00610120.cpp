// ORIGINAL: 0x00610120 FILE
// name      ??1SpriteBox@@QAE@XZ
// size      399 bytes
// spans     0x00610120-0x00610274;0x006116C0-0x006116DB;0x00662FFA-0x0066301A
// prototype void (__thiscall ??1SpriteBox@@QAE@XZ)(SpriteBox* this)
// callers   23   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FA870 0x00610280 0x00611730
// indirect  0x006101ED 0x00610202 0x0061021D
// PRESERVED UNIT - measured SHARED_TAIL.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00610120
// measured tier  SHARED_TAIL
// refusal        1 span(s) are COMDAT-folded and claimed by another function; no per-function verdict is well defined
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00610120/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??1SpriteBox@@QAE@XZ  at 0x00610120  (399 bytes)
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

class Spot;
class SpriteBox;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Spot { public:
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
    ~Spot();
};
extern "C" int __cdecl sub_611730();

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 0, 1
class VCall { public:
    virtual void slot000();  // <-- used
    virtual void slot001();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00663010 = (int *)0x00663010;
static int *const g_006693ac = (int *)0x006693AC;
static int *const g_00669438 = (int *)0x00669438;
static int *const g_0066943c = (int *)0x0066943C;
static int *const g_00670724 = (int *)0x00670724;
static int *const g_00670730 = (int *)0x00670730;
static int *const g_00670738 = (int *)0x00670738;
static int *const g_0067fce8 = (int *)0x0067FCE8;
static int *const g_009b3374 = (int *)0x009B3374;

class SpriteBox { public:
    uint32_t field_0_;

    void close();
    ~SpriteBox();
};

SpriteBox::~SpriteBox() {
    char *self = reinterpret_cast<char *>(this);

    // Ahead of close(), the original rewrites the vtable pointer of two
    // virtual bases (at this-0x8C and, through the same vbtable slot,
    // this+0x3B8) to each base's own vtable - the standard destructor
    // teardown sequence for a class with several virtual bases. The
    // opaque scaffold here declares SpriteBox with no base at all, so
    // this cannot be expressed through real virtual-inheritance syntax;
    // reproduced as the literal vbtable-relative writes instead.
    int *vbSlot = reinterpret_cast<int *>(self - 0x8C);
    int *vbtable = *reinterpret_cast<int **>(vbSlot);
    int off1 = *reinterpret_cast<int *>(reinterpret_cast<char *>(vbtable) + 4);
    *reinterpret_cast<int *>(self - 0x8C + off1) = 0x670738;

    vbtable = *reinterpret_cast<int **>(vbSlot);
    off1 = *reinterpret_cast<int *>(reinterpret_cast<char *>(vbtable) + 4);
    *reinterpret_cast<int *>(self + 0x3B8 + off1) = 0x670730;

    vbtable = *reinterpret_cast<int **>(vbSlot);
    int off2 = *reinterpret_cast<int *>(reinterpret_cast<char *>(vbtable) + 8);
    *reinterpret_cast<int *>(self - 0x8C + off2) = 0x670724;

    vbtable = *reinterpret_cast<int **>(vbSlot);
    int adj1 = *reinterpret_cast<int *>(reinterpret_cast<char *>(vbtable) + 4);
    *reinterpret_cast<int *>(self - 0x90 + adj1) = adj1 - 0x8C;

    vbtable = *reinterpret_cast<int **>(vbSlot);
    int adj2 = *reinterpret_cast<int *>(reinterpret_cast<char *>(vbtable) + 8);
    *reinterpret_cast<int *>(self - 0x90 + adj2) = adj2 - 0xAA4;

    close();

    // NOT REPRODUCED: between close() and the Spot teardown, the original
    // walks a vector of polymorphic pointers at this-0x68 (count/capacity
    // at +8/+0xC/+0x10), rewrites ITS vtable pointer the same
    // vbtable-relative way as above, then for each live element calls a
    // virtual "clear" through the element's own vtable and a second
    // virtual call through a vtable-relative adjustor - the same
    // undeclared-virtual-base entanglement, compounded by per-element
    // dynamic dispatch. Left out rather than guessed; see the report.

    reinterpret_cast<Spot *>(self - 0x7C)->~Spot();
}
