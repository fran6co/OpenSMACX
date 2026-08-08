// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0050EA40
// name           ?blink_timer@@YAXH@Z
// size           997 bytes
// measured tier  MISMATCH
// divergence     40
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0050ea40/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?blink_timer@@YAXH@Z  at 0x0050EA40  (997 bytes)
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
class MapWin;
class Menu;
typedef void * PVOID;
class PlanWin;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class StringBox;
class TutWin;
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
class MapWin { public:
    uint32_t vbtable_pointer_;
    void * owned_;
    uint8_t derived_tail_[0x21A6C - 0x8];
    GraphicWin virtual_base_;
    void set_center(int, int, int);
};
class PlanWin { public:
    void blink();
};
class StringBox { public:
    void clip_ids(unsigned long);
};
class TutWin { public:
    uint8_t unmapped_0_[0x537C];
    int32_t field_537C_;
    int32_t field_5380_;
    uint8_t unmapped_5384_[0x539C - 0x5384];
    int32_t field_539C_;
    uint8_t unmapped_53A0_[0x53A4 - 0x53A0];
    int32_t field_53A4_;
    int32_t field_53A8_;
    int32_t field_53AC_;
    uint8_t unmapped_53B0_[0x53B8 - 0x53B0];
    int32_t field_53B8_;
    uint8_t unmapped_53BC_[0x53C4 - 0x53BC];
    int32_t field_53C4_;
    uint8_t unmapped_53C8_[0x53D4 - 0x53C8];
    int32_t field_53D4_;
    void draw_arrow();
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
    void client_to_screen(int *, int *);
    void get_mouse_pos(int *, int *);
};
void __cdecl draw_cursor();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669330 = (int *)0x00669330;
static int *const g_00669348 = (int *)0x00669348;
static int *const g_0066934c = (int *)0x0066934C;
static int *const g_0068f21c = (int *)0x0068F21C;
static int *const g_006a7628 = (int *)0x006A7628;
static int *const g_007cd2ec = (int *)0x007CD2EC;
static int *const g_007d3c3c = (int *)0x007D3C3C;
static int *const g_007d3c5c = (int *)0x007D3C5C;
static int *const g_00834d70 = (int *)0x00834D70;
static int *const g_008a6270 = (int *)0x008A6270;
static int *const g_008c6e68 = (int *)0x008C6E68;
static int *const g_00915620 = (int *)0x00915620;
static int *const g_009156b0 = (int *)0x009156B0;
static int *const g_00939294 = (int *)0x00939294;
static int *const g_00939298 = (int *)0x00939298;
static int *const g_009392a8 = (int *)0x009392A8;
static int *const g_009392ac = (int *)0x009392AC;
static int *const g_00939430 = (int *)0x00939430;
static int *const g_00939434 = (int *)0x00939434;
static int *const g_00939438 = (int *)0x00939438;
static int *const g_0093943c = (int *)0x0093943C;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;
static int *const g_0094988c = (int *)0x0094988C;
static int *const g_009a6490 = (int *)0x009A6490;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009b2068 = (int *)0x009B2068;
static int *const g_009b7ae4 = (int *)0x009B7AE4;
static int *const g_009b7b14 = (int *)0x009B7B14;
static int *const g_009b7b1c = (int *)0x009B7B1C;
static int *const g_009b7b20 = (int *)0x009B7B20;

// EXTERN-SYMBOL LEVER: this table of MapWin pointers is walked with a
// register increment and its end is a comparison bound, so both addresses
// need the array spelling rather than the context's constant-pointer one.
extern int *g_007d3c3c_tbl[];
extern int *g_007d3c5c_tbl[];

