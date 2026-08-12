// ORIGINAL: 0x004E34D0 FILE
// name      ??1Net@@QAE@XZ
// size      525 bytes
// spans     0x004E34D0-0x004E365C;0x004E3710-0x004E372B;0x0065C810-0x0065C876
// prototype void (__thiscall ??1Net@@QAE@XZ)(Net* this)
// callers   0   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00401BE0 0x00402970 0x00402DD0 0x004C8DB0 0x0062E010 0x006339E0
// indirect  0x004E35D5 0x004E35E2 0x004E3602
// PRESERVED UNIT - measured SHARED_TAIL.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004E34D0
// measured tier  SHARED_TAIL
// refusal        1 span(s) are COMDAT-folded and claimed by another function; no per-function verdict is well defined
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004e34d0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??1Net@@QAE@XZ  at 0x004E34D0  (525 bytes)
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
class NetFifo;
class VoiceTx;

// ---- callees, declared and never defined (a definition would be inlined) ----
class NetFifo { public:
    ~NetFifo();
};
class VoiceTx { public:
    ~VoiceTx();
};
extern "C" int __cdecl fn_00402970();
extern "C" int __cdecl sub_401be0();
extern "C" int __cdecl sub_402dd0();

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 0, 1, 2
class VCall { public:
    virtual void slot000();  // <-- used
    virtual void slot001();  // <-- used
    virtual void slot002();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0065c86c = (int *)0x0065C86C;
static int *const g_006693ac = (int *)0x006693AC;
static int *const g_00669418 = (int *)0x00669418;
static int *const g_0066941c = (int *)0x0066941C;
static int *const g_0066eaf8 = (int *)0x0066EAF8;
static int *const g_0066eafc = (int *)0x0066EAFC;
static int *const g_0066eb04 = (int *)0x0066EB04;
static int *const g_0067b288 = (int *)0x0067B288;
static int *const g_009b3374 = (int *)0x009B3374;

class Net { public:
    void close();
    ~Net();
};

// The given VCall shim declares its three used slots nullary; each of the
// three call sites below pushes one argument, so a second shim (same slot
// layout, real signatures) is added rather than editing the given one.
class VCallArg { public:
    virtual void slot000(int);
    virtual void slot001(void *);
    virtual void slot002(int);
};

Net::~Net() {
    char *self = reinterpret_cast<char *>(this);

    *reinterpret_cast<void **>(self) = g_0066eb04;
    close();

    // Embedded sub-object ending at self+0x750: its own vtable slot sits at
    // -0x24, and a second, virtual-base vtable is reached through the usual
    // vbtable-delta pattern at -0x20 (read the vbtable pointer, add the
    // delta stored at vbtable[1], write the adjusted vtable there).
    char *sub1 = self + 0x750;
    *reinterpret_cast<void **>(sub1 - 0x24) = g_0066941c;
    {
        char *vbase = *reinterpret_cast<char **>(sub1 - 0x20);
        int delta = *reinterpret_cast<int *>(vbase + 4);
        *reinterpret_cast<void **>(sub1 - 0x20 + delta) = g_00669418;
    }
    fn_00402970();

    *reinterpret_cast<int *>(sub1 - 0x10) = 0;
    sub_401be0();

    *reinterpret_cast<void **>(sub1) = g_006693ac;
    *g_009b3374 = *reinterpret_cast<int *>(sub1 + 4);

    *reinterpret_cast<void **>(self + 0x758) = g_006693ac;
    *g_009b3374 = *reinterpret_cast<int *>(self + 0x75c);

    reinterpret_cast<NetFifo *>(self + 0x130)->~NetFifo();
    reinterpret_cast<NetFifo *>(self + 0x10c)->~NetFifo();
    reinterpret_cast<NetFifo *>(self + 0xe8)->~NetFifo();

    // Container at self+0xb0: an intrusive-list collection with its own
    // vtable (element allocator interface at +0) plus a second vtable
    // reached through the same vbtable-delta pattern, this time rooted at
    // self+0xb4.
    char *vec = self + 0xb0;
    *reinterpret_cast<void **>(vec) = g_0066eafc;
    {
        char *vbase = *reinterpret_cast<char **>(self + 0xb4);
        int delta = *reinterpret_cast<int *>(vbase + 4);
        *reinterpret_cast<void **>(self + 0xb4 + delta) = g_0066eaf8;
    }

    if (*reinterpret_cast<char **>(vec + 8) != 0) {
        int count = *reinterpret_cast<int *>(vec + 0x10);
        if (count > 0) {
            int i = 0;
            do {
                char *node = *reinterpret_cast<char **>(vec + 8);
                char *next = *reinterpret_cast<char **>(node + 0xc);
                *reinterpret_cast<char **>(vec + 0xc) = next;
                char *element = *reinterpret_cast<char **>(node + 8);
                reinterpret_cast<VCallArg *>(vec)->slot001(element);
                if (element != 0) {
                    reinterpret_cast<VCallArg *>(element)->slot002(1);
                }
                *reinterpret_cast<int *>(*reinterpret_cast<char **>(vec + 8) + 8) = 0;
                char *node2 = *reinterpret_cast<char **>(vec + 8);
                if (node2 != 0) {
                    char *vbase2 = *reinterpret_cast<char **>(node2);
                    int delta2 = *reinterpret_cast<int *>(vbase2 + 4);
                    reinterpret_cast<VCallArg *>(node2 + delta2)->slot000(1);
                }
                *reinterpret_cast<char **>(vec + 8) =
                    *reinterpret_cast<char **>(vec + 0xc);
                i++;
            } while (i < count);
        }
        *reinterpret_cast<char **>(vec + 8) = 0;
        *reinterpret_cast<int *>(vec + 0x14) = 0;
        *reinterpret_cast<int *>(vec + 0x10) = 0;
    }
    *reinterpret_cast<int *>(vec + 0x14) = 0;

    *g_009b3374 = *reinterpret_cast<int *>(self + 0xd0);
    *reinterpret_cast<void **>(self + 0xcc) = g_006693ac;

    reinterpret_cast<VoiceTx *>(self + 0x58)->~VoiceTx();
}
