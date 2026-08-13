// ORIGINAL: 0x0051E6F0 BYTE_EXACT FILE
// working copy - scaffold materialised by --work
// name      sub_51e6f0
// size      106 bytes
// spans     0x0051E6F0-0x0051E75A
// prototype 
// callers   0   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x006263F0 0x00645470 0x0064FC88

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_51e6f0  at 0x0051E6F0  (106 bytes)
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
extern "C" char *strcat(char *, const char *);
extern "C" int __cdecl _itoa();
void log_say_hex(char *, int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00682820 = (int *)0x00682820;

// `strcat` above is the fast-strcat intrinsic MSVC recognises by name and
// inlines under /O2, which the original does not do (it calls out). Reach
// the real symbol through an untyped nullary alias and a function-pointer
// cast, same lever as `_itoa` below - the indirect call is opaque to the
// name-based intrinsic recognition.
extern "C" int __cdecl _strcat();
typedef char *(__cdecl *StrcatFn)(char *, const char *);
typedef char *(__cdecl *ItoaFn)(int, char *, int);

void __cdecl sub_51e6f0(const char *a1, int a2) {
    char local_154[256];
    char local_54[80];

    local_154[0] = 0;
    reinterpret_cast<ItoaFn>(_itoa)(a2, local_54, 10);
    reinterpret_cast<StrcatFn>(_strcat)(local_154, local_54);
    reinterpret_cast<StrcatFn>(_strcat)(local_154, reinterpret_cast<const char *>(g_00682820));
    reinterpret_cast<StrcatFn>(_strcat)(local_154, a1);
    log_say_hex(local_154, a2, 0, 0);
}
