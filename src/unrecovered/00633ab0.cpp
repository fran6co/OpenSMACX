// ORIGINAL: 0x00633AB0 ?add@NetFifo@@QAEHPAXKKHK@Z 0x00633AB0-0x00633B9A FILE
// RULED-OUT: `*tailSlot` re-dereferenced per store instead of caching the reloaded node pointer once per statement block
// working copy - scaffold materialised by --work
// size      234 bytes
// prototype int (__thiscall ?add@NetFifo@@QAEHPAXKKHK@Z)(NetFifo* this, void*, unsigned int, unsigned int, int, unsigned int)
// callers   5   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00645930
// indirect  0x00633AB9 0x00633AD6 0x00633B00 0x00633B5A 0x00633B8C

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?add@NetFifo@@QAEHPAXKKHK@Z  at 0x00633AB0  (234 bytes)
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

class NetFifo;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" void *memcpy(void *, const void *, unsigned int);
void * mem_get(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669174 = (int *)0x00669174;
static int *const g_0066917c = (int *)0x0066917C;

class NetFifo { public:
    int add(void *, unsigned long, unsigned long, int, unsigned long);
};
typedef void (__stdcall *CritSecProc)(void *);

int NetFifo::add(void * a1, unsigned long a2, unsigned long a3, int a4, unsigned long a5) {
    char *self = reinterpret_cast<char *>(this);
    void *cs = self + 0xc;

    (*reinterpret_cast<CritSecProc *>(g_0066917c))(cs);

    int *headSlot = reinterpret_cast<int *>(self);
    int *tailSlot = reinterpret_cast<int *>(self + 4);
    char *node;

    if (*headSlot == 0) {
        node = reinterpret_cast<char *>(mem_get(0x18));
        *headSlot = reinterpret_cast<int>(node);
        if (node == 0) {
            (*reinterpret_cast<CritSecProc *>(g_00669174))(cs);
            return 4;
        }
    } else {
        node = reinterpret_cast<char *>(mem_get(0x18));
        char *oldTail = reinterpret_cast<char *>(*tailSlot);
        *reinterpret_cast<int *>(oldTail + 0x14) = reinterpret_cast<int>(node);
        node = reinterpret_cast<char *>(*reinterpret_cast<int *>(oldTail + 0x14));
        if (node == 0) {
            (*reinterpret_cast<CritSecProc *>(g_00669174))(cs);
            return 4;
        }
    }

    *tailSlot = reinterpret_cast<int>(node);
    *reinterpret_cast<int *>(node + 0x14) = 0;

    *reinterpret_cast<int *>(reinterpret_cast<char *>(*tailSlot) + 4) = a3;
    *reinterpret_cast<int *>(*tailSlot) = a4;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(*tailSlot) + 8) = a5;

    if (a2 != 0) {
        char *buf = reinterpret_cast<char *>(mem_get(a2));
        *reinterpret_cast<int *>(reinterpret_cast<char *>(*tailSlot) + 0xc) = reinterpret_cast<int>(buf);
        int bufVal = *reinterpret_cast<int *>(reinterpret_cast<char *>(*tailSlot) + 0xc);
        if (bufVal == 0) {
            (*reinterpret_cast<CritSecProc *>(g_00669174))(cs);
            return 4;
        }
        if (a1 != 0) {
            memcpy(buf, a1, a2);
        }
        *reinterpret_cast<int *>(reinterpret_cast<char *>(*tailSlot) + 0x10) = a2;
    }

    *reinterpret_cast<int *>(self + 8) = *reinterpret_cast<int *>(self + 8) + 1;
    (*reinterpret_cast<CritSecProc *>(g_00669174))(cs);
    return 0;
}
