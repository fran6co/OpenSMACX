// ORIGINAL: 0x004DADA0 ?do_check@DipEdit@@QAEXXZ 0x004DADA0-0x004DB071 FILE BYTE_EXACT
// size      721 bytes
// prototype void (__thiscall ?do_check@DipEdit@@QAEXXZ)(DipEdit* this)
// callers   3   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0060EB80
// indirect  0x004DB068
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004DADA0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004dada0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?do_check@DipEdit@@QAEXXZ  at 0x004DADA0  (721 bytes)
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

class CheckBox;
class DipEdit;

// ---- callees, declared and never defined (a definition would be inlined) ----
class CheckBox { public:
    void set_state_id(int, int);
};

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
static int *const g_0096c9f8 = (int *)0x0096C9F8;
static int *const g_0096ca18 = (int *)0x0096CA18;

class DipEdit { public:
    void do_check();
};

// Indexed table bases: both are indexed by a computed register offset
// (a1c*0x20cc + a20*4), so the address itself does work - each needs
// `extern T name[]` under a name distinct from the context's own
// const-pointer global.
extern unsigned char g_table_96c9f8[];
extern unsigned char g_table_96ca18[];

void DipEdit::do_check() {
    char *self = reinterpret_cast<char *>(this);
    CheckBox *cb = reinterpret_cast<CheckBox *>(self + 0x15a0);

    int a1c = *reinterpret_cast<int *>(self + 0xa1c);
    int a20 = *reinterpret_cast<int *>(self + 0xa20);

    cb->set_state_id(8, *reinterpret_cast<unsigned int *>(
        g_table_96c9f8 + a1c * 0x20cc + a20 * 4) & 8);
    a1c = *reinterpret_cast<int *>(self + 0xa1c);
    a20 = *reinterpret_cast<int *>(self + 0xa20);
    cb->set_state_id(0x10, *reinterpret_cast<unsigned int *>(
        g_table_96c9f8 + a1c * 0x20cc + a20 * 4) & 0x10);
    a1c = *reinterpret_cast<int *>(self + 0xa1c);
    a20 = *reinterpret_cast<int *>(self + 0xa20);
    cb->set_state_id(4, *reinterpret_cast<unsigned int *>(
        g_table_96c9f8 + a1c * 0x20cc + a20 * 4) & 4);
    a1c = *reinterpret_cast<int *>(self + 0xa1c);
    a20 = *reinterpret_cast<int *>(self + 0xa20);
    cb->set_state_id(2, *reinterpret_cast<unsigned int *>(
        g_table_96c9f8 + a1c * 0x20cc + a20 * 4) & 2);
    a1c = *reinterpret_cast<int *>(self + 0xa1c);
    a20 = *reinterpret_cast<int *>(self + 0xa20);
    cb->set_state_id(1, *reinterpret_cast<unsigned int *>(
        g_table_96c9f8 + a1c * 0x20cc + a20 * 4) & 1);
    a1c = *reinterpret_cast<int *>(self + 0xa1c);
    a20 = *reinterpret_cast<int *>(self + 0xa20);
    cb->set_state_id(0x1000, *reinterpret_cast<unsigned int *>(
        g_table_96c9f8 + a1c * 0x20cc + a20 * 4) & 0x1000);
    a1c = *reinterpret_cast<int *>(self + 0xa1c);
    a20 = *reinterpret_cast<int *>(self + 0xa20);
    cb->set_state_id(0x2000, *reinterpret_cast<unsigned int *>(
        g_table_96c9f8 + a1c * 0x20cc + a20 * 4) & 0x2000);
    a1c = *reinterpret_cast<int *>(self + 0xa1c);
    a20 = *reinterpret_cast<int *>(self + 0xa20);
    cb->set_state_id(0x20, *reinterpret_cast<unsigned int *>(
        g_table_96c9f8 + a1c * 0x20cc + a20 * 4) & 0x20);
    a1c = *reinterpret_cast<int *>(self + 0xa1c);
    a20 = *reinterpret_cast<int *>(self + 0xa20);
    cb->set_state_id(0x2000000, *reinterpret_cast<unsigned int *>(
        g_table_96c9f8 + a1c * 0x20cc + a20 * 4) & 0x2000000);
    a1c = *reinterpret_cast<int *>(self + 0xa1c);
    a20 = *reinterpret_cast<int *>(self + 0xa20);
    cb->set_state_id(0x400, *reinterpret_cast<unsigned int *>(
        g_table_96c9f8 + a1c * 0x20cc + a20 * 4) & 0x400);
    a1c = *reinterpret_cast<int *>(self + 0xa1c);
    a20 = *reinterpret_cast<int *>(self + 0xa20);
    cb->set_state_id(0x40000, *reinterpret_cast<unsigned int *>(
        g_table_96c9f8 + a1c * 0x20cc + a20 * 4) & 0x40000);
    a1c = *reinterpret_cast<int *>(self + 0xa1c);
    a20 = *reinterpret_cast<int *>(self + 0xa20);
    cb->set_state_id(0x80000008, *reinterpret_cast<unsigned int *>(
        g_table_96ca18 + a1c * 0x20cc + a20 * 4) & 8);
    a1c = *reinterpret_cast<int *>(self + 0xa1c);
    a20 = *reinterpret_cast<int *>(self + 0xa20);
    cb->set_state_id(0x80002000, *reinterpret_cast<unsigned int *>(
        g_table_96ca18 + a1c * 0x20cc + a20 * 4) & 0x2000);

    int vbaseOff = *reinterpret_cast<int *>(*reinterpret_cast<int *>(self + 0x15a0) + 4);
    char *adjusted = self + 0x15a0 + vbaseOff;
    reinterpret_cast<VCall *>(adjusted)->slot062();
}
