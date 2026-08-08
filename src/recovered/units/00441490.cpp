// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00441490
// name           ?add_offer@DiploWin@@QAEXHHH@Z
// size           297 bytes
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00441490/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?add_offer@DiploWin@@QAEXHHH@Z  at 0x00441490  (297 bytes)
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
void __cdecl message_data(int, int, int, int, int, int);

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
    void add_offer(int, int, int);
};

void DiploWin::add_offer(int a1, int a2, int a3) {
    char *self = reinterpret_cast<char *>(this);
    int *count_arr = reinterpret_cast<int *>(self + 0xa1c);
    int *type_arr = reinterpret_cast<int *>(self + 0xa2c);
    int *val_arr = reinterpret_cast<int *>(self + 0xa6c);

    *g_007492d0 = 1;

    int i;
    for (i = 0; i < count_arr[a1]; i++) {
        if (a2 == 4 || a2 == 2 || a2 == 3) {
            int existing = type_arr[i + a1 * 8];
            if (existing == 4 || existing == 2 || existing == 3) {
                goto found;
            }
        } else if (a2 == 1) {
            if (type_arr[i + a1 * 8] == 1) {
                if (a3 <= 0) {
                    kill_offer(a1, i);
                    return;
                }
                goto found;
            }
        } else if (a2 == 0) {
            if (type_arr[i + a1 * 8] == 0 && val_arr[i + a1 * 8] == a3) {
                return;
            }
        } else if (a2 == 5) {
            if (type_arr[i + a1 * 8] == 5 && val_arr[i + a1 * 8] == a3) {
                return;
            }
        }
    }

    {
        int count = count_arr[a1];
        if (count >= 8) {
            return;
        }
        count_arr[a1] = count + 1;
        i = count;
    }

found:
    {
        int slot = i + a1 * 8;
        type_arr[slot] = a2;
        val_arr[slot] = a3;
        *reinterpret_cast<int *>(self + 0xa24) = 0;
        *reinterpret_cast<int *>(self + 0xa28) = 0;
        if (a1 == 0) {
            message_data(0x1506, *reinterpret_cast<int *>(self + 0xab8),
                        a2, a3, a1, a1);
        }
        reinterpret_cast<VCall *>(this)->slot062();
    }
}
