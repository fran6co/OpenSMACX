// ORIGINAL: 0x00625FC0 FILE
// RULED-OUT: direct transcription only tried once; diverges at #1 (push vs
//            mov) in the prologue register-save pattern. fclose is called
//            through a function-pointer cast since its scaffold declaration
//            takes no parameters
// working copy - scaffold materialised by --work
// name      ??0Log@@QAE@PAD@Z
// size      86 bytes
// spans     0x00625FC0-0x00626016
// prototype Log* (__thiscall ??0Log@@QAE@PAD@Z)(Log* this, int8*)
// callers   0   call targets   5
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D4510 0x00634BB0 0x006453E0 0x00645470 0x00645598
// notes     Staged hybrid export redirect calls the source-owned filename constructor

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??0Log@@QAE@PAD@Z  at 0x00625FC0  (86 bytes)
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
typedef char * LPSTR;
class Log;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" char *strcat(char *, const char *);
extern "C" int __cdecl fclose();
extern "C" unsigned int strlen(const char *);
int env_open(char *, char *);
void * mem_get(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00697380 = (int *)0x00697380;

class Log { public:
    LPSTR log_file_;
    BOOL is_disabled_;

    Log(int8*);
};
Log::Log(int8* a1) {
    log_file_ = 0;
    if (a1) {
        void *buf = mem_get(strlen(a1) + 1);
        log_file_ = (LPSTR)buf;
        if (buf) {
            *(char *)buf = 0;
            strcat(log_file_, a1);
            int env = env_open(a1, (char *)g_00697380);
            if (env) {
                ((int (__cdecl *)(int))fclose)(env);
            }
        }
    }
}
