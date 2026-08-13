// ORIGINAL: 0x005D47D0 FILE
// working copy - scaffold materialised by --work
// name      sub_5d47d0
// size      129 bytes
// spans     0x005D47D0-0x005D4851
// prototype 
// callers   0   call targets   4
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D4510 0x005D4680 0x006453E0 0x00645470

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5d47d0  at 0x005D47D0  (129 bytes)
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
class Heap { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
    void * get(int);
};

extern "C" char *strcat(char *, const char *);
extern "C" unsigned int strlen(const char *);
void * mem_get(int);

// `ecx` is copied to a receiver-local register at entry and used to reach
// offsets 0x18/0x1c/0x20 with no stack access, and `ret 4` pops one callee
// side argument - a thiscall member with one pointer-sized parameter, not
// the free __stdcall(int) the scaffold guessed.
class NC5d47d0 { public:
    int method(int *a2);
};

int NC5d47d0::method(int *a2) {
    char *self = reinterpret_cast<char *>(this);
    char *str = *reinterpret_cast<char **>(self + 0x1c);
    if (str != 0) {
        Heap *heap = *reinterpret_cast<Heap **>(self + 0x18);
        char *dest;
        if (heap == 0) {
            dest = (char *)mem_get(strlen(str) + 1);
        } else {
            dest = (char *)heap->get(strlen(str) + 1);
        }
        a2[1] = (int)dest;
        if (dest == 0) {
            return 4;
        }
        *dest = 0;
        strcat(dest, str);
    }
    int *src = *reinterpret_cast<int **>(self + 0x20);
    a2[2] = src[0];
    a2[3] = src[1];
    a2[4] = src[2];
    a2[5] = src[3];
    return 0;
}
