// ORIGINAL: 0x00407410 FILE
// name      ?init@AlphaSave@@QAEXXZ
// size      1291 bytes
// spans     0x00407410-0x00407906;0x00650C60-0x00650C75
// prototype void (__thiscall ?init@AlphaSave@@QAEXXZ)(AlphaSave* this)
// callers   3   call targets   11
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D4EF0 0x005D5250 0x005D7210 0x005D7410 0x005DAC70 0x005DACB0 0x005E2DD3 0x005F7E90 0x00609C60 0x00609C90 0x006139B0
// indirect  0x004077FA 0x00407877 0x00407881 0x004078A7 0x004078B1 0x004078D7 0x004078E1
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00407410
// measured tier  MISMATCH
// divergence     17
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00407410/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?init@AlphaSave@@QAEXXZ  at 0x00407410  (1291 bytes)
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

class AlphaSave;
class AutoSound;
struct BITMAPINFO;
typedef int BOOL;
struct BorderSizing;
struct BoxSpriteParams;
class Buffer;
class Dialog;
struct DialogEntry;
class FileWin;
struct FileWinConstruct;
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

struct DialogEntry {
    uint32_t vtable;
    int id;
    void * payload;
    DialogEntry * next;
    DialogEntry * previous;
    uint32_t secondary_vtable;
    void * heap;
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
    Buffer();
    int box_sprite(RECT *, BoxSpriteParams *);
    int set_font(Font *, Font *, Font *, Font *);
    void set_text_color(int, int, int, int);
    ~Buffer();
};
class Dialog { public:
    LPVOID vtable_;
    Heap heap_;
    Heap * heap_ptr_;
    uint32_t field_1C_;
    uint32_t field_20_;
    uint32_t field_24_;
    uint32_t field_28_;
    uint32_t field_2C_;
    uint32_t field_30_;
    uint32_t field_34_;
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
    uint32_t field_6C_;
    Font * font1_;
    Font * font2_;
    Font * font3_;
    uint32_t text_color_a_;
    uint32_t text_color_2a_;
    uint32_t text_color_3a_;
    uint32_t text_color_b_;
    uint32_t text_color_2b_;
    uint32_t text_color_3b_;
    uint32_t text_color_c_;
    uint32_t text_color_2c_;
    uint32_t text_color_3c_;
    uint32_t text_color_d_;
    uint32_t text_color_2d_;
    uint32_t text_color_3d_;
    uint32_t field_AC_;
    uint32_t field_B0_;
    uint32_t field_B4_;
    uint32_t field_B8_;
    uint32_t field_BC_;
    uint32_t field_C0_;
    DialogEntry * entry_head_;
    DialogEntry * current_entry_;
    int entry_count_;
    int entry_position_;
    uint32_t field_D4_;
    uint32_t field_D8_;
    uint32_t field_DC_;
    uint32_t field_E0_;
    uint32_t field_E4_;
    uint32_t field_E8_;
    int selected_position_;
    uint32_t field_F0_;
    int set_dialog_font(Font *, Font *, Font *);
    void set_dialog_text_color(int, int, int, int);
};
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
    uint8_t unmapped_532_[0x33C4 - 0x532];
    int init(char *, GraphicWin *, FileWinConstruct *);
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
    void fill(int);
    void init(int, int, int, int, char *, int, Win *, Menu *, BorderSizing *);
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
    int is_visible();
};

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 51, 62
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();
    virtual void slot012();
    virtual void slot013();
    virtual void slot014();
    virtual void slot015();
    virtual void slot016();
    virtual void slot017();
    virtual void slot018();
    virtual void slot019();
    virtual void slot020();
    virtual void slot021();
    virtual void slot022();
    virtual void slot023();
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();
    virtual void slot033();
    virtual void slot034();
    virtual void slot035();
    virtual void slot036();
    virtual void slot037();
    virtual void slot038();
    virtual void slot039();
    virtual void slot040();
    virtual void slot041();
    virtual void slot042();
    virtual void slot043();
    virtual void slot044();
    virtual void slot045();
    virtual void slot046();
    virtual void slot047();
    virtual void slot048();
    virtual void slot049();
    virtual void slot050();
    virtual void slot051();  // <-- used
    virtual void slot052();
    virtual void slot053();
    virtual void slot054();
    virtual void slot055();
    virtual void slot056();
    virtual void slot057();
    virtual void slot058();
    virtual void slot059();
    virtual void slot060();
    virtual void slot061();
    virtual void slot062();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_004072d0 = (int *)0x004072D0;
