// ORIGINAL: 0x004C74F0 sub_4c74f0 0x004C74F0-0x004C75A0 FILE
// TRIED: [ecx+N] reads with no matching stack slot -> a __thiscall receiver (Obj4c74f0), not the given __stdcall(int). 0.89 mnemonic sim; first divergence is prologue register/stack-slot allocation for the `this`/`a1` locals, not chased further.
// working copy - scaffold materialised by --work
// size      176 bytes
// prototype 
// callers   0   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x006453E0 0x00645460 0x00645470 0x006458A0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_4c74f0  at 0x004C74F0  (176 bytes)
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
extern "C" char *strcpy(char *, const char *);
extern "C" int __cdecl strrchr();
extern "C" unsigned int strlen(const char *);

// strlen above is the fast intrinsic MSVC recognises by name and inlines
// under /O2; the original calls out to it. Reach the real symbol through an
// untyped nullary alias and a function-pointer cast. strrchr is declared
// nullary by the scaffold; redeclare with its real signature under a
// different name to avoid C2733.
extern "C" int __cdecl _strlen();
typedef unsigned int (__cdecl *StrlenFn)(const char *);
extern "C" char *strrchr_real(const char *, int);

class Obj4c74f0 {
public:
    void sub_4c74f0(char *a1);
};

void Obj4c74f0::sub_4c74f0(char *a1) {
    char *self = reinterpret_cast<char *>(this);
    char dirBuf[0x100];
    strcpy(dirBuf, a1);
    unsigned int len = reinterpret_cast<StrlenFn>(_strlen)(dirBuf);
    if (dirBuf[len - 1] != '\\') {
        dirBuf[len] = '\\';
        dirBuf[len + 1] = 0;
    }
    char **slot = reinterpret_cast<char **>(self);
    for (int i = 0x80; i != 0; --i) {
        char *entry = *slot;
        if (entry != 0) {
            char *str = entry + 0xc;
            if (str != 0) {
                char tmp[0x100];
                strcpy(tmp, dirBuf);
                char *slashPos = strrchr_real(str, '\\');
                char *name = (slashPos != 0) ? slashPos + 1 : str;
                strcat(tmp, name);
                strcpy(str, tmp);
            }
        }
        slot = reinterpret_cast<char **>(reinterpret_cast<char *>(slot) + 4);
    }
}
