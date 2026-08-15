// ORIGINAL: 0x00588340 BYTE_EXACT FILE
// name      ?scroll_normal@@YAXXZ
// size      141 bytes
// spans     0x00588340-0x005883CD
// prototype 
// callers   8   call targets   0
// kind      
// flags     
// calls     (none)
// LEVER: plain sequential stores of the scaffold's own fixed-address
//        globals (`*g_dst = (int)g_src;`) - no address arithmetic here, so
//        the `static T *const` spelling is exactly right, no `extern`
//        rebind needed.
// working copy - scaffold materialised by --work

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?scroll_normal@@YAXXZ  at 0x00588340  (141 bytes)
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
static int *const g_00791d88 = (int *)0x00791D88;
static int *const g_00791db4 = (int *)0x00791DB4;
static int *const g_00791de0 = (int *)0x00791DE0;
static int *const g_00791e0c = (int *)0x00791E0C;
static int *const g_00791e38 = (int *)0x00791E38;
static int *const g_00791e64 = (int *)0x00791E64;
static int *const g_00791e90 = (int *)0x00791E90;
static int *const g_00791ebc = (int *)0x00791EBC;
static int *const g_00791ee8 = (int *)0x00791EE8;
static int *const g_00791f14 = (int *)0x00791F14;
static int *const g_00791f40 = (int *)0x00791F40;
static int *const g_00791f6c = (int *)0x00791F6C;
static int *const g_0079a5e8 = (int *)0x0079A5E8;
static int *const g_009b8dd4 = (int *)0x009B8DD4;
static int *const g_009b8de8 = (int *)0x009B8DE8;
static int *const g_009b8dec = (int *)0x009B8DEC;
static int *const g_009b8df0 = (int *)0x009B8DF0;
static int *const g_009b8df4 = (int *)0x009B8DF4;
static int *const g_009b8df8 = (int *)0x009B8DF8;
static int *const g_009b8dfc = (int *)0x009B8DFC;
static int *const g_009b8e00 = (int *)0x009B8E00;
static int *const g_009b8e04 = (int *)0x009B8E04;
static int *const g_009b8e08 = (int *)0x009B8E08;
static int *const g_009b8e0c = (int *)0x009B8E0C;
static int *const g_009b8e10 = (int *)0x009B8E10;
static int *const g_009b8e14 = (int *)0x009B8E14;
static int *const g_009b8e1c = (int *)0x009B8E1C;
void __cdecl scroll_normal() {
    *g_009b8df4 = (int)g_00791d88;
    *g_009b8df8 = (int)g_00791db4;
    *g_009b8dfc = (int)g_00791de0;
    *g_009b8e0c = (int)g_00791e0c;
    *g_009b8e10 = (int)g_00791e38;
    *g_009b8e14 = (int)g_00791e64;
    *g_009b8de8 = (int)g_00791e90;
    *g_009b8dec = (int)g_00791ebc;
    *g_009b8df0 = (int)g_00791ee8;
    *g_009b8e00 = (int)g_00791f14;
    *g_009b8e04 = (int)g_00791f40;
    *g_009b8e08 = (int)g_00791f6c;
    *g_009b8e1c = (int)g_0079a5e8;
    *g_009b8dd4 = 0x14;
}
