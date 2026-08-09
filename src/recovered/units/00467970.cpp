// ORIGINAL: 0x00467970 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00467970
// name           ?draw_base_dest@MapWin@@QAEXH@Z
// size           1129 bytes
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00467970/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_base_dest@MapWin@@QAEXH@Z  at 0x00467970  (1129 bytes)
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

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 63
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
    virtual void slot062();
    virtual void slot063();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00400000 = (int *)0x00400000;
static int *const g_00800000 = (int *)0x00800000;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_0094988c = (int *)0x0094988C;
static int *const g_0095282c = (int *)0x0095282C;
static int *const g_0097d150 = (int *)0x0097D150;
static int *const g_009a64c8 = (int *)0x009A64C8;
static int *const g_009a64cc = (int *)0x009A64CC;

class MapWin { public:
    uint32_t vbtable_pointer_;
    void * owned_;
    uint8_t derived_tail_[0x21A6C - 0x8];
    GraphicWin virtual_base_;

    void dest_line(int, int, int, int, int, int, int, int, int, int, int);
    void draw_base_dest(int);
};

extern short g_tbl_97d040[];
extern short g_tbl_97d150[];
extern unsigned char g_tbl_9ab88c[];
extern unsigned char g_tbl_94a379[];
extern short g_tbl_952844[];
extern short g_tbl_95283c[];

void MapWin::draw_base_dest(int a1) {
    char *self = reinterpret_cast<char *>(this);
    int *fld = reinterpret_cast<int *>(self);

    bool any = false;
    bool didLine = false;

    if ((fld[0x775c] & 0x40000) != 0) {
        int outer = 0;
        if (0 < *g_009a64cc) {
            short *rec = g_tbl_97d150;
            do {
                if (*reinterpret_cast<unsigned char *>(rec - 0x86) == *g_00939284) {
                    int dir = 0;
                    short *cur = rec;
                    do {
                        short val = *cur;
                        if (val >= 0) {
                            int origY = rec[-0x88];
                            int destX = static_cast<short>(g_tbl_97d040[val * 0x9a]);
                            int destY = static_cast<short>(g_tbl_97d040[val * 0x9a + 1]);
                            int baseY = fld[0x776a];

                            bool skip = (rec[-0x87] < baseY) ||
                                (fld[0x7776] + fld[0x7774] + baseY <= rec[-0x87]);
                            bool isMatch = false;
                            if (!skip) {
                                if (baseY <= destY && destY < fld[0x7776] + fld[0x7774] + baseY) {
                                    int wx = destX;
                                    if ((*g_0094988c & 1) == 0) {
                                        if (destX < fld[0x7769]) {
                                            wx = destX + *g_00949870;
                                        }
                                        if (fld[0x7775] + fld[0x7773] + fld[0x7769] <= wx) {
                                            wx = wx - *g_00949870;
                                        }
                                    }
                                    if (fld[0x7769] <= wx && wx < fld[0x7775] + fld[0x7773] + fld[0x7769]) {
                                        isMatch = true;
                                    }
                                }
                            } else {
                                int wy = origY;
                                if ((*g_0094988c & 1) == 0) {
                                    if (origY < fld[0x7769]) {
                                        wy = *g_00949870 + origY;
                                    }
                                    if (fld[0x7775] + fld[0x7773] + fld[0x7769] <= wy) {
                                        wy = wy - *g_00949870;
                                    }
                                }
                                if (!(wy < fld[0x7769] || fld[0x7775] + fld[0x7773] + fld[0x7769] <= wy)) {
                                    isMatch = true;
                                }
                            }
                            if (isMatch) {
                                int mode = (dir == 0 || dir == 1 || dir == 2) ? 10 : 0;
                                reinterpret_cast<MapWin *>(self)->dest_line(
                                    origY, rec[-0x87], destX, destY, mode,
                                    *g_00939284, dir, 0, 3, 5, 0);
                                didLine = true;
                            }
                        }
                        ++dir;
                        ++cur;
                    } while (dir < 3);
                }
                ++outer;
                rec += 0x9a;
            } while (outer < *g_009a64cc);
        }
    }

    if ((fld[0x775c] & 0xc00000) != 0) {
        int outer2 = 0;
        if (0 < *g_009a64c8) {
            int cVal = 0;
            unsigned char *rec2 = reinterpret_cast<unsigned char *>(g_0095282c);
            do {
                if (rec2[10] == *g_00939284) {
                    unsigned int flagbits = *reinterpret_cast<unsigned int *>(rec2) & 0x1000200;
                    bool cond1 = (flagbits == 0x1000200) && (fld[0x775c] & 0x400000) != 0;
                    bool cond2 = (rec2[13] == 0x18) && (fld[0x775c] & 0x800000) != 0;
                    if (cond1 || cond2) {
                        int dirIdx = *reinterpret_cast<short *>(rec2 + 6);
                        unsigned char lookup = g_tbl_94a379[
                            static_cast<unsigned char>(g_tbl_9ab88c[dirIdx * 0x34]) * 0x90];

                        if (cond1) {
                            unsigned int n = rec2[0xe];
                            int i = 0;
                            unsigned int limit = (1 < rec2[0xe] ? 1 : 0) + n;
                            if (limit != 0) {
                                int prev = 0;
                                do {
                                    int nextI = i + 1;
                                    int a = nextI % static_cast<int>(n + 1) + cVal;
                                    int b = prev % static_cast<int>(n + 1) + cVal;
                                    reinterpret_cast<MapWin *>(self)->dest_line(
                                        g_tbl_95283c[b], g_tbl_952844[b],
                                        g_tbl_95283c[a], g_tbl_952844[a],
                                        dirIdx, rec2[10], lookup,
                                        (flagbits == 0x1000200) ? 1 : 0, 3, 5, 0);
                                    n = rec2[0xe];
                                    prev = nextI;
                                    i = nextI;
                                } while (i < static_cast<int>((1 < rec2[0xe] ? 1 : 0) + n));
                            }
                        } else {
                            reinterpret_cast<MapWin *>(self)->dest_line(
                                *reinterpret_cast<short *>(rec2 - 4),
                                *reinterpret_cast<short *>(rec2 - 2),
                                *reinterpret_cast<short *>(rec2 + 16),
                                *reinterpret_cast<short *>(rec2 + 24),
                                dirIdx, rec2[10], lookup,
                                (flagbits == 0x1000200) ? 1 : 0, 2, 2, 0);

                            if ((*reinterpret_cast<unsigned int *>(rec2) & 0x2000000) != 0 && rec2[0xe] != 0) {
                                int k = 0;
                                short *sp = reinterpret_cast<short *>(rec2 + 0x12);
                                do {
                                    reinterpret_cast<MapWin *>(self)->dest_line(
                                        sp[-1], sp[3], sp[0], sp[4],
                                        dirIdx, rec2[10], lookup,
                                        (flagbits == 0x1000200) ? 1 : 0, 2, 2, 0);
                                    ++k;
                                    ++sp;
                                } while (k < rec2[0xe]);
                            }
                        }
                        any = true;
                    }
                }
                cVal += 0x1a;
                ++outer2;
                rec2 += 0x34;
            } while (outer2 < *g_009a64c8);
        }
    }

    if (a1 != 0 && (didLine || any)) {
        int *vbt = *reinterpret_cast<int **>(self);
        int adj = *reinterpret_cast<int *>(reinterpret_cast<char *>(vbt) + 4);
        reinterpret_cast<VCall *>(self + adj)->slot063();
    }
}
