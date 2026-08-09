// ORIGINAL: 0x00446D10 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00446D10
// name           ?ambience_veh@@YAXH@Z
// size           1246 bytes
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00446d10/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?ambience_veh@@YAXH@Z  at 0x00446D10  (1246 bytes)
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

class FX;

// ---- callees, declared and never defined (a definition would be inlined) ----
class FX { public:
    uint8_t effects_[0x61 * 0x6C];
    uint32_t field_28EC_;
    void play(int);
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00683bf4 = (int *)0x00683BF4;
static int *const g_00749cf8 = (int *)0x00749CF8;
static int *const g_00749fec = (int *)0x00749FEC;
static int *const g_0074a208 = (int *)0x0074A208;
static int *const g_0074a490 = (int *)0x0074A490;
static int *const g_0074a568 = (int *)0x0074A568;
static int *const g_0074a7f0 = (int *)0x0074A7F0;
static int *const g_0074b060 = (int *)0x0074B060;
static int *const g_0074b1a4 = (int *)0x0074B1A4;
static int *const g_0074b498 = (int *)0x0074B498;
static int *const g_0074b7f8 = (int *)0x0074B7F8;
static int *const g_0074b8d0 = (int *)0x0074B8D0;
static int *const g_0074b93c = (int *)0x0074B93C;
static int *const g_0074b9a8 = (int *)0x0074B9A8;
static int *const g_0074ba14 = (int *)0x0074BA14;
static int *const g_0074ba80 = (int *)0x0074BA80;
static int *const g_0074baec = (int *)0x0074BAEC;
static int *const g_0074bb58 = (int *)0x0074BB58;
static int *const g_0074bc9c = (int *)0x0074BC9C;
static int *const g_0074c35c = (int *)0x0074C35C;
static int *const g_0074c3c8 = (int *)0x0074C3C8;
static int *const g_0074c434 = (int *)0x0074C434;
static int *const g_0074c4a0 = (int *)0x0074C4A0;
static int *const g_0074c50c = (int *)0x0074C50C;
static int *const g_0074c578 = (int *)0x0074C578;
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_009a6490 = (int *)0x009A6490;

// Vtable shim. VC6 rejects a free `__thiscall` function pointer (C4234),
// so an indirect virtual call is spelled by calling the Nth virtual of a
// class that is never defined and never instantiated. Only DECLARATION
// ORDER matters. This body dispatches through slot(s): 0x28 (slot010),
// 0x5c (slot023).
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
    virtual void slot010();  // <-- used, offset 0x28
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
    virtual int  slot023();  // <-- used, offset 0x5c
};

// Indexed table bases: each is indexed by a computed register offset, so
// the address itself does work - needs `extern T name[]` under a name
// distinct from any const-pointer global.
extern uint8_t g_x952839[];   // per-ambience-index "already stopped" flag, stride 0x34
extern uint8_t g_x952832[];   // per-ambience-index sound-id (int16), stride 0x34
extern uint8_t g_x952836[];   // per-ambience-index kind byte, stride 0x34
extern uint8_t g_x9ab892[];   // per-sound-id field @+6, stride 0x34
extern uint8_t g_x9ab88c[];   // per-sound-id field @+0 ("bVar1" category), stride 0x34
extern uint8_t g_x9ab88d[];   // per-sound-id field @+1, stride 0x34
extern uint8_t g_x94ae68[];   // table indexed by field@+1 value * 0x10
extern uint8_t g_x946f58[];   // bit-7 flag table indexed by kind*0x167*4
extern uint8_t g_x94a379[];   // table indexed by bVar1*0x90
extern uint8_t g_x94a378[];   // parallel table indexed by bVar1*0x90

