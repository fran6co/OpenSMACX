// ORIGINAL: 0x0051D9F0 ??1Console@@QAE@XZ 0x0051D9F0-0x0051DC10;0x0065D0E0-0x0065D220 FILE
// size      864 bytes
// prototype void (__thiscall ??1Console@@QAE@XZ)(Console* this)
// callers   2   call targets   10
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00462870 0x0051D7D0 0x005D4DD0 0x005D7410 0x005E3820 0x005FAD00 0x00616200 0x00618EE0 0x006252B0 0x006456E4
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0051D9F0
// measured tier  MISMATCH
// divergence     7
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0051d9f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??1Console@@QAE@XZ  at 0x0051D9F0  (864 bytes)
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
class Console;
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
class MapWin;
class Menu;
typedef void * PVOID;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class TextureStore;
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

class Spot { public:
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
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
class Buffer { public:
    LPVOID vtable_;
    uint32_t poOwner_;
    uint32_t field_8_;
    uint32_t field_C_;
    uint32_t field_10_;
    uint32_t field_14_;
    uint32_t field_18_;
    uint32_t field_1C_;
    RECT rect1_;
    RECT rect2_;
    uint32_t field_40_[4];
    uint32_t field_50_;
    LPVOID * ppv_bits_;
    uint32_t field_58_;
    uint32_t field_5C_;
    HDC hdc2_;
    HDC hdc_;
    uint32_t field_68_;
    uint32_t field_6C_;
    HRGN field_70_;
    uint32_t field_74_;
    HBITMAP bitmap_handle_;
    const BITMAPINFO * bitmap_info_;
    uint32_t width_;
    uint32_t height_;
    uint16_t field_88_;
    uint16_t field_8A_;
    uint32_t field_8C_;
    uint32_t field_90_;
    uint32_t field_94_;
    uint32_t field_98_;
    uint32_t field_9C_;
    uint32_t field_A0_;
    int32_t dib_[256];
    uint32_t field_4A4_;
    uint32_t field_4A8_;
    uint32_t field_4AC_;
    Spot spot_;
    uint8_t field_4BC_[80];
    uint32_t field_50C_;
    uint32_t field_510_;
    uint32_t field_514_;
    uint32_t field_518_;
    uint32_t field_51C_;
    uint32_t field_520_;
    uint32_t field_524_;
    uint32_t field_528_;
    Font * font1_;
    Font * font2_;
    Font * font3_;
    Font * font4_;
    uint32_t color_val_1_;
    uint32_t color_2_val_1_;
    uint32_t color_3_val_1_;
    uint32_t color_hyper_val_1_;
    uint32_t color_val_2_;
    uint32_t color_2_val_2_;
    uint32_t color_3_val_2_;
    uint32_t color_hyper_val_2_;
    uint32_t color_val_3_;
    uint32_t color_2_val_3_;
    uint32_t color_3_val_3_;
    uint32_t color_hyper_val_3_;
    uint32_t color_val_4_;
    uint32_t color_2_val_4_;
    uint32_t color_3_val_4_;
    uint32_t color_hyper_val_4_;
    uint32_t field_57C_;
    int8_t field_580_;
    uint32_t field_584_;
    ~Buffer();
};
class GraphicWin { public:
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
    Buffer buffer_;
    uint32_t field_9CC_;
    uint32_t field_9D0_;
    uint32_t field_9D4_;
    uint32_t field_9D8_;
    uint32_t field_9DC_;
    uint32_t field_9E0_;
    uint32_t field_9E4_;
    uint32_t field_9E8_;
    uint32_t field_9EC_;
    uint32_t field_9F0_;
    uint32_t field_9F4_;
    uint32_t field_9F8_;
    uint32_t field_9FC_;
    uint32_t field_A00_;
    uint32_t field_A04_;
    uint32_t poCanvas_;
    uint32_t field_A0C_;
    uint32_t field_A10_;
    ~GraphicWin();
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
    ~Font();
};
class MapWin { public:
    uint32_t vbtable_pointer_;
    void * owned_;
    uint8_t derived_tail_[0x21A6C - 0x8];
    GraphicWin virtual_base_;
    void clear(int);
};
class Menu { public:
    ~Menu();
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
    void close();
};
class TextureStore { public:
    ~TextureStore();
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
void __stdcall fn_006456e4(void*, unsigned int, int, void (*)(void*));

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00406850 = (int *)0x00406850;
static int *const g_006252b0 = (int *)0x006252B0;
static int *const g_00625310 = (int *)0x00625310;
static int *const g_0065d216 = (int *)0x0065D216;
static int *const g_0066a574 = (int *)0x0066A574;
static int *const g_0066a57c = (int *)0x0066A57C;
static int *const g_0066ec10 = (int *)0x0066EC10;
static int *const g_0066ec18 = (int *)0x0066EC18;
static int *const g_0067b7e0 = (int *)0x0067B7E0;

class Console { public:
    uint8_t derived_storage_[0x23D94];
    GraphicWin virtual_base_;

