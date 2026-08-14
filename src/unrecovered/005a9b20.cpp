// ORIGINAL: 0x005A9B20 FILE
// working copy - scaffold materialised by --work
// name      ?save_map_daemon@@YAHPAD@Z
// size      229 bytes
// spans     0x005A9B20-0x005A9C05
// prototype 
// callers   1   call targets   8
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0057D240 0x005A9280 0x00634BB0 0x00645470 0x00645598 0x00645DD0 0x0064603F 0x00646950

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?save_map_daemon@@YAHPAD@Z  at 0x005A9B20  (229 bytes)
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

struct FILE;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" char *strcat(char *, const char *);
extern "C" int __cdecl _fwrite(void*, size_t, size_t, FILE*);
extern "C" int __cdecl fclose();
extern "C" int __cdecl remove();
extern "C" int __cdecl strchr();
int env_open(char *, char *);
int map_data(FILE *, int, int);
void header_write(char *, FILE *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00690da4 = (int *)0x00690DA4;
static int *const g_00690da8 = (int *)0x00690DA8;
static int *const g_00690dac = (int *)0x00690DAC;
static int *const g_00691b3c = (int *)0x00691B3C;
extern "C" char *strchr_real(const char *, int);
extern "C" int fclose_real(FILE *);
extern "C" int remove_real(const char *);

int __cdecl save_map_daemon(char * a1) {
    char localBuf[256];
    int result = 1;
    localBuf[0] = 0;
    strcat(localBuf, a1);
    if (strchr_real(localBuf, '.') == 0) {
        strcat(localBuf, reinterpret_cast<char *>(g_00690da4));
        strcat(localBuf, reinterpret_cast<char *>(*g_00691b3c));
    }
    FILE *f = reinterpret_cast<FILE *>(env_open(localBuf, reinterpret_cast<char *>(g_00690da8)));
    if (f != 0) {
        result = 2;
        header_write(reinterpret_cast<char *>(g_00690dac), f);
        a1 = reinterpret_cast<char *>(5);
        int n = _fwrite(&a1, 4, 1, f);
        if (n != 0) {
            result = 3;
            int r = map_data(f, 1, 0);
            if (r == 0) {
                result = 0;
            }
        }
    }
    fclose_real(f);
    if (result != 0) {
        remove_real(localBuf);
    }
    return result;
}