void __cdecl ambience_veh(int a1) {
    int old;
    int flagsWord;
    int recIdx;
    int sid;
    int sidIdx;
    unsigned char flag1;
    unsigned char val892;
    unsigned char kindVal;
    unsigned char bVar1;
    unsigned char blVal;
    signed char cVar2;
    unsigned char t378;
    unsigned int idx;
    int ret;
    char *effBase;

    if (a1 < 0) {
        return;
    }

    old = *g_00683bf4;
    if (old >= 0 && old != a1 && old < 0x61) {
        effBase = reinterpret_cast<char *>(g_00749cf8);
        reinterpret_cast<VCall *>(effBase + old * 0x6C)->slot010();
    }

    flagsWord = *g_009a6490;
    *g_00683bf4 = a1;
    if ((flagsWord & 0x800) == 0) {
        return;
    }

    recIdx = a1 * 0x34;
    flag1 = g_x952839[recIdx];
    if (flag1 != 0) {
        return;
    }

    sid = *reinterpret_cast<short *>(&g_x952832[recIdx]);
    sidIdx = sid * 0x34;
    val892 = g_x9ab892[sidIdx];
    if (val892 == 8) {
        kindVal = g_x952836[recIdx];
        idx = (unsigned int)kindVal * 359;
        if ((g_x946f58[idx * 4] & 0x80) == 0) {
            goto ef2;
        }
        ret = reinterpret_cast<VCall *>(g_0074c3c8)->slot023();
        if (ret != 0) {
            return;
        }
        reinterpret_cast<FX *>(g_00749cf8)->play(0x5c);
        return;
    }

    bVar1 = g_x9ab88c[sidIdx];
    if (bVar1 != 0) {
        goto bvar1_nonzero;
    }

    blVal = g_x9ab88d[sidIdx];
    if (g_x94ae68[(unsigned int)blVal * 0x10] == bVar1) {
        ret = reinterpret_cast<VCall *>(g_0074b8d0)->slot023();
        if (ret != 0) {
            return;
        }
        goto ef2;
    }

    kindVal = g_x952836[recIdx];
    idx = (unsigned int)kindVal * 359;
    if ((g_x946f58[idx * 4] & 0x80) == 0) {
        ret = reinterpret_cast<VCall *>(g_0074b7f8)->slot023();
        if (ret != 0) {
            return;
        }
        reinterpret_cast<FX *>(g_00749cf8)->play(0x40);
        return;
    }

    if (sid == 0x10 || sid == 0x11 || sid == 0x12) {
        ret = reinterpret_cast<VCall *>(g_0074c434)->slot023();
        if (ret != 0) {
            return;
        }
        reinterpret_cast<FX *>(g_00749cf8)->play(0x5d);
        return;
    }
    if (sid == 8) {
        goto play38;
    }
    if (sid != 0xf) {
        ret = reinterpret_cast<VCall *>(g_0074c35c)->slot023();
        if (ret != 0) {
            return;
        }
        reinterpret_cast<FX *>(g_00749cf8)->play(0x5b);
        return;
    }
    ret = reinterpret_cast<VCall *>(g_0074c4a0)->slot023();
    if (ret != 0) {
        return;
    }
    reinterpret_cast<FX *>(g_00749cf8)->play(0x5e);
    return;

bvar1_nonzero:
    if (bVar1 == 1) {
        ret = reinterpret_cast<VCall *>(g_0074bb58)->slot023();
        if (ret != 0) return;
        reinterpret_cast<FX *>(g_00749cf8)->play(0x48);
        return;
    }
    if (bVar1 == 2) {
        ret = reinterpret_cast<VCall *>(g_0074b1a4)->slot023();
        if (ret != 0) return;
        reinterpret_cast<FX *>(g_00749cf8)->play(0x31);
        return;
    }
    if (sid == 0xe) {
        ret = reinterpret_cast<VCall *>(g_0074c50c)->slot023();
        if (ret != 0) return;
        reinterpret_cast<FX *>(g_00749cf8)->play(0x5f);
        return;
    }
    if (sid == 9) {
        ret = reinterpret_cast<VCall *>(g_0074c578)->slot023();
        if (ret != 0) return;
        reinterpret_cast<FX *>(g_00749cf8)->play(0x60);
        return;
    }
    if (bVar1 == 3) {
        ret = reinterpret_cast<VCall *>(g_0074ba14)->slot023();
        if (ret != 0) return;
        reinterpret_cast<FX *>(g_00749cf8)->play(0x45);
        return;
    }
    if (bVar1 == 4) {
        ret = reinterpret_cast<VCall *>(g_0074ba80)->slot023();
        if (ret != 0) return;
        reinterpret_cast<FX *>(g_00749cf8)->play(0x46);
        return;
    }
    if (bVar1 == 5) {
        ret = reinterpret_cast<VCall *>(g_0074baec)->slot023();
        if (ret != 0) return;
        reinterpret_cast<FX *>(g_00749cf8)->play(0x47);
        return;
    }
    if (bVar1 == 6) {
        ret = reinterpret_cast<VCall *>(g_0074b9a8)->slot023();
        if (ret != 0) return;
        reinterpret_cast<FX *>(g_00749cf8)->play(0x44);
        return;
    }
    if (bVar1 == 7) {
        ret = reinterpret_cast<VCall *>(g_0074b93c)->slot023();
        if (ret != 0) return;
        reinterpret_cast<FX *>(g_00749cf8)->play(0x43);
        return;
    }
    if (bVar1 == 8) {
        ret = reinterpret_cast<VCall *>(g_0074bc9c)->slot023();
        if (ret != 0) return;
        reinterpret_cast<FX *>(g_00749cf8)->play(0x4b);
        return;
    }
    if (sid != 8) {
        idx = (unsigned int)bVar1 * 0x90;
        cVar2 = (signed char)g_x94a379[idx];
        if (cVar2 == 1) {
            ret = reinterpret_cast<VCall *>(g_0074a208)->slot023();
            if (ret != 0) return;
            reinterpret_cast<FX *>(g_00749cf8)->play(0xc);
            return;
        }
        if (cVar2 == 2) {
            ret = reinterpret_cast<VCall *>(g_0074a490)->slot023();
            if (ret != 0) return;
            reinterpret_cast<FX *>(g_00749cf8)->play(0x12);
            return;
        }
        if (val892 == 0xc) {
            ret = reinterpret_cast<VCall *>(g_0074a568)->slot023();
            if (ret != 0) return;
            reinterpret_cast<FX *>(g_00749cf8)->play(0x14);
            return;
        }
        blVal = g_x9ab88d[sidIdx];
        cVar2 = (signed char)g_x94ae68[(unsigned int)blVal * 0x10];
        if (cVar2 != 0) {
            t378 = g_x94a378[idx];
            if ((int)cVar2 <= (int)t378) {
                ret = reinterpret_cast<VCall *>(g_0074a7f0)->slot023();
                if (ret != 0) return;
                reinterpret_cast<FX *>(g_00749cf8)->play(0x1a);
                return;
            }
            if (cVar2 != 0) {
                ret = reinterpret_cast<VCall *>(g_00749fec)->slot023();
                if (ret != 0) return;
                reinterpret_cast<FX *>(g_00749cf8)->play(7);
                return;
            }
        }
        ret = reinterpret_cast<VCall *>(g_0074b060)->slot023();
        if (ret != 0) return;
        reinterpret_cast<FX *>(g_00749cf8)->play(0x2e);
        return;
    }

play38:
    ret = reinterpret_cast<VCall *>(g_0074b498)->slot023();
    if (ret == 0) {
        reinterpret_cast<FX *>(g_00749cf8)->play(0x38);
    }
    return;

ef2:
    reinterpret_cast<FX *>(g_00749cf8)->play(0x42);
    return;
}
