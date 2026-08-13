// ORIGINAL: 0x00614140 FILE
// working copy - scaffold materialised by --work
// name      ?add_def_ext@FileWin@@QAEHPAD@Z
// size      124 bytes
// spans     0x00614140-0x006141BC
// prototype int (__thiscall ?add_def_ext@FileWin@@QAEHPAD@Z)(FileWin* this, int8*)
// callers   3   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006453E0 0x00645470

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?add_def_ext@FileWin@@QAEHPAD@Z  at 0x00614140  (124 bytes)
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

class FileWin;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" char *strcat(char *, const char *);
extern "C" unsigned int strlen(const char *);

class FileWin { public:
    uint8_t unmapped_0_[0x208];
    uint8_t field_208_;
    uint8_t unmapped_209_[0x30C - 0x209];
    uint8_t field_30C_;
    uint8_t unmapped_30D_[0x410 - 0x30D];
    uint8_t field_410_;
    uint8_t unmapped_411_[0x514 - 0x411];
    int32_t field_514_;
    uint8_t unmapped_518_[0x531 - 0x518];
    uint8_t field_531_;
    uint8_t field_532_[0x106];
    uint32_t field_638_;
    uint32_t field_63C_;
    uint32_t field_640_;
    uint32_t field_644_;
    uint32_t field_648_;
    uint32_t field_64C_;
    uint32_t field_650_;
    uint32_t field_654_;
    uint32_t field_658_;
    uint32_t field_65C_;
    uint32_t field_660_;
    uint8_t field_664_[0xB48];
    uint32_t field_11AC_;
    uint8_t field_11B0_[0x440];
    uint32_t field_15F0_;
    uint8_t field_15F4_[0x704];
    uint32_t field_1CF8_;
    uint8_t field_1CFC_[0x440];
    uint32_t field_213C_;
    uint8_t field_2140_[0x6CC];
    uint32_t field_280C_;
    uint8_t field_2810_[0x28];
    uint32_t field_2838_;
    uint32_t field_283C_;
    uint8_t field_2840_[0x2C];
    uint32_t field_286C_;
    uint8_t field_2870_[0x4];
    uint32_t field_2874_;
    uint8_t field_2878_[0x24];
    uint32_t field_289C_;
    uint8_t field_28A0_[0xB20];
    uint32_t field_33C0_;

    int add_def_ext(char *);
};
int FileWin::add_def_ext(char * a1) {
    if (field_514_ == 4) {
        return 1;
    }
    if (a1 == 0) {
        return 0x10;
    }
    char *self = reinterpret_cast<char *>(this);
    char *slot = self + 0x518 + field_514_ * 5;
    *slot = 0;
    if (strlen(a1) > 4) {
        return 3;
    }
    strcat(slot, a1);
    field_514_ = field_514_ + 1;
    return 0;
}
