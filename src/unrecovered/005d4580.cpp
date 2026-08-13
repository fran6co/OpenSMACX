// ORIGINAL: 0x005D4580 BYTE_EXACT FILE
// LEVER: base_ = 0 written explicitly inside the free()-if AND again in the
//        unconditional zero-out pass, matching the original's double store
// working copy - scaffold materialised by --work
// name      ??1Heap@@QAE@XZ
// size      42 bytes
// spans     0x005D4580-0x005D45AA
// prototype void (__thiscall ??1Heap@@QAE@XZ)(Heap* this)
// callers   6   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
// notes     Staged hybrid export redirect calls the source-owned destructor

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??1Heap@@QAE@XZ  at 0x005D4580  (42 bytes)
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

class Heap;
typedef void * LPVOID;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" void free(void *);

class Heap { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;

    ~Heap();
};
Heap::~Heap() {
    if (base_) {
        free(base_);
        base_ = 0;
    }
    err_flags_ = 0;
    current_ = 0;
    base_ = 0;
    free_size_ = 0;
    base_size_ = 0;
}
