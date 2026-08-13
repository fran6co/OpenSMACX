// ORIGINAL: 0x00618EC0 BYTE_EXACT FILE
// LEVER: plain forwarding call to init(a1, a2, a3) in argument order
// working copy - scaffold materialised by --work
// name      ??0Font@@QAE@PADHH@Z
// size      31 bytes
// spans     0x00618EC0-0x00618EDF
// prototype void (__thiscall ??0Font@@QAE@PADHH@Z)(Font* this, LPSTR fontName, int height, int style)
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00618F40
// notes     Staged hybrid export redirect calls the source-owned constructor

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??0Font@@QAE@PADHH@Z  at 0x00618EC0  (31 bytes)
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
    Font(LPSTR, int, int);
};
Font::Font(LPSTR a1, int a2, int a3) {
    init(a1, a2, a3);
}