static int *const g_00650c6b = (int *)0x00650C6B;
static int *const g_00671e40 = (int *)0x00671E40;
static int *const g_006846b0 = (int *)0x006846B0;
static int *const g_0078d528 = (int *)0x0078D528;
static int *const g_0078d690 = (int *)0x0078D690;
static int *const g_007af59c = (int *)0x007AF59C;
static int *const g_007cfe8c = (int *)0x007CFE8C;
static int *const g_007cfeb4 = (int *)0x007CFEB4;
static int *const g_008c6b5c = (int *)0x008C6B5C;
static int *const g_008c6b60 = (int *)0x008C6B60;
static int *const g_008c6b64 = (int *)0x008C6B64;
static int *const g_008c6b6c = (int *)0x008C6B6C;
static int *const g_008c6b70 = (int *)0x008C6B70;
static int *const g_008c6b74 = (int *)0x008C6B74;
static int *const g_008c6b7c = (int *)0x008C6B7C;
static int *const g_008c6b84 = (int *)0x008C6B84;
static int *const g_009156b0 = (int *)0x009156B0;
static int *const g_009b7b1c = (int *)0x009B7B1C;
static int *const g_009b7b20 = (int *)0x009B7B20;

class AlphaSave { public:
    void init();
};

// Fresh extern for the "address itself does work" global: the Win
// object at 0x9156b0 is used both as a plain load (get its stored
// pointer) and as a literal added into an offset / masked by a guard,
// so the const-pointer scaffold spelling would let /O2 fold it away.
extern int g_win009156b0;

