// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0060C350
// name           ?on_redraw@ListBox@@QAEXXZ
// size           628 bytes
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0060c350/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_redraw@ListBox@@QAEXXZ  at 0x0060C350  (628 bytes)
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
    int box(RECT *, int, int);
    int copy(Buffer *, int, int, int, int, int, int);
    int set_clip(RECT *);
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
extern "C" int abs(int);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 10
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
    virtual void slot010();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b7ab8 = (int *)0x009B7AB8;
static int *const g_009b8ee8 = (int *)0x009B8EE8;

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

    void draw_item(int);
    void on_redraw();
};

// The VCall shim's slot010 is declared nullary, but this body's
// dispatch through `(*item)[10]` takes one pointer argument and
// returns int. Second shim, same reasoning the emitter's own comment
// on VCall gives: editing VCall in place would collide with its
// declaration in this unit.
class VCall2 { public:
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
    virtual int slot010(void *);   // <-- used, 1-arg/int-return shim
};

// `this` reaches ListBox's GraphicWin virtual base (self-0x48 is
// where the disassembly reads the vbtable, i.e. the true ListBox
// start), and every access to the GraphicWin/Dialog virtual bases
// re-reads that vbtable and its slot-1 / slot-2 adjustment fresh
// rather than caching it in a local - matching the disassembly, which
// reloads the same [edi]/[eax+4]/[eax+8] sequence at nearly every use
// rather than keeping it live across the many intervening calls.
// `add_base` is separate from `self` because two call sites add the
// slot adjustment to the ListBox's own start (self-0x48) instead of to
// `this` - the vtable is always read from self-0x48 either way.
static char *vbase_off(char *self, char *add_base, int slot) {
    int32_t *vtbl = *reinterpret_cast<int32_t **>(self - 0x48);
    return add_base + vtbl[slot];
}

