// ORIGINAL: 0x004C8F70 FILE
// working copy - scaffold materialised by --work
// name      ?init@VoiceTx@@QAEHKPAXK@Z
// size      151 bytes
// spans     0x004C8F70-0x004C9007
// prototype int (__thiscall ?init@VoiceTx@@QAEHKPAXK@Z)(VoiceTx* this, unsigned int, void*, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C8F7C 0x004C8FA3 0x004C8FC6 0x004C8FE3 0x004C8FFB

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?init@VoiceTx@@QAEHKPAXK@Z  at 0x004C8F70  (151 bytes)
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

class VoiceTx;

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 5, 31, 33
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();  // <-- used
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
    virtual void slot031();  // <-- used
    virtual void slot032();
    virtual void slot033();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0090db24 = (int *)0x0090DB24;
static int *const g_0090db28 = (int *)0x0090DB28;
static int *const g_0090db7c = (int *)0x0090DB7C;

class VoiceTx { public:
    uint8_t pad_0_[0x3C];
    uint32_t field_3c_;

    int init(unsigned long, void *, unsigned long);
};
typedef int (__cdecl *VoiceCreateFn)(void *, int, int);
typedef void (__cdecl *VoiceDestroyFn)(void *);

// Vtable shim for the object at *slot: slot 33 (0x84) takes three args and
// returns int; the emitter's own VCall class (slots 5 and 31, both nullary)
// already covers `this`'s own slots used here.
class VCall2 { public:
    virtual void slot000(); virtual void slot001(); virtual void slot002(); virtual void slot003();
    virtual void slot004(); virtual void slot005(); virtual void slot006(); virtual void slot007();
    virtual void slot008(); virtual void slot009(); virtual void slot010(); virtual void slot011();
    virtual void slot012(); virtual void slot013(); virtual void slot014(); virtual void slot015();
    virtual void slot016(); virtual void slot017(); virtual void slot018(); virtual void slot019();
    virtual void slot020(); virtual void slot021(); virtual void slot022(); virtual void slot023();
    virtual void slot024(); virtual void slot025(); virtual void slot026(); virtual void slot027();
    virtual void slot028(); virtual void slot029(); virtual void slot030(); virtual void slot031();
    virtual void slot032();
    virtual int slot033(unsigned long, void *, unsigned long);
};

int VoiceTx::init(unsigned long a1, void * a2, unsigned long a3) {
    char *self = reinterpret_cast<char *>(this);
    reinterpret_cast<VCall *>(this)->slot005();
    void **slot = reinterpret_cast<void **>(self + 0x3c);
    int result = 0;
    if (*slot == 0) {
        if (*g_0090db7c == 0) {
            return 1;
        }
        VoiceCreateFn create = *reinterpret_cast<VoiceCreateFn *>(g_0090db24);
        result = create(slot, 0, 7);
        if (result != 0) {
            return result;
        }
    }
    void *obj = *slot;
    if (obj != 0) {
        result = reinterpret_cast<VCall2 *>(obj)->slot033(a1, a2, a3);
        if (result != 0 && result != 0x10) {
            if (*g_0090db7c != 0) {
                VoiceDestroyFn destroy = *reinterpret_cast<VoiceDestroyFn *>(g_0090db28);
                destroy(*slot);
            }
            *slot = 0;
        }
        if (*slot != 0) {
            reinterpret_cast<VCall *>(this)->slot031();
        }
    }
    return result;
}
