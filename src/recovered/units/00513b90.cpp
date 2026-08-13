// ORIGINAL: 0x00513B90 FILE
// name      ?veh_turn@Console@@QAEHXZ
// size      1065 bytes
// spans     0x00513B90-0x00513FB9
// prototype int (__thiscall ?veh_turn@Console@@QAEHXZ)(Console* this)
// callers   1   call targets   21
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00446B40 0x00458900 0x00460DD0 0x0046A550 0x0046AE00 0x0046FB10 0x004B9EA0 0x004BC6E0 0x00511BB0 0x0051F200 0x00524870 0x005310F0 0x00531150 0x00531480 0x00532570 0x00579240 0x00593220 0x005C1A20 0x005C1B70 0x005C1D20 0x005F7E90
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00513B90
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00513b90/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?veh_turn@Console@@QAEHXZ  at 0x00513B90  (1065 bytes)
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

typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;

struct vehID;

// ---- callees, declared and never defined (a definition would be inlined) ----
class InfoWin { public:
    void change(int);
};
class MainMenu { public:
    void check(int vehID);
};
class MapWin { public:
    void draw_map(int);
    void main_caption();
};
class NetDaemon { public:
    int action(int vehID, int);
    int automated_lock(int*, int);
    void await_synch();
    void unlock_veh();
};
class StatusWin { public:
    void redraw();
};
class TutWin { public:
    void tour();
};
class Win { public:
    int is_visible();
};
int __cdecl ambience(int);
int __cdecl end_of_game(int);
int __cdecl enemy_veh(int vehID);
int __cdecl veh_ready(int vehID);
int __cdecl veh_selectable(int vehID);
void __cdecl draw_cursor();
void __cdecl synch_veh(int vehID);
void __cdecl veh_skip(int vehID);
void __cdecl wait_loop();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006a7628 = (int *)0x006A7628;
static int *const g_007ad2a0 = (int *)0x007AD2A0;
static int *const g_007d3c3c = (int *)0x007D3C3C;
static int *const g_00876478 = (int *)0x00876478;
static int *const g_008c5568 = (int *)0x008C5568;
static int *const g_008c6e68 = (int *)0x008C6E68;
static int *const g_008cc248 = (int *)0x008CC248;
static int *const g_00939288 = (int *)0x00939288;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093e8e0 = (int *)0x0093E8E0;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_0093f664 = (int *)0x0093F664;
static int *const g_0095282c = (int *)0x0095282C;
static int *const g_00952839 = (int *)0x00952839;
static int *const g_009a6490 = (int *)0x009A6490;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009b2068 = (int *)0x009B2068;

class Console { public:
    void give_hints();
    int veh_turn();
};

int Console::veh_turn() {
    int result;
    int flag10;
    int n;
    int firstReady;
    char *vb;
    int tflags;
    unsigned char torder;
    int zero;

    zero = 0;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23be4) = 1;
    result = zero;
    flag10 = zero;
    n = zero;

    if (!(*reinterpret_cast<unsigned char *>(g_009a6490) & 0x20)) goto L513BDA;
    if (*reinterpret_cast<unsigned char *>(g_00952839 + *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bdc) * 13) != 0) goto L513BDA;
    this->give_hints();

L513BDA:
    firstReady = veh_ready(*reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bdc));
    goto L513BF9;

L513BF4:
L513BF9:
    if (!veh_selectable(*reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bdc))) goto L513C31;
    if (veh_ready(*reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bdc)) != firstReady) goto L513C31;
    if (firstReady != zero) goto L513C3D;
    if (*reinterpret_cast<unsigned char *>(g_00952839 + *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bdc) * 13) == 0) goto L513C3D;

L513C31:
    if (*g_00939288 != zero) goto L513F7C;

L513C3D:
    if (*reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23be4) != zero) goto L513C52;
    if (*reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bf0) != 2) goto L513F7C;

L513C52:
    if (*reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23be8) == zero) goto L513F7C;
    if (*g_009b2068 != zero) goto L513F7C;
    if (end_of_game(zero) != 0) goto L513F7C;
    if (*g_0093f660 != 0) {
        if (*g_009a64c0 & 0x800) goto L513F7C;
    }

    flag10 = 1;
    if (*reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bf0) == 2) {
        *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bf0) = zero;
        *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23be4) =
            (*reinterpret_cast<unsigned char *>(g_00952839 + *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bdc) * 13) == 0x18);
    }

    if (*g_0093e8e0 == zero) goto L513D5B;
    if (*g_00939288 == zero) goto L513D4E;
    ambience(0x1e);
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bd8) = zero;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bf8) = 1;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bfc) = 1;
    if (*reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23c00) != zero) {
        *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23c00) = zero;
        (*reinterpret_cast<MapWin **>(g_007d3c3c))->draw_map(1);
    }

    draw_cursor();
    reinterpret_cast<InfoWin *>(g_007ad2a0)->change(zero);
    reinterpret_cast<StatusWin *>(g_008c5568)->redraw();
    (*reinterpret_cast<MapWin **>(g_007d3c3c))->main_caption();
    reinterpret_cast<MainMenu *>(reinterpret_cast<char *>(this) + 0x22a2c)->check(*reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bdc));
    goto L513BF9;

