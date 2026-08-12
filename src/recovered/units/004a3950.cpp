// ORIGINAL: 0x004A3950 FILE
// name      ?report@ReportIf@@QAEXHHH@Z
// size      899 bytes
// spans     0x004A3950-0x004A3CD3
// prototype void (__thiscall ?report@ReportIf@@QAEXHHH@Z)(ReportIf* this, int, int factionID, int)
// callers   12   call targets   18
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0045DEA0 0x0045E540 0x004A3D20 0x004A7000 0x004A84A0 0x004A9050 0x004A9F90 0x004AA0B0 0x004AA970 0x004AB550 0x004AB600 0x004ABE70 0x004AC140 0x004C3A40 0x005B9F20 0x005D5890 0x005D5A70 0x00609F20
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004A3950
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004a3950/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?report@ReportIf@@QAEXHHH@Z  at 0x004A3950  (899 bytes)
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
class Dialog;
struct DialogEntry;
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
class ListBox;
class Menu;
typedef void * PVOID;
struct RECT;
class ReportIf;
class Scroll;
class Spot;
class Sprite;
class Win;
class WorldWin;

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
};

class Heap { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
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
    void redraw();
    void soft_update();
};
class ListBox { public:
    uint32_t vbtable_pointer_;
    uint32_t field_4_;
    uint32_t field_8_;
    uint32_t field_C_;
    uint32_t field_10_;
    uint32_t field_14_;
    uint32_t field_18_;
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
    uint32_t graphic_vbase_adjust_;
    GraphicWin virtual_base_;
    uint32_t dialog_vbase_adjust_;
    Dialog dialog_;
    void close();
};
class WorldWin { public:
    void hide_all();
};
bool __cdecl has_tech(int, int);
void __cdecl scroll_hide(GraphicWin *, Scroll *);
void __cdecl scroll_show(GraphicWin *, Scroll *);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 2, 62
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();  // <-- used
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
    virtual void slot051();
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
static int *const g_004a3cd4 = (int *)0x004A3CD4;
static int *const g_004a3cf8 = (int *)0x004A3CF8;
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_007b39e8 = (int *)0x007B39E8;
static int *const g_007b4534 = (int *)0x007B4534;
static int *const g_00876478 = (int *)0x00876478;
static int *const g_0087be24 = (int *)0x0087BE24;
static int *const g_0087be84 = (int *)0x0087BE84;
static int *const g_00882570 = (int *)0x00882570;
static int *const g_00882584 = (int *)0x00882584;
static int *const g_008846e4 = (int *)0x008846E4;
static int *const g_008846e8 = (int *)0x008846E8;
static int *const g_00885f30 = (int *)0x00885F30;
static int *const g_008e9f60 = (int *)0x008E9F60;
static int *const g_0096cdac = (int *)0x0096CDAC;
static int *const g_009a649c = (int *)0x009A649C;
static int *const g_009a64c0 = (int *)0x009A64C0;

class ReportIf { public:
    void reset();
    void draw_labs();
    void draw_energy();
    void draw_ops();
    void close_ops();
    void draw_projects();
    void draw_sat();
    void close_sat();
    void draw_security();
    void draw_score();
    void draw_intel();
    void report(int, int, int);
};

// Per-faction struct array, stride 0x20CC bytes / 2099 ints, indexed by faction id.
extern int g_0096cdac_tbl[];
// The starting address of this WorldWin loop is incremented in place
// (stride 0xb4c per element, exactly 2 elements observed: end - start ==
// one stride), so it needs a real relocation rather than a folded literal.
extern char g_007b39e8_base;