// NOT BYTE-EXACT, not re-timeboxed further given the size (628 bytes,
// the largest of this batch) and the double-virtual-base indirection
// (GraphicWin at vbtable slot 1, Dialog-shaped state at slot 2, both
// reached only through a runtime vbtable read this opaque scaffold
// cannot model as real inheritance). Every call site, argument order
// and indirect-call target below is read directly off the
// disassembly; the two nested loops (box() shrinking a rect inward,
// then draw_item() walking visible rows) are transcribed with `goto`
// where the original's flag reuse does not map to structured
// control flow.
void ListBox::on_redraw() {
    char *self = reinterpret_cast<char *>(this);

    int32_t *item = *reinterpret_cast<int32_t **>(vbase_off(self, self, 1) + 0x7c);
    if (item == 0) {
        return;
    }
    *g_009b7ab8 = reinterpret_cast<int32_t>(item);

    {
        char *g1 = vbase_off(self, self, 1);
        reinterpret_cast<Buffer *>(g1 + 0x3fc)->set_clip(
            reinterpret_cast<RECT *>(g1 + 0x3fc + 0x30));
    }

    int32_t (*callback)(void *, int, int, int) =
        *reinterpret_cast<int32_t (**)(void *, int, int, int)>(self - 0x10);
    if (callback != 0) {
        char *arg = (self - 0x48) != 0 ? vbase_off(self, self - 0x48, 1) : self - 0x48;
        if (callback(arg, 0, 0, 0) != 0) {
            goto after_redraw;
        }
    }

    {
        void *arg2 = vbase_off(self, self - 0x48, 1);
        int32_t r = reinterpret_cast<VCall2 *>(item)->slot010(arg2);
        if (r != 0) {
            goto after_redraw;
        }
    }

    {
        char *g2 = vbase_off(self, self, 1);
        if (*reinterpret_cast<int32_t *>(g2 + 0x50) & 0x80000) {
            int32_t sub = *reinterpret_cast<int32_t *>(g2 + 0x7c);
            if (sub != 0) {
                char *g3 = vbase_off(self, self, 2);
                reinterpret_cast<Buffer *>(sub + 0x444)->copy(
                    reinterpret_cast<Buffer *>(g2 + 0x3fc),
                    *reinterpret_cast<int32_t *>(g3 - 0x24),
                    *reinterpret_cast<int32_t *>(g3 - 0x20),
                    0, 0,
                    *reinterpret_cast<int32_t *>(g2 + 0x47c),
                    -*reinterpret_cast<int32_t *>(g2 + 0x480));
                goto after_redraw;
            }
        }
        char *g3 = vbase_off(self, self, 2);
        reinterpret_cast<GraphicWin *>(g2 - 0x48)->fill(*reinterpret_cast<int32_t *>(g3 + 0x24));
    }

after_redraw:
    {
        RECT r;
        char *g2 = vbase_off(self, self, 1);
        r.left = *reinterpret_cast<int32_t *>(g2 + 0x42c);
        r.top = *reinterpret_cast<int32_t *>(g2 + 0x430);
        r.right = *reinterpret_cast<int32_t *>(g2 + 0x434);
        r.bottom = *reinterpret_cast<int32_t *>(g2 + 0x438);

        int32_t i = 0;
        if (*reinterpret_cast<int32_t *>(vbase_off(self, self, 2) + 0x6c) > 0) {
            do {
                char *g3 = vbase_off(self, self, 2);
                int32_t v64 = *reinterpret_cast<int32_t *>(g3 + 0x64);
                int32_t v68 = *reinterpret_cast<int32_t *>(g3 + 0x68);
                char *g2b = vbase_off(self, self, 1);
                reinterpret_cast<Buffer *>(g2b + 0x3fc)->box(
                    reinterpret_cast<RECT *>(&r.right), v64, v68);
                r.left++;
                r.top++;
                r.right--;
                r.bottom--;
                i++;
            } while (i < *reinterpret_cast<int32_t *>(vbase_off(self, self, 2) + 0x6c));
        }
    }

    {
        char *g3b = vbase_off(self, self, 2);
        if (*reinterpret_cast<int32_t *>(g3b + 0x84) == 0) {
            return;
        }

        int32_t idx = *reinterpret_cast<int32_t *>(self - 0x30) - 1;
        int32_t total = *reinterpret_cast<int32_t *>(g3b + 0x84) - 1;
        if (idx <= total) {
            *reinterpret_cast<int32_t *>(g3b + 0x80) = *reinterpret_cast<int32_t *>(g3b + 0x7c);
            if (idx < 0) {
                if (abs(idx) <= *reinterpret_cast<int32_t *>(g3b + 0x84)) {
                    int32_t n = abs(idx);
                    if (n > 0) {
                        do {
                            char *node = *reinterpret_cast<char **>(g3b + 0x80);
                            n--;
                            *reinterpret_cast<int32_t *>(g3b + 0x80) =
                                *reinterpret_cast<int32_t *>(node + 0x10);
                        } while (n != 0);
                    }
                    idx += *reinterpret_cast<int32_t *>(g3b + 0x84);
                    *reinterpret_cast<int32_t *>(g3b + 0x88) = idx;
                }
            } else {
                if (idx > 0) {
                    int32_t n = idx;
                    do {
                        char *node = *reinterpret_cast<char **>(g3b + 0x80);
                        n--;
                        *reinterpret_cast<int32_t *>(g3b + 0x80) =
                            *reinterpret_cast<int32_t *>(node + 0xc);
                    } while (n != 0);
                }
                *reinterpret_cast<int32_t *>(g3b + 0x88) = idx;
            }
        }
    }

    *g_009b8ee8 = 0;
    {
        int32_t outer_i = 0;
        char *g3 = vbase_off(self, self, 2);
        if (*reinterpret_cast<int32_t *>(g3 + 0xc) > 0) {
            do {
                int32_t count3 = *reinterpret_cast<int32_t *>(g3 + 0x10);
                int32_t j = 0;
                if (count3 > 0) {
                    do {
                        int32_t idx2 = *reinterpret_cast<int32_t *>(self - 0x30) + *g_009b8ee8;
                        reinterpret_cast<ListBox *>(self - 0x48)->draw_item(idx2);
                        *g_009b8ee8 = *g_009b8ee8 + 1;
                        g3 = vbase_off(self, self, 2);
                        int32_t remaining =
                            *reinterpret_cast<int32_t *>(g3 + 0x84) - *reinterpret_cast<int32_t *>(self - 0x30);
                        if (*g_009b8ee8 == remaining) {
                            goto outer_continue;
                        }
                        count3 = *reinterpret_cast<int32_t *>(g3 + 0x10);
                        j++;
                    } while (j < count3);
                }
            outer_continue:
                outer_i++;
                g3 = vbase_off(self, self, 2);
            } while (outer_i < *reinterpret_cast<int32_t *>(g3 + 0xc));
        }
    }
}
