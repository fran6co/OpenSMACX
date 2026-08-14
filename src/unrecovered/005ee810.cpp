// ORIGINAL: 0x005EE810 BYTE_EXACT FILE
// LEVER: ecx receiver is a real __thiscall this (int a1,int a2,int a3 ->
//   member fn(a1,a2,a3)); own VCall2 shim gives slot021/slot034 (int,int)
//   and slot007/slot023 int returns, others stay nullary void from VCall.
// working copy - scaffold materialised by --work
// name      sub_5ee810
// size      158 bytes
// spans     0x005EE810-0x005EE8AE
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EE850 0x005EE85B 0x005EE86B 0x005EE876 0x005EE88B 0x005EE89C 0x005EE8A7

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5ee810  at 0x005EE810  (158 bytes)
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
// This body dispatches through slot(s): 7, 21, 23, 34
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
    virtual void slot021();  // <-- used
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
    virtual void slot034();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b7ab8 = (int *)0x009B7AB8;

class VCall2 { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual int  slot007();
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
    virtual void slot021(int, int);
    virtual void slot022();
    virtual int  slot023();
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
    virtual void slot034(int, int);
};

class Obj { public:
    void go(int a1, int a2, int a3);
};

void Obj::go(int a1, int a2, int a3) {
    char *self = reinterpret_cast<char *>(this);
    if ((*reinterpret_cast<unsigned int *>(self + 0x98) & 0x200000) != 0) {
        return;
    }
    if ((*reinterpret_cast<unsigned char *>(self + 0x9c) & 8) != 0) {
        return;
    }
    if (a3 == 0) {
        *g_009b7ab8 = reinterpret_cast<int>(this);
        typedef void (__cdecl *Callback)(int, int);
        Callback cb = *reinterpret_cast<Callback *>(self + 0x41c);
        if (cb != 0) {
            cb(a1, a2);
        }
        reinterpret_cast<VCall2 *>(this)->slot021(a1, a2);
        VCall2 *child = *reinterpret_cast<VCall2 **>(self + 0x28);
        if (child != 0) {
            if (child->slot023() == 0) {
                child->slot007();
            }
        }
    } else {
        reinterpret_cast<VCall2 *>(this)->slot034(a1, a2);
        VCall2 *child = *reinterpret_cast<VCall2 **>(self + 0x58);
        if (child != 0) {
            if (child->slot023() == 0) {
                child->slot007();
            }
        }
    }
}
