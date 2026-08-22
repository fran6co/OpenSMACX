// ORIGINAL: 0x00630C80 ?enum_players@Net@@QAEHKKPBUDPNAME@@KPAX@Z 0x00630C80-0x00630D63 FILE
// TRIED: cached `record` char* recomputed once vs re-derived from g_009bc4b8/g_009be4bc at every use site as the asm does
// working copy - scaffold materialised by --work
// size      227 bytes
// prototype int (__thiscall ?enum_players@Net@@QAEHKKPBUDPNAME@@KPAX@Z)(Net* this, unsigned int, unsigned int, DPNAME*, unsigned int, void*)
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00645470
// indirect  0x00630CB6

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?enum_players@Net@@QAEHKKPBUDPNAME@@KPAX@Z  at 0x00630C80  (227 bytes)
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

struct DPNAME;
class Net;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" char *strcat(char *, const char *);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 20
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();
    virtual void slot012();
    virtual void slot013();
    virtual void slot014();
    virtual void slot015();
    virtual void slot016();
    virtual void slot017();
    virtual void slot018();
    virtual void slot019();
    virtual void slot020();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009bc4b8 = (int *)0x009BC4B8;
static int *const g_009be4bc = (int *)0x009BE4BC;
static int *const g_009be600 = (int *)0x009BE600;
static int *const g_009be608 = (int *)0x009BE608;

class Net { public:
    int enum_players(unsigned long, unsigned long, const DPNAME *, unsigned long, void *);
};
typedef int (__stdcall *EnumCallback)(void *, unsigned long, void *, void *);

int Net::enum_players(unsigned long a1, unsigned long a2, const DPNAME * a3, unsigned long a4, void * a5) {
    int recIdx = *g_009bc4b8;
    char *record = reinterpret_cast<char *>(*g_009be4bc) + recIdx * 0x58;

    *reinterpret_cast<unsigned long *>(record) = a1;

    void *netObj = reinterpret_cast<void *>(*g_009be600);
    unsigned long localFlag = 1;
    void *vtbl = *reinterpret_cast<void **>(netObj);
    EnumCallback fn = *reinterpret_cast<EnumCallback *>(reinterpret_cast<char *>(vtbl) + 0x50);
    fn(netObj, a1, &a1, &localFlag);

    record[0x14] = static_cast<char>(localFlag);

    if (*g_009be608 != 0 &&
        (*reinterpret_cast<unsigned char *>(reinterpret_cast<char *>(*g_009be608) + 0xd8) & 2) != 0) {
        record[0x14] = record[0x14] | 2;
    }

    record[0x15] = 0;
    strcat(record + 0x15, reinterpret_cast<const char *>(reinterpret_cast<const int *>(a3)[3]));

    if ((localFlag & 1) != 0 && *g_009be608 != 0) {
        *reinterpret_cast<unsigned long *>(reinterpret_cast<char *>(*g_009be608) + 0x764) = a1;
    }

    *g_009bc4b8 = *g_009bc4b8 + 1;
    return 1;
}
