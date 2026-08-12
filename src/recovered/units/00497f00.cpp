// ORIGINAL: 0x00497F00 FILE
// name      ?on_right_click@ReportWin@@QAEXHH@Z
// size      495 bytes
// spans     0x00497F00-0x004980EF
// prototype void (__thiscall ?on_right_click@ReportWin@@QAEXHH@Z)(ReportWin* this, int xCoord, int yCoord)
// callers   0   call targets   9
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0040B140 0x0041AE40 0x00497AC0 0x004A0810 0x004EC3B0 0x00530D50 0x00530E50 0x005FAB00 0x006262F0
// indirect  0x00497FB4 0x004980E0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00497F00
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00497f00/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_right_click@ReportWin@@QAEXHH@Z  at 0x00497F00  (495 bytes)
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
class ReportWin;
class Spot;

// ---- callees, declared and never defined (a definition would be inlined) ----
class BaseWin { public:
    void garrison_click(int, int, int, int);
    void soft_zoom(int);
};
class NetDaemon { public:
    int lock_base(int, int, int, int);
    void unlock_base(int);
};
class Spot { public:
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
    int check(int, int, int *, int *);
};
int __cdecl base_compute(int);
void __cdecl log_say(char *, int, int, int);
void __cdecl sat_edit(int, int, int);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 62
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();
    virtual void slot012();
    virtual void slot013();
    virtual void slot014();
    virtual void slot015();
    virtual void slot016();
    virtual void slot017();
    virtual void slot018();
    virtual void slot019();
    virtual void slot020();
    virtual void slot021();
    virtual void slot022();
    virtual void slot023();
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();
    virtual void slot033();
    virtual void slot034();
    virtual void slot035();
    virtual void slot036();
    virtual void slot037();
    virtual void slot038();
    virtual void slot039();
    virtual void slot040();
    virtual void slot041();
    virtual void slot042();
    virtual void slot043();
    virtual void slot044();
    virtual void slot045();
    virtual void slot046();
    virtual void slot047();
    virtual void slot048();
    virtual void slot049();
    virtual void slot050();
    virtual void slot051();
    virtual void slot052();
    virtual void slot053();
    virtual void slot054();
    virtual void slot055();
    virtual void slot056();
    virtual void slot057();
    virtual void slot058();
    virtual void slot059();
    virtual void slot060();
    virtual void slot061();
    virtual void slot062();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00686f1c = (int *)0x00686F1C;
static int *const g_00686f3c = (int *)0x00686F3C;
static int *const g_00689370 = (int *)0x00689370;
static int *const g_006a7628 = (int *)0x006A7628;
static int *const g_00885f40 = (int *)0x00885F40;
static int *const g_008a4160 = (int *)0x008A4160;
static int *const g_0090ea30 = (int *)0x0090EA30;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093a938 = (int *)0x0093A938;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093e8c8 = (int *)0x0093E8C8;
static int *const g_0093e8ec = (int *)0x0093E8EC;
static int *const g_0093e944 = (int *)0x0093E944;
static int *const g_0097d044 = (int *)0x0097D044;
static int *const g_009a64c0 = (int *)0x009A64C0;

class ReportWin { public:
    void right_menu(int, int, int);
    void on_right_click(int, int);
};

extern unsigned char g_0097d044_tbl[];

void ReportWin::on_right_click(int a1, int a2) {
    if (*g_0093a938 == 0
        && (*g_0093e8ec & (1 << (*g_00939284 & 0x1f))) == 0
        && *g_0093e944 == 0
        && *g_0093e8c8 == 0) {
        int local4;
        Spot *spot = reinterpret_cast<Spot *>(reinterpret_cast<char *>(this) + 0x5994);
        int checkResult = spot->check(a1, a2, &a2, &local4);
        if (checkResult >= 0) {
            if (*g_00885f40 == 4) {
                if (a2 > 0x3ff) {
                    int idx1 = a2 - 0x400;
                    if (idx1 < 0x400) {
                        if (g_0097d044_tbl[idx1 * 0x134] != *g_00939284) return;
                        right_menu(idx1, a1, a2);
                        return;
                    }
                    int idx2 = a2 - 0x800;
                    if (g_0097d044_tbl[idx2 * 0x134] != *g_00939284) return;
                    log_say(reinterpret_cast<char *>(g_00686f1c), local4, idx2, 0);
                    reinterpret_cast<BaseWin *>(g_006a7628)->soft_zoom(idx2);
                    reinterpret_cast<BaseWin *>(g_006a7628)->garrison_click(local4, a1, a2, 1);
                    return;
                }
                log_say(reinterpret_cast<char *>(g_00686f3c), 0, 0, 0);
                reinterpret_cast<BaseWin *>(g_006a7628)->soft_zoom(a2);
                if (reinterpret_cast<NetDaemon *>(g_0093cd90)->lock_base(*g_00689370, 0, -1, -1) != 0) {
                    return;
                }
                {
                    int *field = reinterpret_cast<int *>(reinterpret_cast<char *>(*g_0090ea30) + 0x7c);
                    if (*field != 0) {
                        *field = *field - 1;
                    }
                }
                base_compute(1);
                reinterpret_cast<NetDaemon *>(g_0093cd90)->unlock_base(*g_00689370);
            } else if (*g_00885f40 == 6
                       && (*reinterpret_cast<unsigned char *>(g_009a64c0) & 0x40) != 0
                       && (*reinterpret_cast<unsigned char *>(g_009a64c0) & 0x80) != 0) {
                sat_edit(*g_008a4160, local4, a2);
                reinterpret_cast<VCall *>(this)->slot062();
                return;
            }
            reinterpret_cast<VCall *>(this)->slot062();
        }
    }
}
