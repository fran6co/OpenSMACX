// ORIGINAL: 0x005D4C50 sub_5d4c50 0x005D4C50-0x005D4CBC FILE
// working copy - scaffold materialised by --work
// size      108 bytes
// prototype 
// callers   0   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x005D4680 0x006453E0 0x00645470

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5d4c50  at 0x005D4C50  (108 bytes)
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

// `strcat`/`strlen` above are the fast intrinsics MSVC recognises by name
// and inlines under /O2; the original calls out to both. Reach the real
// symbols through untyped nullary aliases and function-pointer casts.
extern "C" int __cdecl _strlen();
typedef unsigned int (__cdecl *StrlenFn)(const char *);
extern "C" int __cdecl _strcat();
typedef char *(__cdecl *StrcatFn)(char *, const char *);

// ecx is `this` (a source object with a string ptr at 0x1c and an owning
// Heap ptr at 0x18); the stack arg a1 is a destination struct (result ptr
// at +4, flag at +8). Two distinct receivers, so a1 stays a plain int.
class Src5d4c50 { public:
    int sub_5d4c50(int a1);
};

int Src5d4c50::sub_5d4c50(int a1) {
    char *out = reinterpret_cast<char *>(a1);
    char *self = reinterpret_cast<char *>(this);
    *reinterpret_cast<int *>(out + 8) = 0;
    if (*reinterpret_cast<char **>(self + 0x1c) != 0) {
        void *result;
        if (*reinterpret_cast<int *>(self + 0x18) == 0) {
            result = mem_get(reinterpret_cast<StrlenFn>(_strlen)(*reinterpret_cast<char **>(self + 0x1c)) + 1);
        } else {
            result = reinterpret_cast<Heap *>(*reinterpret_cast<int *>(self + 0x18))->get(
                reinterpret_cast<StrlenFn>(_strlen)(*reinterpret_cast<char **>(self + 0x1c)) + 1);
        }
        *reinterpret_cast<void **>(out + 4) = result;
        if (result == 0) {
            return 4;
        }
        *reinterpret_cast<char *>(result) = 0;
        reinterpret_cast<StrcatFn>(_strcat)(
            reinterpret_cast<char *>(*reinterpret_cast<void **>(out + 4)),
            *reinterpret_cast<char **>(self + 0x1c));
    }
    return 0;
}