    void close();
    ~Console();
};

Console::~Console() {
    char *self = reinterpret_cast<char *>(this);

    // Virtual-base vtable-slot fixups: each is a vbtable dereference
    // (load the vptr, read its second entry as a this-adjustment, store
    // through the adjusted address) reproduced literally from the disasm.
    char *vbase = self - 0x23d94;
    int32_t vtbl_a = *reinterpret_cast<int32_t *>(vbase);
    int32_t off_a = *reinterpret_cast<int32_t *>(vtbl_a + 4);
    *reinterpret_cast<int32_t *>(vbase + off_a) = *g_0066ec18;

    int32_t vtbl_b = *reinterpret_cast<int32_t *>(vbase);
    int32_t off_b = *reinterpret_cast<int32_t *>(vtbl_b + 4);
    *reinterpret_cast<int32_t *>(self - 0x23950 + off_b) = *g_0066ec10;

    int32_t vtbl_c = *reinterpret_cast<int32_t *>(vbase);
    int32_t off_c = *reinterpret_cast<int32_t *>(vtbl_c + 4);
    *reinterpret_cast<int32_t *>(self - 0x23d98 + off_c) = off_c - 0x23d94;

    close();

    reinterpret_cast<Sprite *>(self - 0x40)->close();
    reinterpret_cast<Sprite *>(self - 0x6c)->close();

    char *edi = self - 0x1368;
    fn_006456e4(edi + 0x1118, 0x2c, 3,
                reinterpret_cast<void (*)(void *)>(g_00406850));
    reinterpret_cast<Sprite *>(edi + 0x10ec)->close();
    reinterpret_cast<Buffer *>(edi + 0xb64)->~Buffer();
    reinterpret_cast<Menu *>(edi)->~Menu();

    reinterpret_cast<Time *>(self - 0x1390)->~Time();
    reinterpret_cast<Buffer *>(self - 0x1918)->~Buffer();
    reinterpret_cast<GraphicWin *>(self - 0x232c)->~GraphicWin();

    self += 0xffffdcd8;  // -0x2328: rebase onto the MapWin virtual base

    char *vbase2 = self - 0x21a6c;
    int32_t vtbl2_a = *reinterpret_cast<int32_t *>(vbase2);
    int32_t off2_a = *reinterpret_cast<int32_t *>(vtbl2_a + 4);
    *reinterpret_cast<int32_t *>(vbase2 + off2_a) = *g_0066a57c;

    int32_t vtbl2_b = *reinterpret_cast<int32_t *>(vbase2);
    int32_t off2_b = *reinterpret_cast<int32_t *>(vtbl2_b + 4);
    *reinterpret_cast<int32_t *>(self - 0x21628 + off2_b) = *g_0066a574;

    int32_t vtbl2_c = *reinterpret_cast<int32_t *>(vbase2);
    int32_t off2_c = *reinterpret_cast<int32_t *>(vtbl2_c + 4);
    *reinterpret_cast<int32_t *>(self - 0x21a70 + off2_c) = off2_c - 0x21a6c;

    reinterpret_cast<MapWin *>(self)->clear(0);

    fn_006456e4(self - 0x2b18, 0xabc, 4,
                reinterpret_cast<void (*)(void *)>(g_00625310));

    reinterpret_cast<Font *>(self - 0x2b4c)->~Font();
    reinterpret_cast<Font *>(self - 0x2b78)->~Font();
    reinterpret_cast<Font *>(self - 0x2ba4)->~Font();
    reinterpret_cast<Buffer *>(self - 0x3134)->~Buffer();
    reinterpret_cast<Buffer *>(self - 0x36bc)->~Buffer();
    reinterpret_cast<Buffer *>(self - 0x3c44)->~Buffer();
    reinterpret_cast<TextureStore *>(self - 0x3f60)->~TextureStore();

    fn_006456e4(self - 0x210e0, 0x260, 0xc4,
                reinterpret_cast<void (*)(void *)>(g_006252b0));

    self += 0xfffde5a0;  // rebase again before the final array teardown
    fn_006456e4(self, 0x260, 4,
                reinterpret_cast<void (*)(void *)>(g_006252b0));
}
