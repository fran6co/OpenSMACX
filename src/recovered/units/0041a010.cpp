// ORIGINAL: 0x0041A010 FILE
// name      ?prev@BaseWin@@QAEXXZ
// size      333 bytes
// spans     0x0041A010-0x0041A15D
// prototype void (__thiscall ?prev@BaseWin@@QAEXXZ)(BaseWin* this)
// callers   2   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0041A920 0x004E39D0 0x005BF310 0x0064FD20
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0041A010
// measured tier  MISMATCH
// divergence     30
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0041a010/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?prev@BaseWin@@QAEXXZ  at 0x0041A010  (333 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" int __cdecl _strcmpi();
int __cdecl X_pop(const char *, int (__cdecl *)());
void __cdecl set_base(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00682c58 = (int *)0x00682C58;
static int *const g_0090ea30 = (int *)0x0090EA30;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093a938 = (int *)0x0093A938;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009a64cc = (int *)0x009A64CC;

class BaseWin { public:
    uint8_t pad_0_[0x40B10];
    uint32_t field_40b10_;

    void zoom(int, int);
    void prev();
};

extern "C" int __strcmpi(const char *, const char *);
extern unsigned char g_0097d044[];
extern unsigned char g_0097d053[];

void BaseWin::prev() {
    char *self = reinterpret_cast<char *>(this);
    set_base(*reinterpret_cast<int *>(self + 0x40B0C));

    if (*reinterpret_cast<uint8_t *>(*reinterpret_cast<char **>(g_0090ea30) + 4) == *g_00939284 ||
        (*reinterpret_cast<uint8_t *>(g_009a64c0) & 0x80)) {
        if (*g_0093a938 != 0) {
            X_pop(reinterpret_cast<const char *>(g_00682c58), 0);
            return;
        }

        int result = *reinterpret_cast<int *>(self + 0x40B0C);
        int bestLess = -1;

        if (*g_009a64cc > 0) {
            int i = 0;
            int off = 0;
            int offLess = -0x134;
            int offWrap = result * 0x134;
            do {
                int baseIdx = *reinterpret_cast<int *>(self + 0x40B0C);
                if (i != baseIdx) {
                    int baseOff = baseIdx * 0x134;
                    const unsigned char *factionI = g_0097d044 + off;
                    const unsigned char *factionCur = g_0097d044 + baseOff;
                    if (*factionI == *factionCur) {
                        const char *nameI = reinterpret_cast<const char *>(g_0097d053 + off);
                        const char *nameCur = reinterpret_cast<const char *>(g_0097d053 + baseOff);
                        if (__strcmpi(nameI, nameCur) < 0) {
                            const char *nameLess = reinterpret_cast<const char *>(g_0097d053 + offLess);
                            if (bestLess < 0 || __strcmpi(nameI, nameLess) > 0) {
                                bestLess = i;
                                offLess = off;
                            }
                        }
                        const char *nameWrap = reinterpret_cast<const char *>(g_0097d053 + offWrap);
                        if (__strcmpi(nameI, nameWrap) > 0) {
                            result = i;
                            offWrap = off;
                        }
                    }
                }
                i++;
                off += 0x134;
            } while (i < *g_009a64cc);

            if (bestLess >= 0) {
                result = bestLess;
            }
        }

        *reinterpret_cast<int *>(self + 0x40B0C) = result;
        zoom(result, 0);
    }
}
