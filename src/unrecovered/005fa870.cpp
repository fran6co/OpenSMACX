// ORIGINAL: 0x005FA870 BYTE_EXACT FILE
// LEVER: spots_ is stored to 0 twice - once inside the `if (spots_)` branch
//        right after free(), and again unconditionally after the resets
// working copy - scaffold materialised by --work
// name      ??1Spot@@QAE@XZ
// size      46 bytes
// spans     0x005FA870-0x005FA89E
// prototype void (__thiscall ??1Spot@@QAE@XZ)(Spot* this)
// callers   110   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
// notes     Staged hybrid export redirect calls the source-owned destructor

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??1Spot@@QAE@XZ  at 0x005FA870  (46 bytes)
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

struct RECT;
class Spot;

// ---- callees, declared and never defined (a definition would be inlined) ----
struct RECT {
    long left;
    long top;
    long right;
    long bottom;
};

extern "C" void free(void *);

class Spot { public:
    struct SpotInternal {
        RECT rect;
        int type;
        int position;
    };
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;

    ~Spot();
};
Spot::~Spot() {
    if (spots_) {
        free(spots_);
        spots_ = 0;
    }
    max_count_ = 0;
    add_count_ = 0;
    spots_ = 0;
}
