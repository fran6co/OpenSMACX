// ORIGINAL: 0x00606440 FILE
// name      ?on_mousewheel_up@Scroll@@QAEXH@Z
// size      284 bytes
// spans     0x00606440-0x0060655C
// prototype void (__thiscall ?on_mousewheel_up@Scroll@@QAEXH@Z)(Scroll* this, int)
// callers   4   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x006064BC 0x006064EE 0x0060651C 0x00606535 0x00606540 0x00606551
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00606440
// measured tier  MISMATCH
// divergence     7
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00606440/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_mousewheel_up@Scroll@@QAEXH@Z  at 0x00606440  (284 bytes)
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

class Scroll;

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 7, 23, 48, 62
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();  // <-- used
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
    virtual void slot023();  // <-- used
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
    virtual void slot048();  // <-- used
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
static int *const g_009b7ab8 = (int *)0x009B7AB8;

class Scroll { public:
    void on_mousewheel_up(int);
};

typedef void (__cdecl *ScrollCallback)(int, int);

class VCallSlot48 { public:
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
    virtual void slot048(int, int);
};

class VCallSlot23 { public:
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
    virtual int slot023();
};

void Scroll::on_mousewheel_up(int a1) {
    char *self = reinterpret_cast<char *>(this);

    VCall *owner = *reinterpret_cast<VCall **>(self + 0xC4);
    if (!owner) {
        return;
    }
    *g_009b7ab8 = reinterpret_cast<int>(owner);

    if (*reinterpret_cast<int *>(self + 0xA2C) - 1 < *reinterpret_cast<int *>(self + 0xA20)) {
        *reinterpret_cast<int *>(self + 0xA2C) = *reinterpret_cast<int *>(self + 0xA20);
    } else {
        *reinterpret_cast<int *>(self + 0xA2C) -= 1;
    }

    if (*reinterpret_cast<int *>(self + 0xA28) != 0) {
        *reinterpret_cast<int *>(self + 0xA2C) =
            (*reinterpret_cast<int *>(self + 0xA24) -
             *reinterpret_cast<int *>(self + 0xA2C)) +
            *reinterpret_cast<int *>(self + 0xA20);
    }

    if (a1 != 0) {
        if (*reinterpret_cast<int *>(self + 0xA28) == 0) {
            reinterpret_cast<VCallSlot48 *>(*reinterpret_cast<VCall **>(self + 0xC4))
                ->slot048(*reinterpret_cast<int *>(self + 0xA18),
                          *reinterpret_cast<int *>(self + 0xA2C));

            ScrollCallback cb = *reinterpret_cast<ScrollCallback *>(self + 0x2144);
            if (cb) {
                cb(*reinterpret_cast<int *>(self + 0xA18),
                   *reinterpret_cast<int *>(self + 0xA2C));
            }
        } else {
            reinterpret_cast<VCallSlot48 *>(*reinterpret_cast<VCall **>(self + 0xC4))
                ->slot048(*reinterpret_cast<int *>(self + 0xA18),
                          (*reinterpret_cast<int *>(self + 0xA20) -
                           *reinterpret_cast<int *>(self + 0xA2C)) +
                              *reinterpret_cast<int *>(self + 0xA24));

            ScrollCallback cb = *reinterpret_cast<ScrollCallback *>(self + 0x2144);
            if (cb) {
                cb(*reinterpret_cast<int *>(self + 0xA18),
                   (*reinterpret_cast<int *>(self + 0xA24) -
                    *reinterpret_cast<int *>(self + 0xA2C)) +
                       *reinterpret_cast<int *>(self + 0xA20));
            }
        }
    }

    VCall *sub = *reinterpret_cast<VCall **>(
        *reinterpret_cast<char **>(self + 0xC4) + 0x80);
    if (sub) {
        if (reinterpret_cast<VCallSlot23 *>(sub)->slot023() == 0) {
            reinterpret_cast<VCall *>(sub)->slot007();
        }
    }

    *reinterpret_cast<int *>(self + 0xA3C) = -1;
    reinterpret_cast<VCall *>(this)->slot062();
}
