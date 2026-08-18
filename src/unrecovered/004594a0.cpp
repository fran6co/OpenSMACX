// ORIGINAL: 0x004594A0 ??1InfoWin@@QAE@XZ 0x004594A0-0x004594FB;0x00655240-0x00655260 FILE
// working copy - scaffold materialised by --work
// size      123 bytes
// prototype void (__thiscall ??1InfoWin@@QAE@XZ)(InfoWin* this)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00616200 0x00618EE0 0x0062C010

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??1InfoWin@@QAE@XZ  at 0x004594A0  (123 bytes)
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
class InfoWin;
typedef char * LPSTR;
class PushButton;
class Time;
typedef unsigned int UINT_PTR;

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
    ~Font();
};

class PushButton { public:
    ~PushButton();
};

class Time { public:
    int unk_tgl_;
    UINT_PTR id_event_;
    void * callback1_;
    void * callback2_;
    int cb_param2_;
    int cb_param1_;
    uint32_t count_;
    int unk_1_;
    uint32_t resolution_;
    int unk_2_;
    ~Time();
};


// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00655256 = (int *)0x00655256;
static int *const g_00675488 = (int *)0x00675488;

class InfoWin { public:
    ~InfoWin();
};
InfoWin::~InfoWin() {
    char *self = reinterpret_cast<char *>(this);
    reinterpret_cast<PushButton *>(self + 0x9d0)->~PushButton();
    reinterpret_cast<Font *>(self + 0x58)->~Font();
    reinterpret_cast<Time *>(self + 0x30)->~Time();
}
