// ORIGINAL: 0x00470A90 FILE
// name      ?init@MapWin@@QAEXHH@Z
// size      1074 bytes
// spans     0x00470A90-0x00470EC2
// prototype void (__thiscall ?init@MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   5   call targets   9
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0046FA20 0x0046FB80 0x00470920 0x005C89B0 0x005D7670 0x005EDF50 0x006169A0 0x00645470 0x0064FC88
// indirect  0x00470EA9
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00470A90
// measured tier  MISMATCH
// divergence     5
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00470a90/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?init@MapWin@@QAEXHH@Z  at 0x00470A90  (1074 bytes)
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
struct ExtDirectDraw;
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
class Strings;
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
    int init(int, int, int, ExtDirectDraw *);
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

class Strings { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
    BOOL is_populated_;
    int get(int);
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
    void set_caption(char *);
};

extern "C" char *strcat(char *, const char *);
extern "C" int __cdecl _itoa();
void __cdecl say_year(char *);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 62
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
    virtual void slot062();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00400020 = (int *)0x00400020;
static int *const g_00682820 = (int *)0x00682820;
static int *const g_00682e94 = (int *)0x00682E94;
static int *const g_006863a8 = (int *)0x006863A8;
static int *const g_0068f21c = (int *)0x0068F21C;
static int *const g_006e81b4 = (int *)0x006E81B4;
static int *const g_006e81b8 = (int *)0x006E81B8;
static int *const g_006e81bc = (int *)0x006E81BC;
static int *const g_006e81c0 = (int *)0x006E81C0;
static int *const g_007d3c40 = (int *)0x007D3C40;
static int *const g_008eb48c = (int *)0x008EB48C;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00939edc = (int *)0x00939EDC;
static int *const g_00939f08 = (int *)0x00939F08;
static int *const g_00939f0c = (int *)0x00939F0C;
static int *const g_00939f10 = (int *)0x00939F10;
static int *const g_00939f14 = (int *)0x00939F14;
static int *const g_00939f48 = (int *)0x00939F48;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_00945778 = (int *)0x00945778;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

class MapWin { public:
    uint32_t vbtable_pointer_;
    void * owned_;
    uint8_t derived_tail_[0x21A6C - 0x8];
    GraphicWin virtual_base_;

    void UNK5(int);
    void do_image_buttons();
    void init(RECT *, char *, int);
    void init(int, int);
};

// Indexed table base (0x939F48): the address itself does arithmetic
// (idx*0x10 + base), so it needs a real extern array, not the fixed
// pointer constant - see the EXTERN-SYMBOL / INDEXED TABLE BASE levers.
struct MapWinLayoutEntry { int left, top, right, bottom; };
extern MapWinLayoutEntry g_939f48_layout[];

// The DAT_00946d4c/50/dd4 block is one record of a per-scenario table,
// stride 0x59C, indexed by DAT_00939284. Same lever as above.
struct MapWinYearRecord {
    int field0;
    int field1;
    char pad[0x288 - 8];
    char label[0x59C - 0x288];
};
extern MapWinYearRecord g_939284_year_records[];

// The emitter's `_itoa()` shim is nullary; this call needs the real CRT
// prototype. A second `extern "C"` declaration conflicts (C2733), so the
// call site reinterprets the existing declaration's function type instead
// of adding one.
typedef char *(__cdecl *ItoaFn)(int, char *, int);

