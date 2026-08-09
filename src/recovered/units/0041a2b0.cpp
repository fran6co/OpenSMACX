// ORIGINAL: 0x0041A2B0 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0041A2B0
// name           ?rename@BaseWin@@QAEXXZ
// size           518 bytes
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0041a2b0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?rename@BaseWin@@QAEXXZ  at 0x0041A2B0  (518 bytes)
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

class BaseWin;
class NetDaemon;

// ---- callees, declared and never defined (a definition would be inlined) ----
class NetDaemon { public:
    int lock_base(int, int, int, int);
    void unlock_base(int);
};
extern "C" char *strcpy(char *, const char *);
int __cdecl X_pop_ask(const char *, int, char *, int (__cdecl *)(), int);
int __cdecl base_compute(int);
int __cdecl parse_says(int, char *, int, int);
unsigned int __cdecl random(int, int);
void __cdecl cause_friction(int, int, int);
void __cdecl draw_radius(int, int, int, int);
void __cdecl set_base(int);
void __cdecl set_treaty(int, int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00682c70 = (int *)0x00682C70;
static int *const g_00689370 = (int *)0x00689370;
static int *const g_0090ea30 = (int *)0x0090EA30;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093a938 = (int *)0x0093A938;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_0096c9f8 = (int *)0x0096C9F8;
static int *const g_0097d0b8 = (int *)0x0097D0B8;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009bb5e8 = (int *)0x009BB5E8;

class BaseWin { public:
    uint8_t pad_0_[0x40B10];
    uint32_t field_40b10_;

    void draw_info(int);
    void rename();
};

// EXTERN-SYMBOL LEVER: 0x97d0b8 and 0x96c9f8 are both indexed with a
// register-scaled offset (`[reg*4 + imm]`), not a plain load/store, so a
// `static int *const` would let /O2 fold the address literal and drop the
// addressing mode the original uses. An extern array keeps it.
extern int g_0097d0b8_arr[];
extern int g_0096c9f8_arr[];

#pragma function(strcpy)

void BaseWin::rename() {
    char *self = reinterpret_cast<char *>(this);
    int32_t &field_40b0c = *reinterpret_cast<int32_t *>(self + 0x40B0C);

    int oldVal = g_0097d0b8_arr[field_40b0c * 77];

    set_base(field_40b0c);
    base_compute(0);

    if (*g_0093f660 != 0 && *g_0093a938 == 0 &&
        oldVal != g_0097d0b8_arr[field_40b0c * 77]) {
        if (reinterpret_cast<NetDaemon *>(g_0093cd90)->lock_base(*g_00689370, 0, -1, -1) == 0) {
            reinterpret_cast<NetDaemon *>(g_0093cd90)->unlock_base(*g_00689370);
        }
    }

    if (*reinterpret_cast<unsigned char *>(*reinterpret_cast<char **>(g_0090ea30) + 4) == *g_00939284 ||
        (*reinterpret_cast<unsigned char *>(g_009a64c0) & 0x80) != 0) {
        if (reinterpret_cast<NetDaemon *>(g_0093cd90)->lock_base(*g_00689370, 0, -1, -1) == 0) {
            parse_says(0, *reinterpret_cast<char **>(g_0090ea30) + 0x13, -1, -1);

            if (X_pop_ask(reinterpret_cast<const char *>(g_00682c70), 0x18,
                          *reinterpret_cast<char **>(g_0090ea30) + 0x13, 0, 0) != 0) {
                reinterpret_cast<NetDaemon *>(g_0093cd90)->unlock_base(*g_00689370);
                return;
            }

            strcpy(*reinterpret_cast<char **>(g_0090ea30) + 0x13,
                   reinterpret_cast<const char *>(g_009bb5e8));

            if (*reinterpret_cast<unsigned char *>(*reinterpret_cast<char **>(g_0090ea30) + 4) == *g_00939284) {
                unsigned char bVar1 = *reinterpret_cast<unsigned char *>(*reinterpret_cast<char **>(g_0090ea30) + 5);
                if (bVar1 != *g_00939284 &&
                    ((1 << bVar1) & *reinterpret_cast<unsigned char *>(g_009a64e8)) == 0 &&
                    *g_0093f660 == 0) {
                    cause_friction(bVar1, *g_00939284, 3);

                    unsigned char bVar2 = *reinterpret_cast<unsigned char *>(*reinterpret_cast<char **>(g_0090ea30) + 5);
                    int table_val = g_0096c9f8_arr[*g_00939284 + bVar2 * 2099];
                    if ((table_val & 7) != 0 && (table_val & 0x20) != 0) {
                        if (random(0, 2) == 0) {
                            unsigned char bVar3 = *reinterpret_cast<unsigned char *>(*reinterpret_cast<char **>(g_0090ea30) + 5);
                            set_treaty(bVar3, *g_00939284, 0x400, 1);
                        }
                    }
                }
            }

            reinterpret_cast<NetDaemon *>(g_0093cd90)->unlock_base(*g_00689370);
            draw_info(1);
            draw_radius(*reinterpret_cast<short *>(*reinterpret_cast<char **>(g_0090ea30)),
                        *reinterpret_cast<short *>(*reinterpret_cast<char **>(g_0090ea30) + 2), 3, 2);
        }
    }
}
