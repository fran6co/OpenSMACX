// ORIGINAL: 0x004C8290 sub_4c8290 0x004C8290-0x004C8348 FILE
// RULED-OUT: extern "C" __thiscall free function (C4234); switched to a class-member sub_4c8290 like sibling anonymous subs. Signature changed __stdcall(int) -> __thiscall member void(char*): ecx used as this, ret 4 pops one stack arg. First divergence #11 add/pop.
// working copy - scaffold materialised by --work
// size      184 bytes
// prototype 
// callers   1   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006453E0 0x00645460 0x00645470 0x0064557F 0x0064558A 0x006458A0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_4c8290  at 0x004C8290  (184 bytes)
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
int __cdecl fn_0064557f();
int __cdecl fn_0064558a();

// The scaffold's decl for this address is nullary; the real callee takes
// a pointer/length, so it is redeclared here under a different name.
extern "C" char * __cdecl real_strrchr_6458a0(const char *, int);
extern "C" void __cdecl op_delete_64557f(void *);
extern "C" void * __cdecl op_new_64558a(unsigned int);

// The disassembly reads [ecx+0x1c]/[ecx+0x20] with no matching stack
// slot for a receiver, and `ret 4` pops exactly the one stack arg - a
// __thiscall member, not the __stdcall(int) IDA guessed.
class Obj4C8290 { public:
    void sub_4c8290(char *a1);
};

void Obj4C8290::sub_4c8290(char *a1) {
    char *self = reinterpret_cast<char *>(this);
    char **pField1c = reinterpret_cast<char **>(self + 0x1c);
    if (*pField1c != 0) {
        op_delete_64557f(*pField1c);
    }
    *pField1c = 0;
    if (a1 != 0) {
        char buf[256];
        char *field20 = *reinterpret_cast<char **>(self + 0x20);
        if (field20 != 0) {
            strcpy(buf, field20);
            char *slash = real_strrchr_6458a0(a1, 0x5c);
            if (slash != 0) {
                strcat(buf, slash + 1);
            } else {
                strcat(buf, a1);
            }
        } else {
            strcpy(buf, a1);
        }
        unsigned int len = strlen(buf);
        char *newbuf = (char *)op_new_64558a(len + 1);
        *pField1c = newbuf;
        strcpy(newbuf, buf);
    }
}
