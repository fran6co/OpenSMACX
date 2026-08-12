// ORIGINAL: 0x00633A50 FILE
// name      ?close@NetFifo@@QAEXXZ
// size      91 bytes
// spans     0x00633A50-0x00633AAB
// prototype void (__thiscall ?close@NetFifo@@QAEXXZ)(NetFifo* this)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
// indirect  0x00633A59 0x00633AA1
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00633A50
// measured tier  MISMATCH
// divergence     15
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00633a50/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?close@NetFifo@@QAEXXZ  at 0x00633A50  (91 bytes)
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
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
typedef signed char int8;
typedef unsigned char uint8;

// ---- callees, declared and never defined (a definition would be inlined) ----
// Respelled from the scaffold's `int __cdecl _free();`: VC6 mangles an
// extern "C" `free` to the `_free` symbol the catalogue expects, and the
// real CRT signature takes the pointer the body frees.
extern "C" void free(void *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669174 = (int *)0x00669174;
static int *const g_0066917c = (int *)0x0066917C;

class NetFifo { public:
    void close();
};

typedef void (__stdcall *CritSectionFn)(void *);

// Shadow struct: the fields this body needs, reached at the offsets the
// disassembly proves - 0xc is a payload pointer, 0x14 the intrusive
// singly-linked next pointer.
struct NetFifoNode {
    uint8_t pad_0_[0xc];
    void *data_;
    uint8_t pad_10_[0x14 - 0xc - 4];
    NetFifoNode *next_;
};

void NetFifo::close() {
    char *self = reinterpret_cast<char *>(this);
    (*reinterpret_cast<CritSectionFn *>(g_0066917c))(self + 0xc);
    while (*reinterpret_cast<NetFifoNode **>(self) != 0) {
        NetFifoNode *node = *reinterpret_cast<NetFifoNode **>(self);
        NetFifoNode *next = node->next_;
        void *data = node->data_;
        if (data != 0) {
            free(data);
        }
        (*reinterpret_cast<NetFifoNode **>(self))->data_ = 0;
        if (*reinterpret_cast<NetFifoNode **>(self) != 0) {
            free(*reinterpret_cast<NetFifoNode **>(self));
        }
        *reinterpret_cast<NetFifoNode **>(self) = next;
    }
    *reinterpret_cast<int *>(self + 4) = 0;
    *reinterpret_cast<int *>(self + 8) = 0;
    (*reinterpret_cast<CritSectionFn *>(g_00669174))(self + 0xc);
}
