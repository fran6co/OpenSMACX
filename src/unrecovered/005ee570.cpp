// ORIGINAL: 0x005EE570 sub_5ee570 0x005EE570-0x005EE60E FILE BYTE_EXACT
// symbol    ?handler@Sub5ee570Rec@@QAEXHHH@Z
// working copy - scaffold materialised by --work
// size      158 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EE5B0 0x005EE5BB 0x005EE5CB 0x005EE5D6 0x005EE5EB 0x005EE5FC 0x005EE607

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5ee570  at 0x005EE570  (158 bytes)
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

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 7, 23, 28, 38
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();  // <-- used
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
    virtual void slot023();  // <-- used
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();  // <-- used
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();
    virtual void slot033();
    virtual void slot034();
    virtual void slot035();
    virtual void slot036();
    virtual void slot037();
    virtual void slot038();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b7ab8 = (int *)0x009B7AB8;
// SIGNATURE CHANGED: `mov esi, ecx` at entry with no matching stack slot
// for a1 - the receiver is `this` (thiscall), and ret 0xc pops exactly
// the 3 remaining stack args, so the stdcall/int fallback is wrong.
class VCall28 { public:
    virtual void slot000(); virtual void slot001(); virtual void slot002(); virtual void slot003();
    virtual void slot004(); virtual void slot005(); virtual void slot006(); virtual void slot007();
    virtual void slot008(); virtual void slot009(); virtual void slot010(); virtual void slot011();
    virtual void slot012(); virtual void slot013(); virtual void slot014(); virtual void slot015();
    virtual void slot016(); virtual void slot017(); virtual void slot018(); virtual void slot019();
    virtual void slot020(); virtual void slot021(); virtual void slot022(); virtual void slot023();
    virtual void slot024(); virtual void slot025(); virtual void slot026(); virtual void slot027();
    virtual void slot028(int, int);
};

class VCall38 { public:
    virtual void slot000(); virtual void slot001(); virtual void slot002(); virtual void slot003();
    virtual void slot004(); virtual void slot005(); virtual void slot006(); virtual void slot007();
    virtual void slot008(); virtual void slot009(); virtual void slot010(); virtual void slot011();
    virtual void slot012(); virtual void slot013(); virtual void slot014(); virtual void slot015();
    virtual void slot016(); virtual void slot017(); virtual void slot018(); virtual void slot019();
    virtual void slot020(); virtual void slot021(); virtual void slot022(); virtual void slot023();
    virtual void slot024(); virtual void slot025(); virtual void slot026(); virtual void slot027();
    virtual void slot028(); virtual void slot029(); virtual void slot030(); virtual void slot031();
    virtual void slot032(); virtual void slot033(); virtual void slot034(); virtual void slot035();
    virtual void slot036(); virtual void slot037();
    virtual void slot038(int, int);
};

class VCallRet { public:
    virtual void slot000(); virtual void slot001(); virtual void slot002(); virtual void slot003();
    virtual void slot004(); virtual void slot005(); virtual void slot006(); virtual void slot007();
    virtual void slot008(); virtual void slot009(); virtual void slot010(); virtual void slot011();
    virtual void slot012(); virtual void slot013(); virtual void slot014(); virtual void slot015();
    virtual void slot016(); virtual void slot017(); virtual void slot018(); virtual void slot019();
    virtual void slot020(); virtual void slot021(); virtual void slot022();
    virtual int slot023();
};

class Sub5ee570Rec { public:
    void handler(int param2, int param3, int param4);
};

void Sub5ee570Rec::handler(int param2, int param3, int param4) {
    char *self = reinterpret_cast<char *>(this);
    if ((*reinterpret_cast<int *>(self + 0x98) & 0x200000) != 0) return;
    if ((*reinterpret_cast<uint8_t *>(self + 0x9c) & 8) != 0) return;
    if (param4 == 0) {
        *g_009b7ab8 = reinterpret_cast<int>(this);
        typedef void (__cdecl *CBFn)(int, int);
        CBFn cb = *reinterpret_cast<CBFn *>(self + 0x40c);
        if (cb != 0) {
            cb(param2, param3);
        }
        reinterpret_cast<VCall28 *>(this)->slot028(param2, param3);
        void *child = *reinterpret_cast<void **>(self + 0x44);
        if (child != 0) {
            if (reinterpret_cast<VCallRet *>(child)->slot023() == 0) {
                reinterpret_cast<VCall *>(child)->slot007();
            }
        }
    } else {
        reinterpret_cast<VCall38 *>(this)->slot038(param2, param3);
        void *child2 = *reinterpret_cast<void **>(self + 0x68);
        if (child2 != 0) {
            if (reinterpret_cast<VCallRet *>(child2)->slot023() == 0) {
                reinterpret_cast<VCall *>(child2)->slot007();
            }
        }
    }
}
