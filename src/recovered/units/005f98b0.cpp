// ORIGINAL: 0x005F98B0 FILE
// name      ?on_redraw@PullDown@@QAEXXZ
// size      1103 bytes
// spans     0x005F98B0-0x005F9CFF
// prototype void (__thiscall ?on_redraw@PullDown@@QAEXXZ)(PullDown* this)
// callers   0   call targets   12
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D5250 0x005D5350 0x005DAC70 0x005DACB0 0x005DC7C0 0x005DCAB0 0x005DCF40 0x005DD450 0x005E1A80 0x005E3203 0x005E4B4A 0x006453E0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005F98B0
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005f98b0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_redraw@PullDown@@QAEXXZ  at 0x005F98B0  (1103 bytes)
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
class PullDown;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class Win;

// ---- callees, declared and never defined (a definition would be inlined) ----
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
    int box(RECT *, int, int);
    int set_font(Font *, Font *, Font *, Font *);
    int text_line_height();
    int text_width(char *, int);
    int write_l(char *, RECT *, int);
    int write_right_l(char *, RECT *, int);
    void hline(int, int, int, int);
    void set_text_color(int, int, int, int);
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
    int fill(RECT *, int);
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
    int draw(Buffer *, int, int, int, int, int);
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

extern "C" unsigned int strlen(const char *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00696e04 = (int *)0x00696E04;
static int *const g_00696e08 = (int *)0x00696E08;
static int *const g_00696e0c = (int *)0x00696E0C;
static int *const g_00696e10 = (int *)0x00696E10;
static int *const g_00696e14 = (int *)0x00696E14;
static int *const g_00696e18 = (int *)0x00696E18;
static int *const g_00696e1c = (int *)0x00696E1C;
static int *const g_00697108 = (int *)0x00697108;
static int *const g_009b7b58 = (int *)0x009B7B58;
static int *const g_009b7b5c = (int *)0x009B7B5C;
static int *const g_009b7b60 = (int *)0x009B7B60;
static int *const g_009b7b64 = (int *)0x009B7B64;
static int *const g_009b7b68 = (int *)0x009B7B68;
static int *const g_009bb484 = (int *)0x009BB484;

class PullDown { public:
    void on_redraw();
};

// Callback stored at self+0xf38/0xf3c: called either with no args (redraw
// hook) or (self, int, RECT*) (per-item draw hook). Both shapes are used
// below; neither is in the catalogue, so they are spelled as raw function
// pointers rather than catalogued callees.
typedef void(__cdecl *RedrawHookFn)(void *);
typedef void(__cdecl *ItemHookFn)(void *, int, RECT *);

void PullDown::on_redraw() {
    char *self = reinterpret_cast<char *>(this);
    Buffer *buf = reinterpret_cast<Buffer *>(self + 0x444);

    if (*g_009b7b68 == 0) {
        *g_009b7b68 = *g_009bb484;
    }
    int iVar4 = buf->set_font(reinterpret_cast<Font *>(*g_009b7b68), 0, 0, 0);
    if (iVar4 == 0) {
        RECT rect = *reinterpret_cast<RECT *>(self + 0x474);
        int iStack_10 = rect.left, iStack_c = rect.top, iStack_8 = rect.right,
            iStack_4 = rect.bottom;

        *reinterpret_cast<int *>(self + 0xf38) = *g_009b7b58;
        void *pcVar3 = *reinterpret_cast<void **>(g_009b7b5c);
        *reinterpret_cast<void **>(self + 0xf3c) = pcVar3;

        if (pcVar3 == 0) {
            reinterpret_cast<GraphicWin *>(self)->fill(*g_00696e08);
            iVar4 = 0;
            if (*g_00696e1c > 0) {
                RECT tmp;
                tmp.left = iStack_10;
                tmp.top = iStack_c;
                tmp.right = iStack_8;
                tmp.bottom = iStack_4;
                do {
                    buf->box(&tmp, *g_00696e14, *g_00696e18);
                    tmp.left++;
                    tmp.right--;
                    tmp.top++;
                    tmp.bottom--;
                    iVar4++;
                } while (iVar4 < *g_00696e1c);
                iStack_10 = tmp.left;
                iStack_c = tmp.top;
                iStack_8 = tmp.right;
                iStack_4 = tmp.bottom;
            }
            iStack_10 -= *g_00696e1c;
            iStack_8 += *g_00696e1c;
            iStack_c -= *g_00696e1c;
            iStack_4 += *g_00696e1c;
        } else {
            reinterpret_cast<RedrawHookFn>(pcVar3)(self);
        }

        int iVar9 = iStack_8 - iStack_10;
        iStack_4 = buf->text_line_height();
        iStack_c = *g_00696e1c;
        int iStack_2c = 0;
        iVar4 = *g_00696e04 + *g_00696e1c;
        iStack_4 = *g_00696e1c + iStack_4;
        iStack_8 = iVar9 - iVar4;

        if (*reinterpret_cast<int *>(self + 0xf20) > 0) {
            char *puVar10 = self + 0xa1c;
            iStack_10 = iVar4;
            do {
                iVar9 = *reinterpret_cast<int *>(puVar10 - 4);
                unsigned int uVar1 = *reinterpret_cast<unsigned int *>(puVar10 + 8);
                int iVar5 = *reinterpret_cast<int *>(puVar10 + 0xc);
                char *_Str = *reinterpret_cast<char **>(puVar10);

                if ((uVar1 & 1) != 0) {
                    if ((uVar1 & 8) != 0) {
                        Sprite *spr = *reinterpret_cast<Sprite **>(g_00697108);
                        Buffer *iconBuf = self ? buf : 0;
                        int yy = ((iStack_4 - spr->iHeight_) - iStack_c) / 2 + iStack_c;
                        spr->draw(iconBuf, spr->cTransparentIndex_, iVar4, yy, 1, 1);
                        spr = *reinterpret_cast<Sprite **>(g_00697108);
                        iVar4 = iStack_10 + spr->iWidth_;
                        iStack_10 = iVar4;
                    }
                    int uVar13 = *g_00696e18;
                    if ((uVar1 & 4) == 0) {
                        int uVar2;
                        if (iStack_2c == *reinterpret_cast<int *>(self + 0xf28)) {
                            if (*reinterpret_cast<void **>(self + 0xf38) == 0) {
                                RECT r2;
                                r2.left = iVar4 - *g_00696e04;
                                r2.top = iStack_c;
                                r2.right = iStack_8 + *g_00696e04;
                                r2.bottom = iStack_4;
                                reinterpret_cast<GraphicWin *>(self)->fill(&r2, *g_009b7b60);
                                iStack_10 = r2.left + *g_00696e04;
                                iStack_8 = r2.right - *g_00696e04;
                                uVar13 = *g_00696e08;
                                uVar2 = *g_00696e0c;
                            } else {
                                RECT r3;
                                r3.left = iStack_10;
                                r3.top = iStack_c;
                                r3.right = iStack_8;
                                r3.bottom = iStack_4;
                                reinterpret_cast<ItemHookFn>(
                                    *reinterpret_cast<void **>(self + 0xf38))(self, 1, &r3);
                                iStack_10 = r3.left;
                                iStack_c = r3.top;
                                iStack_8 = r3.right;
                                iStack_4 = r3.bottom;
                                uVar13 = *g_00696e08;
                                uVar2 = *g_00696e0c;
                            }
                        } else {
                            uVar13 = *g_00696e10;
                            uVar2 = *g_009b7b64;
                            if (*reinterpret_cast<void **>(self + 0xf38) != 0) {
                                RECT r4;
                                r4.left = iStack_10;
                                r4.top = iStack_c;
                                r4.right = iStack_8;
                                r4.bottom = iStack_4;
                                reinterpret_cast<ItemHookFn>(
                                    *reinterpret_cast<void **>(self + 0xf38))(self, 0, &r4);
                                iStack_10 = r4.left;
                                iStack_c = r4.top;
                                iStack_8 = r4.right;
                                iStack_4 = r4.bottom;
                                uVar13 = *g_00696e10;
                                uVar2 = *g_009b7b64;
                            }
                        }
                        if ((uVar1 & 2) == 0) {
                            uVar13 = uVar2;
                        }
                        buf->set_text_color(uVar13, -1, 1, 1);

                        char *strLabel2 = *reinterpret_cast<char **>(puVar10 - 4);
                        if (strLabel2 != 0) {
                            unsigned int sVar6 = strlen(strLabel2);
                            RECT r5;
                            r5.left = iStack_10;
                            r5.top = iStack_c;
                            r5.right = iStack_8;
                            r5.bottom = iStack_4;
                            buf->write_l(strLabel2, &r5, sVar6);
                        }
                        if (iVar5 != 0) {
                            int uVar13b = *reinterpret_cast<int *>(self + 0x980);
                            int fontLead = *reinterpret_cast<int *>(*g_009b7b68 + 0x14);
                            int iVar11 = iStack_4 - iStack_c;
                            int iVar7 = buf->text_width(reinterpret_cast<char *>(iVar5), 1);
                            int iVar8 = buf->text_line_height();
                            iVar8 = iStack_4 - (iVar11 - iVar8) / 2;
                            int iVar9b = buf->text_width(reinterpret_cast<char *>(iVar9),
                                                          iVar5 - iVar9);
                            buf->hline(iVar9b + iStack_10, iVar9b + iStack_10 + iVar7,
                                       iVar8 - fontLead, uVar13b);
                        }
                        if (_Str != 0) {
                            unsigned int sVar6 = strlen(_Str);
                            RECT r6;
                            r6.left = iStack_10;
                            r6.top = iStack_c;
                            r6.right = iStack_8;
                            r6.bottom = iStack_4;
                            buf->write_right_l(_Str, &r6, sVar6);
                        }
                    } else {
                        int iVar4b = *reinterpret_cast<int *>(self + 0x4c4);
                        int iVar9c = *g_00696e1c * -2;
                        int iVar5b = buf->text_line_height();
                        buf->hline(*g_00696e1c, *g_00696e1c + iVar4b + iVar9c,
                                   iVar5b / 2 + iStack_c, uVar13);
                        uVar13 = *g_00696e14;
                        iVar4b = *reinterpret_cast<int *>(self + 0x4c4);
                        iVar9c = *g_00696e1c * -2;
                        iVar5b = buf->text_line_height();
                        buf->hline(*g_00696e1c, *g_00696e1c + iVar4b + iVar9c,
                                   iVar5b / 2 + 1 + iStack_c, uVar13);
                    }
                    if ((uVar1 & 8) != 0) {
                        Sprite *spr = *reinterpret_cast<Sprite **>(g_00697108);
                        iStack_10 -= spr->iWidth_;
                    }
                    iVar4 = buf->text_line_height();
                    iStack_c += iVar4;
                    iStack_4 += iVar4;
                    iVar4 = iStack_10;
                }
                iStack_2c++;
                puVar10 += 0x14;
            } while (iStack_2c < *reinterpret_cast<int *>(self + 0xf20));
        }
    }
}
