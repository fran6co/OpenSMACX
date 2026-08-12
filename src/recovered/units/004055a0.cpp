// ORIGINAL: 0x004055A0 FILE
// name      ?POP3@@YAXPBDPBDHHH@Z
// size      1028 bytes
// spans     0x004055A0-0x00405910;0x00650839-0x006508CD
// prototype 
// callers   2   call targets   14
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00401100 0x00402500 0x00406380 0x004064D0 0x00406880 0x0045C0F0 0x005D4DD0 0x00600860 0x00600F00 0x006051D0 0x00605370 0x00644F3A 0x00645470 0x00645550
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004055A0
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004055a0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?POP3@@YAXPBDPBDHHH@Z  at 0x004055A0  (1028 bytes)
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

struct int8;
struct sectionID;
struct srcFileID;

// ---- callees, declared and never defined (a definition would be inlined) ----
class BasePop { public:
    BasePop();
    void close();
    ~BasePop();
};
class FlatButton { public:
    ~FlatButton();
};
class GraphicWin { public:
    ~GraphicWin();
};
class MainInterface { public:
    void set_message(int8*, int, int, int, int);
};
class Popup { public:
    void start(int8* srcFileID, int8* sectionID, int, int8*, int, GraphicWin*);
};
class Scroll { public:
    Scroll();
    void close();
};
class StringStruct { public:
    int add(int);
    int next_entry();
};
int __cdecl __alloca_probe();
int __cdecl _abs();
int __cdecl _strcat();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006508c3 = (int *)0x006508C3;
static int *const g_006695c0 = (int *)0x006695C0;
static int *const g_006695c8 = (int *)0x006695C8;
static int *const g_00669d50 = (int *)0x00669D50;
static int *const g_00669d58 = (int *)0x00669D58;
static int *const g_00671b40 = (int *)0x00671B40;
static int *const g_00682820 = (int *)0x00682820;
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_0093a7b0 = (int *)0x0093A7B0;
static int *const g_0093a800 = (int *)0x0093A800;
static int *const g_0093a950 = (int *)0x0093A950;
static int *const g_0093a95c = (int *)0x0093A95C;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009a64e9 = (int *)0x009A64E9;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b8aa8 = (int *)0x009B8AA8;

// Signature change: the scaffold's `int8*` params respell the catalogue's
// `PBD` (pointer to const char) - see the brief's Contract head.
extern "C" int __cdecl my_abs(int);
extern "C" char *__cdecl my_strcat(char *, const char *);

void __cdecl POP3(const char * a1, const char * a2, int a3, int a4, int a5) {
    // BasePop and Scroll are opaque callee types here (methods only, no
    // agreed size), so these local (stack, non-pointer) instances can't get
    // the original's real frame slots - see the WALL note filed for this
    // address (same class of issue as 0x004589C0/0x00497AC0, but for
    // BasePop/Scroll instead of PullDown).
    BasePop bp;
    Scroll scroll1;
    char *self = reinterpret_cast<char *>(&bp);

    reinterpret_cast<Popup *>(&bp)->start(
        reinterpret_cast<int8 *>(g_009b8aa8), reinterpret_cast<int8 *>(const_cast<char *>(a1)), -1, 0, 0, 0);

    int history_count = *reinterpret_cast<int *>(self + 0x2190);
    *reinterpret_cast<unsigned char *>(g_009b86a0) = 0;

    int *history_node = 0;
    int *history_node_orig = 0;
    if (history_count - 1 >= -1) {
        history_node = *reinterpret_cast<int **>(self + 0x2188);
        history_node_orig = history_node;
        if (my_abs(-1) <= history_count) {
            int steps = my_abs(-1);
            if (steps > 0) {
                do {
                    history_node = reinterpret_cast<int *>(history_node[4]);
                    --steps;
                } while (steps != 0);
            }
        }
    }

    for (int i = 0; i < history_count; ++i) {
        if (i != 0) {
            my_strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(g_00682820));
        }
        reinterpret_cast<StringStruct *>(self + 0x2180)->next_entry();
        char *text;
        if (history_node == 0) {
            text = 0;
        } else if (history_node_orig == 0) {
            text = *reinterpret_cast<char **>(4);
        } else {
            text = *reinterpret_cast<char **>(*reinterpret_cast<char **>(history_node + 2) + 4);
        }
        my_strcat(reinterpret_cast<char *>(g_009b86a0), text);
    }

    if (*g_0093a95c == 0) {
        reinterpret_cast<MainInterface *>(g_007ae820)->set_message(
            reinterpret_cast<int8 *>(g_009b86a0), 0x5f, a4, 1, 1);
        scroll1.close();
        bp.close();
        FlatButton fb1;
        FlatButton fb2;
    } else {
        if (a3 < 1) {
            unsigned char bit = 1;
            char *record = reinterpret_cast<char *>(g_0093a800);
            int limit = *g_00949870;
            do {
                unsigned char mask = static_cast<unsigned char>(1 << (bit & 0x1f));
                if ((*reinterpret_cast<unsigned char *>(g_009a64e9) & mask) != 0 &&
                    (*reinterpret_cast<unsigned char *>(g_009a64e8) & mask) != 0 &&
                    a5 >= 0 && a5 < *g_00949874 && a4 >= 0 && a4 < limit) {
                    *reinterpret_cast<char **>(record - 0x1c) = reinterpret_cast<char *>(g_009b86a0);
                    *reinterpret_cast<int *>(record) = 0;
                    *reinterpret_cast<int *>(record + 4) = 0;
                    reinterpret_cast<StringStruct *>(record - 0x20)->add(
                        (limit * a4 + a5) | 0x80000000);
                    limit = *g_00949870;
                }
                record += 0x30;
                ++bit;
            } while (record < reinterpret_cast<char *>(g_0093a950));
        } else if (a5 >= 0 && a5 < *g_00949874 && a4 >= 0 && a4 < *g_00949870) {
            char *record = reinterpret_cast<char *>(g_0093a7b0) + a3 * 0x30;
            *reinterpret_cast<char **>(record) = reinterpret_cast<char *>(g_009b86a0);
            *reinterpret_cast<int *>(record + 4) = 0;
            *reinterpret_cast<int *>(record + 8) = 0;
            reinterpret_cast<StringStruct *>(reinterpret_cast<char *>(g_0093a7b0) + a3 * 0x30 - 4)->add(
                (*g_00949870 * a4 + a5) | 0x80000000);
        }
        scroll1.close();
        bp.close();
        FlatButton fb1;
        FlatButton fb2;
    }
}
