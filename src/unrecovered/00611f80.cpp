// ORIGINAL: 0x00611F80 FILE
// working copy - scaffold materialised by --work
// name      ?edit@EditGroup@@QAEHPADPADH@Z
// size      192 bytes
// spans     0x00611F80-0x00612040
// prototype int (__thiscall ?edit@EditGroup@@QAEHPADPADH@Z)(EditGroup* this, int8*, int8*, int)
// callers   7   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00401100 0x005D4510 0x00644EF2 0x006453E0 0x00645470

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?edit@EditGroup@@QAEHPADPADH@Z  at 0x00611F80  (192 bytes)
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

class EditGroup;
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

extern "C" char *strcat(char *, const char *);
extern "C" unsigned int strlen(const char *);
extern "C" void free(void *);
void * mem_get(int);

class EditGroup { public:
    int edit(char *, char *, int);
};
int EditGroup::edit(char * a1, char * a2, int a3) {
    char *self = reinterpret_cast<char *>(this);
    int *vbtable = *reinterpret_cast<int **>(self);
    int disp = vbtable[2];
    char *vbase = self + disp;
    int idx = *reinterpret_cast<int *>(vbase + 0xcc);
    if (idx == 10) {
        return 1;
    }
    if (a2 != 0) {
        char **slot = reinterpret_cast<char **>(self + 0x2c) + idx;
        if (*slot != 0) {
            free(*slot);
        }
        unsigned int len = strlen(a2);
        void *mem = mem_get(len + 1);
        *slot = reinterpret_cast<char *>(mem);
        if (mem == 0) {
            return 4;
        }
        **slot = 0;
        strcat(*slot, a2);
    }
    int *vbtable2 = *reinterpret_cast<int **>(self);
    int disp2 = vbtable2[2];
    char *vbase2 = self + disp2;
    *reinterpret_cast<char **>(vbase2 + 0xd8) = a1;
    *reinterpret_cast<int *>(vbase2 + 0xdc) = 0;
    *reinterpret_cast<int *>(vbase2 + 0xe0) = 0;
    StringStruct *ss = reinterpret_cast<StringStruct *>(vbase2 + 0xbc);
    if (ss->add(idx) != 0) {
        return 4;
    }
    reinterpret_cast<int *>(self + 0x54)[idx] = a3;
    return 0;
}
