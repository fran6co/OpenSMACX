// ORIGINAL: 0x00632CC0 sub_632cc0 0x00632CC0-0x00632DAA FILE
// RULED-OUT: bare `strcat` intrinsic-expands to strlen(scasb)+memcpy; `#pragma function(strcat)` forces the real call. ecx is `this` (not the stdcall the IDA guess implied) - rewritten as a thiscall member of a throwaway class taking (char*, int, unsigned char), matching the "ret 0xc" 3-stack-arg cleanup. do-while / for over the 16-slot search loop UNROLLS the first iteration (duplicated body) under /O2, which the original does not show; `while(true){...; if(cond) break;}` keeps a single instance matching the original's layout, but still emits `jge` where the original has `jl` at the loop's back edge - flipping the break condition's polarity didn't change it. 84% mnemonic match, single divergence region.
// working copy - scaffold materialised by --work
// size      234 bytes
// prototype 
// callers   2   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0062D5F0 0x00633180 0x00645470
// indirect  0x00632D6B 0x00632D9E

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_632cc0  at 0x00632CC0  (234 bytes)
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
extern "C" int __cdecl sub_62d5f0();
extern "C" int __stdcall sub_633180(int);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 1, 33
class VCall { public:
    virtual void slot000();
    virtual void slot001();  // <-- used
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();
    virtual void slot012();
    virtual void slot013();
    virtual void slot014();
    virtual void slot015();
    virtual void slot016();
    virtual void slot017();
    virtual void slot018();
    virtual void slot019();
    virtual void slot020();
    virtual void slot021();
    virtual void slot022();
    virtual void slot023();
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();
    virtual void slot033();  // <-- used
};
#pragma function(strcat)

class VObjMain { public:
    virtual void slot0dummy();
    virtual void slot1(char *, int);
};
class VObjThird { public:
    void slotAny();
};
union Shim33 { void (VObjThird::*fn)(int, int, int); void *raw; };

class MainObj { public:
    void enter62d5f0();
};
class SubObjB0 { public:
    void enter633180(int);
};

class Obj632cc0 { public:
    void method(char *a1, int a2, unsigned char a3);
};

void Obj632cc0::method(char *a1, int a2, unsigned char a3) {
    char *self = reinterpret_cast<char *>(this);
    int i = 0;
    char *entry = self + 0x154;
    while (true) {
        if (*reinterpret_cast<int *>(entry) == 0) {
            if (i < 0x10) {
                *reinterpret_cast<int *>(entry) = a2;
                *reinterpret_cast<unsigned char *>(entry + 0x14) = a3;
                char *buf = entry + 0x15;
                *buf = 0;
                strcat(buf, a1);
                if ((a3 & 1) != 0) {
                    *reinterpret_cast<int *>(self + 0x764) = a2;
                }
            }
            break;
        }
        if (*reinterpret_cast<int *>(entry) == a2) {
            return;
        }
        i = i + 1;
        entry += 0x58;
        if (i >= 0x10) {
            break;
        }
    }

    *reinterpret_cast<int *>(self + 0x6dc) = *reinterpret_cast<int *>(self + 0x6dc) + 1;
    if (*reinterpret_cast<int *>(self + 0x760) == *reinterpret_cast<int *>(self + 0x764)) {
        reinterpret_cast<MainObj *>(self)->enter62d5f0();
    }
    char *entry2 = self + 0x154 + i * 0x58;
    reinterpret_cast<VObjMain *>(self)->slot1(entry2 + 0x15, *reinterpret_cast<int *>(entry2));
    reinterpret_cast<SubObjB0 *>(self + 0xb0)->enter633180(*reinterpret_cast<int *>(entry2));

    void *p;
    if (*reinterpret_cast<int *>(self + 0xb8) == 0) {
        p = 0;
    } else {
        p = *reinterpret_cast<void **>(*reinterpret_cast<int *>(self + 0xbc) + 8);
    }
    Shim33 shim;
    shim.raw = (*reinterpret_cast<void ***>(p))[0x84 / 4];
    (reinterpret_cast<VObjThird *>(p)->*shim.fn)(0x1000, 0, 0);
}
