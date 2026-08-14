// ORIGINAL: 0x004BA1A0 FILE
// RULED-OUT: real Caviar/Font/Spot members on StatusWin is C2011 against the
//            regenerated scaffold (StatusWin already declared opaque).
//            placement-new + try{}catch(...){throw;} (WorldWin's 0x004C4BF0
//            pattern) recovers the /GX frame shape but not the per-subobject
//            unwind dispatch (4 label thunks) real member layout would give.
// working copy - scaffold materialised by --work
// name      ??0StatusWin@@QAE@XZ
// size      182 bytes
// spans     0x004BA1A0-0x004BA217;0x00659A60-0x00659A9F
// prototype void (__thiscall ??0StatusWin@@QAE@XZ)(StatusWin* this)
// callers   1   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005FA860 0x00616DA0 0x00618EA0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??0StatusWin@@QAE@XZ  at 0x004BA1A0  (182 bytes)
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
class Caviar;
class Font;
typedef void * HFONT;
typedef char * LPSTR;
struct RECT;
class Spot;
class StatusWin;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Caviar { public:
    float scene_scale_;
    uint32_t field_4_;
    uint8_t field_8_[0xC];
    uint32_t field_14_;
    uint8_t field_18_[0x4];
    uint32_t field_1C_;
    uint32_t field_20_;
    uint32_t field_24_;
    uint32_t field_28_;
    int32_t field_2C_;
    int32_t field_30_;
    int32_t field_34_;
    uint32_t field_38_;
    uint32_t field_3C_;
    uint32_t field_40_;
    uint32_t field_44_;
    uint32_t field_48_;
    uint32_t field_4C_;
    uint32_t field_50_;
    uint32_t field_54_;
    uint32_t field_58_;
    uint32_t field_5C_;
    uint32_t field_60_;
    uint32_t field_64_;
    uint32_t field_68_;
    float field_6C_;
    float field_70_;
    uint32_t field_74_;
    uint32_t field_78_;
    uint32_t field_7C_;
    uint32_t field_80_;
    uint32_t field_84_;
    uint32_t field_88_;
    uint32_t field_8C_;
    uint32_t field_90_;
    uint32_t field_94_;
    uint32_t field_98_;
    uint32_t field_9C_;
    uint32_t field_A0_;
    uint8_t field_A4_;
    uint8_t field_A5_[0x63];
    int32_t field_108_;
    uint8_t field_10C_[0x640];
    uint32_t field_74C_;
    uint32_t field_750_;
    uint32_t field_754_;
    uint8_t field_758_[0xC74];
    uint32_t field_13CC_;
    Caviar();
};

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
    Font();
};

struct RECT {
    long left;
    long top;
    long right;
    long bottom;
};

class Spot { public:
    struct SpotInternal {
        RECT rect;
        int type;
        int position;
    };
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
    Spot();
};


// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00659a95 = (int *)0x00659A95;
static int *const g_00678d00 = (int *)0x00678D00;

class StatusWin { public:
    StatusWin();
};
inline void *operator new(unsigned int, void *place) { return place; }

StatusWin::StatusWin() {
    char *self = reinterpret_cast<char *>(this);
    new (reinterpret_cast<void *>(self + 0x30)) Caviar();
    try {
        new (reinterpret_cast<void *>(self + 0x1530)) Font();
        new (reinterpret_cast<void *>(self + 0x1558)) Font();
        new (reinterpret_cast<void *>(self + 0x1580)) Font();
        new (reinterpret_cast<void *>(self + 0x15E0)) Spot();
    } catch (...) {
        throw;
    }
}