void MapWin::init(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);

    reinterpret_cast<Buffer *>(self + 0x1E938)->init(100, 0x32, 0, 0);
    *reinterpret_cast<int *>(self + 0x1DD74) = 1;

    RECT rect;
    if (this == reinterpret_cast<MapWin *>(*g_007d3c40)) {
        rect.left = *g_00939f08;
        rect.top = *g_00939f0c;
        rect.right = *g_00939f10;
        rect.bottom = *g_00939f14;
        *reinterpret_cast<int *>(self + 0x1DD98) = -8;
    } else if (a1 == 0) {
        int idx = *reinterpret_cast<int *>(self + 0x1DD78);
        int threshold = (*g_0093f660 != 0) ? 2 : 1;
        if (idx < threshold) {
            if (((1 << idx) & *g_00939edc) == 0) {
                this->UNK5(idx);
            }
            idx = *reinterpret_cast<int *>(self + 0x1DD78);
            MapWinLayoutEntry &entry = g_939f48_layout[idx];
            rect.left = entry.left;
            rect.top = entry.top;
            rect.right = entry.right;
            rect.bottom = entry.bottom;
            if (idx != 0) {
                *reinterpret_cast<int *>(self + 0x1DD98) = -8;
            }
        } else {
            int v = idx * 0x28;
            rect.left = v;
            rect.top = v;
            rect.right = v + 0x190;
            rect.bottom = v + 0x190;
        }
    } else if (a1 == 3) {
        rect.left = *g_00939f08;
        rect.top = *g_00939f0c;
        rect.right = *g_00939f10;
        rect.bottom = *g_00939f14;
    } else if (a1 == 2) {
        rect.left = *g_006e81b4;
        rect.top = *g_006e81b8;
        rect.right = *g_006e81bc;
        rect.bottom = *g_006e81c0;
    } else {
        rect.left = *g_00939f08;
        rect.top = *g_00939f0c;
        rect.right = *g_00939f10;
        rect.bottom = *g_00939f14;
    }

    char *msg = reinterpret_cast<char *>(g_009b86a0);
    *msg = 0;

    int *tbl = *reinterpret_cast<int **>(g_009b90f8);
    Strings *strings = reinterpret_cast<Strings *>(g_009b90d8);

    if (a1 == 0) {
        strcat(msg, reinterpret_cast<char *>(strings->get(tbl[0x1D])));

        if (*reinterpret_cast<int *>(self + 0x1DD78) == 0) {
            *msg = 0;
            int n = *g_00939284;
            MapWinYearRecord &rec = g_939284_year_records[n];
            *g_009bbfec = rec.field0;
            *g_009bbff0 = rec.field1;
            strcat(msg, rec.label);
            strcat(msg, reinterpret_cast<char *>(g_00682e94));
            strcat(msg, reinterpret_cast<char *>(strings->get(tbl[0])));
            strcat(msg, reinterpret_cast<char *>(g_00682820));
            say_year(msg);
        } else {
            strcat(msg, reinterpret_cast<char *>(g_00682820));
            strcat(msg, reinterpret_cast<char *>(g_006863a8));
            char numbuf[80];
            reinterpret_cast<ItoaFn>(_itoa)(
                *reinterpret_cast<int *>(self + 0x1DD78) + 1, numbuf, 10);
            strcat(msg, numbuf);
        }
    } else {
        int val = (a1 == 3) ? tbl[0x204] : tbl[0x1E];
        strcat(msg, reinterpret_cast<char *>(strings->get(val)));
    }

    int flags = 0x20000430;
    if (a1 == 3) {
        flags = 0x400020;
    } else if (a1 == 0 && *reinterpret_cast<int *>(self + 0x1DD78) != 0) {
        flags = 0x20010430;
    }
    if (this == reinterpret_cast<MapWin *>(0x8EB48C)) {
        flags = (flags & 0xFFFFFFEF) | 0x40400001;
    }
    this->init(&rect, msg, flags);

    if (a1 == 0) {
        *reinterpret_cast<int *>(self + 0x1DD70) = 0x10ee3;
        *reinterpret_cast<int *>(self + 0x1DD98) =
            -static_cast<int>(*reinterpret_cast<int *>(self + 0x1DD78) != 0) & 0xFFFFFFF8;
    } else {
        *reinterpret_cast<int *>(self + 0x1DD70) = 0x80000082;
        *reinterpret_cast<int *>(self + 0x1DD98) = -7;
    }

    if (*reinterpret_cast<int *>(self + 0x1DD78) != 0) {
        int *vt = *reinterpret_cast<int **>(self);
        int off = *reinterpret_cast<int *>(reinterpret_cast<char *>(vt) + 4);
        *reinterpret_cast<int *>(self + off + 0xF8) = *g_00945778;
    }

    if (a1 == 0) {
        this->do_image_buttons();

        int flags2 = *reinterpret_cast<int *>(self + 0x1DD70) & 0x9FFFFFFF;
        int idx2 = *reinterpret_cast<int *>(self + 0x1DD78);
        *reinterpret_cast<int *>(self + 0x1DD70) = flags2;

        if (idx2 == 0) {
            if (*g_0093f660 == 0) {
                flags2 |= 0x60000000;
            } else {
                flags2 |= 0x20000000;
            }
            *reinterpret_cast<int *>(self + 0x1DD70) = flags2;
        } else {
            if (idx2 == 1) {
                flags2 |= 0x40000000;
                *reinterpret_cast<int *>(self + 0x1DD70) = flags2;
            }
            *msg = 0;
            int val2 = (flags2 & 0x40000000) ? tbl[0x100] : tbl[0x101];
            strcat(msg, reinterpret_cast<char *>(strings->get(val2)));

            int *vt2 = *reinterpret_cast<int **>(self);
            int off2 = *reinterpret_cast<int *>(reinterpret_cast<char *>(vt2) + 4);
            reinterpret_cast<Win *>(self + off2)->set_caption(msg);
        }

        if (a2 == 0) {
            if (*g_0068f21c != 0) {
                *reinterpret_cast<int *>(self + 0x1DD70) &= 0xFFFFF73F;
            }
            int *vt3 = *reinterpret_cast<int **>(self);
            int off3 = *reinterpret_cast<int *>(reinterpret_cast<char *>(vt3) + 4);
            reinterpret_cast<VCall *>(self + off3)->slot062();
            *reinterpret_cast<int *>(self + 0x1DD70) |= 0x8C0;
        }
    }
}
