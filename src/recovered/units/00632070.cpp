// ORIGINAL: 0x00632070 ?enable_join@Net@@QAEXXZ 0x00632070-0x006320DA FILE BYTE_EXACT
// size      106 bytes
// prototype void (__thiscall ?enable_join@Net@@QAEXXZ)(Net* this)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2
// indirect  0x00632091 0x006320B1 0x006320C7
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00632070
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00632070/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?enable_join@Net@@QAEXXZ  at 0x00632070  (106 bytes)
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

class Net;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" void free(void *);
void * __cdecl mem_get(int);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 22, 31
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
    virtual void __stdcall slot022(int, void *);  // <-- used
    virtual void slot023();
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void __stdcall slot031(void *, int);  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009be600 = (int *)0x009BE600;

class Net { public:
    void enable_join();
};

void Net::enable_join() {
    // DAT_009be600 is loaded FOUR times total in the target: once by the
    // null check, reused (not reloaded) for the first virtual call, then
    // reloaded fresh for the second and third. That is exactly the
    // assign-in-condition idiom below - the null check captures the value
    // into `p` and the first call reuses `p`, while the later two calls
    // read the global directly again (each after an intervening call the
    // compiler cannot prove leaves the global alone).
    VCall *p;
    if ((p = reinterpret_cast<VCall *>(*g_009be600)) != 0 &&
        (*reinterpret_cast<unsigned int *>(reinterpret_cast<char *>(this) + 0xd8) & 0x10000) != 0) {
        int local;
        p->slot022(0, &local);
        void *result = mem_get(local);
        reinterpret_cast<VCall *>(*g_009be600)->slot022(reinterpret_cast<int>(result), &local);
        *reinterpret_cast<unsigned int *>(reinterpret_cast<char *>(result) + 4) &= ~0x20;
        reinterpret_cast<VCall *>(*g_009be600)->slot031(result, 0);
        if (result != 0) {
            free(result);
        }
    }
}

