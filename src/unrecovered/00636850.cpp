// ORIGINAL: 0x00636850 Video_unk3 0x00636850-0x006368C8 FILE
// RULED-OUT: reads `[ecx+N]` with no matching stack `this` -> changed from the scaffold's free `__stdcall Video_unk3(int,int,int,int)` to a thiscall member (own `VideoSelf3` class, since this file's scaffold never declares `class Video`). Diverges at instr #16 (original 'and' vs rebuilt 'mov') around the flags mask/store.
// working copy - scaffold materialised by --work
// size      120 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006365B0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: Video_unk3  at 0x00636850  (120 bytes)
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
extern "C" void *g_video_table[];

class VideoSelf3 { public:
    int unk1();
    int unk3(int, int, int, int);
};

int VideoSelf3::unk3(int a1, int a2, int a3, int a4) {
    char *self = reinterpret_cast<char *>(this);
    if (a3 == 0) {
        return 0x10;
    }
    unk1();
    *reinterpret_cast<int *>(self + 0x1020) = a1;
    *reinterpret_cast<int *>(self + 0x1018) = a3;
    *reinterpret_cast<int *>(self + 0x1024) = a2;
    unsigned int flags = static_cast<unsigned int>(a4) & 0xffff7fff;
    *reinterpret_cast<unsigned int *>(self + 0x4ac) = flags;
    if ((static_cast<unsigned int>(a4) & 0x20) != 0) {
        flags = flags | 4;
        *reinterpret_cast<unsigned int *>(self + 0x4ac) = flags;
    }
    if (*g_009be68c > 10) {
        return 1;
    }
    g_video_table[*g_009be68c] = self;
    *g_009be68c = *g_009be68c + 1;
    return 0;
}
