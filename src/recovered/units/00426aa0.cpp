// ORIGINAL: 0x00426AA0 FILE
// name      ?on_redraw@CouncWin@@QAEXXZ
// size      1025 bytes
// spans     0x00426AA0-0x00426EA1
// prototype void (__thiscall ?on_redraw@CouncWin@@QAEXXZ)(CouncWin* this)
// callers   0   call targets   10
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00425DB0 0x00426470 0x0045C290 0x005D8000 0x005DAC70 0x005DACB0 0x005DDAB0 0x005E2DD3 0x006169A0 0x00645470
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00426AA0
// measured tier  MISMATCH
// divergence     19
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00426aa0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_redraw@CouncWin@@QAEXXZ  at 0x00426AA0  (1025 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under BOTH cl 12.00.8168 and
// i686-w64-mingw32-g++ -std=c++11. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.

typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;

struct BoxSpriteParams;
struct Font;
struct RECT;

// The scaffold only forward-declares RECT (never completes it); other
// addresses' briefs (e.g. 0x00442DC0) show its real shape.
struct RECT {
    long left;
    long top;
    long right;
    long bottom;
};

// ---- callees, declared and never defined (a definition would be inlined) ----
class Buffer { public:
    int box_sprite(RECT*, BoxSpriteParams*);
    int set_clip(RECT* rect);
    int set_font(Font*, Font*, Font*, Font*);
    int wrap_cent(char*, RECT*);
    void set_text_color(int, int, int, int);
};
class MainInterface { public:
    void restore_back(int);
};
class Strings { public:
    int get(int);
};
// `int8*` (the on-disk scaffold's spelling, via an opaque `struct int8;`)
// does not mangle to `PAD` the way a fresh brief's `char*` does - see
// address 0x00442DC0's note that the catalogue's `int8` means `char`, not
// a struct. Fixed per rule 5.
extern "C" char *strcat(char *, const char *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066931c = (int *)0x0066931C;
static int *const g_00682820 = (int *)0x00682820;
static int *const g_00682e94 = (int *)0x00682E94;
static int *const g_00682e98 = (int *)0x00682E98;
static int *const g_00682e9c = (int *)0x00682E9C;
static int *const g_00683224 = (int *)0x00683224;
static int *const g_0068fa30 = (int *)0x0068FA30;
static int *const g_0078d528 = (int *)0x0078D528;
static int *const g_0078d5a0 = (int *)0x0078D5A0;
static int *const g_0078d690 = (int *)0x0078D690;
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_007aec64 = (int *)0x007AEC64;
static int *const g_007aec94 = (int *)0x007AEC94;
static int *const g_007af5c0 = (int *)0x007AF5C0;
static int *const g_007af5c4 = (int *)0x007AF5C4;
static int *const g_007af5c8 = (int *)0x007AF5C8;
static int *const g_007af5cc = (int *)0x007AF5CC;
static int *const g_009a6614 = (int *)0x009A6614;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

class CouncWin { public:
    void draw_leader(int factionID);
    void draw_info();
    void on_redraw();
};

extern char mz_00946a50[];

void CouncWin::on_redraw() {
    char *self = reinterpret_cast<char *>(this);

    reinterpret_cast<Buffer *>(g_007aec64)->set_clip(reinterpret_cast<RECT *>(g_007aec94));
    Buffer *buf = reinterpret_cast<Buffer *>(self + 0x444);
    buf->set_clip(reinterpret_cast<RECT *>(reinterpret_cast<char *>(buf) + 0x30));

    reinterpret_cast<MainInterface *>(g_007ae820)->restore_back(1);

    RECT rect;
    rect.left = *g_007af5c4 + 4;
    rect.top = *g_007af5c0 + 4;
    rect.right = *g_007af5c8 - 4;
    rect.bottom = *g_007af5cc - 4;
    buf->box_sprite(&rect, reinterpret_cast<BoxSpriteParams *>(g_0078d5a0));

    rect.left = 0x17;
    rect.top = 0;
    rect.right = *reinterpret_cast<int *>(self + 0x4c4) - 0x37;
    rect.bottom = 1 - *reinterpret_cast<int *>(self + 0x4c8);
    buf->box_sprite(&rect, reinterpret_cast<BoxSpriteParams *>(g_0078d690));

    rect.left += 4;
    rect.top += 4;
    rect.right -= 4;
    rect.bottom -= 4;
    buf->box_sprite(&rect, reinterpret_cast<BoxSpriteParams *>(g_0078d5a0));

    draw_info();
    for (int i = 1; i < 8; ++i) {
        draw_leader(i);
    }

    if (*reinterpret_cast<int *>(self + 0xa24) == 0) {
        return;
    }

    rect = *reinterpret_cast<RECT *>(self + 0xba4);
    buf->box_sprite(&rect, reinterpret_cast<BoxSpriteParams *>(g_0078d528));
    *reinterpret_cast<char *>(g_009b86a0) = 0;
    rect.right -= 3;
    rect.left += 3;
    rect.top += 4;
    rect.bottom -= 2;

    int state = *reinterpret_cast<int *>(self + 0xa24);
    int result;
    if (state == -2) {
        buf->set_text_color(0xf9, -1, 1, 1);
        result = *reinterpret_cast<int *>(*g_009b90f8 + 0x7e4);
    } else if (state == -1) {
        buf->set_text_color(0xf9, -1, 1, 1);
        if (*reinterpret_cast<int *>(self + 0xa1c) != 0) {
            result = reinterpret_cast<Strings *>(g_009b90d8)->get(
                *reinterpret_cast<int *>(*g_009b90f8 + 0x7e0));
            strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(result));
            if (*reinterpret_cast<int *>(self + 0xa1c) == 1) {
                strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(g_00682820));
                strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(g_00682e9c));
                result = reinterpret_cast<Strings *>(g_009b90d8)->get(
                    *reinterpret_cast<int *>(*g_009b90f8 + 0xe6c));
                strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(result));
                strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(g_00682e98));
            }
            goto after_switch;
        }
        result = *reinterpret_cast<int *>(*g_009b90f8 + 0x804);
    } else if (state == 1) {
        int idx = *reinterpret_cast<int *>(self + 0xa20);
        char *player = mz_00946a50 + idx * 0x59c;
        int a1c = *reinterpret_cast<int *>(self + 0xa1c);
        int color;
        if (a1c == 0) {
            color = g_0068fa30[idx];
        } else {
            color = 0xfa;
        }
        buf->set_text_color(color, -1, 1, 1);
        if (a1c == 0) {
            *g_009bbff0 = 0;
            *g_009bbfec = *reinterpret_cast<int *>(player);
            strcat(reinterpret_cast<char *>(g_009b86a0), player + 0x34);
            strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(g_00682820));
            int off = (*g_009a6614 != *reinterpret_cast<int *>(self + 0xa28)) ? 0x1fd : 0x200;
            result = *reinterpret_cast<int *>(*g_009b90f8 + off * 4);
        } else if (a1c == 1) {
            result = reinterpret_cast<Strings *>(g_009b90d8)->get(
                *reinterpret_cast<int *>(*g_009b90f8 + 0xe10));
            strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(result));
            strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(g_00682e94));
            *g_009bbff0 = 0;
            *g_009bbfec = *reinterpret_cast<int *>(player);
            strcat(reinterpret_cast<char *>(g_009b86a0), player + 0x34);
            strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(g_00683224));
            buf->set_text_color(g_0068fa30[idx], -1, 1, 1);
            goto after_switch;
        } else {
            result = *reinterpret_cast<int *>(*g_009b90f8 + 0x7dc);
        }
    } else {
        goto after_switch;
    }
    result = reinterpret_cast<Strings *>(g_009b90d8)->get(result);
    strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(result));

after_switch:
    buf->set_font(reinterpret_cast<Font *>(self + 0xb7c), 0, 0, 0);
    (*reinterpret_cast<char *(__stdcall *)(char *)>(g_0066931c))(reinterpret_cast<char *>(g_009b86a0));
    buf->wrap_cent(reinterpret_cast<char *>(g_009b86a0), &rect);
}
