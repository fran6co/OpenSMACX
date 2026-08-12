// ORIGINAL: 0x0048FFB0 FILE
// name      ?hide_option_buttons@PrefWin@@QAEXXZ
// size      371 bytes
// spans     0x0048FFB0-0x00490123
// prototype void (__thiscall ?hide_option_buttons@PrefWin@@QAEXXZ)(PrefWin* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0048FFE0 0x0048FFFC 0x00490018 0x00490034 0x00490043 0x00490052 0x00490061 0x00490070 0x0049007F 0x0049008E 0x0049009D 0x004900AC 0x004900C8 0x004900D7 0x004900F3 0x00490102 0x0049011E
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0048FFB0
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0048ffb0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?hide_option_buttons@PrefWin@@QAEXXZ  at 0x0048FFB0  (371 bytes)
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

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 2
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00490124 = (int *)0x00490124;

class PrefWin { public:
    void hide_option_buttons();
};

void PrefWin::hide_option_buttons() {
    char *self = reinterpret_cast<char *>(this);
    int mode = *reinterpret_cast<int *>(self + 0xa18);
    switch (mode) {
    case 0: {
        int *desc = *reinterpret_cast<int **>(self + 0x5974);
        reinterpret_cast<VCall *>(self + 0x5974 + desc[1])->slot002();
        return;
    }
    case 1: {
        int *desc = *reinterpret_cast<int **>(self + 0x649c);
        reinterpret_cast<VCall *>(self + 0x649c + desc[1])->slot002();
        return;
    }
    case 3: {
        int *desc = *reinterpret_cast<int **>(self + 0x6fc4);
        reinterpret_cast<VCall *>(self + 0x6fc4 + desc[1])->slot002();
        return;
    }
    case 4: {
        int *desc = *reinterpret_cast<int **>(self + 0x7aec);
        reinterpret_cast<VCall *>(self + 0x7aec + desc[1])->slot002();
        reinterpret_cast<VCall *>(self + 0xa78c)->slot002();
        reinterpret_cast<VCall *>(self + 0xc8d8)->slot002();
        reinterpret_cast<VCall *>(self + 0xea24)->slot002();
        reinterpret_cast<VCall *>(self + 0x10b70)->slot002();
        reinterpret_cast<VCall *>(self + 0x14e08)->slot002();
        reinterpret_cast<VCall *>(self + 0x164a0)->slot002();
        reinterpret_cast<VCall *>(self + 0x15954)->slot002();
        reinterpret_cast<VCall *>(self + 0x16fec)->slot002();
        return;
    }
    case 2: {
        int *desc = *reinterpret_cast<int **>(self + 0x8614);
        reinterpret_cast<VCall *>(self + 0x8614 + desc[1])->slot002();
        reinterpret_cast<VCall *>(self + 0x1a868)->slot002();
        return;
    }
    case 6: {
        int *desc = *reinterpret_cast<int **>(self + 0x913c);
        VCall *primary = reinterpret_cast<VCall *>(self + 0x913c + desc[1]);
        primary->slot002();
        reinterpret_cast<VCall *>(self + 0x1a868)->slot002();
        return;
    }
    case 5: {
        int *desc = *reinterpret_cast<int **>(self + 0x9c64);
        reinterpret_cast<VCall *>(self + 0x9c64 + desc[1])->slot002();
        break;
    }
    }
}
