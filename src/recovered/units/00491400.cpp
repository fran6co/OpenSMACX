// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00491400
// name           ?on_button_clicked@PrefWin@@QAEXH@Z
// size           1088 bytes
// measured tier  NO_COMPILE
// refusal        u00491400.cpp(61) : error C2061: syntax error : identifier 'Palette' u00491400.cpp(89) : error C2065: 'int8' : undeclared identifier u00491400.cpp(89) : error C
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00491400/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_button_clicked@PrefWin@@QAEXH@Z  at 0x00491400  (1088 bytes)
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

struct position;

// ---- callees, declared and never defined (a definition would be inlined) ----
class BaseButton { public:
    ~BaseButton();
};
class EditBox { public:
    void close();
};
class FlatButton { public:
    void close();
};
class Font { public:
    ~Font();
};
class Gamma { public:
    Gamma();
    void close();
    void exec(Palette*, float, int);
};
class GraphicWin { public:
    ~GraphicWin();
};
class Palette { public:
    int __as(Palette*);
    ~Palette();
};
class PushButton { public:
    ~PushButton();
};
class Scroll { public:
    void close();
    void set_pos(int position);
};
class Time { public:
    ~Time();
};
class Wave_Device { public:
    int set_group_volume(unsigned int, unsigned int);
    void set_volume(unsigned int);
};
int __cdecl __alloca_probe();
int __cdecl __ftol();
int __cdecl default_prefs();
int __cdecl default_prefs2();
int __cdecl default_warn();
void __cdecl prefs_put(int8* lpKeyName, int, int);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 16, 62
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
    virtual void slot016();  // <-- used
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
static int *const g_00491794 = (int *)0x00491794;
static int *const g_00658b62 = (int *)0x00658B62;
static int *const g_0066974c = (int *)0x0066974C;
static int *const g_00669754 = (int *)0x00669754;
static int *const g_00669d50 = (int *)0x00669D50;
static int *const g_00669d58 = (int *)0x00669D58;
static int *const g_0066a030 = (int *)0x0066A030;
static int *const g_0066a038 = (int *)0x0066A038;
static int *const g_0066c3d0 = (int *)0x0066C3D0;
static int *const g_0066c3d8 = (int *)0x0066C3D8;
static int *const g_0066d58c = (int *)0x0066D58C;
static int *const g_006781b8 = (int *)0x006781B8;
static int *const g_006846a8 = (int *)0x006846A8;
static int *const g_00686d70 = (int *)0x00686D70;
static int *const g_0074daa0 = (int *)0x0074DAA0;
static int *const g_0090d978 = (int *)0x0090D978;
static int *const g_0094c590 = (int *)0x0094C590;
static int *const g_009523a0 = (int *)0x009523A0;

class PrefWin { public:
    void destroy();
    void get_preferences();
    void change_volume_button_text(int);
    void change_volume_button_state(int);
    void on_button_clicked(int);
};

// Second vtable shim: slot 16 is called through a DIFFERENT receiver
// (*g_0074daa0, not `this`) and takes one int argument, unlike the
// nullary VCall.slot016 the emitter declared for the `this`-relative
// call at slot 62.
class VCall2 { public:
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
    virtual void slot016(int);  // <-- used, takes the pushed arg
};

void PrefWin::on_button_clicked(int a1) {
    char *self = reinterpret_cast<char *>(this);
    Gamma gamma;

    switch (a1) {
    case 6: {
        reinterpret_cast<Palette *>(g_0094c590)->__as(reinterpret_cast<Palette *>(g_009523a0));
        gamma.exec(reinterpret_cast<Palette *>(g_0094c590), *reinterpret_cast<float *>(g_006846a8), 0);
        int ticks = static_cast<int>(*reinterpret_cast<float *>(g_006846a8) * *reinterpret_cast<float *>(g_0066d58c));
        prefs_put(reinterpret_cast<int8 *>(g_00686d70), ticks, 0);
        break;
    }
    case 0x5a: {
        *reinterpret_cast<int *>(self + 0xa30) = default_prefs();
        *reinterpret_cast<int *>(self + 0xa34) = default_prefs2();
        *reinterpret_cast<int *>(self + 0xa38) = default_prefs();
        *reinterpret_cast<int *>(self + 0xa3c) = default_prefs2();
        *reinterpret_cast<int *>(self + 0xa40) = default_prefs();
        *reinterpret_cast<int *>(self + 0xa44) = default_prefs2();
        *reinterpret_cast<int *>(self + 0xa4c) = default_prefs();
        *reinterpret_cast<int *>(self + 0xa50) = default_prefs2();
        *reinterpret_cast<int *>(self + 0xa48) = default_warn();
        *reinterpret_cast<int *>(self + 0xa54) = default_prefs();
        *reinterpret_cast<int *>(self + 0xa58) = default_prefs2();
        *reinterpret_cast<int *>(self + 0x15820) = 1;
        *reinterpret_cast<int *>(self + 0x16eb8) = 1;
        *reinterpret_cast<int *>(self + 0x1636c) = 1;
        *reinterpret_cast<int *>(self + 0x17a04) = 1;
        reinterpret_cast<Scroll *>(self + 0xa78c)->set_pos(0x7f);
        reinterpret_cast<Wave_Device *>(g_0090d978)->set_volume(0x7f);
        reinterpret_cast<Scroll *>(self + 0xc8d8)->set_pos(0x7f);
        reinterpret_cast<Wave_Device *>(g_0090d978)->set_group_volume(0, 0x7f);
        reinterpret_cast<Scroll *>(self + 0xea24)->set_pos(0x5f);
        reinterpret_cast<Wave_Device *>(g_0090d978)->set_group_volume(1, 0x5f);
        reinterpret_cast<Scroll *>(self + 0x10b70)->set_pos(0x3f);
        if (*reinterpret_cast<int **>(g_0074daa0) != 0) {
            void *obj = *reinterpret_cast<void **>(g_0074daa0);
            reinterpret_cast<VCall2 *>(obj)->slot016(0x3f);
        }
        get_preferences();
        break;
    }
    case 0x5c:
        *reinterpret_cast<int *>(self + 0xa5c) = 1;
        // fall through
    case 0x5b:
        destroy();
        break;
    case 0x5e:
        if (*reinterpret_cast<int *>(self + 0xa14) == 2) {
            *reinterpret_cast<int *>(self + 0xa18) = 2;
            *reinterpret_cast<int *>(self + 0xa14) = 6;
        } else {
            *reinterpret_cast<int *>(self + 0xa18) = 6;
            *reinterpret_cast<int *>(self + 0xa14) = 2;
        }
        break;
    }

    change_volume_button_state(a1);
    change_volume_button_text(a1);
    reinterpret_cast<VCall *>(this)->slot062();
}
