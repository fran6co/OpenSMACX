// ORIGINAL: 0x00590500 sub_590500 0x00590500-0x0059050A;0x00590C20-0x00590CAF FILE
// working copy - scaffold materialised by --work
// size      153 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_590500  at 0x00590500  (153 bytes)
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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0044c880 = (int *)0x0044C880;
static int *const g_0044c910 = (int *)0x0044C910;
static int *const g_0044c940 = (int *)0x0044C940;
static int *const g_0044c980 = (int *)0x0044C980;
static int *const g_0044c9b0 = (int *)0x0044C9B0;
static int *const g_0044c9e0 = (int *)0x0044C9E0;
static int *const g_0044ca10 = (int *)0x0044CA10;
static int *const g_0044ca40 = (int *)0x0044CA40;
static int *const g_0044cad0 = (int *)0x0044CAD0;
static int *const g_00590510 = (int *)0x00590510;
static int *const g_005905c0 = (int *)0x005905C0;
static int *const g_005906f0 = (int *)0x005906F0;
static int *const g_00590800 = (int *)0x00590800;
static int *const g_00590910 = (int *)0x00590910;
static int *const g_00590990 = (int *)0x00590990;
static int *const g_00590a10 = (int *)0x00590A10;
static int *const g_00590aa0 = (int *)0x00590AA0;
static int *const g_00590b30 = (int *)0x00590B30;
static int *const g_00590ba0 = (int *)0x00590BA0;
static int *const g_00945a60 = (int *)0x00945A60;

extern "C" void sub_590510();
extern "C" void sub_44c910();
extern "C" void sub_5905c0();
extern "C" void sub_44c940();
extern "C" void sub_5906f0();
extern "C" void sub_44c980();
extern "C" void sub_590800();
extern "C" void sub_44c9b0();
extern "C" void sub_590910();
extern "C" void sub_44ca40();
extern "C" void sub_590a10();
extern "C" void sub_44ca10();
extern "C" void sub_590aa0();
extern "C" void sub_44c880();
extern "C" void sub_590b30();
extern "C" void sub_44cad0();
extern "C" void sub_590ba0();
extern "C" void sub_44c9e0();
extern "C" void sub_590990();

extern "C" int __cdecl sub_590500() {
    int *p = (int *)0x945a60;
    p[2] = 0;
    p[3] = (int)&sub_590510;
    p[4] = (int)&sub_44c910;
    p[5] = (int)&sub_5905c0;
    p[6] = (int)&sub_44c940;
    p[7] = (int)&sub_5906f0;
    p[8] = (int)&sub_44c980;
    p[9] = (int)&sub_590800;
    p[10] = (int)&sub_44c9b0;
    p[11] = (int)&sub_590910;
    p[12] = (int)&sub_44ca40;
    p[13] = (int)&sub_590a10;
    p[14] = (int)&sub_44ca10;
    p[15] = (int)&sub_590aa0;
    p[16] = (int)&sub_44c880;
    p[17] = (int)&sub_590b30;
    p[18] = (int)&sub_44cad0;
    p[19] = (int)&sub_590ba0;
    p[20] = (int)&sub_44c9e0;
    p[21] = (int)&sub_590990;
    return 0;
}
