// ORIGINAL: 0x004413F0 FILE
// RULED-OUT: `int idx = a2;` declared inside the inner `if (a2 < count - 1)`
//            (mismatch, idx assign shifted 5 instructions late) and hoisted
//            above it with a separate `countMinus1` temp (MNEMONIC_ONLY,
//            worse). Landed the inner-if placement: MISMATCH #23, mnemonic
//            similarity 0.98, same byte count (156/156).
// working copy - scaffold materialised by --work
// name      ?kill_offer@DiploWin@@QAEXHH@Z
// size      156 bytes
// spans     0x004413F0-0x0044148C
// prototype void (__thiscall ?kill_offer@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   3   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00592EE0
// indirect  0x0044147F

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?kill_offer@DiploWin@@QAEXHH@Z  at 0x004413F0  (156 bytes)
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

class DiploWin;

// ---- callees, declared and never defined (a definition would be inlined) ----
void message_data(int, int, int, int, int, int);

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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_007492d0 = (int *)0x007492D0;

class DiploWin { public:
    uint8_t pad_0_[0xA1C];
    uint32_t field_a1c_;

    void kill_offer(int, int);
};
void DiploWin::kill_offer(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    if (a1 == 0) {
        message_data(0x1507, *reinterpret_cast<int *>(self + 0xAB8), a2, 0, 0, 0);
    }
    int count = *reinterpret_cast<int *>(self + 0xA1C + a1 * 4);
    if (a2 < count) {
        if (a2 < count - 1) {
            char *rec = self + 0xA2C + a1 * 32 + a2 * 4;
            int idx = a2;
            do {
                idx = idx + 1;
                *reinterpret_cast<int *>(rec) = *reinterpret_cast<int *>(rec + 4);
                *reinterpret_cast<int *>(rec + 0x40) = *reinterpret_cast<int *>(rec + 0x44);
                rec += 4;
                count = *reinterpret_cast<int *>(self + 0xA1C + a1 * 4);
            } while (idx < count - 1);
        }
        count = *reinterpret_cast<int *>(self + 0xA1C + a1 * 4) - 1;
        *reinterpret_cast<int *>(self + 0xA1C + a1 * 4) = count;
        *g_007492d0 = 1;
        *reinterpret_cast<int *>(self + 0xA24) = 0;
        *reinterpret_cast<int *>(self + 0xA28) = 0;
        reinterpret_cast<VCall *>(this)->slot062();
    }

}
