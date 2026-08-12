// ORIGINAL: 0x00424720 FILE
// name      ?set_vote@CouncWin@@QAEXHH@Z
// size      332 bytes
// spans     0x00424720-0x0042486C
// prototype void (__thiscall ?set_vote@CouncWin@@QAEXHH@Z)(CouncWin* this, int, int)
// callers   4   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00424410 0x0052BE60 0x00592EE0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00424720
// measured tier  MISMATCH
// divergence     11
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00424720/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?set_vote@CouncWin@@QAEXHH@Z  at 0x00424720  (332 bytes)
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

class CouncWin;

// ---- callees, declared and never defined (a definition would be inlined) ----
int __cdecl council_get_vote(int, int, int);
void __cdecl message_data(int, int, int, int, int, int);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 1, 2, 62
class VCall { public:
    virtual void slot000();
    virtual void slot001();  // <-- used
    virtual void slot002();  // <-- used
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
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093a95c = (int *)0x0093A95C;
static int *const g_0093e8c0 = (int *)0x0093E8C0;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_009a64e8 = (int *)0x009A64E8;

class CouncWin { public:
    int make_resolution(int);
    void set_vote(int, int);
};

// Slot 1 is called with an argument (`push 0` before the call), but the
// generated VCall shim declares every slot nullary - a second shim class
// with slot001 taking an int expresses that call site.
class VCall1 { public:
    virtual void slot000();
    virtual void slot001(int);  // <-- used
};

void CouncWin::set_vote(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);

    if (*reinterpret_cast<int32_t *>(self + 0xa24) == 0) {
        int32_t *arr = reinterpret_cast<int32_t *>(self + 0xa54);
        arr[a1] = a2;

        bool allVoted = true;
        uint32_t mask = *g_009a64e8;
        for (int32_t i = 0; i < 8; i++) {
            if ((mask & 0xff & (1 << i)) != 0 && arr[i] != 0) {
                allVoted = false;
            }
        }

        if (allVoted) {
            int32_t repeat = 3;
            do {
                uint32_t mask2 = *g_009a64e8;
                for (int32_t i = 0; i < 8; i++) {
                    int32_t bit = 1 << i;
                    if ((bit & (mask2 >> 8)) != 0 && (bit & mask2) == 0) {
                        arr[i] = council_get_vote(
                            i, *reinterpret_cast<int32_t *>(self + 0xa1c),
                            *reinterpret_cast<int32_t *>(self + 0xa20));
                        mask2 = *g_009a64e8;
                    }
                }
                repeat--;
            } while (repeat != 0);
        }

        reinterpret_cast<VCall *>(this)->slot062();

        if (*g_0093f660 == 0) {
            make_resolution(1);
        } else if (*g_0093e8c0 != 0) {
            if (make_resolution(0) != 0) {
                message_data(0x2603, 0, 0, 0, 0, 0);
            }
        }

        if (*g_0093a95c != 0 && a1 == *g_00939284) {
            char *obj = self + 0xd94;
            for (int32_t k = 0; k < 6; k++) {
                reinterpret_cast<VCall *>(obj)->slot002();
                obj += 0xb4c;
            }
            reinterpret_cast<VCall1 *>(self + 0x4610)->slot001(0);
        }
    }
}