// Second vtable shim: slot 51 (0xcc) is called elsewhere with zero
// arguments, but this body's call to it passes one int argument, so a
// second shim declares that slot with the argument it actually needs.
class VCallArg1 { public:
    virtual void s00(); virtual void s01(); virtual void s02(); virtual void s03();
    virtual void s04(); virtual void s05(); virtual void s06(); virtual void s07();
    virtual void s08(); virtual void s09(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15();
    virtual void s16(); virtual void s17(); virtual void s18(); virtual void s19();
    virtual void s20(); virtual void s21(); virtual void s22(); virtual void s23();
    virtual void s24(); virtual void s25(); virtual void s26(); virtual void s27();
    virtual void s28(); virtual void s29(); virtual void s30(); virtual void s31();
    virtual void s32(); virtual void s33(); virtual void s34(); virtual void s35();
    virtual void s36(); virtual void s37(); virtual void s38(); virtual void s39();
    virtual void s40(); virtual void s41(); virtual void s42(); virtual void s43();
    virtual void s44(); virtual void s45(); virtual void s46(); virtual void s47();
    virtual void s48(); virtual void s49(); virtual void s50();
    virtual void s51(int);  // <-- used, offset 0xcc, takes one int
};

// Reused chase for the Dialog-view thunk stored inside the FileWin
// pointer at self+0x3280: the object there stores this-adjustments at
// +4 and +8 for two different embedded/virtual bases.
static char *dialogView(char *self, int slotOffset) {
    int *base = *reinterpret_cast<int **>(self + 0x3280);
    int adj = *reinterpret_cast<int *>(reinterpret_cast<char *>(base) + slotOffset);
    return self + 0x3280 + adj;
}

void AlphaSave::init() {
    char *self = reinterpret_cast<char *>(this);
    Buffer local;

    int *ptrAt9156b0 = *reinterpret_cast<int **>(g_009156b0);
    int off1 = *reinterpret_cast<int *>(reinterpret_cast<char *>(ptrAt9156b0) + 4);
    Win *winForVisible = reinterpret_cast<Win *>(reinterpret_cast<char *>(&g_win009156b0) + off1);
    int visible = winForVisible->is_visible();

    int uVar3 = (visible == 0) ? 1 : 0;
    int uVar4 = (visible != 0) ? 0x17 : 0;

    int xPos = (*g_009b7b1c - 0x13c) / 2 - uVar4;
    int yPos = (*g_009b7b20 - *g_007af59c) - (uVar3 + 0x1a) - 0x177;
    int wSize = (visible != 0) ? 0x18a : 0x14e;
    int hSize = (visible != 0) ? 0x191 : 0x189;

    Win *parent;
    if (visible == 0) {
        parent = 0;
    } else {
        int *pv = reinterpret_cast<int *>(&g_win009156b0);
        int off2 = *reinterpret_cast<int *>(*reinterpret_cast<int **>(pv) + 4);
        parent = reinterpret_cast<Win *>(reinterpret_cast<char *>(pv) + off2);
    }

    reinterpret_cast<GraphicWin *>(self)->init(
        xPos, yPos, wSize, hSize,
        (char *)0, 0x1001000, parent, (Menu *)0, (BorderSizing *)0);

    reinterpret_cast<GraphicWin *>(self)->fill(9);

    RECT rect;
    rect.left = *reinterpret_cast<long *>(self + 0x474);
    rect.top = *reinterpret_cast<long *>(self + 0x478);
    rect.right = *reinterpret_cast<long *>(self + 0x47c);
    rect.bottom = *reinterpret_cast<long *>(self + 0x480);

    if (visible == 0) {
        reinterpret_cast<Buffer *>(self + 0x444)->box_sprite(&rect, reinterpret_cast<BoxSpriteParams *>(g_006846b0));
        rect.left += 9;
        rect.top += 9;
        rect.right -= 9;
        rect.bottom -= 9;
    } else {
        rect.left += 0x17;
        rect.right -= 0x37;
        rect.bottom += 0xa;
        reinterpret_cast<Buffer *>(self + 0x444)->box_sprite(&rect, reinterpret_cast<BoxSpriteParams *>(g_0078d690));
    }

    rect.right -= 3;
    rect.left += 3;
    rect.bottom -= 3;
    rect.top += 3;
    reinterpret_cast<Buffer *>(self + 0x444)->box_sprite(&rect, reinterpret_cast<BoxSpriteParams *>(g_0078d528));

    // FileWinConstruct scratch: layout derived directly from the stack
    // slots the disassembly writes (-0x6c..-0x28), not from the Ghidra
    // local_NN names, which do not line up with the real offsets here.
    unsigned char fwc[0x48];
    int *fw = reinterpret_cast<int *>(fwc);
    fw[0] = (visible != 0) ? (0xfff7ffe0 + 0x80020) : 0x80020;
    if (visible != 0) {
        fw[1] = uVar4 + 5;
        fw[2] = uVar3 + 0x20;
        fw[3] = uVar4 + 0x136;
        fw[4] = uVar3 + 0x140;
        fw[5] = uVar4 + 8;
        fw[6] = uVar3 + 0x142;
        fw[7] = uVar4 + 0x134;
        fw[8] = uVar3 + 0x156;
        fw[9] = uVar4 + 10;
        fw[10] = uVar3 + 0x15c;
        fw[11] = uVar4 + 0x99;
        fw[12] = uVar3 + 0x172;
        fw[13] = uVar4 + 0xa2;
        fw[14] = uVar3 + 0x15c;
        fw[15] = uVar4 + 0x132;
        fw[16] = uVar3 + 0x172;
    } else {
        fw[1] = uVar4 + 0xe;
        fw[2] = uVar3 + 0x29;
        fw[3] = uVar4 + 0x13f;
        fw[4] = uVar3 + 0x149;
        fw[5] = uVar4 + 0x11;
        fw[6] = uVar3 + 0x14b;
        fw[7] = uVar4 + 0x13d;
        fw[8] = uVar3 + 0x15f;
        fw[9] = uVar4 + 0x13;
        fw[10] = uVar3 + 0x165;
        fw[11] = uVar4 + 0xa2;
        fw[12] = uVar3 + 0x17b;
        fw[13] = uVar4 + 0xab;
        fw[14] = uVar3 + 0x165;
        fw[15] = uVar4 + 0x13b;
        fw[16] = uVar3 + 0x17b;
    }
    fw[17] = uVar4 + 8;

    reinterpret_cast<FileWin *>(self + 0xa14)->init(
        (char *)0, reinterpret_cast<GraphicWin *>(self), reinterpret_cast<FileWinConstruct *>(fw));

    {
        char *dlg = dialogView(self, 8);
        reinterpret_cast<Dialog *>(dlg)->set_dialog_font(reinterpret_cast<Font *>(g_007cfeb4), 0, 0);
    }
    {
        char *dlg = dialogView(self, 8);
        reinterpret_cast<Dialog *>(dlg)->set_dialog_text_color(*g_008c6b5c, *g_008c6b60, 1, 1);
    }

    *reinterpret_cast<int *>(self + 0x3290) = *g_008c6b64;
    {
        char *dlg = dialogView(self, 8);
        *reinterpret_cast<int *>(dlg + 0x48) = 0x16;
    }
    {
        char *dlg = dialogView(self, 8);
        *reinterpret_cast<int *>(dlg + 0x4c) = 1;
    }
    {
        char *dlg = dialogView(self, 8);
        *reinterpret_cast<int *>(dlg + 0xb4) = 0;
    }
    *reinterpret_cast<void (**)()>(self + 0x32b0) = reinterpret_cast<void (*)()>(g_004072d0);
    *reinterpret_cast<int *>(self + 0x32ac) = 0;
    *reinterpret_cast<int *>(self + 0x3294) = -1;
    {
        char *dlg = dialogView(self, 8);
        int val = *reinterpret_cast<int *>(dlg + 0x20);
        *reinterpret_cast<int *>(dlg + 0x20) = val & ~4;
    }
    {
        char *dlg = dialogView(self, 4);
        reinterpret_cast<VCall *>(dlg)->slot062();
    }

    reinterpret_cast<Buffer *>(self + 0x2b50)->set_font(reinterpret_cast<Font *>(g_007cfeb4), 0, 0, 0);
    *reinterpret_cast<int *>(self + 0x3228) = *g_008c6b84;
    reinterpret_cast<Buffer *>(self + 0x2b50)->set_text_color(*g_008c6b6c, *g_008c6b70, 1, 1);

    *reinterpret_cast<int *>(self + 0x3234) = *g_008c6b7c;
    *reinterpret_cast<int *>(self + 0x3238) = *g_008c6b7c;
    *reinterpret_cast<int *>(self + 0x322c) = *g_008c6b74;
    *reinterpret_cast<int *>(self + 0x3230) = *g_008c6b7c;

    reinterpret_cast<VCallArg1 *>(self + 0x270c)->s51(1);
    reinterpret_cast<VCall *>(self + 0x270c)->slot062();

    reinterpret_cast<Buffer *>(self + 0x1074 + 0x444)->set_font(reinterpret_cast<Font *>(g_007cfe8c), 0, 0, 0);
    reinterpret_cast<VCall *>(self + 0x1074)->slot062();
    reinterpret_cast<VCall *>(self + 0x1074)->slot062();

    reinterpret_cast<Buffer *>(self + 0x1bc0 + 0x444)->set_font(reinterpret_cast<Font *>(g_007cfe8c), 0, 0, 0);
    reinterpret_cast<VCall *>(self + 0x1bc0)->slot062();
    reinterpret_cast<VCall *>(self + 0x1bc0)->slot062();
}
