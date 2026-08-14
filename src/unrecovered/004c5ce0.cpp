// ORIGINAL: 0x004C5CE0 FILE
// RULED-OUT: memset(0xb*4) reproduces the `rep stosd`; the FreeLibrary-style
//        import-thunk call needed `__stdcall`, not `__cdecl` (the cdecl
//        form left a spurious `add esp,4`). MNEMONIC_ONLY, same size and
//        mnemonic count as the original - only some operand encodes differ.
// working copy - scaffold materialised by --work
// name      ?init_sound@@YAHPAXK@Z
// size      174 bytes
// spans     0x004C5CE0-0x004C5D8E
// prototype 
// callers   1   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C4F40 0x004C57A0 0x004C5A10 0x004C5E50
// indirect  0x004C5D00 0x004C5D35

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?init_sound@@YAHPAXK@Z  at 0x004C5CE0  (174 bytes)
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

class Midi_Device;
class Wave_Device;
class Wave_In_Device;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Midi_Device { public:
    uint32_t vtable_storage_;
    uint32_t field_4_;
    uint32_t field_8_;
    uint32_t field_C_;
    uint32_t field_10_;
    uint32_t field_14_;
    uint32_t field_18_;
    uint32_t field_1C_;
    int init(void *, unsigned long);
};

class Wave_Device { public:
    int init(void *, unsigned long);
};

class Wave_In_Device { public:
    uint32_t vtable_storage_;
    uint32_t field_4_;
    uint32_t field_8_;
    uint32_t field_C_;
    uint32_t field_10_;
    uint32_t field_14_;
    uint32_t field_18_;
    uint32_t field_1C_;
    int init(void *, unsigned long);
};

int load_sound_dll();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669128 = (int *)0x00669128;
static int *const g_0090d950 = (int *)0x0090D950;
static int *const g_0090d978 = (int *)0x0090D978;
static int *const g_0090db24 = (int *)0x0090DB24;
static int *const g_0090db2c = (int *)0x0090DB2C;
static int *const g_0090db50 = (int *)0x0090DB50;
static int *const g_0090db78 = (int *)0x0090DB78;
static int *const g_0090db7c = (int *)0x0090DB7C;
extern "C" void *memset(void *, int, unsigned int);
typedef void (__cdecl *ZeroArgFn)(int, int);
typedef void (__stdcall *FreeLibraryFn)(int);

int __cdecl init_sound(void * a1, unsigned long a2) {
    int loadResult = load_sound_dll();
    if (loadResult != 0) {
        return loadResult;
    }
    if (*g_0090db78 != 0) {
        (*reinterpret_cast<ZeroArgFn *>(g_0090db2c))(0, 0);
    }
    Wave_Device *waveDevice = reinterpret_cast<Wave_Device *>(g_0090d978);
    int result = waveDevice->init(a1, a2);
    if (result != 0) {
        *g_0090db7c = 0;
        if (*g_0090db78 != 0) {
            (*reinterpret_cast<FreeLibraryFn *>(g_00669128))(*g_0090db78);
            *g_0090db78 = 0;
        }
        memset(g_0090db24, 0, 0xb * 4);
        return result;
    }
    if ((a2 & 2) != 0) {
        Midi_Device *midiDevice = reinterpret_cast<Midi_Device *>(g_0090d950);
        midiDevice->init(a1, 2);
    }
    if ((a2 & 8) != 0) {
        Wave_In_Device *waveInDevice = reinterpret_cast<Wave_In_Device *>(g_0090db50);
        waveInDevice->init(a1, a2);
    }
    *g_0090db7c = 1;
    return 0;
}
