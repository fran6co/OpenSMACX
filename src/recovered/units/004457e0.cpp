// ORIGINAL: 0x004457E0 FILE
// name      ?load_music@@YAXH@Z
// size      1062 bytes
// spans     0x004457E0-0x00445BA4;0x00654CF0-0x00654D52
// prototype 
// callers   2   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004471F0 0x004C8460 0x00645470 0x0064557F 0x0064558A 0x0064FD20
// indirect  0x00445822 0x00445B83 0x00445B8E
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004457E0
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004457e0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?load_music@@YAXH@Z  at 0x004457E0  (1062 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
class Ambience { public:
    Ambience();
};
class FactionAmbience { public:
    FactionAmbience();
};
int __cdecl __strcmpi();
int __cdecl _strcat();
int __cdecl fn_0064557f();
int __cdecl fn_0064558a();

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 2, 4, 75
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();  // <-- used
    virtual void slot003();
    virtual void slot004();  // <-- used
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
static int *const g_00654d48 = (int *)0x00654D48;
static int *const g_0066b888 = (int *)0x0066B888;
static int *const g_0066b9f0 = (int *)0x0066B9F0;
static int *const g_0066bb58 = (int *)0x0066BB58;
static int *const g_0066bcc0 = (int *)0x0066BCC0;
static int *const g_0066be28 = (int *)0x0066BE28;
static int *const g_0066bf90 = (int *)0x0066BF90;
static int *const g_00675030 = (int *)0x00675030;
static int *const g_00683a30 = (int *)0x00683A30;
static int *const g_00683a48 = (int *)0x00683A48;
static int *const g_00683a60 = (int *)0x00683A60;
static int *const g_00683a78 = (int *)0x00683A78;
static int *const g_00683a90 = (int *)0x00683A90;
static int *const g_00683aa8 = (int *)0x00683AA8;
static int *const g_00683ac0 = (int *)0x00683AC0;
static int *const g_00683c54 = (int *)0x00683C54;
static int *const g_00683c58 = (int *)0x00683C58;
static int *const g_00683c60 = (int *)0x00683C60;
static int *const g_00683c68 = (int *)0x00683C68;
static int *const g_00683c70 = (int *)0x00683C70;
static int *const g_00683c78 = (int *)0x00683C78;
static int *const g_00683c84 = (int *)0x00683C84;
static int *const g_00683c90 = (int *)0x00683C90;
static int *const g_00683c98 = (int *)0x00683C98;
static int *const g_0074daa0 = (int *)0x0074DAA0;
static int *const g_00946a6c = (int *)0x00946A6C;
static int *const g_009a6490 = (int *)0x009A6490;
static int *const g_009b86a0 = (int *)0x009B86A0;

extern "C" char *strcat(char *, const char *);
extern "C" int stricmp(const char *, const char *);
extern char g_tbl_946a6c[];

struct ShellFA : public FactionAmbience { char pad[0x74]; };
struct ShellA70 : public Ambience { char pad[0x70]; };
struct ShellA74 : public Ambience { char pad[0x74]; };
struct ShellA78 : public Ambience { char pad[0x78]; };
struct ShellA80 : public Ambience { char pad[0x80]; };

class VCall2 { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002(int);
};
class VCall4 { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004(char *);
};

void __cdecl load_music(int a1) {
    char *msg = reinterpret_cast<char *>(g_009b86a0);
    *msg = 0;
    strcat(msg, reinterpret_cast<char *>(g_00683c54));

    if (*g_0074daa0 != 0) {
        reinterpret_cast<VCall2 *>(*g_0074daa0)->slot002(1);
        *g_0074daa0 = 0;
    }

    char *name = *reinterpret_cast<char **>(g_tbl_946a6c + a1 * 1436);
    void *obj;

    if (stricmp(name, reinterpret_cast<char *>(g_00683c58)) != 0) {
        if (stricmp(name, reinterpret_cast<char *>(g_00683c60)) != 0) {
            if (stricmp(name, reinterpret_cast<char *>(g_00683c68)) != 0) {
                if (stricmp(name, reinterpret_cast<char *>(g_00683c70)) != 0) {
                    if (stricmp(name, reinterpret_cast<char *>(g_00683c78)) != 0) {
                        if (stricmp(name, reinterpret_cast<char *>(g_00683c84)) != 0) {
                            if (stricmp(name, reinterpret_cast<char *>(g_00683c90)) != 0) {
                                strcat(msg, reinterpret_cast<char *>(g_00683c98));
                                ShellA70 *p = new ShellA70();
                                if (p != 0) {
                                    char *base = reinterpret_cast<char *>(p);
                                    *reinterpret_cast<int *>(base + 0x58) = 0;
                                    *reinterpret_cast<int *>(base + 0x5c) = 0;
                                    *reinterpret_cast<int *>(base + 0x60) = 0;
                                    *reinterpret_cast<int *>(base + 0x64) = 0;
                                    base[0x6c] = 0;
                                    base[0x6d] = 0;
                                    *reinterpret_cast<int *>(base + 0x68) = 0;

                                    *reinterpret_cast<int *>(base) = reinterpret_cast<int>(g_0066b888);
                                }
                                obj = p;
                            } else {
                                strcat(msg, reinterpret_cast<char *>(g_00683ac0));
                                ShellA74 *p = new ShellA74();
                                if (p != 0) {
                                    char *base = reinterpret_cast<char *>(p);
                                    *reinterpret_cast<int *>(base + 0x58) = 0;
                                    *reinterpret_cast<int *>(base + 0x5c) = 0;
                                    *reinterpret_cast<int *>(base + 0x60) = 0;
                                    *reinterpret_cast<int *>(base + 0x64) = 0;
                                    base[0x6c] = 0;
                                    base[0x6d] = 0;
                                    *reinterpret_cast<int *>(base + 0x68) = 0;

                                    *reinterpret_cast<int *>(base) = reinterpret_cast<int>(g_0066bf90);
                                    *reinterpret_cast<int *>(base + 0x70) = 0;
                                }
                                obj = p;
                            }
                        } else {
                            strcat(msg, reinterpret_cast<char *>(g_00683aa8));
                            ShellA80 *p = new ShellA80();
                            if (p != 0) {
                                char *base = reinterpret_cast<char *>(p);
                                *reinterpret_cast<int *>(base + 0x58) = 0;
                                    *reinterpret_cast<int *>(base + 0x5c) = 0;
                                    *reinterpret_cast<int *>(base + 0x60) = 0;
                                    *reinterpret_cast<int *>(base + 0x64) = 0;
                                    base[0x6c] = 0;
                                    base[0x6d] = 0;
                                    *reinterpret_cast<int *>(base + 0x68) = 0;

                                *reinterpret_cast<int *>(base) = reinterpret_cast<int>(g_0066b9f0);
                                *reinterpret_cast<int *>(base + 0x70) = 1;
                                *reinterpret_cast<int *>(base + 0x74) = 0;
                                *reinterpret_cast<int *>(base + 0x78) = 0x14;
                                *reinterpret_cast<int *>(base + 0x7c) = 0xf;
                            }
                            obj = p;
                        }
                    } else {
                        strcat(msg, reinterpret_cast<char *>(g_00683a90));
                        ShellA78 *p = new ShellA78();
                        if (p != 0) {
                            char *base = reinterpret_cast<char *>(p);
                            *reinterpret_cast<int *>(base + 0x58) = 0;
                                    *reinterpret_cast<int *>(base + 0x5c) = 0;
                                    *reinterpret_cast<int *>(base + 0x60) = 0;
                                    *reinterpret_cast<int *>(base + 0x64) = 0;
                                    base[0x6c] = 0;
                                    base[0x6d] = 0;
                                    *reinterpret_cast<int *>(base + 0x68) = 0;

                            *reinterpret_cast<int *>(base) = reinterpret_cast<int>(g_0066bb58);
                            *reinterpret_cast<int *>(base + 0x70) = 0;
                            *reinterpret_cast<int *>(base + 0x74) = 0;
                        }
                        obj = p;
                    }
                } else {
                    strcat(msg, reinterpret_cast<char *>(g_00683a78));
                    ShellA74 *p = new ShellA74();
                    if (p != 0) {
                        char *base = reinterpret_cast<char *>(p);
                        *reinterpret_cast<int *>(base + 0x58) = 0;
                                    *reinterpret_cast<int *>(base + 0x5c) = 0;
                                    *reinterpret_cast<int *>(base + 0x60) = 0;
                                    *reinterpret_cast<int *>(base + 0x64) = 0;
                                    base[0x6c] = 0;
                                    base[0x6d] = 0;
                                    *reinterpret_cast<int *>(base + 0x68) = 0;

                        *reinterpret_cast<int *>(base) = reinterpret_cast<int>(g_0066be28);
                        *reinterpret_cast<int *>(base + 0x70) = 0;
                    }
                    obj = p;
                }
            } else {
                strcat(msg, reinterpret_cast<char *>(g_00683a60));
                ShellA70 *p = new ShellA70();
                if (p != 0) {
                    char *base = reinterpret_cast<char *>(p);
                    *reinterpret_cast<int *>(base + 0x58) = 0;
                                    *reinterpret_cast<int *>(base + 0x5c) = 0;
                                    *reinterpret_cast<int *>(base + 0x60) = 0;
                                    *reinterpret_cast<int *>(base + 0x64) = 0;
                                    base[0x6c] = 0;
                                    base[0x6d] = 0;
                                    *reinterpret_cast<int *>(base + 0x68) = 0;

                    *reinterpret_cast<int *>(base) = reinterpret_cast<int>(g_0066bcc0);
                }
                obj = p;
            }
        } else {
            strcat(msg, reinterpret_cast<char *>(g_00683a48));
            ShellFA *p = new ShellFA();
            if (p != 0) {
                char *base = reinterpret_cast<char *>(p);
                *reinterpret_cast<int *>(base) = reinterpret_cast<int>(g_0066be28);
                *reinterpret_cast<int *>(base + 0x70) = 0;
            }
            obj = p;
        }
    } else {
        strcat(msg, reinterpret_cast<char *>(g_00683a30));
        ShellFA *p = new ShellFA();
        if (p != 0) {
            char *base = reinterpret_cast<char *>(p);
            *reinterpret_cast<int *>(base) = reinterpret_cast<int>(g_0066bf90);
            *reinterpret_cast<int *>(base + 0x70) = 0;
        }
        obj = p;
    }

    int flag = *g_009a6490;
    *g_0074daa0 = reinterpret_cast<int>(obj);
    if ((flag & 0x800) != 0 && obj != 0) {
        reinterpret_cast<VCall4 *>(obj)->slot004(msg);
        reinterpret_cast<VCall *>(*g_0074daa0)->slot075();
    }
}
