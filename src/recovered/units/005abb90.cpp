// ORIGINAL: 0x005ABB90 FILE
// name      ?save_map@@YAXXZ
// size      401 bytes
// spans     0x005ABB90-0x005ABD0C;0x0066221F-0x00662234
// prototype 
// callers   1   call targets   15
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0059E510 0x005A9B20 0x005FCBB0 0x006169A0 0x00634BE0 0x00634C20 0x00634C30 0x00634DB0 0x00634E80 0x00634ED0 0x00645460 0x00645470 0x00645660 0x00645DD0 0x00645E90
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005ABB90
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005abb90/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?save_map@@YAXXZ  at 0x005ABB90  (401 bytes)
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

typedef int BOOL;
class FileBox;
typedef void * LPVOID;
class Strings;

// ---- callees, declared and never defined (a definition would be inlined) ----
class FileBox { public:
    FileBox();
    int add_filter(char *, char *);
    int set_def_ext(char *);
    int set_def_name(char *);
    void save();
    ~FileBox();
};
class Strings { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
    BOOL is_populated_;
    int get(int);
};
extern "C" char *strcat(char *, const char *);
extern "C" char *strcpy(char *, const char *);
extern "C" int __cdecl strchr();
extern "C" int __cdecl strncpy();
extern "C" int strcmp(const char *, const char *);
int __cdecl save_map_daemon(char *);
void __cdecl do_all_draws();
void __cdecl prefs_put(char *, char *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066222a = (int *)0x0066222A;
static int *const g_0067f0d8 = (int *)0x0067F0D8;
static int *const g_00690f50 = (int *)0x00690F50;
static int *const g_00690f54 = (int *)0x00690F54;
static int *const g_00690f58 = (int *)0x00690F58;
static int *const g_00691b3c = (int *)0x00691B3C;
static int *const g_0094a2bc = (int *)0x0094A2BC;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

typedef char *(FileBox::*SaveFn)();
typedef char *(__cdecl *StrchrFn)(const char *, int);
typedef char *(__cdecl *StrncpyFn)(char *, const char *, size_t);

void __cdecl save_map() {
    FileBox box;
    box.set_def_name(reinterpret_cast<char *>(g_0094a2bc));
    box.set_def_ext(reinterpret_cast<char *>(*g_00691b3c));

    char filterBuf[0x100];
    filterBuf[0] = 0;
    strcat(filterBuf, reinterpret_cast<char *>(g_00690f50));
    strcat(filterBuf, reinterpret_cast<char *>(*g_00691b3c));

    Strings *strings = reinterpret_cast<Strings *>(g_009b90d8);
    int filterName = strings->get(
        *reinterpret_cast<int *>(reinterpret_cast<char *>(*reinterpret_cast<int **>(g_009b90f8)) + 0x678));
    box.add_filter(filterBuf, reinterpret_cast<char *>(filterName));

    char *result = (box.*reinterpret_cast<SaveFn>(&FileBox::save))();
    do_all_draws();

    if (result != 0) {
        char nameBuf[0x100];
        strcpy(nameBuf, result);
        if (strcmp(nameBuf, reinterpret_cast<char *>(g_00690f54)) == 0) {
            strcpy(nameBuf, reinterpret_cast<char *>(g_009b86a0));
        }
        char pathBuf[0x100];
        strcpy(pathBuf, nameBuf);
        char *dot = reinterpret_cast<StrchrFn>(strchr)(pathBuf, 0x2e);
        if (dot != 0) {
            *dot = 0;
        }
        reinterpret_cast<StrncpyFn>(strncpy)(reinterpret_cast<char *>(g_0094a2bc), pathBuf, 0x4b);
        prefs_put(reinterpret_cast<char *>(g_00690f58), pathBuf);
        save_map_daemon(nameBuf);
    }
}
