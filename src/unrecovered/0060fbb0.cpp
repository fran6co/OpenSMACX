// ORIGINAL: 0x0060FBB0 FILE
// RULED-OUT: literal register-mirroring translation of the offset chain
//            ([this-0x1c] outer descriptor, +8 offset, base+0x28/0x48/0x4c
//            denominators, base+0x3c multiplier, base+0xd0/0xd4 bit
//            tracking, then the outer[+4] vtable-slot-62 dispatch). Both
//            an intermediate-locals form and an inline-division form
//            diverge at instr #8 (original 'cdq' vs rebuilt 'imul') around
//            where the sign-extend for the first idiv is scheduled.
// working copy - scaffold materialised by --work
// name      ?on_mouse_move@CheckBox@@QAEXHH@Z
// size      123 bytes
// spans     0x0060FBB0-0x0060FC2B
// prototype void (__thiscall ?on_mouse_move@CheckBox@@QAEXHH@Z)(CheckBox* this, int, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0060FC1E

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_mouse_move@CheckBox@@QAEXHH@Z  at 0x0060FBB0  (123 bytes)
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

class CheckBox;

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 62
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
    virtual void slot058();
    virtual void slot059();
    virtual void slot060();
    virtual void slot061();
    virtual void slot062();  // <-- used
};

class CheckBox { public:
    void on_mouse_move(int, int);
};
void CheckBox::on_mouse_move(int a1, int a2) {
    int self_i = reinterpret_cast<int>(this);
    char *outer = *reinterpret_cast<char **>(self_i - 0x1c);
    int off = *reinterpret_cast<int *>(outer + 8);
    char *base = reinterpret_cast<char *>(self_i + off);
    int adj = *reinterpret_cast<int *>(base + 0x28);
    int denom_w = *reinterpret_cast<int *>(base + 0x48) + adj;
    int denom_h = *reinterpret_cast<int *>(base + 0x4c) + adj;
    int col = a1 / denom_w;
    int row = (a2 * (*reinterpret_cast<int *>(base + 0x3c))) / denom_h;
    int bit = col + row;
    unsigned int mask = *reinterpret_cast<unsigned int *>(base + 0xd0);
    unsigned int flag = 1u << bit;
    if ((flag & mask) != 0) {
        int cur = *reinterpret_cast<int *>(base + 0xd4);
        if (cur != -1) {
            *reinterpret_cast<int *>(base + 0xd4) = -1;
        }
    } else {
        *reinterpret_cast<int *>(base + 0xd4) = bit;
    }
    char *outer2 = *reinterpret_cast<char **>(self_i - 0x1c);
    int base4 = *reinterpret_cast<int *>(outer2 + 4);
    int self3 = base4 + self_i - 0x1c;
    reinterpret_cast<VCall *>(self3)->slot062();
}
