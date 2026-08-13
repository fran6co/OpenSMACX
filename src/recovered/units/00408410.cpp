// ORIGINAL: 0x00408410 FILE
// name      ?load_inverts@@YAXXZ
// size      91 bytes
// spans     0x00408410-0x0040846B
// prototype 
// callers   0   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FD530 0x005FD550 0x005FD570 0x005FD740 0x00645460
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00408410
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00408410/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?load_inverts@@YAXXZ  at 0x00408410  (91 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
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
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
typedef char int8;
typedef unsigned char uint8;

// ---- callees, declared and never defined (a definition would be inlined) ----
// Fixed to match a fresh brief: the live catalogue emits `strcpy`/`char*`
// here, not `_strcpy`/`int8*` - this scaffold predates that.
char *text_get();
extern "C" char *strcpy(char *, const char *);
int text_item_number();
int text_open(char *, char *);
void text_close();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00682824 = (int *)0x00682824;
static int *const g_00682830 = (int *)0x00682830;
static int *const g_006a7528 = (int *)0x006A7528;
static int *const g_006eee60 = (int *)0x006EEE60;
static int *const g_009b7d00 = (int *)0x009B7D00;

void __cdecl load_inverts() {
    if (text_open((char *)g_00682830, (char *)g_00682824) == 0) {
        text_get();
        int count = text_item_number();
        *g_006eee60 = count;
        if (count > 0) {
            char *dest = reinterpret_cast<char *>(g_006a7528);
            int i = 0;
            do {
                text_get();
                strcpy(dest, *reinterpret_cast<char **>(g_009b7d00));
                count = *g_006eee60;
                i++;
                dest += 0x20;
            } while (i < count);
        }
        text_close();
    }
}
