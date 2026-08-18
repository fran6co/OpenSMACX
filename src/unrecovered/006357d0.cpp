// ORIGINAL: 0x006357D0 sub_6357d0 0x006357D0-0x00635835 FILE
// working copy - scaffold materialised by --work
// size      101 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006465F0
// indirect  0x00635814

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_6357d0  at 0x006357D0  (101 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" void *memset(void *, int, unsigned int);

typedef long (__stdcall *LockFn)(void *, void *, void *, unsigned long, void *);

class Surface { public:
    int lock();
};

int Surface::lock() {
    char *self = reinterpret_cast<char *>(this);
    void **surface_ptr = reinterpret_cast<void **>(self + 0xc);
    if (*surface_ptr == 0) {
        return 0;
    }

    int *cached = reinterpret_cast<int *>(self + 0x10);
    if (*cached != 0) {
        return *cached;
    }

    char desc[0x6c];
    memset(desc, 0, 0x6c);
    *reinterpret_cast<int *>(desc) = 0x6c;

    void *surface = *surface_ptr;
    void *vtable = *reinterpret_cast<void **>(surface);
    LockFn lock_fn = *reinterpret_cast<LockFn *>(reinterpret_cast<char *>(vtable) + 0x64);
    int hr = lock_fn(surface, 0, desc, 0x21, 0);
    if (hr != 0) {
        return 0;
    }

    int pitch = *reinterpret_cast<int *>(desc + 0x10);
    void *lp_surface = *reinterpret_cast<void **>(desc + 0x24);
    *reinterpret_cast<int *>(self + 0x14) = pitch;
    *reinterpret_cast<void **>(self + 0x10) = lp_surface;
    return reinterpret_cast<int>(lp_surface);
}
