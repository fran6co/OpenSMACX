// ORIGINAL: 0x006086F0 ?fill_entry@StringList@@QAEHPAUStringStruct@@@Z 0x006086F0-0x00608761 FILE
// working copy - scaffold materialised by --work
// size      113 bytes
// prototype int (__thiscall ?fill_entry@StringList@@QAEHPAUStringStruct@@@Z)(StringList* this, StringStruct*)
// callers   0   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x005D4680 0x006453E0 0x00645470

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?fill_entry@StringList@@QAEHPAUStringStruct@@@Z  at 0x006086F0  (113 bytes)
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
class StringList;
class StringStruct;
struct StringStructEntry;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Heap { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
    void * get(int);
};

class StringStruct { public:
    uint32_t primary_abi_word_;
    uint32_t virtual_base_abi_word_;
    StringStructEntry * head_;
    StringStructEntry * current_;
    int entry_count_;
    int current_position_;
    void * allocator_;
    uint32_t allocation_base_abi_word_;
    void * allocation_owner_;
};

struct StringStructEntry {
    uint32_t abi_word;
    int id;
    int payload;
    StringStructEntry * next;
    StringStructEntry * previous;
    uint32_t secondary_abi_word;
    void * allocation_owner;
};

extern "C" char *strcat(char *, const char *);
extern "C" unsigned int strlen(const char *);
void * mem_get(int);

class StringList { public:
    uint32_t primary_abi_word_;
    uint32_t virtual_base_abi_word_;
    StringStructEntry * head_;
    StringStructEntry * current_;
    int entry_count_;
    int current_position_;
    void * allocator_;
    uint32_t field_1C_;
    uint32_t field_20_;
    uint32_t field_24_;
    uint32_t allocation_base_abi_word_;
    void * allocation_owner_;

    int fill_entry(StringStruct *);
};
int StringList::fill_entry(StringStruct * a1) {
    char *self = reinterpret_cast<char *>(this);
    char *dst = reinterpret_cast<char *>(a1);
    char *str = *reinterpret_cast<char **>(self + 0x1c);
    if (str != 0) {
        Heap *heap = *reinterpret_cast<Heap **>(self + 0x18);
        char *buf;
        if (heap == 0) {
            buf = reinterpret_cast<char *>(mem_get(static_cast<int>(strlen(str)) + 1));
        } else {
            buf = reinterpret_cast<char *>(heap->get(static_cast<int>(strlen(str)) + 1));
        }
        *reinterpret_cast<char **>(dst + 4) = buf;
        if (buf == 0) {
            return 4;
        }
        *buf = 0;
        strcat(buf, str);
    }
    *reinterpret_cast<uint32_t *>(dst + 8) = *reinterpret_cast<uint32_t *>(self + 0x20);
    *reinterpret_cast<uint32_t *>(dst + 0xc) = *reinterpret_cast<uint32_t *>(self + 0x24);
    return 0;
}
