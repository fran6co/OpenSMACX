// ORIGINAL: 0x004BB380 FILE
// name      ?tour5@TutWin@@QAEXXZ
// size      1041 bytes
// spans     0x004BB380-0x004BB791
// prototype void (__thiscall ?tour5@TutWin@@QAEXXZ)(TutWin* this)
// callers   1   call targets   12
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0045D380 0x004B2690 0x004B26B0 0x004B26D0 0x004B2740 0x004BA970 0x004BC5A0 0x004BC640 0x004BDFE0 0x005F7E90 0x00645470 0x0064FC88
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004BB380
// measured tier  NO_COMPILE
// refusal        u004bb380.cpp(157) : error C2065: 'strcat' : undeclared identifier u004bb380.cpp(161) : error C2065: '_itoa' : undeclared identifier
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004bb380/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?tour5@TutWin@@QAEXXZ  at 0x004BB380  (1041 bytes)
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

struct GraphicWin;
struct RECT;
struct Sprite;
struct factionID;
struct int8;

// ---- callees, declared and never defined (a definition would be inlined) ----
class SocialWin { public:
    void close();
    void hide();
    void init(int factionID);
    void show(int);
};
class SubInterface { public:
    void release_iface_mode();
};
class Win { public:
    int is_visible();
};
int __cdecl __itoa();
int __cdecl _strcat();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068766c = (int *)0x0068766C;
static int *const g_00687678 = (int *)0x00687678;
static int *const g_00687688 = (int *)0x00687688;
static int *const g_00687698 = (int *)0x00687698;
static int *const g_006876a8 = (int *)0x006876A8;
static int *const g_006876b0 = (int *)0x006876B0;
static int *const g_006876bc = (int *)0x006876BC;
static int *const g_006876cc = (int *)0x006876CC;
static int *const g_006876d8 = (int *)0x006876D8;
static int *const g_006876e4 = (int *)0x006876E4;
static int *const g_006876f4 = (int *)0x006876F4;
static int *const g_00687704 = (int *)0x00687704;
static int *const g_007d3c3c = (int *)0x007D3C3C;
static int *const g_008a6270 = (int *)0x008A6270;
static int *const g_008a6c84 = (int *)0x008A6C84;
static int *const g_008c52d0 = (int *)0x008C52D0;
static int *const g_008c52e0 = (int *)0x008C52E0;
static int *const g_008c5380 = (int *)0x008C5380;
static int *const g_008c5390 = (int *)0x008C5390;
static int *const g_008c54c0 = (int *)0x008C54C0;
static int *const g_008c54d0 = (int *)0x008C54D0;
static int *const g_008c54e0 = (int *)0x008C54E0;
static int *const g_008c54f0 = (int *)0x008C54F0;
static int *const g_008c5510 = (int *)0x008C5510;
static int *const g_008c5520 = (int *)0x008C5520;
static int *const g_008c5530 = (int *)0x008C5530;
static int *const g_008c5540 = (int *)0x008C5540;
static int *const g_008c5550 = (int *)0x008C5550;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_009b86a0 = (int *)0x009B86A0;

class TutWin { public:
    void do_soc(RECT*, int8*, int);
    void iface_rect(RECT*, int*, int*);
    void soc_rect(RECT*, int*, int*);
    int tut_win(GraphicWin*, int8*, int, int, Sprite*, int, int, int);
    void tour5();
};

// The double-indirect "active window" resolve below reads a value from
// this global and chases its vtable (vtbl = *ptr; adjusted = ptr +
// *(int*)(vtbl+4)); the address of the global itself is a plain load, so
// the context's folded pointer constant is fine here.
static GraphicWin *resolve_win() {
    int *ptr = *reinterpret_cast<int **>(g_007d3c3c);
    if (ptr == 0) {
        return 0;
    }
    int *vtbl = reinterpret_cast<int *>(*ptr);
    int offset = *reinterpret_cast<int *>(reinterpret_cast<char *>(vtbl) + 4);
    return reinterpret_cast<GraphicWin *>(reinterpret_cast<char *>(ptr) + offset);
}

