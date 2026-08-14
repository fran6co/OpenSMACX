// ORIGINAL: 0x005243A0 FILE
// RULED-OUT: extern "C" __stdcall(int,int) contract (ecx has no matching stack
//            slot, so it is __thiscall); own Obj_5243a0 class (3 embedded
//            RECTs) + VCall slot058 reaches MNEMONIC_ONLY, same 187 bytes,
//            same mnemonics, an operand still differs
// working copy - scaffold materialised by --work
// name      sub_5243a0
// size      187 bytes
// spans     0x005243A0-0x0052445B
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005FA7E0
// indirect  0x005243DB 0x0052440B 0x0052444E

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5243a0  at 0x005243A0  (187 bytes)
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

struct RECT;

// ---- callees, declared and never defined (a definition would be inlined) ----
struct RECT {
    long left;
    long top;
    long right;
    long bottom;
};

int in_box(int, int, RECT *);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 58
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
    virtual void slot031();
    virtual void slot032();
    virtual void slot033();
    virtual void slot034();
    virtual void slot035();
    virtual void slot036();
    virtual void slot037();
    virtual void slot038();
    virtual void slot039();
    virtual void slot040();
    virtual void slot041();
    virtual void slot042();
    virtual void slot043();
    virtual void slot044();
    virtual void slot045();
    virtual void slot046();
    virtual void slot047();
    virtual void slot048();
    virtual void slot049();
    virtual void slot050();
    virtual void slot051();
    virtual void slot052();
    virtual void slot053();
    virtual void slot054();
    virtual void slot055();
    virtual void slot056();
    virtual void slot057();
    virtual void slot058();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00939ff8 = (int *)0x00939FF8;

class Obj_5243a0 { public:
    void *vtable_;
    uint8_t pad_4_[0xa10];
    int32_t field_a14_;
    uint8_t pad_a18_[0xa44 - 0xa18];
    RECT rect1_;
    RECT rect2_;
    RECT rect3_;

    void handler(int, int);
};

void Obj_5243a0::handler(int a1, int a2) {
    field_a14_ = 0;
    if (in_box(a1, a2, &rect1_) != 0) {
        field_a14_ = 1;
        reinterpret_cast<VCall *>(this)->slot058();
        return;
    }
    if (in_box(a1, a2, &rect2_) != 0) {
        field_a14_ = 2;
        reinterpret_cast<VCall *>(this)->slot058();
        return;
    }
    if (in_box(a1, a2, &rect3_) != 0 ||
        in_box(a1, a2, reinterpret_cast<RECT *>(g_00939ff8)) != 0) {
        field_a14_ = 3;
    }
    reinterpret_cast<VCall *>(this)->slot058();
}
