// ORIGINAL: 0x005FD880 ??0Text@@QAE@H@Z 0x005FD880-0x005FD8CF FILE BYTE_EXACT
// LEVER: fields reached by offset through the opaque class (byte at 0x0, dwords at 0x150/0x154/0x158/0x15c), store order matching disasm
// working copy - scaffold materialised by --work
// size      79 bytes
// prototype _Text* (__thiscall ??0Text@@QAE@H@Z)(_Text* this, int)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D4510
// notes     Staged hybrid export redirect calls the source-owned constructor

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??0Text@@QAE@H@Z  at 0x005FD880  (79 bytes)
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

class Text;
struct _Text;

// ---- callees, declared and never defined (a definition would be inlined) ----
void * mem_get(int);

class Text { public:
    Text(int);
};
Text::Text(int a1) {
    char *self = reinterpret_cast<char *>(this);
    *self = 0;
    *reinterpret_cast<int *>(self + 0x150) = 0;
    *reinterpret_cast<int *>(self + 0x154) = 0;
    *reinterpret_cast<int *>(self + 0x158) = 0;
    *reinterpret_cast<int *>(self + 0x15c) = 0;
    void *p = mem_get(a1);
    *reinterpret_cast<void **>(self + 0x158) = p;
    if (p != 0) {
        *reinterpret_cast<void **>(self + 0x15c) = mem_get(a1);
    }
}
