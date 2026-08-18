// ORIGINAL: 0x00618EE0 ??1Font@@QAE@XZ 0x00618EE0-0x00618F2D FILE BYTE_EXACT
// working copy - scaffold materialised by --work
// size      77 bytes
// prototype void (__thiscall ??1Font@@QAE@XZ)(Font* this)
// callers   53   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00644EF2
// indirect  0x00618F00 0x00618F11
// notes     Staged hybrid export redirect calls the source-owned destructor

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??1Font@@QAE@XZ  at 0x00618EE0  (77 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" void free(void *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669044 = (int *)0x00669044;
static int *const g_00669058 = (int *)0x00669058;

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

    ~Font();
};

typedef int (__stdcall *DeleteFontProc)(HFONT);
typedef int (__stdcall *FreeFotProc)(LPSTR);

Font::~Font() {
    unk_1_ = -1;
    height_ = 0;
    line_height_ = 0;
    ascent_ = 0;
    descent_ = 0;
    if (font_obj_) {
        (*reinterpret_cast<DeleteFontProc *>(g_00669058))(font_obj_);
        font_obj_ = 0;
    }
    if (fot_file_name_) {
        (*reinterpret_cast<FreeFotProc *>(g_00669044))(fot_file_name_);
        if (fot_file_name_) {
            free(fot_file_name_);
        }
        fot_file_name_ = 0;
    }
}
