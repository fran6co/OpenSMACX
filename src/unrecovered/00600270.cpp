// ORIGINAL: 0x00600270 ?stop@MCIVideo@@QAEXXZ 0x00600270-0x006002EB FILE
// working copy - scaffold materialised by --work
// size      123 bytes
// prototype void (__thiscall ?stop@MCIVideo@@QAEXXZ)(MCIVideo* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x006002A3 0x006002B8 0x006002CD

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?stop@MCIVideo@@QAEXXZ  at 0x00600270  (123 bytes)
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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066937c = (int *)0x0066937C;
static int *const g_009b8190 = (int *)0x009B8190;

class MCIVideo { public:
    uint8_t pad_0_[0x4];
    uint32_t field_4_;

    void stop();
};
typedef unsigned long (__stdcall *MciSendCommandFn)(unsigned int, unsigned int, unsigned long, unsigned long);

void MCIVideo::stop() {
    unsigned short devId = static_cast<unsigned short>(field_4_);
    if (devId != 0) {
        MciSendCommandFn fn = reinterpret_cast<MciSendCommandFn>(*g_0066937c);
        unsigned long genParams[2] = {0, 0};
        unsigned long openParams[6] = {0, 0, 0, 0, 0, 0};
        unsigned long closeParams[2] = {0, 0};
        fn(devId, 0x841, 0x40000, reinterpret_cast<unsigned long>(genParams));
        fn(devId, 0x808, 0, reinterpret_cast<unsigned long>(openParams));
        fn(devId, 0x804, 0, reinterpret_cast<unsigned long>(closeParams));
        field_4_ &= 0xffff0000;
        *g_009b8190 = 0;
        unsigned char *b0 = reinterpret_cast<unsigned char *>(this);
        *b0 &= 0xfe;
    }
}
