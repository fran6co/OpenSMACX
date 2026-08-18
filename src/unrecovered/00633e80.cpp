// ORIGINAL: 0x00633E80 sub_633e80 0x00633E80-0x00633F65 FILE
// RULED-OUT: extern "C" __stdcall(4 int) verbatim head (ecx used with no stack slot -> reads as __thiscall); rewritten as thiscall member on a new FifoNode633e80 class
// working copy - scaffold materialised by --work
// size      229 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00644EF2 0x00645930
// indirect  0x00633E8A 0x00633E97 0x00633EBE 0x00633ED2 0x00633F56

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_633e80  at 0x00633E80  (229 bytes)
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
extern "C" void *memcpy(void *, const void *, unsigned int);
extern "C" void free(void *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669174 = (int *)0x00669174;
static int *const g_0066917c = (int *)0x0066917C;
typedef void (__stdcall *CritSecProc)(void *);

class FifoNode633e80 { public:
    int remove(int a1, int a2, int a3, int a4);
};

int FifoNode633e80::remove(int a1, int a2, int a3, int a4) {
    char *self = reinterpret_cast<char *>(this);
    void *cs = self + 0xc;

    (*reinterpret_cast<CritSecProc *>(g_0066917c))(cs);

    char *node = reinterpret_cast<char *>(*reinterpret_cast<int *>(self));
    if (node == 0) {
        (*reinterpret_cast<CritSecProc *>(g_00669174))(cs);
        return 0;
    }

    char *prev = 0;
    while (*reinterpret_cast<int *>(node) != a3) {
        prev = node;
        node = reinterpret_cast<char *>(*reinterpret_cast<int *>(node + 0x14));
        if (node == 0) {
            (*reinterpret_cast<CritSecProc *>(g_00669174))(cs);
            return 0;
        }
    }

    if (a4 != 0) {
        *reinterpret_cast<int *>(a4) = *reinterpret_cast<int *>(node + 0x10);
    }
    char *buf = reinterpret_cast<char *>(*reinterpret_cast<int *>(node + 0xc));
    if (buf != 0 && a1 != 0) {
        int size = *reinterpret_cast<int *>(node + 0x10);
        memcpy(reinterpret_cast<void *>(a1), buf, size);
        buf = reinterpret_cast<char *>(*reinterpret_cast<int *>(node + 0xc));
        if (buf != 0) {
            free(buf);
        }
        *reinterpret_cast<int *>(node + 0xc) = 0;
    }

    if (prev == 0) {
        int *headSlot = reinterpret_cast<int *>(self);
        char *head = reinterpret_cast<char *>(*headSlot);
        *headSlot = *reinterpret_cast<int *>(head + 0x14);
    } else {
        *reinterpret_cast<int *>(prev + 0x14) = *reinterpret_cast<int *>(node + 0x14);
    }

    if (a2 != 0) {
        *reinterpret_cast<int *>(a2) = *reinterpret_cast<int *>(node + 4);
    }

    int ret = *reinterpret_cast<int *>(node + 8);
    free(node);
    *reinterpret_cast<int *>(self + 8) = *reinterpret_cast<int *>(self + 8) - 1;
    (*reinterpret_cast<CritSecProc *>(g_00669174))(cs);
    return ret;
}
