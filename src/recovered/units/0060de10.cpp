// ORIGINAL: 0x0060DE10 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0060DE10
// name           ?on_left_down@RadioButton@@QAEXHH@Z
// size           523 bytes
// measured tier  MISMATCH
// divergence     6
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0060de10/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_left_down@RadioButton@@QAEXHH@Z  at 0x0060DE10  (523 bytes)
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
class RadioButton;
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
    int text_line_height();
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
    void soft_update();
};
extern "C" int abs(int);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 44
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
    virtual void slot044();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b7ab8 = (int *)0x009B7AB8;

class RadioButton { public:
    void draw_item(int, int);
    void on_left_down(int, int);
};


// Extra vtable shim: the emitter's own VCall.slot044 is nullary, but this
// body's dispatch through slot 44 (call [eax+0xb0]) pushes one int argument
// first, so a second shim with the right signature is needed alongside it.
class VCallArg { public:
    virtual void slot00();
    virtual void slot01();
    virtual void slot02();
    virtual void slot03();
    virtual void slot04();
    virtual void slot05();
    virtual void slot06();
    virtual void slot07();
    virtual void slot08();
    virtual void slot09();
    virtual void slot10();
    virtual void slot11();
    virtual void slot12();
    virtual void slot13();
    virtual void slot14();
    virtual void slot15();
    virtual void slot16();
    virtual void slot17();
    virtual void slot18();
    virtual void slot19();
    virtual void slot20();
    virtual void slot21();
    virtual void slot22();
    virtual void slot23();
    virtual void slot24();
    virtual void slot25();
    virtual void slot26();
    virtual void slot27();
    virtual void slot28();
    virtual void slot29();
    virtual void slot30();
    virtual void slot31();
    virtual void slot32();
    virtual void slot33();
    virtual void slot34();
    virtual void slot35();
    virtual void slot36();
    virtual void slot37();
    virtual void slot38();
    virtual void slot39();
    virtual void slot40();
    virtual void slot41();
    virtual void slot42();
    virtual void slot43();
    virtual void slot44(int);
};

