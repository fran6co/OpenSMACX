// ORIGINAL: 0x004C4D10 BYTE_EXACT FILE
// working copy - scaffold materialised by --work
// name      sub_4c4d10
// size      92 bytes
// spans     0x004C4D10-0x004C4D6C
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_4c4d10  at 0x004C4D10  (92 bytes)
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
static int *const g_0090d944 = (int *)0x0090D944;
static int *const g_0090d948 = (int *)0x0090D948;

// `ecx+N` with no stack access -> __thiscall member of a doubly linked
// list node, unlinking itself from a head/tail pair kept in globals.
class ListNode4C4D10 { public:
    int32_t field_0_;
    int32_t flags_;
    uint8_t pad_8_[0x10];
    ListNode4C4D10 * prev_;
    ListNode4C4D10 * next_;

    int unlink();
};

int ListNode4C4D10::unlink() {
    if (!(flags_ & 2)) {
        return 0;
    }
    if (prev_ != 0) {
        prev_->next_ = next_;
    } else {
        *g_0090d944 = reinterpret_cast<int>(next_);
    }
    if (next_ != 0) {
        next_->prev_ = prev_;
    } else {
        *g_0090d948 = reinterpret_cast<int>(prev_);
    }
    flags_ &= ~2;
    next_ = 0;
    prev_ = 0;
    return 0;
}