L513D4E:
    result = 1;
    wait_loop();
    goto L513BF9;

L513D5B:
    if (*reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23d24) == 0) goto L513D9C;
    vb = reinterpret_cast<char *>(g_0095282c) + *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bdc) * 52;
    tflags = *reinterpret_cast<int *>(vb);
    if (tflags & 0x200) goto L513D9C;
    if ((tflags & 0x1000200) == 0x1000200) goto L513D9C;
    if (tflags & 0x4000) goto L513D9C;
    if (*reinterpret_cast<unsigned char *>(vb + 13) == 0) goto L513F70;

L513D9C:
    if (*g_00939288 == zero) goto L513F43;
    if (veh_ready(*reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bdc)) == 0) goto L513F23;
    if (reinterpret_cast<Win *>(g_006a7628)->is_visible() != 0) goto L513F43;
    if (reinterpret_cast<Win *>(g_00876478)->is_visible() != 0) goto L513F43;
    vb = reinterpret_cast<char *>(g_0095282c) + *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bdc) * 52;
    tflags = *reinterpret_cast<int *>(vb);
    if (tflags & 0x4200) goto L513E9B;
    torder = *reinterpret_cast<unsigned char *>(vb + 13);
    if (torder == 0) {
        n = zero;
        goto L513E6F;
    }
    n = n + 1;
    if (n <= (torder != 0x18 ? 20 : 100)) goto L513E6A;
    if (torder == 0x18) {
        *reinterpret_cast<unsigned char *>(vb + 13) = 0;
    } else {
        veh_skip(*reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bdc));
    }

    if (*g_0093f660 != 0) {
        synch_veh(*reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bdc));
        reinterpret_cast<NetDaemon *>(g_0093cd90)->await_synch();
        reinterpret_cast<NetDaemon *>(g_0093cd90)->unlock_veh();
    }
    goto L513BF4;

L513E6A:
L513E6F:
    if (reinterpret_cast<NetDaemon *>(g_0093cd90)->action(*reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bdc), zero) != 0) goto L513BF9;
    g_0095282c[*reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bdc) * 13] |= 0x4000000;
    goto L513F43;

L513E9B:
    if (reinterpret_cast<NetDaemon *>(g_0093cd90)->automated_lock(reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bdc), 4) != 0) goto L513F7C;
    (*g_0093f664)++;
    n = n + 1;
    if (n <= 20) goto L513F05;
    veh_skip(*reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bdc));
    if (*g_0093f660 == zero) goto L513EFA;
    synch_veh(*reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bdc));
    reinterpret_cast<NetDaemon *>(g_0093cd90)->await_synch();
    reinterpret_cast<NetDaemon *>(g_0093cd90)->unlock_veh();

L513EFA:
    (*g_0093f664)--;
    goto L513BF9;

L513F05:
    if (enemy_veh(*reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bdc)) != 0) goto L513F99;
    (*g_0093f664)--;
    goto L513BF9;

L513F23:
    vb = reinterpret_cast<char *>(g_0095282c) + *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bdc) * 52;
    if (*reinterpret_cast<unsigned char *>(vb + 13) != 0) goto L513F76;
    tflags = *reinterpret_cast<int *>(vb);
    if (tflags & 0x4200) goto L513F76;

L513F43:
    result = 1;
    wait_loop();
    if (*g_008cc248 != zero) {
        *g_008cc248 = zero;
        reinterpret_cast<TutWin *>(g_008c6e68)->tour();
    }
    goto L513BF9;

L513F70:
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23d24) = zero;

L513F76:
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bdc) = -1;

L513F7C:
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23be4) = zero;
    if (flag10 != zero) {
        *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bdc) = -1;
    }
    return result;

L513F99:
    if (*g_0093f660 != zero) {
        reinterpret_cast<NetDaemon *>(g_0093cd90)->unlock_veh();
        *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bdc) = -1;
    }
    (*g_0093f664)--;
    goto L513F7C;
}
