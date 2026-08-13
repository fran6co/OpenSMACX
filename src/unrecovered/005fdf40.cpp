// ORIGINAL: 0x005FDF40 BYTE_EXACT FILE
// working copy - scaffold materialised by --work
// name      ??0TextIndex@@QAE@XZ
// size      31 bytes
// spans     0x005FDF40-0x005FDF5F
// prototype void (__thiscall ??0TextIndex@@QAE@XZ)(TextIndex* this)
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D4560
// notes     Staged hybrid export redirect calls the source-owned constructor

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??0TextIndex@@QAE@XZ  at 0x005FDF40  (31 bytes)
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
class TextIndex;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Heap { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
    Heap();
};


class TextIndex { public:
    char file_name_[256];
    uint32_t section_count_;
    Heap heap_;

    TextIndex();
};
TextIndex::TextIndex() {
    section_count_ = 0;
    file_name_[0] = 0;
}
