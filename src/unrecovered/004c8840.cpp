// ORIGINAL: 0x004C8840 BYTE_EXACT FILE
// LEVER: the g_0090db28 function pointer is __cdecl, not __stdcall - the
//        original has `add esp,4` after that call; __stdcall dropped it
//        and cost a MISMATCH at the trailing `add`/`jmp`.
// working copy - scaffold materialised by --work
// name      sub_4c8840
// size      72 bytes
// spans     0x004C8840-0x004C8888
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C884C 0x004C8860 0x004C8870 0x004C887E

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_4c8840  at 0x004C8840  (72 bytes)
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
// This body dispatches through slot(s): 14, 23, 32
class VCall { public:
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
    virtual void slot014();  // <-- used
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
    virtual void slot032();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0090db28 = (int *)0x0090DB28;
static int *const g_0090db7c = (int *)0x0090DB7C;

// `ecx+0x3c` with no stack access -> __thiscall member, not a free
// __cdecl function. VCall's slot023 needs a checked (non-void) return,
// which the scaffold's shared VCall doesn't give the emitted slots, so
// it is shadowed here under its own name rather than edited in place.
class VCallAlt4C8840 { public:
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
    virtual void slot014();  // <-- used
    virtual void slot015();
    virtual void slot016();
    virtual void slot017();
    virtual void slot018();
    virtual void slot019();
    virtual void slot020();
    virtual void slot021();
    virtual void slot022();
    virtual int slot023();  // <-- used, checked
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();  // <-- used, on `this`
};

class Obj4C8840 { public:
    void * pad_0_[0xF];
    void * field_3c_;

    int method();
};

int Obj4C8840::method() {
    if (field_3c_) {
        if (reinterpret_cast<VCallAlt4C8840 *>(field_3c_)->slot023() == 0) {
            if (*g_0090db7c) {
                (*reinterpret_cast<void (__cdecl **)(void *)>(g_0090db28))(field_3c_);
            }
        } else {
            reinterpret_cast<VCallAlt4C8840 *>(field_3c_)->slot014();
        }
        field_3c_ = 0;
    }
    reinterpret_cast<VCallAlt4C8840 *>(this)->slot032();
    return 0;
}
