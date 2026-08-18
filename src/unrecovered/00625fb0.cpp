// ORIGINAL: 0x00625FB0 ??0Log@@QAE@XZ 0x00625FB0-0x00625FC0 FILE BYTE_EXACT
// LEVER: field stores in declaration order, matching the disassembly's store order directly
// working copy - scaffold materialised by --work
// size      16 bytes
// prototype Log* (__thiscall ??0Log@@QAE@XZ)(Log* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned constructor

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??0Log@@QAE@XZ  at 0x00625FB0  (16 bytes)
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

class Log { public:
    LPSTR log_file_;
    BOOL is_disabled_;

    Log();
};
Log::Log() {
    log_file_ = 0;
    is_disabled_ = 0;
}
