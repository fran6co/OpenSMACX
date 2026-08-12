// ORIGINAL: 0x0041B4F0 FILE
// name      ?staple_the_bastards@BaseWin@@QAEXXZ
// size      669 bytes
// spans     0x0041B4F0-0x0041B78D
// prototype void (__thiscall ?staple_the_bastards@BaseWin@@QAEXXZ)(BaseWin* this)
// callers   1   call targets   10
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00421670 0x0048C0A0 0x004CA7F0 0x004E39D0 0x004EC3B0 0x00530D50 0x00530E50 0x00531300 0x00592EE0 0x00625EC0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0041B4F0
// measured tier  MISMATCH
// divergence     67
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0041b4f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?staple_the_bastards@BaseWin@@QAEXXZ  at 0x0041B4F0  (669 bytes)
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
    void await_exec(int);
    void unlock_base(int);
};
bool __cdecl has_fac(int, int, int);
int __cdecl base_compute(int);
int __cdecl parse_says(int, char *, int, int);
int __cdecl popp(char *, const char *, int, const char *, int (__cdecl *)());
void __cdecl action_staple(int);
void __cdecl message_data(int, int, int, int, int, int);
void __cdecl set_base(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00682ce0 = (int *)0x00682CE0;
static int *const g_00682cec = (int *)0x00682CEC;
static int *const g_00682d00 = (int *)0x00682D00;
static int *const g_00682d10 = (int *)0x00682D10;
static int *const g_00682d24 = (int *)0x00682D24;
static int *const g_00682d34 = (int *)0x00682D34;
static int *const g_00682d44 = (int *)0x00682D44;
static int *const g_00682d54 = (int *)0x00682D54;
static int *const g_00682d64 = (int *)0x00682D64;
static int *const g_00682d74 = (int *)0x00682D74;
static int *const g_00682d80 = (int *)0x00682D80;
static int *const g_00682d90 = (int *)0x00682D90;
static int *const g_00689370 = (int *)0x00689370;
static int *const g_00691b0c = (int *)0x00691B0C;
static int *const g_0090ea30 = (int *)0x0090EA30;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093a938 = (int *)0x0093A938;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_0096cc6c = (int *)0x0096CC6C;
static int *const g_0097d044 = (int *)0x0097D044;
static int *const g_0097d0b8 = (int *)0x0097D0B8;
static int *const g_009a64c0 = (int *)0x009A64C0;

class BaseWin { public:
    uint8_t pad_0_[0x40B10];
    uint32_t field_40b10_;

    void staple_the_bastards();
};

extern int g_0097d0b8_arr[];
extern uint8_t g_0097d044_records[];
extern int g_0096cc6c_arr[];
extern uint8_t g_00946f58_arr[];
extern NetDaemon g_0093cd90_obj;
extern const char g_00682ce0_str[];
extern const char g_00682cec_str[];
extern const char g_00682d00_str[];
extern const char g_00682d10_str[];
extern const char g_00682d24_str[];
extern const char g_00682d34_str[];
extern const char g_00682d44_str[];
extern const char g_00682d54_str[];
extern const char g_00682d64_str[];
extern const char g_00682d74_str[];
extern const char g_00682d80_str[];
extern const char g_00682d90_str[];

void BaseWin::staple_the_bastards() {
    char *self = reinterpret_cast<char *>(this);

    int before = g_0097d0b8_arr[*reinterpret_cast<int32_t *>(self + 0x40b0c) * 0x4d];
    set_base(*reinterpret_cast<int32_t *>(self + 0x40b0c));
    base_compute(0);

    if (*g_0093f660 != 0 && *g_0093a938 == 0 &&
        before != g_0097d0b8_arr[*reinterpret_cast<int32_t *>(self + 0x40b0c) * 0x4d]) {
        if (g_0093cd90_obj.lock_base(*g_00689370, 0, -1, -1) == 0) {
            g_0093cd90_obj.unlock_base(*g_00689370);
        }
    }

    if (g_0097d044_records[*reinterpret_cast<int32_t *>(self + 0x40b0c) * 0x134] != *g_00939284 &&
        (*reinterpret_cast<uint8_t *>(g_009a64c0) & 0x80) == 0) {
        return;
    }

    unsigned int uVar2 = *reinterpret_cast<uint8_t *>(reinterpret_cast<char *>(*g_0090ea30) + 4);
    parse_says(0, reinterpret_cast<char *>(*g_0090ea30) + 0x13, -1, -1);
    int iVar1 = has_fac(0x23, *g_00689370, 0);

    if (*reinterpret_cast<int32_t *>(reinterpret_cast<char *>(g_0096cc6c_arr) + uVar2 * 0x20cc) + iVar1 * 2 < 0) {
        if ((g_00946f58_arr[uVar2 * 0x59c] & 0x80) == 0) {
            popp((char *)*g_00691b0c, g_00682d10_str, 0, g_00682d00_str, 0);
            return;
        }
        popp((char *)*g_00691b0c, g_00682cec_str, 0, g_00682ce0_str, 0);
        return;
    }

    if (*(reinterpret_cast<char *>(*g_0090ea30) + 8) != '\0') {
        if ((g_00946f58_arr[uVar2 * 0x59c] & 0x80) == 0) {
            popp((char *)*g_00691b0c, g_00682d54_str, 0, g_00682d44_str, 0);
            return;
        }
        popp((char *)*g_00691b0c, g_00682d34_str, 0, g_00682d24_str, 0);
        return;
    }

    if (g_0093cd90_obj.lock_base(*g_00689370, 0, -1, -1) != 0) {
        return;
    }

    if ((g_00946f58_arr[uVar2 * 0x59c] & 0x80) == 0) {
        if (popp((char *)*g_00691b0c, g_00682d90_str, 0, g_00682d80_str, 0) == 0) {
            goto unlock_tail;
        }
    } else {
        if (popp((char *)*g_00691b0c, g_00682d74_str, 0, g_00682d64_str, 0) == 0) {
            g_0093cd90_obj.unlock_base(*g_00689370);
            return;
        }
    }

    if (*g_0093f660 != 0) {
        message_data(0x2414, 0, *g_00689370, 0, 0, 0);
        g_0093cd90_obj.await_exec(1);
        g_0093cd90_obj.unlock_base(*g_00689370);
        return;
    }
    action_staple(*g_00689370);
unlock_tail:
    g_0093cd90_obj.unlock_base(*g_00689370);
}
