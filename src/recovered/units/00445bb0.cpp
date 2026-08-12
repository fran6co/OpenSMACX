// ORIGINAL: 0x00445BB0 FILE
// name      ?init@Effect@@QAEXPADJ@Z
// size      278 bytes
// spans     0x00445BB0-0x00445CC6
// prototype void (__thiscall ?init@Effect@@QAEXPADJ@Z)(Effect* this, int8*, unsigned int32)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C69B0 0x00645470 0x00645598 0x00645646 0x00645DD0
// indirect  0x00445CB9
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00445BB0
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00445bb0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?init@Effect@@QAEXPADJ@Z  at 0x00445BB0  (278 bytes)
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

class Effect;
class Wave;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Wave { public:
    uint32_t vtable_storage_;
    uint32_t volume_;
    uint32_t field_8_;
    uint8_t memset_region_[0x24];
    uint32_t field_30_;
    uint32_t field_34_;
    uint32_t field_38_;
    void * device_;
    uint32_t field_40_;
    Wave * chain_prev_;
    Wave * chain_next_;
    void * fname_;
    uint32_t field_50_;
    uint8_t flags_54_;
    uint8_t pad_55_[3];
    int32_t pitch_;
    float reverb_mix_;
    int32_t ms_length_;
    uint32_t start_time_;
    uint32_t group_slot_;
    void init(char *, unsigned long);
};
extern "C" char *strcat(char *, const char *);
extern "C" int __cdecl fclose();
extern "C" int __cdecl fopen();
extern "C" int __cdecl strchr();

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 35
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
    virtual void slot035();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00445650 = (int *)0x00445650;
static int *const g_004456a0 = (int *)0x004456A0;
static int *const g_004457c0 = (int *)0x004457C0;
static int *const g_00683ca4 = (int *)0x00683CA4;
static int *const g_00683ca8 = (int *)0x00683CA8;
static int *const g_00683cb0 = (int *)0x00683CB0;
static int *const g_00683cb4 = (int *)0x00683CB4;
static int *const g_009b7a88 = (int *)0x009B7A88;
static int *const g_009b7a98 = (int *)0x009B7A98;
static int *const g_009b7aa0 = (int *)0x009B7AA0;

class Effect { public:
    void init(char *, long);
};

#pragma function(strlen, strcat)

typedef char *(__cdecl *StrchrFn)(const char *, int);
typedef int (__cdecl *FopenFn)(const char *, const char *);
typedef int (__cdecl *FcloseFn)(int);

void Effect::init(char * a1, long a2) {
    Wave *self = reinterpret_cast<Wave *>(this);

    *g_009b7a88 = reinterpret_cast<int32_t>(g_004457c0);
    *g_009b7a98 = reinterpret_cast<int32_t>(g_00445650);
    *g_009b7aa0 = reinterpret_cast<int32_t>(g_004456a0);

    if ((static_cast<uint8_t>(a2) & 0x81) == 0) {
        a2 |= 4;
    }

    char buffer[0x100];
    buffer[0] = 0;
    strcat(buffer, reinterpret_cast<const char *>(g_00683ca4));
    strcat(buffer, a1);

    if (reinterpret_cast<StrchrFn>(strchr)(buffer, '.') != 0) {
        goto has_dot1;
    }
    strcat(buffer, reinterpret_cast<const char *>(g_00683ca8));
has_dot1:

    int file = reinterpret_cast<FopenFn>(fopen)(buffer, reinterpret_cast<const char *>(g_00683cb0));
    if (file == 0) {
        buffer[0] = 0;
        strcat(buffer, a1);
        if (reinterpret_cast<StrchrFn>(strchr)(buffer, '.') != 0) {
            goto has_dot2;
        }
        strcat(buffer, reinterpret_cast<const char *>(g_00683cb4));
has_dot2:
        ;
    } else {
        reinterpret_cast<FcloseFn>(fclose)(file);
    }

    self->init(buffer, a2);

    if ((static_cast<uint8_t>(a2) & 0x84) == 0) {
        reinterpret_cast<VCall *>(self)->slot035();
    }
}
