// ORIGINAL: 0x00433820 FILE
// name      ?on_mouse_move@SelectPartWin@@QAEXHH@Z
// size      276 bytes
// spans     0x00433820-0x00433934
// prototype void (__thiscall ?on_mouse_move@SelectPartWin@@QAEXHH@Z)(SelectPartWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00433D60
// indirect  0x004338F8
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00433820
// measured tier  MISMATCH
// divergence     1
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00433820/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_mouse_move@SelectPartWin@@QAEXHH@Z  at 0x00433820  (276 bytes)
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

class SelectPartWin;

// ---- callees, declared and never defined (a definition would be inlined) ----

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 63
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
    virtual void slot062();
    virtual void slot063();  // <-- used
};

class SelectPartWin { public:
    int draw_part(int, int);
    void on_mouse_move(int, int);
};

void SelectPartWin::on_mouse_move(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    int idx;
    int col_count;
    if (a1 < 4 || a2 < 4 ||
        a1 >= (col_count = *reinterpret_cast<int *>(self + 0x2b6c)) * 108 + 4 ||
        a2 >= *reinterpret_cast<int *>(self + 0x2b70) * 94 + 4) {
        idx = *reinterpret_cast<int *>(self + 0x2b80);
    } else {
        idx = *reinterpret_cast<int *>(self + 0x2b80);
        int base = *reinterpret_cast<int *>(self + 0x2b60);
        int new_idx = (a2 - 4) / 94 * col_count + (a1 - 4) / 108 + base;
        if (new_idx == idx) {
            return;
        }
        if (new_idx < *reinterpret_cast<int *>(self + 0x2b74)) {
            int result = draw_part(new_idx, 1);
            *reinterpret_cast<int *>(self + 0x4648) = result;
            int old = *reinterpret_cast<int *>(self + 0x2b80);
            if (old >= 0) {
                draw_part(old, 0);
            }
            *reinterpret_cast<int *>(self + 0x2b80) = new_idx;
            reinterpret_cast<VCall *>(this)->slot063();
            return;
        }
    }
    if (idx >= 0) {
        draw_part(idx, 0);
    }
    *reinterpret_cast<int *>(self + 0x4648) = 0;
    *reinterpret_cast<int *>(self + 0x2b80) = -1;
}
