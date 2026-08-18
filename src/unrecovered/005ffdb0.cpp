// ORIGINAL: 0x005FFDB0 ?close@MCIVideo@@QAEXXZ 0x005FFDB0-0x005FFE3D FILE
// working copy - scaffold materialised by --work
// size      141 bytes
// prototype void (__thiscall ?close@MCIVideo@@QAEXXZ)(MCIVideo* this)
// callers   4   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C51C0
// indirect  0x005FFDE2 0x005FFDF6 0x005FFE0A

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?close@MCIVideo@@QAEXXZ  at 0x005FFDB0  (141 bytes)
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

class MCIVideo;
class Wave_Device;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Wave_Device { public:
    void enable();
};


// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066937c = (int *)0x0066937C;
static int *const g_0090d978 = (int *)0x0090D978;
static int *const g_009b8190 = (int *)0x009B8190;

class MCIVideo { public:
    uint8_t pad_0_[0x4];
    uint32_t field_4_;

    void close();
};
void MCIVideo::close() {
    char *self = reinterpret_cast<char *>(this);
    unsigned short *field4 = reinterpret_cast<unsigned short *>(self + 4);
    if (*field4 != 0) {
        typedef unsigned long (__stdcall *MciSendCmdFn)(unsigned int, unsigned int, unsigned long, void *);
        MciSendCmdFn mciSendCommandA = reinterpret_cast<MciSendCmdFn>(*g_0066937c);

        unsigned int local1[6] = {0, 0, 0, 0, 0, 0};
        mciSendCommandA(*field4, 0x841, 0x40000, local1);

        unsigned int local2[6] = {0, 0, 0, 0, 0, 0};
        mciSendCommandA(*field4, 0x808, 0, local2);

        unsigned int local3[6] = {0, 0, 0, 0, 0, 0};
        mciSendCommandA(*field4, 0x804, 0, local3);

        *field4 = 0;
        *g_009b8190 = 0;
        int *field0 = reinterpret_cast<int *>(self);
        *field0 = *field0 & 0xfffffffe;
    }
    *field4 = 0;
    *reinterpret_cast<int *>(self + 8) = 0;
    *reinterpret_cast<int *>(self + 0xc) = 0;
    *reinterpret_cast<int *>(self + 0x468) = 0;
    reinterpret_cast<Wave_Device *>(g_0090d978)->enable();
}
