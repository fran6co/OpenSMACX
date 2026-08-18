// ORIGINAL: 0x0050F960 ?on_post_redraw_nc_buffer@Console@@QAEXPAUBuffer@@H@Z 0x0050F960-0x0050FB08 FILE
// size      424 bytes
// prototype void (__thiscall ?on_post_redraw_nc_buffer@Console@@QAEXPAUBuffer@@H@Z)(Console* this, Buffer*, int)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E4B4A
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0050F960
// measured tier  MISMATCH
// divergence     95
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0050f960/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_post_redraw_nc_buffer@Console@@QAEXPAUBuffer@@H@Z  at 0x0050F960  (424 bytes)
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

class Console { public:
    uint8_t derived_storage_[0x23D94];
    GraphicWin virtual_base_;

    void on_post_redraw_nc_buffer(Buffer *, int);
};

// The catalogued receiver (`Console`) puts `virtual_base_` (a GraphicWin)
// at offset 0x23D94, but this body indexes `this` with NEGATIVE offsets
// down to -0x23D94 and never a positive one near that size - the incoming
// `this` (bare `mov esi,ecx`, no adjustment) already points at the
// GraphicWin subobject, not at the Console start. A byte-exact encoding of
// `[esi-0x23D94]` needs `this` to equal that address directly, so the
// receiver here is a fresh opaque class positioned exactly there rather
// than `Console` reached via a positive-offset member. `Console` above is
// left declared as the catalogue emitted it - unreferenced, undefined
// member is fine since nothing calls it in this unit.
class ConsoleGfxView { public:
    void on_post_redraw_nc_buffer(Buffer *, int);
};

void ConsoleGfxView::on_post_redraw_nc_buffer(Buffer *a1, int a2) {
    char *self = reinterpret_cast<char *>(this);

    if (a1 == 0) {
        return;
    }

    if (a2 == 1) {
        int *p = *reinterpret_cast<int **>(self - 0x23d94);
        int v1 = *reinterpret_cast<int *>(self - 0x50);
        Sprite *sp = reinterpret_cast<Sprite *>(self - 0x6c);
        int off = *reinterpret_cast<int *>(reinterpret_cast<char *>(p) + 4);
        char *base = self - 0x23d94 + off;
        int idx = *reinterpret_cast<int *>(base + 0x11c);
        int v2 = (idx >= 0) ? idx : *reinterpret_cast<int *>(base + 0x118);
        int temp1 = *reinterpret_cast<int *>(base + 0x158) -
                    *reinterpret_cast<int *>(base + 0x150) -
                    *reinterpret_cast<int *>(base + 0x148) -
                    *reinterpret_cast<int *>(base + 0x114) +
                    *reinterpret_cast<int *>(base + 0x140);
        int half1 = v1 / 2;
        int v3 = *reinterpret_cast<int *>(self - 0xc);
        int xcalc = temp1 - half1 + v3 - v2;
        int v4 = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x80);
        unsigned char transparency =
            *reinterpret_cast<unsigned char *>(reinterpret_cast<char *>(sp) + 8);
        int iwidth = *reinterpret_cast<int *>(reinterpret_cast<char *>(sp) + 0x18);
        int ycalc = v4 - iwidth;
        sp->draw(a1, transparency, ycalc, xcalc, 1, 1);

        int *p2 = *reinterpret_cast<int **>(self - 0x23d94);
        int iheight = *reinterpret_cast<int *>(reinterpret_cast<char *>(sp) + 0x1c);
        int off2 = *reinterpret_cast<int *>(reinterpret_cast<char *>(p2) + 4);
        char *base2 = self - 0x23d94 + off2;
        int idx2 = *reinterpret_cast<int *>(base2 + 0x11c);
        int v2b = (idx2 >= 0) ? idx2 : *reinterpret_cast<int *>(base2 + 0x118);
        int temp1b = *reinterpret_cast<int *>(base2 + 0x158) -
                     *reinterpret_cast<int *>(base2 + 0x150) -
                     *reinterpret_cast<int *>(base2 + 0x148) -
                     *reinterpret_cast<int *>(base2 + 0x114) +
                     *reinterpret_cast<int *>(base2 + 0x140);
        int half1b = iheight / 2;
        int v4b = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x80);
        int v3b = *reinterpret_cast<int *>(self - 8);
        int ycalc2 = temp1b - half1b + v3b - v2b;
        int iwidth2 = *reinterpret_cast<int *>(reinterpret_cast<char *>(sp) + 0x18);
        int xcalc2 = v4b - iwidth2;
        unsigned char transparency2 =
            *reinterpret_cast<unsigned char *>(reinterpret_cast<char *>(sp) + 8);
        sp->draw(a1, transparency2, xcalc2, ycalc2, 1, 1);
        return;
    }

    if (a2 == 2) {
        int c1 = *reinterpret_cast<int *>(self - 0x24);
        int v4c = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x84);
        int v1c = *reinterpret_cast<int *>(self - 0x28);
        Sprite *sp2 = reinterpret_cast<Sprite *>(self - 0x40);
        int negsum = -(c1 + v4c);
        int half_v1c = v1c / 2;
        int ycomp = *reinterpret_cast<int *>(self - 0x14) - half_v1c;
        int *p3 = *reinterpret_cast<int **>(self - 0x23d94);
        int off3 = *reinterpret_cast<int *>(reinterpret_cast<char *>(p3) + 4);
        int tableval = *reinterpret_cast<int *>(self - 0x23c7c + off3);
        int ycomp2 = ycomp + tableval;
        unsigned char transparency3 =
            *reinterpret_cast<unsigned char *>(reinterpret_cast<char *>(sp2) + 8);
        sp2->draw(a1, transparency3, ycomp2, negsum, 1, 1);

        int iheight2 = *reinterpret_cast<int *>(reinterpret_cast<char *>(sp2) + 0x1c);
        int v4d = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x84);
        int sumA_neg = -(iheight2 + v4d);
        int iwidth2 = *reinterpret_cast<int *>(reinterpret_cast<char *>(sp2) + 0x18);
        int half_iwidth2 = iwidth2 / 2;
        int c2 = *reinterpret_cast<int *>(self - 0x10) - half_iwidth2;
        int *p4 = *reinterpret_cast<int **>(self - 0x23d94);
        int off4 = *reinterpret_cast<int *>(reinterpret_cast<char *>(p4) + 4);
        c2 += *reinterpret_cast<int *>(self - 0x23c7c + off4);
        unsigned char transparency4 =
            *reinterpret_cast<unsigned char *>(reinterpret_cast<char *>(sp2) + 8);
        sp2->draw(a1, transparency4, c2, sumA_neg, 1, 1);
    }
}
