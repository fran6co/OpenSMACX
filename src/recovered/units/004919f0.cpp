// ORIGINAL: 0x004919F0 ?change_volume_button_state@PrefWin@@QAEXH@Z 0x004919F0-0x00491BE9 FILE
// size      505 bytes
// prototype void (__thiscall ?change_volume_button_state@PrefWin@@QAEXH@Z)(PrefWin* this, int)
// callers   1   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004457E0 0x004918C0 0x004C51C0 0x004C51D0 0x004C53A0 0x004C5400
// indirect  0x00491ADD 0x00491B47
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004919F0
// measured tier  MISMATCH
// divergence     152
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004919f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?change_volume_button_state@PrefWin@@QAEXH@Z  at 0x004919F0  (505 bytes)
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

class PrefWin;
class Wave_Device;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Wave_Device { public:
    int disable_group(unsigned int);
    int enable_group(unsigned int);
    void disable();
    void enable();
};
void __cdecl load_music(int);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 2, 75
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();  // <-- used
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
    virtual void slot062();
    virtual void slot063();
    virtual void slot064();
    virtual void slot065();
    virtual void slot066();
    virtual void slot067();
    virtual void slot068();
    virtual void slot069();
    virtual void slot070();
    virtual void slot071();
    virtual void slot072();
    virtual void slot073();
    virtual void slot074();
    virtual void slot075();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00491bec = (int *)0x00491BEC;
static int *const g_0074daa0 = (int *)0x0074DAA0;
static int *const g_0090d978 = (int *)0x0090D978;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_009a6490 = (int *)0x009A6490;
static int *const g_009a6494 = (int *)0x009A6494;

class PrefWin { public:
    void change_volume_button_text(int);
    void change_volume_button_state(int);
};

// widened for the one-int-arg call in case 0x58's cleanup path
// (`push 1; call dword ptr [edx+8]`, slot 2 of the same primary vtable
// that slot075 below belongs to)
class VCallArg { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002(int);  // <-- used, widened
};

void PrefWin::change_volume_button_state(int a1) {
    char *self = reinterpret_cast<char *>(this);

    // Physical block order follows the ORIGINAL's layout after the jump
    // table (0x57, 0x56, 0x58, 0x55), not numeric case order - MSVC lays
    // switch arms out in source order, and matching that order was what
    // took this from misaligned-from-instruction-4 to matching in full.
    switch (a1) {
    case 0x57:
        if (*reinterpret_cast<int *>(self + 0x16eb8) != 0) {
            *reinterpret_cast<int *>(self + 0x16eb8) = 0;
            reinterpret_cast<Wave_Device *>(g_0090d978)->disable_group(0);
            return;
        }
        *reinterpret_cast<int *>(self + 0x16eb8) = 1;
        if (*reinterpret_cast<int *>(self + 0x15820) == 0) {
            *reinterpret_cast<int *>(self + 0x15820) = 1;
            change_volume_button_text(0x55);
        }
        reinterpret_cast<Wave_Device *>(g_0090d978)->enable_group(0);
        return;

    case 0x56:
        if (*reinterpret_cast<int *>(self + 0x1636c) != 0) {
            reinterpret_cast<Wave_Device *>(g_0090d978)->disable_group(1);
            *reinterpret_cast<int *>(self + 0x1636c) = 0;
            return;
        }
        *reinterpret_cast<int *>(self + 0x1636c) = 1;
        if (*reinterpret_cast<int *>(self + 0x15820) == 0) {
            *reinterpret_cast<int *>(self + 0x15820) = 1;
            change_volume_button_text(0x55);
        }
        reinterpret_cast<Wave_Device *>(g_0090d978)->enable_group(1);
        return;

    case 0x58: {
        if (*reinterpret_cast<int *>(self + 0x17a04) != 0) {
            void *obj = *reinterpret_cast<void **>(g_0074daa0);
            if (obj != 0) {
                reinterpret_cast<VCallArg *>(obj)->slot002(1);
                *reinterpret_cast<void **>(g_0074daa0) = 0;
            }
            *reinterpret_cast<int *>(self + 0x17a04) = 0;
            return;
        }
        *reinterpret_cast<int *>(self + 0x17a04) = 1;
        if (*reinterpret_cast<int *>(self + 0x15820) == 0) {
            *reinterpret_cast<int *>(self + 0x15820) = 1;
            change_volume_button_text(0x55);
        }
        int music = *g_00939284;
        *g_009a6490 |= 0x800;
        load_music(music);
        if (*reinterpret_cast<void **>(g_0074daa0) != 0) {
            reinterpret_cast<VCall *>(*reinterpret_cast<void **>(g_0074daa0))->slot075();
            return;
        }
        break;
    }

    case 0x55:
        if (*reinterpret_cast<int *>(self + 0x15820) != 0) {
            *reinterpret_cast<int *>(self + 0x15820) = 0;
            *reinterpret_cast<int *>(self + 0x17a04) = 0;
            *reinterpret_cast<int *>(self + 0x16eb8) = 0;
            *reinterpret_cast<int *>(self + 0x1636c) = 0;
            reinterpret_cast<Wave_Device *>(g_0090d978)->disable();
            int flags1 = *g_009a6490;
            int flags2 = *g_009a6494;
            flags1 &= ~0xC00;
            flags2 &= ~0x4000;
            *g_009a6490 = flags1;
            *g_009a6494 = flags2;
        } else {
            *reinterpret_cast<int *>(self + 0x15820) = 1;
            *reinterpret_cast<int *>(self + 0x17a04) = 1;
            *reinterpret_cast<int *>(self + 0x16eb8) = 1;
            *reinterpret_cast<int *>(self + 0x1636c) = 1;
            reinterpret_cast<Wave_Device *>(g_0090d978)->enable();
        }
        change_volume_button_text(0x58);
        change_volume_button_text(0x57);
        change_volume_button_text(0x56);
        break;
    }
}
