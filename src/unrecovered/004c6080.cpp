// ORIGINAL: 0x004C6080 ??0Sound@@QAE@XZ 0x004C6080-0x004C611A;0x004C8450-0x004C8457;0x00659ED0-0x00659EE2 FILE
// TRIED: no tier is reachable for ANY body - SHARED_TAIL, the helper at 0x004C8450 (mov [ecx],0x66e444; ret) is COMDAT-folded with another catalogued function, same wall as VoiceRx::VoiceRx() at 0x004C8960. Straight-line field reconstruction below, no attempt at the SEH try/catch unwind frame (bare try/catch with rethrow produced no fs:[0] frame at all under this compiler's flags; the scaffold gives Sound no base class to hang the automatic base/derived vtable-swap unwind on, same as the already-tried ~UV2Player shape).
// working copy - scaffold materialised by --work
// size      179 bytes
// prototype void (__thiscall ??0Sound@@QAE@XZ)(Sound* this)
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x006465F0
// indirect  0x004C6102

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??0Sound@@QAE@XZ  at 0x004C6080  (179 bytes)
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

class Sound;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" void *memset(void *, int, unsigned int);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 0
class VCall { public:
    virtual void slot000();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00659ed8 = (int *)0x00659ED8;
static int *const g_0066e3c0 = (int *)0x0066E3C0;
static int *const g_0066e444 = (int *)0x0066E444;
static int *const g_006790c0 = (int *)0x006790C0;

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

    Sound();
};
class VCall1 { public: virtual void slot000(int); };

Sound::Sound() {
    vtable_storage_ = (uint32_t)(int)g_0066e444;
    volume_ = 0x7f;
    pan_8_ = 0;
    memset(&field_C_, 0, 0x24);
    loop_flag_30_ = 0;
    memset(&flags_40_, 0, sizeof(flags_40_));
    vtable_storage_ = (uint32_t)(int)g_0066e3c0;
    chain_prev_ = 0;
    chain_next_ = 0;
    device_ = 0;
    fname_ = 0;
    flags_40_ &= 0xfffffffe;
    fade_38_ = 0x3e8;
    if (device_ != 0) {
        ((VCall1 *)device_)->slot000(0x3e8);
    }
    type_ = 0;
}
