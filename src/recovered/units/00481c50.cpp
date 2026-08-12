// ORIGINAL: 0x00481C50 BYTE_EXACT FILE
// name      ??0NetWin@@QAE@XZ
// size      272 bytes
// spans     0x00481C50-0x00481D08;0x00657491-0x006574E9
// prototype void (__thiscall ??0NetWin@@QAE@XZ)(NetWin* this)
// callers   1   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D4CF0 0x005FA860 0x00607CF0 0x0060E670 0x00614E50 0x00629110
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00481C50
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00481c50/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??0NetWin@@QAE@XZ  at 0x00481C50  (272 bytes)
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
class BaseButton;
class Buffer;
class ButtonGroup;
class CheckBox;
class EditBox;
class FlatButton;
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
class NetWin;
typedef void * PVOID;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class StringBox;
class Time;
typedef unsigned int UINT_PTR;
class Win;

class AutoSound { public:
    PVOID vtable_;
    int val_1_;
    int val_2_;
    int val_3_;
    int val_4_;
    int val_5_;
    int val_6_;
    int val_7_;
    int val_8_;
    int val_9_;
    int val_10_;
    int val_11_;
    int val_12_;
    int val_13_;
    int val_14_;
    int val_15_;
    int val_16_;
    int val_17_;
    int val_18_;
    int val_19_;
    int val_20_;
    int val_21_;
    int val_22_;
    int val_23_;
    int val_24_;
    int val_25_;
    int val_26_;
    int val_27_;
    int val_28_;
    int val_29_;
    int val_30_;
    int val_31_;
    int val_32_;
    int val_33_;
    int val_34_;
    int val_35_;
    int val_36_;
    int val_37_;
};

class Heap { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
};

struct RECT {
    long left;
    long top;
    long right;
    long bottom;
};

// Buffer's full layout is not needed here: GraphicWin and FlatButton are
// only ever passed to external constructors below (never accessed by
// field), so they stay opaque and Buffer (which embedded `Spot spot_` by
// value, forcing Spot complete before it - a pre-existing scaffold
// ordering bug, C2079 otherwise) is dropped rather than reordered.
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
};

// BaseButton's full layout is unused after GraphicWin/FlatButton went
// opaque; ButtonGroup only holds `BaseButton *`, which needs no
// complete type.

class ButtonGroup { public:
    BaseButton * buttons_[32];
    uint32_t count_;
    uint32_t field_84_;
    uint32_t field_88_;
    uint32_t field_8C_;
    uint32_t field_90_;
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
};

class Sprite { public:
    int ppszFileName_;
    int pcBits_;
    char cTransparentIndex_;
    char pad1_;
    char pad2_;
    char pad3_;
    int iSpriteWidth2_;
    int iSpriteWidth_;
    int iSpriteHeight_;
    int iWidth_;
    int iHeight_;
    int iLeftOffset_;
    int iTopOffset_;
    int fObj1Exists_;
};

class Win { public:
    AutoSound auto_sound_;
    uint32_t iFlags_;
    uint32_t iSomeFlag_;
    uint32_t field_A0_;
    uint32_t field_A4_;
    uint32_t poWinBase_;
    uint32_t iVertScaleDenom_;
    uint32_t iVertScaleNum_;
    Buffer * buffer1_;
    Buffer * buffer2_;
    Buffer * buffer3_;
    Buffer * buffer4_;
    Win * win_parent_;
    uint32_t field_C8_;
    uint32_t field_CC_;
    uint32_t field_D0_;
    uint32_t field_D4_;
    uint32_t field_D8_;
    Heap heap_;
    Menu * menu_;
    uint32_t field_F4_;
    uint32_t field_F8_;
    uint32_t field_FC_;
    uint32_t field_100_;
    uint32_t field_104_;
    uint32_t field_108_;
    uint32_t field_10C_;
    uint32_t field_110_;
    int caption_height_;
    int border_thickness_;
    int bottom_border_thickness_;
    uint32_t field_120_;
    uint32_t field_124_;
    uint32_t field_128_;
    uint32_t field_12C_;
    uint32_t field_130_;
    uint32_t field_134_;
    uint32_t field_138_;
    RECT outer_rect_;
    RECT client_rect_;
    uint32_t field_15C_;
    uint32_t field_160_;
    uint32_t field_164_;
    uint32_t field_168_;
    uint32_t field_16C_;
    uint32_t field_170_;
    uint32_t field_174_;
    uint32_t field_178_;
    uint32_t field_17C_;
    uint32_t field_180_;
    uint32_t field_184_;
    Sprite * cursor_sprite_;
    uint32_t field_18C_;
    uint32_t field_190_;
    HCURSOR * cursor_handle_;
    int cursor_name_;
    uint32_t field_19C_;
    uint32_t field_1A0_;
    Win * children_[150];
    int child_count_;
    uint32_t field_400_;
    uint32_t field_404_;
    uint32_t field_408_;
    uint32_t field_40C_;
    uint32_t field_410_;
    uint32_t field_414_;
    uint32_t field_418_;
    uint32_t field_41C_;
    uint32_t field_420_;
    uint32_t field_424_;
    uint32_t field_428_;
    uint32_t field_42C_;
    uint32_t field_430_;
    uint32_t field_434_;
    uint32_t field_438_;
    Scroll * scroll_vert_;
    Scroll * scroll_horz_;
};

