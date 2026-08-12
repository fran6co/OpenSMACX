// ORIGINAL: 0x006339E0 FILE
// name      ??1NetFifo@@QAE@XZ
// size      98 bytes
// spans     0x006339E0-0x00633A42
// prototype void (__thiscall ??1NetFifo@@QAE@XZ)(NetFifo* this)
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
// indirect  0x006339E9 0x00633A31 0x00633A38
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x006339E0
// measured tier  MISMATCH
// divergence     15
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/006339e0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??1NetFifo@@QAE@XZ  at 0x006339E0  (98 bytes)
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
extern "C" void free(void *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669170 = (int *)0x00669170;
static int *const g_00669174 = (int *)0x00669174;
static int *const g_0066917c = (int *)0x0066917C;

class NetFifo { public:
    ~NetFifo();
};

struct NetFifoNode {
    int pad0[3];
    void *data_;
    int pad1;
    NetFifoNode *next_;
};

typedef void (__stdcall *EnterCriticalSectionFn)(void *);
typedef void (__stdcall *LeaveCriticalSectionFn)(void *);
typedef void (__stdcall *DeleteCriticalSectionFn)(void *);

// NOT BYTE_EXACT. Closest reached: 96/98 bytes, mnemonic similarity
// 0.988, edit_count 1 (verify_recovered_function.py --json). The only
// divergence: the original reads the freed node's `next_` and `data_`
// fields through TWO registers (`mov eax,[esi]; mov ecx,eax; mov
// edi,[eax+0x14]; mov eax,[ecx+0xc]`) - it copies the node pointer to
// ecx before using it a second time even though eax is still live and
// unclobbered. Every source spelling tried here (inlining both field
// reads, naming an intermediate `node` local, reordering data-before-
// next and next-before-data) instead has the compiler reuse eax
// directly for the second read (`mov eax,[esi]; mov edi,[eax+0x14];
// mov eax,[eax+0xc]`), one mov shorter. Ruled out as a source-form
// question: this is REGISTER ALLOCATION - same field reads, same
// order, the backend's choice of a redundant copy versus reuse of a
// still-live register, not reachable from C++ call-site spelling.
// Everything else - the EnterCriticalSection/LeaveCriticalSection/
// DeleteCriticalSection calls, the free-list walk, the loop's
// rotated shape reusing `next` for the trailing `head_ != 0` check -
// matches exactly.
NetFifo::~NetFifo() {
    char *self = reinterpret_cast<char *>(this);
    NetFifoNode **head = reinterpret_cast<NetFifoNode **>(self);
    void *cs = self + 0xc;

    (reinterpret_cast<EnterCriticalSectionFn>(*g_0066917c))(cs);
    while (*head != 0) {
        NetFifoNode *node = *head;
        NetFifoNode *next = node->next_;
        if (node->data_ != 0) {
            free(node->data_);
        }
        (*head)->data_ = 0;
        if (*head != 0) {
            free(*head);
        }
        *head = next;
    }
    *reinterpret_cast<int *>(self + 4) = 0;
    *reinterpret_cast<int *>(self + 8) = 0;
    (reinterpret_cast<LeaveCriticalSectionFn>(*g_00669174))(cs);
    (reinterpret_cast<DeleteCriticalSectionFn>(*g_00669170))(cs);
}
