// ORIGINAL: 0x00636FC0 ?process@Video@@QAEXXZ 0x00636FC0-0x0063706F FILE BYTE_EXACT
// LEVER: one 32-bit `flags` load reused for the first `& 0x8000`/`& 0x10` pair (matches the original's single `mov eax` feeding both `test ah,0x80` and `test al,0x10`), fresh byte reloads for every later `& 0x10` check (matches the repeated `mov al, [esi+0x4ac]` after each intervening call), and `while (r != 0 && (r = decode_frame(0)) != 0) r = play_frame();` for the tail loop. First try, byte exact.
// working copy - scaffold materialised by --work
// size      175 bytes
// prototype void (__thiscall ?process@Video@@QAEXXZ)(Video* this)
// callers   0   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00626250 0x00637910 0x00637C30 0x00637D70
// indirect  0x00636FEC 0x00637023

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?process@Video@@QAEXXZ  at 0x00636FC0  (175 bytes)
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

class Video;

// ---- callees, declared and never defined (a definition would be inlined) ----
void log_say(char *, char *, int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669368 = (int *)0x00669368;
static int *const g_00698870 = (int *)0x00698870;
static int *const g_0069887c = (int *)0x0069887C;
static int *const g_00698894 = (int *)0x00698894;
static int *const g_006988a0 = (int *)0x006988A0;

typedef unsigned long (__stdcall *TimeGetTimeFn)();

class Video { public:
    int decode_frame(int);
    int read_frame();
    int play_frame();
    void process();
};
void Video::process() {
    char *self = reinterpret_cast<char *>(this);
    unsigned int flags = *reinterpret_cast<unsigned int *>(self + 0x4ac);
    if ((flags & 0x8000) != 0) {
        TimeGetTimeFn get_time = *reinterpret_cast<TimeGetTimeFn *>(g_00669368);
        if ((flags & 0x10) == 0) {
            if (play_frame() != 0) {
                return;
            }
            log_say(reinterpret_cast<char *>(g_0069887c), reinterpret_cast<char *>(g_00698870),
                    static_cast<int>(get_time()), 0, 0);
            if ((*reinterpret_cast<unsigned char *>(self + 0x4ac) & 0x10) == 0) {
                play_frame();
                if (decode_frame(0) != 0) {
                    return;
                }
            }
        }
        log_say(reinterpret_cast<char *>(g_006988a0), reinterpret_cast<char *>(g_00698894),
                static_cast<int>(get_time()), 0, 0);
        if ((*reinterpret_cast<unsigned char *>(self + 0x4ac) & 0x10) == 0) {
            int r = play_frame();
            while (r != 0 && (r = decode_frame(0)) != 0) {
                r = play_frame();
            }
        }
        read_frame();
    }
}
