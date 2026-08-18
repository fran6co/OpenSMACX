// ORIGINAL: 0x005EE9F0 sub_5ee9f0 0x005EE9F0-0x005EEA8E FILE
// working copy - scaffold materialised by --work
// size      158 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EEA30 0x005EEA3B 0x005EEA4B 0x005EEA56 0x005EEA6B 0x005EEA7C 0x005EEA87

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5ee9f0  at 0x005EE9F0  (158 bytes)
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
// This body dispatches through slot(s): 7, 23, 24, 37
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
    virtual void slot024();  // <-- used
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
    virtual void slot036();
    virtual void slot037();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b7ab8 = (int *)0x009B7AB8;

// This body dispatches through slots 7, 23, 24 (with a bool-ish return) and
// 37 (two int args) - the scaffold's VCall types every slot void() with no
// args, so a second shim is needed for the slots actually called with
// arguments or a return value.
class VCall2 { public:
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
    virtual int  slot023();
    virtual void slot024(int, int);
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
    virtual void slot036();
    virtual void slot037(int, int);
};

typedef void (__cdecl *Fn428Callback)(int, int);

// Evidence: [ecx+N] reads with no matching stack slot in the given 3-int
// __stdcall head mean the receiver is `this`, not a1 - so this is spelled as
// a __thiscall member on a locally introduced class instead.
class Recv5EE9F0 { public:
    void fn(int a2, int a3, int a4);
};

void Recv5EE9F0::fn(int a2, int a3, int a4) {
    char *self = reinterpret_cast<char *>(this);
    unsigned int flags98 = *reinterpret_cast<unsigned int *>(self + 0x98);
    unsigned char byte9c = *reinterpret_cast<unsigned char *>(self + 0x9c);
    Recv5EE9F0 *other;

    if ((flags98 & 0x200000) != 0) {
        return;
    }
    if ((byte9c & 8) != 0) {
        return;
    }

    if (a4 == 0) {
        Fn428Callback cb = *reinterpret_cast<Fn428Callback *>(self + 0x428);

        *g_009b7ab8 = reinterpret_cast<int>(this);
        if (cb != 0) {
            cb(a2, a3);
        }
        reinterpret_cast<VCall2 *>(this)->slot024(a2, a3);
        other = *reinterpret_cast<Recv5EE9F0 **>(self + 0x34);
        if (other != 0) {
            if (reinterpret_cast<VCall2 *>(other)->slot023() == 0) {
                reinterpret_cast<VCall2 *>(other)->slot007();
            }
        }
    } else {
        reinterpret_cast<VCall2 *>(this)->slot037(a2, a3);
        other = *reinterpret_cast<Recv5EE9F0 **>(self + 0x64);
        if (other != 0) {
            if (reinterpret_cast<VCall2 *>(other)->slot023() == 0) {
                reinterpret_cast<VCall2 *>(other)->slot007();
            }
        }
    }
}