// ---- callees, declared and never defined (a definition would be inlined) ----
//
// GraphicWin and FlatButton were emitted with their full field lists, but
// NetWin's constructor never touches a field of either - it only default-
// constructs them as subobjects and, right after, patches the vtable
// pointers those constructors already left behind (see the body). Their
// real per-field layout is therefore not load-bearing here, and dropping
// it sidesteps a pre-existing scaffold bug: with the full field list,
// GraphicWin/FlatButton embed `Buffer buffer_` BY VALUE, and Buffer
// embeds `Spot spot_` BY VALUE - but the emitter placed Buffer's
// definition before Spot's, so ANY body against this address failed
// NO_COMPILE (C2079 'spot_' uses undefined class 'Spot'), independent of
// what was written into `NetWin::NetWin`. Confirmed by feeding a trivial
// `NetWin::NetWin() {}` through the emitted scaffold unchanged: same error,
// same line. GraphicWin/FlatButton opaque removes the only two BY-VALUE
// uses of Buffer, so Buffer's (and BaseButton's, AutoSound-adjacent, all
// now-unused) definitions were dropped rather than reordered.
//
// A destructor is declared (never defined - like the constructor) for
// every subobject type. The original protects each already-constructed
// member with a `push -1 / push handler / mov fs:[0]` frame that the SEH
// funclets (0x00657491-0x006574E4, an EH-unwind-funclet cluster, excluded
// from the byte comparison the same way every 0x0065xxxx/0x0066xxxx
// funclet is) tear down on exception. A type that this translation unit
// believes has a TRIVIAL destructor gives the compiler nothing to call on
// that path and it drops the frame entirely - so every subobject
// constructed here needs a declared (non-trivial-by-declaration) dtor
// purely to keep that frame in the PRIMARY span, which IS compared.
class CheckBox { public:
    CheckBox(int);
    ~CheckBox();
};
class EditBox { public:
    EditBox();
    ~EditBox();
};
class FlatButton { public:
    FlatButton();
    ~FlatButton();
};
class GraphicWin { public:
    GraphicWin();
    ~GraphicWin();
};
class Spot { public:
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
    Spot();
    ~Spot();
};
class StringBox { public:
    StringBox();
    ~StringBox();
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006574df = (int *)0x006574DF;
static int *const g_0066cce4 = (int *)0x0066CCE4;
static int *const g_0066ccec = (int *)0x0066CCEC;
static int *const g_00676fb8 = (int *)0x00676FB8;

// Real inheritance and real (default-constructed) members, not placement
// new at each offset: a placement-new onto a RUNTIME address (`this` or
// `this + N`) compiles to `cmp p,0 / je SKIP / call ctor` under every flag
// combination tried here - a guard the original does not have, since
// `call GraphicWin::GraphicWin()` etc. are plain unconditional calls.
// Confirmed as a standing wall on this exact shape by two prior
// recoveries against their own committed originals
// (build/byte-match/00633750 - CheckButton, and build/byte-match/005ffd80
// - MCIVideo); real subobject construction sidesteps it entirely, the way
// build/byte-match/004ba6b0 (TutWin : public Popup) already established
// for a single empty base. GraphicWin is the base (offset 0, matching
// `mov esi,ecx` then `call` with no `lea`); Spot/StringBox/EditBox/
// CheckBox/FlatButton/FlatButton are members in construction order,
// separated by literal padding computed from the disassembly's `lea ecx,
// [esi+N]` operands rather than from any class's guessed size - Spot is
// the one exception, its 3-field, 0xC layout already lands the following
// member (StringBox) at exactly 0xD40 with zero padding, so it is left
// real rather than papered over.
class NetWin : public GraphicWin { public:
    char pad_to_spot_[0xD34];
    Spot spot_;
    StringBox string_box_;
    char pad_to_editbox_[0x38E0 - 0xD40 - 1];
    EditBox edit_box_;
    char pad_to_checkbox_[0x4454 - 0x38E0 - 1];
    CheckBox check_box_;
    char pad_to_flatbutton1_[0x4F7C - 0x4454 - 1];
    FlatButton flat_button1_;
    char pad_to_flatbutton2_[0x5AC8 - 0x4F7C - 1];
    FlatButton flat_button2_;

    NetWin();
};

NetWin::NetWin() : check_box_(1) {
    char *self = reinterpret_cast<char *>(this);
    *reinterpret_cast<void **>(self) = g_0066ccec;
    *reinterpret_cast<void **>(self + 0x444) = g_0066cce4;
    *reinterpret_cast<int *>(self + 0x772C) = 0;
    *reinterpret_cast<int *>(self + 0x7730) = 0;
}
