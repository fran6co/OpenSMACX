// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00428550
// name           ??1CouncWin@@QAE@XZ
// size           312 bytes
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00428550/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??1CouncWin@@QAE@XZ  at 0x00428550  (312 bytes)
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

class AutoSound;
struct BITMAPINFO;
typedef int BOOL;
class Buffer;
class CouncWin;
class Font;
class GraphicWin;
typedef void * HBITMAP;
struct HCURSOR;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
class Heap;
typedef char * LPSTR;
typedef void * LPVOID;
class Menu;
typedef void * PVOID;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class Win;

// AutoSound/RECT/Buffer/Heap/Sprite/Win are dropped: this destructor never
// touches a field of GraphicWin (it only default-destroys it as a
// subobject and, before that, tears down its own members by offset), so
// GraphicWin goes opaque below - the same move as the NetWin constructor
// (build/byte-match/00481c50) - and every type only GraphicWin's full
// field list needed becomes unused. That sidesteps a pre-existing
// scaffold bug: Buffer embeds `Spot spot_` BY VALUE and this emitter
// placed Buffer's definition before Spot's real one, so any body here
// failed NO_COMPILE (C2079 'spot_' uses undefined class 'Spot')
// independent of what was written into `CouncWin::~CouncWin` - confirmed
// by feeding a trivial `CouncWin::~CouncWin() {}` through the scaffold
// unchanged.

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
// Opaque (constructor unused here, but declared for symmetry with the
// destructor - this destructor never touches a field of GraphicWin, only
// default-destroys it as a subobject after tearing down its own members
// by offset).
class GraphicWin { public:
    GraphicWin();
    ~GraphicWin();
};
class Spot { public:
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
    ~Spot();
};
// FlatButton is likewise opaque: `buttons_` is destroyed as a real array
// member so the compiler emits its OWN call to the vector-destructor
// iterator (`??_M@YGXPAXIHP6EX0@Z@Z`) - the exact same runtime helper the
// original calls, since this is compiled with the same cl 12.00.8168.
// `pad_` exists purely so `sizeof(FlatButton)` is the 0xB4C the disassembly's
// element-size argument and the two consecutive-constructor offsets in
// build/byte-match/00481c50 (0x4F7C, 0x5AC8) both independently proved.
class FlatButton { public:
    char pad_[0xB4C];
    ~FlatButton();
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00406880 = (int *)0x00406880;
static int *const g_00652caa = (int *)0x00652CAA;
static int *const g_00673698 = (int *)0x00673698;

// Real inheritance and real members, not explicit pseudo-destructor calls
// on `this + N`: writing this as a chain of `reinterpret_cast<Font *>(self
// + N)->~Font();` calls is ordinary sequential code with no exception
// relationship the compiler can see between them, so it does not emit the
// `push -1 / push handler / mov fs:[0]` frame the original has (the
// funclets it protects, 0x00652C40-0x00652CAA, are an EH-unwind-funclet
// cluster excluded from the byte comparison the same way every
// 0x0065xxxx/0x0066xxxx funclet is, but that PROLOGUE sits in the primary
// span, which IS compared). Real subobjects sidestep this the same way
// build/byte-match/00481c50 (NetWin's constructor) does for the
// mirror-image case: the compiler generates the frame itself once every
// member has a declared (non-trivial-by-declaration) destructor, and an
// implicit `~CouncWin` body needs no statements of its own - the member
// teardown chain (reverse declaration order: buttons_, the six Fonts,
// spot_, then the GraphicWin base) already reproduces the disassembly.
class CouncWin : public GraphicWin { public:
    char pad_to_spot_[0xA48];
    Spot spot_;
    char pad_to_font1_[0xAB4 - 0xA48 - sizeof(Spot)];
    Font font1_;
    Font font2_;
    Font font3_;
    Font font4_;
    Font font5_;
    Font font6_;
    char pad_to_buttons_[0xD94 - 0xB7C - sizeof(Font)];
    FlatButton buttons_[6];

    ~CouncWin();
};

CouncWin::~CouncWin() {
}
