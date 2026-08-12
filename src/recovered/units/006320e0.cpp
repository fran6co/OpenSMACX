// ORIGINAL: 0x006320E0 FILE
// name      ?check_polling@Net@@QAEXXZ
// size      432 bytes
// spans     0x006320E0-0x00632290
// prototype void (__thiscall ?check_polling@Net@@QAEXXZ)(Net* this)
// callers   4   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0062F8A0 0x00632350
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x006320E0
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/006320e0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?check_polling@Net@@QAEXXZ  at 0x006320E0  (432 bytes)
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

class Net;

// ---- callees, declared and never defined (a definition would be inlined) ----

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669368 = (int *)0x00669368;
static int *const g_009be600 = (int *)0x009BE600;
static int *const g_009be60c = (int *)0x009BE60C;
static int *const g_009be610 = (int *)0x009BE610;

class Net { public:
    int send_packet_type(void *, int, unsigned long, int, int);
    int drop_player(unsigned long, int);
    void check_polling();
};

typedef unsigned long (__stdcall *FnTimeGetTime)();

void Net::check_polling() {
    char *self = reinterpret_cast<char *>(this);

    if (*g_009be600 == 0) {
        return;
    }

    FnTimeGetTime timeGetTime = *reinterpret_cast<FnTimeGetTime *>(g_00669368);

    unsigned long now = timeGetTime();
    if (*reinterpret_cast<unsigned int *>(self + 0xdc) >= now - 2000) {
        return;
    }
    if (*g_009be610 != 0) {
        return;
    }

    *g_009be610 = 1;
    send_packet_type(0, 0, 0, 0x20, 0);
    now = timeGetTime();
    *reinterpret_cast<unsigned int *>(self + 0xdc) = now;
    *g_009be610 = 0;

    if (*g_009be60c != 0) {
        return;
    }
    *g_009be60c = 1;

    if ((*reinterpret_cast<unsigned int *>(self + 0xd8) & 0x40000) != 0) {
        if (*reinterpret_cast<int *>(self + 0x760) == *reinterpret_cast<int *>(self + 0x764)) {
            int *p = reinterpret_cast<int *>(self + 0x154);
            int count = 0x10;
            do {
                if (*p != 0 && *p != *reinterpret_cast<int *>(self + 0x760)) {
                    int lastTime = p[4];
                    if (lastTime != 0) {
                        now = timeGetTime();
                        if (*reinterpret_cast<unsigned int *>(self + 0xe0) < now - lastTime) {
                            if (drop_player(*reinterpret_cast<unsigned long *>(p), 0) == 0) {
                                now = timeGetTime();
                                p[4] = now;
                            }
                        }
                    }
                }
                p += 0x16;
                count--;
            } while (count != 0);
            *g_009be60c = 0;
            return;
        }
        if (*reinterpret_cast<int *>(self + 0x760) == *reinterpret_cast<int *>(self + 0x1ac)) {
            int lastTime = *reinterpret_cast<int *>(self + 0x164);
            if (lastTime != 0) {
                now = timeGetTime();
                if (*reinterpret_cast<unsigned int *>(self + 0xe0) < now - lastTime) {
                    if (drop_player(*reinterpret_cast<unsigned long *>(self + 0x154), 0) == 0) {
                        now = timeGetTime();
                        *reinterpret_cast<unsigned int *>(self + 0x164) = now;
                        *g_009be60c = 0;
                        return;
                    }
                }
            }
        } else {
            int count = 0;
            unsigned int *p = reinterpret_cast<unsigned int *>(self + 0x164);
            for (;;) {
                unsigned int t = *p;
                if (t == 0) {
                    return;
                }
                unsigned long nowInner = timeGetTime();
                if (nowInner - t < *reinterpret_cast<unsigned int *>(self + 0xe0)) {
                    return;
                }
                count++;
                p += 0x16;
                if (count >= 2) {
                    break;
                }
            }
            if (drop_player(0xffffffff, 0) == 0) {
                now = timeGetTime();
                *reinterpret_cast<unsigned int *>(self + 0x1bc) = now;
                *reinterpret_cast<unsigned int *>(self + 0x164) = now;
            }
        }
    }
    *g_009be60c = 0;
}
