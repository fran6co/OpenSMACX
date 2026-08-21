// ORIGINAL: 0x005EE950 sub_5ee950 0x005EE950-0x005EE9EE FILE BYTE_EXACT
// symbol    ?sub_5ee950@Recv5ee950@@QAEXHHH@Z
// LEVER: [ecx+N] reads with no stack `this` -> a __thiscall receiver class (Recv5ee950), not the given __stdcall free function. Two shims: VCallSelf (slot023/036 take (int,int) on `this`) and VCallChild (slot023 returns int(), slot007 void()) on the 0x30/0x60 children - same slot index, different vtables, so one shim could not serve both.
// working copy - scaffold materialised by --work
// size      158 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EE990 0x005EE99B 0x005EE9AB 0x005EE9B6 0x005EE9CB 0x005EE9DC 0x005EE9E7

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5ee950  at 0x005EE950  (158 bytes)
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
// This body dispatches through slot(s): 7, 23, 36
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
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();
    virtual void slot033();
    virtual void slot034();
    virtual void slot035();
    virtual void slot036();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b7ab8 = (int *)0x009B7AB8;
class VCallSelf { public:
    virtual void slot000();
    virtual void slot001();
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
    virtual void slot023(int, int);
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();
    virtual void slot033();
    virtual void slot034();
    virtual void slot035();
    virtual void slot036(int, int);
};

class VCallChild { public:
    virtual void slot000();
    virtual void slot001();
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
    virtual int slot023();
};

typedef void (__cdecl *MouseFn)(int, int);

class Recv5ee950 {
public:
    void sub_5ee950(int a1, int a2, int a3);
};

void Recv5ee950::sub_5ee950(int a1, int a2, int a3) {
    char *self = reinterpret_cast<char *>(this);
    if ((*reinterpret_cast<uint32_t *>(self + 0x98) & 0x200000) != 0) {
        return;
    }
    if ((*reinterpret_cast<uint8_t *>(self + 0x9c) & 8) != 0) {
        return;
    }
    if (a3 == 0) {
        *g_009b7ab8 = reinterpret_cast<int>(this);
        MouseFn fn = *reinterpret_cast<MouseFn *>(self + 0x424);
        if (fn != 0) {
            fn(a1, a2);
        }
        reinterpret_cast<VCallSelf *>(this)->slot023(a1, a2);
        void *child = *reinterpret_cast<void **>(self + 0x30);
        if (child != 0) {
            if (reinterpret_cast<VCallChild *>(child)->slot023() == 0) {
                reinterpret_cast<VCallChild *>(child)->slot007();
            }
        }
    } else {
        reinterpret_cast<VCallSelf *>(this)->slot036(a1, a2);
        void *child = *reinterpret_cast<void **>(self + 0x60);
        if (child != 0) {
            if (reinterpret_cast<VCallChild *>(child)->slot023() == 0) {
                reinterpret_cast<VCallChild *>(child)->slot007();
            }
        }
    }
}
