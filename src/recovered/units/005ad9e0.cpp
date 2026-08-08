// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005AD9E0
// name           ?timer_callback@ReplayWin@@QAEXXZ
// size           492 bytes
// measured tier  MISMATCH
// divergence     6
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005ad9e0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?timer_callback@ReplayWin@@QAEXXZ  at 0x005AD9E0  (492 bytes)
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

class ReplayWin;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" char *strcat(char *, const char *);
extern "C" int __cdecl _itoa();
extern "C" void *memcpy(void *, const void *, unsigned int);
int __cdecl game_year(int);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 58, 62
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
    virtual void slot059();
    virtual void slot060();
    virtual void slot061();
    virtual void slot062();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006913fc = (int *)0x006913FC;
static int *const g_00945e00 = (int *)0x00945E00;
static int *const g_009469f8 = (int *)0x009469F8;
static int *const g_009a64d4 = (int *)0x009A64D4;
static int *const g_009bc054 = (int *)0x009BC054;
// Indexed as `base + register` (a running byte offset), never as a bare
// load - the table base must stay a real symbol or /O2 folds the address
// arithmetic away.
extern uint8_t g_00950324[];

// 8-byte scheduled-event record read out of g_00950324 by memcpy.
struct ReplayEvent {
    int8_t type;
    int8_t value;
    int16_t turn;
    int16_t a;
    int16_t b;
};

// 12-byte slot in the 0x200-entry table living at this+0xa20.
struct MapEntry {
    int32_t a;
    int32_t b;
    int32_t c;
};

typedef char *(__cdecl *ItoaFn)(int, char *, int);

class ReplayWin { public:
    void make_bases();
    void timer_callback();
};

void ReplayWin::timer_callback() {
    // Reach fields by offset - the class is deliberately empty.
    char *self = reinterpret_cast<char *>(this);

    if (*reinterpret_cast<int *>(self + 0xa18) < *g_009a64d4) {
        goto process;
    }
    if (*reinterpret_cast<int *>(self + 0xa18) < *g_009a64d4 + 0x32) {
        return;
    }
    reinterpret_cast<VCall *>(this)->slot058();
    return;

process:
    {
        int next_turn = *reinterpret_cast<int *>(self + 0xa18) + 1;

        if (*reinterpret_cast<int *>(self + 0xa1c) < *g_009469f8) {
            do {
                ReplayEvent rec;
                memcpy(&rec, g_00950324 + *reinterpret_cast<int *>(self + 0xa1c),
                       sizeof(rec));

                if (rec.turn > *reinterpret_cast<int *>(self + 0xa18)) {
                    next_turn = rec.turn;
                    goto finish;
                }

                *reinterpret_cast<int *>(self + 0xa1c) += 8;

                if (rec.turn == *reinterpret_cast<int *>(self + 0xa18)) {
                    MapEntry *table = reinterpret_cast<MapEntry *>(self + 0xa20);
                    switch (rec.type) {
                    case 0: {
                        for (int i = 0; i < 0x200; i++) {
                            if (table[i].a < 0) {
                                table[i].a = rec.a;
                                table[i].b = rec.b;
                                table[i].c = rec.value;
                                break;
                            }
                        }
                        break;
                    }
                    case 1: {
                        for (int i = 0; i < 0x200; i++) {
                            if (table[i].a == rec.a && table[i].b == rec.b) {
                                table[i].c = rec.value;
                                break;
                            }
                        }
                        break;
                    }
                    case 2: {
                        for (int i = 0; i < 0x200; i++) {
                            if (table[i].a == rec.a && table[i].b == rec.b) {
                                table[i].c = -1;
                                table[i].b = -1;
                                table[i].a = -1;
                                break;
                            }
                        }
                        break;
                    }
                    }
                }
            } while (*reinterpret_cast<int *>(self + 0xa1c) < *g_009469f8);
        }

    finish:
        char *msg = self + 0x2220;
        msg[0] = 0;
        const char *suffix = (*g_009bc054 != 0)
            ? reinterpret_cast<const char *>(g_00945e00)
            : reinterpret_cast<const char *>(g_006913fc);
        strcat(msg, suffix);

        int year = game_year(*reinterpret_cast<int *>(self + 0xa18));
        char numbuf[0x50];
        reinterpret_cast<ItoaFn>(_itoa)(year, numbuf, 10);
        strcat(msg, numbuf);

        make_bases();
        reinterpret_cast<VCall *>(this)->slot062();

        *reinterpret_cast<int *>(self + 0xa18) = next_turn;
    }
}
