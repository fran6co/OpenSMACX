// ORIGINAL: 0x005EE8B0 sub_5ee8b0 0x005EE8B0-0x005EE94E FILE BYTE_EXACT
// symbol    ?run@Obj5ee8b0@@QAEXHHH@Z
// working copy - scaffold materialised by --work
// size      158 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EE8F0 0x005EE8FB 0x005EE90B 0x005EE916 0x005EE92B 0x005EE93C 0x005EE947

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5ee8b0  at 0x005EE8B0  (158 bytes)
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
// This body dispatches through slot(s): 7, 22, 23, 35
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
    virtual void slot022();  // <-- used
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
    virtual void slot035();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b7ab8 = (int *)0x009B7AB8;

// A second vtable shim: the generated VCall above types every slot
// void(), but slot022/slot035 here take two ints and slot023 returns
// a value. Declaration order still pins the offset.
class VCall2 { public:
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
    virtual void slot022(int, int);  // <-- used
    virtual int slot023();  // <-- used
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
    virtual void slot035(int, int);  // <-- used
};

class Obj5ee8b0 : public VCall2 {
public:
    void run(int a1, int a2, int a3);
};

void Obj5ee8b0::run(int a1, int a2, int a3) {
    char *self = (char *)this;
    if ((*(unsigned int *)(self + 0x98) & 0x200000) != 0) return;
    if ((*(unsigned char *)(self + 0x9c) & 8) != 0) return;
    if (a3 == 0) {
        *g_009b7ab8 = (int)this;
        void (__cdecl *fp)(int, int) = *(void (__cdecl **)(int, int))(self + 0x420);
        if (fp != 0) {
            fp(a1, a2);
        }
        this->slot022(a1, a2);
        Obj5ee8b0 *child = *(Obj5ee8b0 **)(self + 0x2c);
        if (child != 0) {
            if (child->slot023() == 0) {
                child->slot007();
            }
        }
    } else {
        this->slot035(a1, a2);
        Obj5ee8b0 *child = *(Obj5ee8b0 **)(self + 0x5c);
        if (child != 0) {
            if (child->slot023() == 0) {
                child->slot007();
            }
        }
    }
}
