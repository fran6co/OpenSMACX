// ORIGINAL: 0x0060E980 FILE
// working copy - scaffold materialised by --work
// name      ?item@CheckBox@@QAEHPADHH@Z
// size      141 bytes
// spans     0x0060E980-0x0060EA0D
// prototype int (__thiscall ?item@CheckBox@@QAEHPADHH@Z)(CheckBox* this, int8*, int, int)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00401100

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?item@CheckBox@@QAEHPADHH@Z  at 0x0060E980  (141 bytes)
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

class CheckBox;
class StringStruct;
struct StringStructEntry;

// ---- callees, declared and never defined (a definition would be inlined) ----
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
    int add(int);
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


class CheckBox { public:
    int item(char *, int, int);
};
int CheckBox::item(char * a1, int a2, int a3) {
    if (a1 == 0) {
        return 3;
    }
    char *self = (char *)this;
    int base = *(int *)(*(int *)self + 8);
    unsigned char bit = (unsigned char)*(int *)(self + base + 0xcc);
    unsigned int flags = *(unsigned int *)(self + base + 0xec);
    if (a3 == 0) {
        flags &= ~(1u << (bit & 0x1f));
    } else {
        flags |= (1u << (bit & 0x1f));
    }
    *(unsigned int *)(self + base + 0xec) = flags;
    base = *(int *)(*(int *)self + 8);
    *(char **)(self + base + 0xd8) = a1;
    *(int *)(self + base + 0xdc) = 0;
    *(int *)(self + base + 0xe0) = 0;
    StringStruct *ss = (StringStruct *)(self + base + 0xbc);
    int r = ss->add(a2);
    return -(r != 0) & 4;
}
