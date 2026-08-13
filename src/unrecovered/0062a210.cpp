// ORIGINAL: 0x0062A210 BYTE_EXACT FILE
// LEVER: fseek scaffolds nullary (not in declfix CRT_SIGNATURES); take &fseek
//        and cast to the real function-pointer type instead of redeclaring
// working copy - scaffold materialised by --work
// name      ?UNK2@Flic@@QAEHXZ
// size      107 bytes
// spans     0x0062A210-0x0062A27B
// prototype int (__thiscall ?UNK2@Flic@@QAEHXZ)(Flic* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00647330

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?UNK2@Flic@@QAEHXZ  at 0x0062A210  (107 bytes)
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

class Flic;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" int __cdecl fseek();

typedef int (__cdecl *FseekFn)(void *, long, int);

class Flic { public:
    int UNK2();
};
int Flic::UNK2() {
    char *self = reinterpret_cast<char *>(this);
    if ((*reinterpret_cast<unsigned char *>(self + 0x5b4) & 1) == 0) {
        return 7;
    }
    unsigned int frame = *reinterpret_cast<unsigned int *>(self + 0xa10);
    unsigned int last = *reinterpret_cast<unsigned short *>(self + 0xa1a);
    FseekFn pfseek = (FseekFn)&fseek;
    if (frame == last) {
        pfseek(*reinterpret_cast<void **>(self + 0x5b8),
               *reinterpret_cast<long *>(self + 0xa68), 0);
        return 0;
    }
    pfseek(*reinterpret_cast<void **>(self + 0x5b8),
           *reinterpret_cast<long *>(self + 0xab4), 1);
    *reinterpret_cast<int *>(self + 0xa10) += 1;
    return 0;
}