void RadioButton::on_left_down(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);

    // ---- refresh the "current selection" cache, then rewind the sibling
    // chain by one entry before scanning ----
    {
        int32_t vbp = *reinterpret_cast<int32_t *>(self - 0x18);
        int32_t gw_off = *reinterpret_cast<int32_t *>(vbp + 4);
        *g_009b7ab8 = *reinterpret_cast<int32_t *>(self + gw_off + 0xac);
    }
    char *edi = self - 0x18;
    {
        int32_t vbp = *reinterpret_cast<int32_t *>(edi);
        int32_t dlg_off = *reinterpret_cast<int32_t *>(vbp + 8);
        char *dlgbase = self + dlg_off;
        int32_t cnt = *reinterpret_cast<int32_t *>(dlgbase + 0xb4) - 1;
        char *dp = dlgbase + 0xa4;
        if (cnt >= -1) {
            int32_t n = abs(-1);
            int32_t limit = *reinterpret_cast<int32_t *>(dp + 0x10);
            *reinterpret_cast<int32_t *>(dp + 0xc) = *reinterpret_cast<int32_t *>(dp + 8);
            if (n <= limit) {
                n = abs(-1);
                if (n > 0) {
                    do {
                        int32_t cur = *reinterpret_cast<int32_t *>(dp + 0xc);
                        n--;
                        int32_t nxt = *reinterpret_cast<int32_t *>(cur + 0x10);
                        *reinterpret_cast<int32_t *>(dp + 0xc) = nxt;
                    } while (n != 0);
                }
                int32_t last = *reinterpret_cast<int32_t *>(dp + 0x10) - 1;
                *reinterpret_cast<int32_t *>(dp + 0x14) = last;
            }
        }
    }

    // ---- walk siblings, accumulating line height, until the click's
    // vertical offset (a2) is used up ----
    int32_t loop_first = 1;
    while (a2 > 0) {
        if (!loop_first) {
            int32_t vbp = *reinterpret_cast<int32_t *>(edi);
            int32_t dlg_off = *reinterpret_cast<int32_t *>(vbp + 8);
            char *dlgbase = self + dlg_off;
            int32_t cnt = *reinterpret_cast<int32_t *>(dlgbase + 0xb4) - 1;
            if (*reinterpret_cast<int32_t *>(dlgbase + 0xb8) == cnt) {
                return;
            }
        }

        int32_t vbp = *reinterpret_cast<int32_t *>(edi);
        loop_first = 0;
        int32_t dlg_off = *reinterpret_cast<int32_t *>(vbp + 8);
        char *dp = self + dlg_off + 0xa4;
        int32_t p_ac = *reinterpret_cast<int32_t *>(self + dlg_off + 0xac);
        if (p_ac != 0) {
            int32_t d0c = *reinterpret_cast<int32_t *>(dp + 0xc);
            int32_t d10 = *reinterpret_cast<int32_t *>(dp + 0x10);
            d0c = *reinterpret_cast<int32_t *>(d0c + 0xc);
            *reinterpret_cast<int32_t *>(dp + 0xc) = d0c;
            int32_t d14 = *reinterpret_cast<int32_t *>(dp + 0x14) + 1;
            *reinterpret_cast<int32_t *>(dp + 0x14) = d14;
            if (d14 == d10) {
                *reinterpret_cast<int32_t *>(dp + 0x14) = loop_first;
            }
        }
        int32_t v = *reinterpret_cast<int32_t *>(dp + 0xc);

        int32_t ebx_val;
        if (v == 0) {
            ebx_val = 0;
        } else if (p_ac == 0) {
            char *nullp = 0;
            ebx_val = *reinterpret_cast<int32_t *>(nullp + 8);
        } else {
            int32_t w = *reinterpret_cast<int32_t *>(v + 8);
            ebx_val = *reinterpret_cast<int32_t *>(w + 8);
        }

        int32_t vbp2 = *reinterpret_cast<int32_t *>(edi);
        int32_t gw_off = *reinterpret_cast<int32_t *>(vbp2 + 4);
        int32_t h = reinterpret_cast<Buffer *>(self + gw_off + 0x42c)->text_line_height();
        int32_t prod = h * ebx_val;

        int32_t vbp3 = *reinterpret_cast<int32_t *>(edi);
        int32_t dlg_off2 = *reinterpret_cast<int32_t *>(vbp3 + 8);
        char *dlgbase2 = self + dlg_off2;
        int32_t sum = *reinterpret_cast<int32_t *>(dlgbase2 + 0x2c) +
                      *reinterpret_cast<int32_t *>(dlgbase2 + 0x50);
        int32_t chosen = (sum > prod) ? sum : prod;
        a2 = a2 - chosen;
    }

    // ---- locate the entry actually under the click, then either leave it
    // alone (already selected) or move the selection and redraw both ----
    {
        int32_t vbp = *reinterpret_cast<int32_t *>(edi);
        int32_t dlg_off = *reinterpret_cast<int32_t *>(vbp + 8);
        char *dlgbase = self + dlg_off;
        int32_t denom = *reinterpret_cast<int32_t *>(dlgbase + 0x4c) +
                        *reinterpret_cast<int32_t *>(dlgbase + 0x2c);
        int32_t quot = a1 / denom;
        int32_t target = quot * *reinterpret_cast<int32_t *>(dlgbase + 0x40) +
                          *reinterpret_cast<int32_t *>(dlgbase + 0xb8);
        int32_t fd4 = *reinterpret_cast<int32_t *>(dlgbase + 0xd4);

        if (fd4 == target) {
            if ((*reinterpret_cast<uint8_t *>(dlgbase + 8) & 4) == 0) {
                return;
            }
            int32_t gw_off2 = *reinterpret_cast<int32_t *>(vbp + 4);
            int32_t obj = *reinterpret_cast<int32_t *>(self + gw_off2 + 0xac);
            if (obj == 0) {
                return;
            }
            reinterpret_cast<VCallArg *>(obj)->slot44(-1);
            return;
        }

        reinterpret_cast<RadioButton *>(edi)->draw_item(fd4, 0);
        {
            int32_t vbp2 = *reinterpret_cast<int32_t *>(edi);
            int32_t dlg_off2 = *reinterpret_cast<int32_t *>(vbp2 + 8);
            *reinterpret_cast<int32_t *>(self + dlg_off2 + 0xd4) = target;
        }
        {
            typedef void(__cdecl * NotifyFn)(int32_t);
            NotifyFn fn = *reinterpret_cast<NotifyFn *>(self - 0xc);
            if (fn != 0) {
                int32_t vbp3 = *reinterpret_cast<int32_t *>(edi);
                int32_t dlg_off3 = *reinterpret_cast<int32_t *>(vbp3 + 8);
                int32_t cur = *reinterpret_cast<int32_t *>(self + dlg_off3 + 0xd4);
                fn(cur);
            }
        }
        reinterpret_cast<RadioButton *>(edi)->draw_item(target, 1);
        {
            int32_t vbp4 = *reinterpret_cast<int32_t *>(edi);
            int32_t dlg_off4 = *reinterpret_cast<int32_t *>(vbp4 + 8);
            *reinterpret_cast<int32_t *>(self + dlg_off4 + 0xd8) = -1;
        }
        {
            int32_t vbp5 = *reinterpret_cast<int32_t *>(edi);
            int32_t gw_off3 = *reinterpret_cast<int32_t *>(vbp5 + 4);
            reinterpret_cast<GraphicWin *>(self + gw_off3)->soft_update();
        }
        edi = reinterpret_cast<char *>(*reinterpret_cast<int32_t *>(edi));
        {
            int32_t dlg_off5 = *reinterpret_cast<int32_t *>(edi + 8);
            if ((*reinterpret_cast<uint8_t *>(self + dlg_off5 + 8) & 4) == 0) {
                return;
            }
            int32_t gw_off4 = *reinterpret_cast<int32_t *>(edi + 4);
            int32_t obj2 = *reinterpret_cast<int32_t *>(self + gw_off4 + 0xac);
            if (obj2 == 0) {
                return;
            }
            reinterpret_cast<VCallArg *>(obj2)->slot44(-1);
        }
    }
}