void TutWin::tour5() {
    int wasVisible = reinterpret_cast<Win *>(g_008a6270)->is_visible();
    if (!wasVisible) {
        reinterpret_cast<SocialWin *>(g_008a6270)->init(-*g_00939284);
        reinterpret_cast<SocialWin *>(g_008a6270)->show(0);
    }

    int a3, a4;

    this->soc_rect(reinterpret_cast<RECT *>(g_008c5390), &a4, &a3);
    if (this->tut_win(resolve_win(), reinterpret_cast<int8 *>(g_0068766c), a4, a3,
                       reinterpret_cast<Sprite *>(0), 2, -1, -1) == 0) {
        goto fail;
    }

    this->soc_rect(reinterpret_cast<RECT *>(g_008c5510), &a3, &a4);
    if (this->tut_win(resolve_win(), reinterpret_cast<int8 *>(g_00687678), a3, a4,
                       reinterpret_cast<Sprite *>(0), 2, -1, -1) == 0) {
        goto fail;
    }

    this->iface_rect(reinterpret_cast<RECT *>(g_008c54c0), &a3, &a4);
    if (this->tut_win(resolve_win(), reinterpret_cast<int8 *>(g_00687688), a3, a4,
                       reinterpret_cast<Sprite *>(0), 2, -1, -1) == 0) {
        goto fail;
    }

    this->soc_rect(reinterpret_cast<RECT *>(g_008c52d0), &a3, &a4);
    if (this->tut_win(resolve_win(), reinterpret_cast<int8 *>(g_00687698), a3, a4,
                       reinterpret_cast<Sprite *>(0), 2, -1, -1) == 0) {
        goto fail;
    }

    int i;
    for (i = 0; i < 10; i++) {
        *reinterpret_cast<char *>(g_009b86a0) = 0;
        strcat(reinterpret_cast<char *>(g_009b86a0),
               reinterpret_cast<char *>(g_006876a8));
        char numbuf[80];
        typedef char *(__cdecl * ItoaFn)(int, char *, int);
        reinterpret_cast<ItoaFn>(&_itoa)(i, numbuf, 10);
        strcat(reinterpret_cast<char *>(g_009b86a0), numbuf);

        this->soc_rect(reinterpret_cast<RECT *>(
                           reinterpret_cast<char *>(g_008c52e0) + i * 0x10),
                       &a4, &a3);
        if (this->tut_win(resolve_win(), reinterpret_cast<int8 *>(g_009b86a0), a4,
                           a3, reinterpret_cast<Sprite *>(0), 2, -1, -1) == 0) {
            goto fail;
        }
    }

    this->iface_rect(reinterpret_cast<RECT *>(g_008c54d0), &a4, &a3);
    if (this->tut_win(resolve_win(), reinterpret_cast<int8 *>(g_006876b0), a4, a3,
                       reinterpret_cast<Sprite *>(0), 2, -1, -1) == 0) {
        goto fail;
    }

    this->iface_rect(reinterpret_cast<RECT *>(g_008c54f0), &a4, &a3);
    if (this->tut_win(resolve_win(), reinterpret_cast<int8 *>(g_006876bc), a4, a3,
                       reinterpret_cast<Sprite *>(0), 2, -1, -1) == 0) {
        goto fail;
    }

    this->iface_rect(reinterpret_cast<RECT *>(g_008c54e0), &a4, &a3);
    if (this->tut_win(resolve_win(), reinterpret_cast<int8 *>(g_006876cc), a4, a3,
                       reinterpret_cast<Sprite *>(0), 2, -1, -1) == 0) {
        goto fail;
    }

    this->iface_rect(reinterpret_cast<RECT *>(g_008c5550), &a4, &a3);
    if (this->tut_win(resolve_win(), reinterpret_cast<int8 *>(g_006876d8), a4, a3,
                       reinterpret_cast<Sprite *>(0), 2, -1, -1) == 0) {
        goto fail;
    }

    this->soc_rect(reinterpret_cast<RECT *>(g_008c5520), &a4, &a3);
    if (this->tut_win(resolve_win(), reinterpret_cast<int8 *>(g_006876e4), a4, a3,
                       reinterpret_cast<Sprite *>(0), 2, -1, -1) == 0) {
        goto fail;
    }

    this->soc_rect(reinterpret_cast<RECT *>(g_008c5530), &a4, &a3);
    if (this->tut_win(resolve_win(), reinterpret_cast<int8 *>(g_006876f4), a4, a3,
                       reinterpret_cast<Sprite *>(0), 2, -1, -1) == 0) {
        goto fail;
    }

    this->do_soc(reinterpret_cast<RECT *>(g_008c5540),
                 reinterpret_cast<int8 *>(g_00687704), 2);

fail:
    if (!wasVisible) {
        reinterpret_cast<SocialWin *>(g_008a6270)->hide();
        reinterpret_cast<SubInterface *>(g_008a6c84)->release_iface_mode();
        reinterpret_cast<SocialWin *>(g_008a6270)->close();
    }
}
