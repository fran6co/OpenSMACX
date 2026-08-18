// ORIGINAL: 0x00633BA0 sub_633ba0 0x00633BA0-0x00633C81 FILE
// RULED-OUT: extern "C" __stdcall(5 int) verbatim head (ecx used with no stack slot -> reads as __thiscall); rewritten as thiscall member on a new FifoNode633ba0 class
// working copy - scaffold materialised by --work
// size      225 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00645930
// indirect  0x00633BA9 0x00633BC6 0x00633BF0 0x00633C42 0x00633C73

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_633ba0  at 0x00633BA0  (225 bytes)
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
void * mem_get(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669174 = (int *)0x00669174;
static int *const g_0066917c = (int *)0x0066917C;
typedef void (__stdcall *CritSecProc)(void *);

class FifoNode633ba0 { public:
    int add(int a1, int a2, int a3, int a4, int a5);
};

int FifoNode633ba0::add(int a1, int a2, int a3, int a4, int a5) {
    char *self = reinterpret_cast<char *>(this);
    void *cs = self + 0xc;

    (*reinterpret_cast<CritSecProc *>(g_0066917c))(cs);

    int *headSlot = reinterpret_cast<int *>(self);
    char *node;

    if (*headSlot == 0) {
        node = reinterpret_cast<char *>(mem_get(0x18));
        *headSlot = reinterpret_cast<int>(node);
        if (node == 0) {
            (*reinterpret_cast<CritSecProc *>(g_00669174))(cs);
            return 4;
        }
        *reinterpret_cast<int *>(self + 4) = reinterpret_cast<int>(node);
        *reinterpret_cast<int *>(node + 0x14) = 0;
    } else {
        node = reinterpret_cast<char *>(mem_get(0x18));
        if (node == 0) {
            (*reinterpret_cast<CritSecProc *>(g_00669174))(cs);
            return 4;
        }
        *reinterpret_cast<int *>(node + 0x14) = *headSlot;
        *headSlot = reinterpret_cast<int>(node);
    }

    int *head = reinterpret_cast<int *>(*headSlot);
    *reinterpret_cast<int *>(reinterpret_cast<char *>(head) + 4) = a3;
    head = reinterpret_cast<int *>(*headSlot);
    *head = a4;
    head = reinterpret_cast<int *>(*headSlot);
    *reinterpret_cast<int *>(reinterpret_cast<char *>(head) + 8) = a5;

    if (a2 != 0) {
        char *buf = reinterpret_cast<char *>(mem_get(a2));
        head = reinterpret_cast<int *>(*headSlot);
        *reinterpret_cast<int *>(reinterpret_cast<char *>(head) + 0xc) = reinterpret_cast<int>(buf);
        head = reinterpret_cast<int *>(*headSlot);
        int bufVal = *reinterpret_cast<int *>(reinterpret_cast<char *>(head) + 0xc);
        if (bufVal == 0) {
            (*reinterpret_cast<CritSecProc *>(g_00669174))(cs);
            return 4;
        }
        if (a1 != 0) {
            memcpy(buf, reinterpret_cast<void *>(a1), a2);
        }
        head = reinterpret_cast<int *>(*headSlot);
        *reinterpret_cast<int *>(reinterpret_cast<char *>(head) + 0x10) = a2;
    }

    *reinterpret_cast<int *>(self + 8) = *reinterpret_cast<int *>(self + 8) + 1;
    (*reinterpret_cast<CritSecProc *>(g_00669174))(cs);
    return 0;
}
