// ORIGINAL: 0x004C81C0 sub_4c81c0 0x004C81C0-0x004C8286 FILE
// working copy - scaffold materialised by --work
// size      198 bytes
// prototype 
// callers   0   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C8290 0x006453E0 0x00645460 0x00645470 0x0064557F 0x0064558A

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_4c81c0  at 0x004C81C0  (198 bytes)
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
extern "C" int __stdcall sub_4c8290(int);
extern "C" unsigned int strlen(const char *);
int __cdecl fn_0064557f();
int __cdecl fn_0064558a();
// The disassembly reads [ecx+0x1c]/[ecx+0x20] with no matching stack slot
// for a receiver, and `ret 4` pops exactly the one stack arg - a __thiscall
// member, not the __stdcall(int) the scaffold guessed. The scaffold's
// callee declarations for 0x64557f/0x64558a are nullary; the real callees
// take a pointer/size, so they are redeclared under different names here,
// matching the sibling recovery at 0x004C8290.
extern "C" void __cdecl op_delete_64557f(void *);
extern "C" void * __cdecl op_new_64558a(unsigned int);

// sub_4c8290 (0x004C8290) is itself a __thiscall member taking one char*
// arg (see src/unrecovered/004c8290.cpp). A free `__thiscall` typedef is
// C4234, so it is called through a member-function-pointer union shim
// instead, with the pointer built directly from the literal address.
class Obj4C81C0Callee { public:
    void sub_4c8290(char *);
};
union Sub4C8290Shim {
    void (Obj4C81C0Callee::*m)(char *);
    void *raw;
};

class Obj4C81C0 { public:
    void sub_4c81c0(char *a1);
};

void Obj4C81C0::sub_4c81c0(char *a1) {
    char *self = reinterpret_cast<char *>(this);
    char buf[256];
    char **pField20 = reinterpret_cast<char **>(self + 0x20);
    if (*pField20 != 0) {
        op_delete_64557f(*pField20);
    }
    *pField20 = 0;
    unsigned int len = strlen(a1);
    strcpy(buf, a1);
    if (buf[len - 1] != '\\') {
        buf[len] = '\\';
        buf[len + 1] = 0;
        ++len;
    }
    ++len;
    void *mem = op_new_64558a(len);
    *pField20 = reinterpret_cast<char *>(mem);
    strcpy(*pField20, buf);
    char **pField1c = reinterpret_cast<char **>(self + 0x1c);
    if (*pField1c != 0) {
        unsigned int flen = strlen(*pField1c);
        char *end = *pField1c + flen;
        char *p = end;
        while (p != *pField1c) {
            if (*(p - 1) == '\\') break;
            --p;
        }
        strcat(buf, p);
        Sub4C8290Shim shim;
        shim.raw = reinterpret_cast<void *>(0x004C8290);
        (reinterpret_cast<Obj4C81C0Callee *>(self)->*shim.m)(buf);
    }
}
