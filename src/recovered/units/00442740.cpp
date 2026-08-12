// ORIGINAL: 0x00442740 FILE
// name      ?do_it@DiploWin@@QAEXXZ
// size      605 bytes
// spans     0x00442740-0x0044299D
// prototype void (__thiscall ?do_it@DiploWin@@QAEXXZ)(DiploWin* this)
// callers   3   call targets   8
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005391C0 0x00539460 0x005395F0 0x005397B0 0x00539820 0x005398C0 0x00592EE0 0x005B9F20
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00442740
// measured tier  MISMATCH
// divergence     13
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00442740/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?do_it@DiploWin@@QAEXXZ  at 0x00442740  (605 bytes)
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
bool __cdecl has_tech(int, int);
int __cdecl diplo_lock(int);
void __cdecl diplo_unlock();
void __cdecl message_data(int, int, int, int, int, int);
void __cdecl net_double_cross(int, int, int, int);
void __cdecl net_energy(int, int, int, int, int);
void __cdecl net_tech(int, int, int, int);
void __cdecl net_treaty_on(int, int, int, int);

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
static int *const g_004429a0 = (int *)0x004429A0;
static int *const g_007492cc = (int *)0x007492CC;
static int *const g_0093a95c = (int *)0x0093A95C;
static int *const g_0096c9f8 = (int *)0x0096C9F8;
static int *const g_0096cc00 = (int *)0x0096CC00;
static int *const g_009a64e8 = (int *)0x009A64E8;

class DiploWin { public:
    uint8_t pad_0_[0xA1C];
    uint32_t field_a1c_;

    void do_it();
};

// Indexed by a register (civ id * 8396) - same per-civ stride table seen
// elsewhere in this codebase (0x0045BE80 uses the same 0x96cc00/8396 shape).
extern unsigned char g_0096cc00x[];
// Indexed by techId*4 + civId*8396 - a byte flag grid, not the int array the
// pointer-constant scaffolding form would suggest.
extern unsigned char g_0096c9f8x[];

void DiploWin::do_it() {
    int *base = reinterpret_cast<int *>(this);

    if ((base[0x2ad] <= base[0x2ae] || *g_0093a95c != 0) && *g_007492cc < 2) {
        *g_007492cc = 2;
        if (diplo_lock(200) == 0) {
            int side = 0;
            do {
                int count = base[side + 0x287];
                if (count > 0) {
                    int *rec = base + side * 8 + 0x28b;
                    int i = 0;
                    do {
                        int myId, otherId;
                        if (side == 0) {
                            myId = base[0x2ad];
                            otherId = base[0x2ae];
                        } else {
                            myId = base[0x2ae];
                            otherId = base[0x2ad];
                        }
                        switch (*rec) {
                        case 0: {
                            int val = base[i + side * 8 + 0x29b];
                            if (val > 0x58 || !has_tech(val, otherId)) {
                                net_tech(otherId, val, myId, 1);
                            }
                            break;
                        }
                        case 1: {
                            int cap = *reinterpret_cast<int *>(g_0096cc00x + myId * 8396);
                            int val = base[i + side * 8 + 0x29b];
                            int amt = val;
                            if (val < 0 || cap < 0) {
                                amt = 0;
                            } else if (cap < val) {
                                amt = cap;
                            }
                            net_energy(myId, -amt, otherId, amt, 1);
                            break;
                        }
                        case 2:
                            net_treaty_on(myId, otherId, 1, 1);
                            break;
                        case 3:
                            net_treaty_on(myId, otherId, 2, 1);
                            break;
                        case 4:
                            net_treaty_on(myId, otherId, 4, 1);
                            break;
                        case 5: {
                            int techId = base[i + side * 8 + 0x29b];
                            if (((1 << (techId & 0x1f)) & *reinterpret_cast<unsigned char *>(g_009a64e8)) != 0 &&
                                (g_0096c9f8x[techId * 4 + myId * 8396] & 0x10) == 0) {
                                message_data(0x150f, techId, myId, otherId, 0, 0);
                            }
                            net_treaty_on(myId, techId, 8, 0);
                            net_double_cross(myId, techId, otherId, 1);
                            break;
                        }
                        }
                        ++rec;
                        ++i;
                    } while (i < base[side + 0x287]);
                }
                ++side;
            } while (side < 2);

            diplo_unlock();
            *g_007492cc = 1;
            base[0x28a] = 3;
            base[0x289] = 3;
            base[0x288] = 0;
            base[0x287] = 0;
            message_data(0x150d, base[0x2ae], 0, 0, 0, 0);
            reinterpret_cast<VCall *>(this)->slot062();
        }
    }
}
