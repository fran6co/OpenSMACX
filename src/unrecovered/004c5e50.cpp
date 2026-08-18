// ORIGINAL: 0x004C5E50 ?load_sound_dll@@YAHXZ 0x004C5E50-0x004C5F68 FILE
// RULED-OUT: direct pointer-typed calls (no intermediate function-pointer locals) for the three import-thunk calls (LoadLibraryA/FreeLibrary/the version fn ptr) so VC6 folds them to `call dword ptr [addr]` instead of loading the address into a register first; a `do { GetProcAddress(...) } while` loop for the fill-in-table pass. Landed as MISMATCH: the do-while still diverges right where the original skips the redundant reload of `DAT_0090db78` on the loop's first iteration (a PRE optimisation tied to the value already being live in a register from the code just above the loop) - not reproduced by restating the same load inside the loop body.
// working copy - scaffold materialised by --work
// size      280 bytes
// prototype
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C9080 0x0062D390
// indirect  0x004C5E65 0x004C5E9C 0x004C5ED1 0x004C5EFD

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?load_sound_dll@@YAHXZ  at 0x004C5E50  (280 bytes)
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
extern "C" int __cdecl sub_4c9080();
extern "C" int __cdecl sub_62d390();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066e258 = (int *)0x0066E258;
static int *const g_0066e2d8 = (int *)0x0066E2D8;
static int *const g_0066e2f0 = (int *)0x0066E2F0;
static int *const g_0066e370 = (int *)0x0066E370;
static int *const g_00687b14 = (int *)0x00687B14;
static int *const g_0090db4c = (int *)0x0090DB4C;
static int *const g_0090db50 = (int *)0x0090DB50;
typedef int (__stdcall *LoadLibraryAFn)(const char *);
typedef int (__stdcall *FreeLibraryFn)(int);
typedef void *(__stdcall *GetProcAddressFn)(int, unsigned int);
typedef unsigned int (__cdecl *VersionFn)();
typedef int (__cdecl *MsgBoxFn)(const char *, const char *);

static int *const g_0090db78 = (int *)0x0090DB78;
static int *const g_0090db7c = (int *)0x0090DB7C;
static void **const g_0090db24 = (void **)0x0090DB24;
static int *const g_00669120 = (int *)0x00669120;
static int *const g_00669124 = (int *)0x00669124;
static int *const g_00669128 = (int *)0x00669128;

int __cdecl load_sound_dll() {
    if (*g_0090db78 == 0) {
        *g_0090db78 = (*reinterpret_cast<LoadLibraryAFn *>(g_00669120))(reinterpret_cast<const char *>(g_00687b14));
        if (*g_0090db78 == 0) {
            return 1;
        }
        GetProcAddressFn pGetProcAddress = reinterpret_cast<GetProcAddressFn>(*g_00669124);
        int index = 0;
        void **slot = g_0090db24;
        do {
            void *proc = pGetProcAddress(*g_0090db78, (index + 1) & 0xffff);
            *slot = proc;
            if (proc == 0) {
                *g_0090db7c = 0;
                if (*g_0090db78 != 0) {
                    FreeLibraryFn pFreeLibrary = *reinterpret_cast<FreeLibraryFn *>(g_00669128);
                    pFreeLibrary(*g_0090db78);
                    *g_0090db78 = 0;
                }
                *g_0090db7c = 0;
                for (int i = 0; i < 0xb; ++i) {
                    g_0090db24[i] = 0;
                }
                return 1;
            }
            ++slot;
            ++index;
        } while ((int)slot < (int)g_0090db50);

        unsigned int version;
        if (*g_0090db78 == 0) {
            version = 0;
        } else {
            VersionFn pVersion = *reinterpret_cast<VersionFn *>(g_0090db4c);
            version = pVersion();
        }
        MsgBoxFn pMsgBox = reinterpret_cast<MsgBoxFn>(sub_4c9080);
        if ((version & 0xff00) != 0x100) {
            pMsgBox(reinterpret_cast<const char *>(g_0066e258), reinterpret_cast<const char *>(g_0066e2d8));
        }
        int jackalVer = sub_62d390();
        if (jackalVer != 1) {
            pMsgBox(reinterpret_cast<const char *>(g_0066e2f0), reinterpret_cast<const char *>(g_0066e2d8));
        }
        unsigned int jackalVer2 = sub_62d390();
        if (jackalVer2 != ((version >> 8) & 0xff)) {
            pMsgBox(reinterpret_cast<const char *>(g_0066e370), reinterpret_cast<const char *>(g_0066e2d8));
        }
    }
    return 0;
}
