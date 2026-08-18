// ORIGINAL: 0x004B4990 ?UNK1@StatusWin@@QAEXXZ 0x004B4990-0x004B4A1E FILE BYTE_EXACT
// working copy - scaffold materialised by --work
// size      142 bytes
// prototype void (__thiscall ?UNK1@StatusWin@@QAEXXZ)(StatusWin* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00618F40

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?UNK1@StatusWin@@QAEXXZ  at 0x004B4990  (142 bytes)
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
class Font;
typedef void * HFONT;
typedef char * LPSTR;
class StatusWin;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Font { public:
    int unk_1_;
    BOOL is_fot_set_;
    HFONT font_obj_;
    int line_height_;
    int height_;
    int internal_leading_;
    int ascent_;
    int descent_;
    int pad_;
    LPSTR fot_file_name_;
    int init(char *, int, int);
};


// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00691b2c = (int *)0x00691B2C;
static int *const g_009b7b1c = (int *)0x009B7B1C;

class StatusWin { public:
    void UNK1();
};
void StatusWin::UNK1() {
    char *self = reinterpret_cast<char *>(this);
    int size = (*g_009b7b1c != 800) ? 12 : 10;
    if (size != *reinterpret_cast<int *>(self + 0x15ac)) {
        *reinterpret_cast<int *>(self + 0x15ac) = size;
        reinterpret_cast<Font *>(self + 0x1530)->init((char *)*g_00691b2c, size, 0);
        reinterpret_cast<Font *>(self + 0x1558)->init((char *)*g_00691b2c, size, 2);
        reinterpret_cast<Font *>(self + 0x1580)->init((char *)*g_00691b2c, size, 1);
    }
    if (*g_009b7b1c != 800) {
        *reinterpret_cast<int *>(self + 0x1530) = 4;
        *reinterpret_cast<int *>(self + 0x1558) = 4;
        *reinterpret_cast<int *>(self + 0x1580) = 4;
    }
}
