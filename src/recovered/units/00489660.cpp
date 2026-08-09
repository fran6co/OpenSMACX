// ORIGINAL: 0x00489660 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00489660
// name           ?on_redraw@PickWin@@QAEXXZ
// size           1175 bytes
// measured tier  MISMATCH
// divergence     25
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00489660/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_redraw@PickWin@@QAEXXZ  at 0x00489660  (1175 bytes)
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
struct BoxSpriteParams;
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
class Menu;
typedef void * PVOID;
class PickWin;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class Strings;
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
    int box_sprite(RECT *, BoxSpriteParams *);
    int put_pixel(int, int, int);
    int write_cent_l(char *, RECT *, int);
    void hline(int, int, int, int);
    void vline(int, int, int, int);
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
    int get_selected_id();
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
    int draw(Buffer *, int, int, int);
};
class Strings { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
    BOOL is_populated_;
    int get(int);
};
extern "C" char *strcat(char *, const char *);
extern "C" unsigned int strlen(const char *);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 1
class VCall { public:
    virtual void slot000();
    virtual void slot001();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0078d5a0 = (int *)0x0078D5A0;
static int *const g_0078d618 = (int *)0x0078D618;
static int *const g_0078d690 = (int *)0x0078D690;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class PickWin { public:
    void on_redraw();
};

class VCall2 { public:
    virtual void slot000();
    virtual void slot001(int);  // <-- used, takes one int
};

void PickWin::on_redraw() {
    ((GraphicWin *)this)->fill(9);

    Buffer *buf = (Buffer *)((char *)this + 0x444);

    buf->box_sprite((RECT *)((char *)this + 0x623c), (BoxSpriteParams *)g_0078d690);

    RECT *r2 = (RECT *)((char *)this + 0x624c);
    buf->box_sprite(r2, (BoxSpriteParams *)g_0078d5a0);
    r2->left += 3;
    r2->top += 3;
    r2->right -= 3;
    r2->bottom -= 3;
    buf->box_sprite(r2, (BoxSpriteParams *)g_0078d618);

    *(char *)g_009b86a0 = 0;
    int strAddr = ((Strings *)g_009b90d8)->get(*(int *)(*(int *)g_009b90f8 + 0x1168));
    strcat((char *)g_009b86a0, (char *)strAddr);
    char *msg = (char *)g_009b86a0;
    if (msg != 0) {
        unsigned len = strlen(msg);
        buf->write_cent_l(msg, r2, len);
    }
    r2->left -= 3;
    r2->top -= 3;
    r2->right += 3;
    r2->bottom += 3;

    int local_20 = *(int *)((char *)this + 0x86b4) + 0x15;
    int local_28 = *(int *)((char *)this + 0x86ac) - 3;
    int local_24 = *(int *)((char *)this + 0x86b0) - 3;
    int local_1c = *(int *)((char *)this + 0x86b8) + 3;
    {
        RECT r3 = {local_28, local_24, local_20, local_1c};
        buf->box_sprite(&r3, (BoxSpriteParams *)g_0078d5a0);
    }

    int *p86bc = (int *)((char *)this + 0x86bc);
    char *dialogBase = (char *)p86bc + *(int *)((char *)(*p86bc) + 8);
    int local_14 = ((Dialog *)dialogBase)->get_selected_id();

    int iVar4 = *(int *)((char *)this + 0x9244);
    int local_8 = iVar4 + 10;
    int local_18 = local_8 + 0x39;
    int local_c = (*(int *)((char *)this + 0x9240) + *(int *)((char *)this + 0x9248)) / 2;
    int local_10 = *(int *)((char *)this + 0x86ac) - 10;
    int iVar5 = (local_10 + *(int *)((char *)this + 0x9248)) / 2;

    if (*(int *)((char *)this + 0x12640) == 0x105) {
        buf->vline(local_c, *(int *)((char *)this + 0x925c) - 2, local_18 - 2, 0xe0);
        buf->put_pixel(local_c + 1, local_8 + 0x38, 0xe0);
        buf->hline(local_c + 2, iVar5, local_8 + 0x39, 0xe0);
        buf->vline(iVar5, local_8 + 2, local_8 + 0x70, 0xe0);
        local_18 = iVar5 + 1;
        local_14 = iVar5 + 2;
        local_c = 0;
        do {
            int piv = local_8 + local_c * 0x13;
            int iv5b;
            if (local_c == 0) {
                iv5b = 1;
            } else {
                iv5b = -(int)(local_c == 6);
            }
            buf->put_pixel(local_18, iv5b + piv, 0xe0);
            buf->hline(local_14, local_10, piv, 0xe0);
            Buffer *masked = this ? buf : 0;
            Sprite *spr = (Sprite *)((char *)this + 0x8680);
            unsigned char flag = *((char *)spr + 8);
            spr->draw(masked, flag, local_10, piv - 6);
            local_c++;
        } while (local_c < 7);
    } else if (local_14 > 0 && local_14 < 8) {
        local_14 = local_14 - 1;
        local_c = local_14 * 0x13 + local_8;
        buf->hline(*(int *)((char *)this + 0x9248), iVar5 - 2, local_8, 0xe0);
        if (local_14 == 0) {
            buf->hline(iVar5 - 2, iVar5 + 2, local_8, 0xe0);
        } else {
            buf->put_pixel(iVar5 - 1, local_8 + 1, 0xe0);
            buf->put_pixel(iVar5 + 1, local_c - 1, 0xe0);
            buf->vline(iVar5, local_8 + 2, local_c - 2, 0xe0);
        }
        buf->hline(iVar5 + 2, local_10, local_c, 0xe0);
        Buffer *masked = this ? buf : 0;
        Sprite *spr = (Sprite *)((char *)this + 0x8680);
        unsigned char flag = *((char *)spr + 8);
        spr->draw(masked, flag, local_10, local_c - 6);
    }

    ((VCall2 *)((char *)this + 0x628c))->slot001(0);
    ((VCall2 *)((char *)this + 0x6dd8))->slot001(0);
    ((VCall2 *)((char *)this + 0x7924))->slot001(0);
    ((VCall2 *)((char *)this + 0xd6a8))->slot001(0);
    ((VCall2 *)((char *)this + 0xe1f4))->slot001(0);
    ((VCall2 *)((char *)this + 0xed40))->slot001(0);
    ((VCall2 *)((char *)this + 0xf88c))->slot001(0);
    ((VCall2 *)((char *)this + 0x103d8))->slot001(0);
    ((VCall2 *)((char *)this + 0x10f24))->slot001(0);
    ((VCall2 *)((char *)this + 0x11a70))->slot001(0);

    {
        int *p86bc2 = (int *)((char *)this + 0x86bc);
        char *adjBase = (char *)p86bc2 + *(int *)((char *)(*p86bc2) + 4);
        ((VCall2 *)adjBase)->slot001(0);
    }

    ((VCall2 *)((char *)this + 0x9270))->slot001(0);
    ((VCall2 *)((char *)this + 0x9dbc))->slot001(0);
    ((VCall2 *)((char *)this + 0xa908))->slot001(0);
    ((VCall2 *)((char *)this + 0xcaec))->slot001(0);
    ((VCall2 *)((char *)this + 0xbfa0))->slot001(0);
    ((VCall2 *)((char *)this + 0xb454))->slot001(0);
}
