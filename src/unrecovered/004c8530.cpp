// ORIGINAL: 0x004C8530 ?load@Ambience@@QAEHPBD@Z 0x004C8530-0x004C85EF FILE
// RULED-OUT: reused-parameter-slot byte/garbage-upper-bits trick for the sub_4c7cc0 arg (UB, not attempted); passed the plain flag bit instead. Ambience reinterpreted as Sound* for the trailing Sound::load call (single-inheritance-at-0 evidence: ecx=edi unchanged across the call). First divergence #3 mov/push.
// working copy - scaffold materialised by --work
// size      191 bytes
// prototype int (__thiscall ?load@Ambience@@QAEHPBD@Z)(Ambience* this, int8*)
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C6280 0x004C7CC0 0x006005D0 0x00645460
// indirect  0x004C8582 0x004C85A5

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?load@Ambience@@QAEHPBD@Z  at 0x004C8530  (191 bytes)
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

class Ambience;
class Sound;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Sound { public:
    uint32_t vtable_storage_;
    uint32_t volume_;
    int32_t pan_8_;
    uint32_t field_C_;
    uint32_t field_10_;
    uint32_t field_14_;
    uint32_t field_18_;
    uint32_t field_1C_;
    uint32_t field_20_;
    uint32_t field_24_;
    uint32_t field_28_;
    uint32_t field_2C_;
    uint32_t loop_flag_30_;
    uint32_t delay_;
    uint32_t fade_38_;
    void * device_;
    uint32_t flags_40_;
    Sound * chain_prev_;
    Sound * chain_next_;
    void * fname_;
    uint32_t type_;
    int load(const char *);
};

extern "C" char *strcpy(char *, const char *);
extern "C" int __stdcall sub_4c7cc0(int);
int filefind_get(char *);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 5
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0090db24 = (int *)0x0090DB24;
static int *const g_0090db7c = (int *)0x0090DB7C;

class Ambience { public:
    int load(const char *);
};
int Ambience::load(const char * a1) {
    char buf[256];
    strcpy(buf, a1);
    int fileHandle = filefind_get(buf);
    if (fileHandle == 0) {
        return 10;
    }

    Sound **pSound = reinterpret_cast<Sound **>(reinterpret_cast<char *>(this) + 0x3c);
    Sound *sound = *pSound;
    if (sound != 0) {
        reinterpret_cast<VCall *>(this)->slot005();
        sound = *pSound;
    }
    if (sound == 0) {
        if (*g_0090db7c == 0) {
            return 1;
        }
        typedef int (__cdecl *FnCreate)(Sound **, int, int);
        FnCreate fn = reinterpret_cast<FnCreate>(*g_0090db24);
        int err = fn(pSound, fileHandle, 5);
        if (err != 0) {
            return err;
        }
    }
    sound = *pSound;
    char *self = reinterpret_cast<char *>(this);
    unsigned char al = *reinterpret_cast<unsigned char *>(self + 0x54) & 1;
    unsigned char *soundFlagByte = reinterpret_cast<unsigned char *>(reinterpret_cast<char *>(sound) + 0x54);
    *soundFlagByte = (*soundFlagByte & ~1) | al;
    if (*reinterpret_cast<int *>(reinterpret_cast<char *>(sound) + 0x3c) != 0) {
        sub_4c7cc0(al);
    }
    return reinterpret_cast<Sound *>(this)->load(reinterpret_cast<const char *>(fileHandle));
}