void ReportIf::report(int a1, int a2, int a3) {
    char *self = reinterpret_cast<char *>(this);
    int mode = *reinterpret_cast<int *>(self + 0x1e1dc);

    switch (mode) {
    case 2:
        reinterpret_cast<ListBox *>(self + 0xa2d0)->close();
        reinterpret_cast<VCall *>(self + 0xba0c)->slot002();
        reinterpret_cast<VCall *>(self + 0xc558)->slot002();
        break;
    case 3:
        reinterpret_cast<ListBox *>(g_0087be84)->close();
        // fall through
    case 0:
        reinterpret_cast<ListBox *>(self + 0xa2d0)->close();
        reinterpret_cast<ListBox *>(self + 0xae24)->close();
        break;
    case 4:
        close_ops();
        break;
    case 5:
        if (*reinterpret_cast<int *>(self + 8) != 4 &&
            *reinterpret_cast<int *>(self + 8) != 5) {
            reinterpret_cast<WorldWin *>(g_008e9f60)->hide_all();
            char *p = &g_007b39e8_base;
            do {
                reinterpret_cast<VCall *>(p)->slot002();
                p += 0xb4c;
            } while (p <= reinterpret_cast<char *>(g_007b4534));
        }
        break;
    case 6:
        close_sat();
        break;
    case 7:
        reinterpret_cast<VCall *>(self + 0x1bf9c)->slot002();
        reinterpret_cast<VCall *>(self + 0x1d634)->slot002();
        reinterpret_cast<VCall *>(self + 0x1cae8)->slot002();
        break;
    }

    if (mode > 0) {
        *reinterpret_cast<int *>(self + 0x47a4 + mode * 0xb4c) = -1;
        reinterpret_cast<VCall *>(self + 0x3cec + mode * 0xb4c)->slot062();
    }
    if (a1 > 0) {
        *reinterpret_cast<int *>(self + a1 * 0xb4c + 0x47a4) = 2;
        reinterpret_cast<VCall *>(self + a1 * 0xb4c + 0x3cec)->slot062();
    }

    *reinterpret_cast<int *>(self + 0x1e22c) = a3;
    *reinterpret_cast<int *>(self + 8) = a1;
    *reinterpret_cast<int *>(self + 0x1e228) = a2;

    if (mode != 2 && *g_0087be24 != -1) {
        int best;
        if ((*reinterpret_cast<uint8_t *>(g_009a64c0) & 0x80) == 0 &&
            (*reinterpret_cast<int *>(g_009a649c) & 0x200) != 0) {
            if (*g_00885f30 >= 0 && *g_00885f30 <= 0x59 &&
                has_tech(*g_00885f30, a2) != 0) {
                best = *g_00885f30;
            } else {
                int i = 0;
                int result;
                do {
                    result = has_tech(i, *reinterpret_cast<int *>(self + 0x1e228));
                    best = i;
                    if (result != 0)
                        break;
                    i++;
                    best = *g_00885f30;
                } while (i < 0x59);
            }
        } else {
            best = g_0096cdac_tbl[a2 * 2099];
        }
        *g_00885f30 = best;
        if (*g_00885f30 < 0 || *g_00885f30 > 0x59) {
            *g_00885f30 = 0;
        }
        *g_0087be24 = -1;
    }

    if (a2 > 0) {
        int *p = reinterpret_cast<int *>(self + 0x1e224);
        for (int i = 0x12; i != 0; i--) {
            *p = *(p - 1);
            p--;
        }

        int new_mode = *reinterpret_cast<int *>(self + 8);
        *reinterpret_cast<int *>(self + 0x1e1dc) = new_mode;
        if (new_mode == 4 || new_mode == 5 || new_mode == 7) {
            if (new_mode != *reinterpret_cast<int *>(self + 0x1e1e0)) {
                *g_008846e8 = 1;
            }
            if (*reinterpret_cast<int *>(self + 0x1e23c) != 0) {
                *g_008846e8 = 1;
                *reinterpret_cast<int *>(self + 0x1e23c) = 0;
            }
        } else if (*g_00882570 != 0) {
            scroll_hide(reinterpret_cast<GraphicWin *>(g_00876478),
                        reinterpret_cast<Scroll *>(g_00882584));
            *g_008846e8 = 0;
            *g_008846e4 = 0;
            *g_00882570 = 0;
        }

        reset();
        switch (*reinterpret_cast<int *>(self + 8)) {
        case 0:
            draw_intel();
            break;
        case 2:
            draw_labs();
            break;
        case 3:
            draw_energy();
            break;
        case 4:
            draw_ops();
            break;
        case 5:
            draw_projects();
            break;
        case 6:
            draw_sat();
            break;
        case 7:
            draw_security();
            break;
        case 8:
            draw_score();
            break;
        }

        reinterpret_cast<GraphicWin *>(g_007ae820)->soft_update();
        reinterpret_cast<GraphicWin *>(g_00876478)->redraw();

        if (*g_008846e4 != 0 && *reinterpret_cast<int *>(self + 0x1e238) == 0) {
            scroll_show(reinterpret_cast<GraphicWin *>(g_00876478),
                        reinterpret_cast<Scroll *>(g_00882584));
            *g_008846e4 = 0;
            *g_00882570 = 1;
        }
    }
}
