// ORIGINAL: 0x006368D0 Video_unk4 0x006368D0-0x00636954 FILE
// working copy - scaffold materialised by --work
// size      132 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006365B0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: Video_unk4  at 0x006368D0  (132 bytes)
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
extern "C" int __cdecl Video_unk1();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009be640 = (int *)0x009BE640;
static int *const g_009be68c = (int *)0x009BE68C;
class Video4This {
public:
    void call_video_unk1();
    int video_unk4(int a2, int a3, int a4, unsigned int a5);
};

int Video4This::video_unk4(int a2, int a3, int a4, unsigned int a5) {
    if (a4 == 0) {
        return 0x10;
    }
    call_video_unk1();
    char *self = reinterpret_cast<char *>(this);
    *reinterpret_cast<int *>(self + 0x1020) = a2;
    *reinterpret_cast<int *>(self + 0x1018) = a4;
    *reinterpret_cast<int *>(self + 0x101c) = a4 + 0x444;
    *reinterpret_cast<int *>(self + 0x1024) = a3;
    unsigned int v = a5 & 0xffff7fff;
    *reinterpret_cast<unsigned int *>(self + 0x4ac) = v;
    if (a5 & 0x20) {
        v = v | 4;
        *reinterpret_cast<unsigned int *>(self + 0x4ac) = v;
    }
    int cnt = *g_009be68c;
    if (cnt > 10) {
        return 1;
    }
    g_009be640[cnt] = reinterpret_cast<int>(self);
    *g_009be68c = cnt + 1;
    return 0;
}
