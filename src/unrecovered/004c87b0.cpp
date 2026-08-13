// ORIGINAL: 0x004C87B0 FILE
// working copy - scaffold materialised by --work
// name      sub_4c87b0
// size      137 bytes
// spans     0x004C87B0-0x004C8839
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C87DC 0x004C8802 0x004C881A 0x004C882D

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_4c87b0  at 0x004C87B0  (137 bytes)
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
// This body dispatches through slot(s): 31, 33
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
    virtual void slot031();  // <-- used
    virtual void slot032();
    virtual void slot033();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0090db24 = (int *)0x0090DB24;
static int *const g_0090db28 = (int *)0x0090DB28;
static int *const g_0090db7c = (int *)0x0090DB7C;
// Signature change: [ecx+0x3c]/[ecx] are read with no matching stack push,
// so the receiver is `this` (a real __thiscall member), not a plain
// __stdcall(a1..a4) free function. Dispatches through its own vtable slot
// 31 and through *(this+0x3c)'s vtable slot 33. The generated VCall shim
// above types every slot void(), so slot 33 (which takes 4 ints and
// returns int) needs its own shim class instead of reusing it.
class VCall4c87b0 { public:
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
    virtual int slot033(int, int, int, int);
};

class Obj4c87b0 : public VCall4c87b0 {
public:
    unsigned char pad_4_[0x38];
    VCall4c87b0 *field_3C_;

    int sub_4c87b0(int a1, int a2, int a3, int a4);
};

int Obj4c87b0::sub_4c87b0(int a1, int a2, int a3, int a4) {
    int result = 0;
    if (field_3C_ == 0) {
        if (*g_0090db7c == 0) {
            return 1;
        }
        typedef int (__cdecl *CreateFn)(void *, int, int);
        CreateFn create = reinterpret_cast<CreateFn>(*g_0090db24);
        result = create(&field_3C_, 0, 4);
        if (result != 0) {
            return result;
        }
    }
    if (field_3C_ != 0) {
        result = field_3C_->slot033(a1, a2, a3, a4);
        if (result != 0) {
            if (*g_0090db7c != 0) {
                typedef int (__cdecl *DestroyFn)(void *);
                DestroyFn destroy = reinterpret_cast<DestroyFn>(*g_0090db28);
                destroy(field_3C_);
            }
            field_3C_ = 0;
        }
        slot031();
    }
    return result;
}
