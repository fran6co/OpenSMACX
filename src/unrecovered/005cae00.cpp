// ORIGINAL: 0x005CAE00 sub_5cae00 0x005CAE00-0x005CAE79 FILE BYTE_EXACT
// working copy - scaffold materialised by --work
// size      121 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
// indirect  0x005CAE1B

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5cae00  at 0x005CAE00  (121 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" void free(void *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669378 = (int *)0x00669378;

// `mov esi, ecx` with no stack access for the receiver - this is a
// __thiscall member reached via __fastcall's ecx (Ghidra's single param).
// `call dword ptr [0x669378]` is an indirect call through an import slot
// (mmioClose(HMMIO, UINT), __stdcall per the Win32 ABI).
typedef void (__stdcall *MmioCloseFn)(void *, int);

class Src5cae00 { public:
    int sub_5cae00();
};

int Src5cae00::sub_5cae00() {
    char *self = reinterpret_cast<char *>(this);
    void *obj = *reinterpret_cast<void **>(self);
    if ((*reinterpret_cast<unsigned int *>(reinterpret_cast<char *>(obj) + 4) & 0x80000) == 0) {
        void *handle = *reinterpret_cast<void **>(reinterpret_cast<char *>(obj) + 0xa0);
        if (handle != 0) {
            reinterpret_cast<MmioCloseFn>(*g_00669378)(handle, 0);
        }
    }
    void *p1 = *reinterpret_cast<void **>(self + 0x1c);
    if (p1 != 0) {
        free(p1);
    }
    void *p2 = *reinterpret_cast<void **>(self + 0x20);
    if (p2 != 0) {
        free(p2);
    }
    int *dst = reinterpret_cast<int *>(self + 0x1e4);
    int count = 0x18;
    do {
        *dst = 0;
        dst += 4;
        count--;
    } while (count != 0);
    obj = *reinterpret_cast<void **>(self);
    if (*reinterpret_cast<int *>(reinterpret_cast<char *>(obj) + 0x7c) != 0) {
        dst = reinterpret_cast<int *>(self + 0x64);
        count = 0x18;
        do {
            *dst = 0;
            dst += 4;
            count--;
        } while (count != 0);
    }
    return 0;
}