void __cdecl blink_timer(int a1) {
    if (*g_0068f21c != 0) return;
    if (*g_00915620 != 0) return;

    if (reinterpret_cast<Win *>(g_006a7628)->is_visible()) {
        reinterpret_cast<TutWin *>(g_008c6e68)->draw_arrow();
        return;
    }
    if (reinterpret_cast<Win *>(g_008a6270)->is_visible()) {
        reinterpret_cast<TutWin *>(g_008c6e68)->draw_arrow();
        return;
    }

    reinterpret_cast<PlanWin *>(g_00834d70)->blink();
    reinterpret_cast<StringBox *>(g_007cd2ec)->clip_ids(0x96);

    int t1 = *g_00939298;
    unsigned char cl = *reinterpret_cast<unsigned char *>(g_009a64c0);
    int t2 = *g_0093f660;
    if (t1 != 0) goto Lchk9b2068;
    if (t2 == 0) goto Learly;
    if (!(cl & 2)) goto Learly;

Lchk9b2068:
    if (*g_009b2068 == 0) goto Lchk939294;

Learly:
    *g_009392ac = 0;
    return;

Lchk939294:
    {
        int t3 = *g_00939294;
        if (t3 != 0) goto Lb09;
        if (t2 == 0) return;
        if (!(cl & 2)) return;
    }

Lb09:
    *g_009392a8 = (*g_009392a8 == 0) ? 1 : 0;
    draw_cursor();

    {
        typedef int(__cdecl *Fn1_t)(int);
        typedef int(__cdecl *Fn2_t)(int);
        typedef int(__cdecl *Fn3_t)(int);
        int r1 = reinterpret_cast<Fn1_t>(*g_00669330)(1);
        int esiVal = *g_009b7b14;
        int blinkCol = static_cast<unsigned char>(r1 >> 8);
        int r2 = reinterpret_cast<Fn2_t>(*g_00669348)(-6);
        int r3 = reinterpret_cast<Fn3_t>(*g_0066934c)(r2);
        if (r3 != esiVal) return;
        if (reinterpret_cast<Win *>(g_008c6e68)->is_visible()) return;
        if (*g_00939430 == -1) return;
        if (*g_00939434 == -1) return;
        if (*g_00939438 == -1) return;
        if (*g_0093943c == -1) return;

        unsigned char hi = static_cast<unsigned char>(*g_009a6490 >> 8);
        if (!(hi & 0x10)) {
            if (blinkCol == 0) {
                if (*g_009b7ae4 == 0) return;
            }
        }

        for (int **edi_ptr = g_007d3c3c_tbl; edi_ptr < g_007d3c5c_tbl; ++edi_ptr) {
            int col = blinkCol;
            int *ptrVal = *edi_ptr;
            if (ptrVal == 0) continue;
            char *pv = reinterpret_cast<char *>(ptrVal);

            if (edi_ptr != g_007d3c3c_tbl) {
                if (*reinterpret_cast<int *>(pv + 0x1dd74) == 0) continue;
            }
            if (col != 0) {
                if (*reinterpret_cast<int *>(pv + 0x1dd80) == 0) continue;
            }

            int sum = *reinterpret_cast<int *>(pv + 0x1ddd4)
                    + *reinterpret_cast<int *>(pv + 0x1ddcc);
            if (sum >= *g_00949870) continue;

            int rowAdj = *reinterpret_cast<int *>(pv + 0x1dda0);
            col = *reinterpret_cast<int *>(pv + 0x1dd9c);

            int mouseCol, mouseRow;
            {
                void **vt = *reinterpret_cast<void ***>(ptrVal);
                int voff = reinterpret_cast<int *>(vt)[1];
                reinterpret_cast<Win *>(pv + voff)->get_mouse_pos(&mouseCol, &mouseRow);
            }
            {
                void **vt = *reinterpret_cast<void ***>(ptrVal);
                int voff = reinterpret_cast<int *>(vt)[1];
                reinterpret_cast<Win *>(pv + voff)->client_to_screen(&mouseCol, &mouseRow);
            }

            unsigned char wrapFlag = *reinterpret_cast<unsigned char *>(g_0094988c);

            if (mouseCol == 0) {
                col -= (*reinterpret_cast<int *>(pv + 0x1ddcc) + 2) / 3;
                if (!(wrapFlag & 1)) {
                    if (col < 0) {
                        col += *g_00949870;
                    } else {
                        int w = *g_00949870;
                        if (col >= w) col -= w;
                    }
                }
            }

            {
                int center = *g_009b7b1c;
                if (mouseCol >= center - 0xa && mouseCol < center + 0xa) {
                    int edxv = (*reinterpret_cast<int *>(pv + 0x1ddcc) + 2) / 3 + col;
                    if (wrapFlag & 1) {
                        col = edxv;
                    } else if (edxv < 0) {
                        col = *g_00949870 + edxv;
                    } else {
                        int w = *g_00949870;
                        if (edxv >= w) edxv -= w;
                        col = edxv;
                    }
                }
            }

            if (mouseRow == 0) {
                if (*reinterpret_cast<int *>(pv + 0x1dda8) > -2) {
                    int esiv = *reinterpret_cast<int *>(pv + 0x1dda8) + 2;
                    int quot = (*reinterpret_cast<int *>(pv + 0x1ddd0) + 2) / 3;
                    int delta;
                    if (quot < 0 || esiv < 0) {
                        delta = 0;
                    } else if (quot <= esiv) {
                        delta = quot;
                    } else {
                        delta = esiv;
                    }
                    rowAdj -= delta;
                }
            }

            {
                int centerRow = *g_009b7b20;
                if (mouseRow >= centerRow - 0xa && mouseRow < centerRow + 0xa) {
                    int scratch = *g_00949874 + 2;
                    int c = *reinterpret_cast<int *>(pv + 0x1ddd8);
                    int s = *reinterpret_cast<int *>(pv + 0x1dda8);
                    if (s + c * 2 - 2 < scratch) {
                        int cc = (2 - c) * 2 - s + *g_00949874;
                        int quot = (*reinterpret_cast<int *>(pv + 0x1ddd0) + 2) / 3;
                        int addAmt;
                        if (quot < 0 || cc < 0) {
                            addAmt = 0;
                        } else if (quot > cc) {
                            addAmt = cc;
                        } else {
                            addAmt = quot;
                        }
                        rowAdj += addAmt;
                    }
                }
            }

            if (col & 1) {
                col -= 1;
            }
            if (rowAdj & 1) {
                col += 1;
            }

            if (col == *reinterpret_cast<int *>(pv + 0x1dd9c)
                && rowAdj == *reinterpret_cast<int *>(pv + 0x1dda0)) {
                continue;
            }
            if (rowAdj < 0) continue;
            if (rowAdj >= *g_00949874) continue;
            if (col < 0) continue;
            if (col >= *g_00949870) continue;

            if (ptrVal == g_009156b0) {
                *reinterpret_cast<int *>(pv + 0x21a44) = 1;
            }
            reinterpret_cast<MapWin *>(ptrVal)->set_center(col, rowAdj, 1);
            *reinterpret_cast<int *>(pv + 0x21a44) = 0;
        }
    }
}
