// ORIGINAL: 0x00466CA0 FILE
// name      ?draw_vehicles@MapWin@@QAEXHHHHH@Z
// size      1263 bytes
// spans     0x00466CA0-0x0046718F
// prototype void (__thiscall ?draw_vehicles@MapWin@@QAEXHHHHH@Z)(MapWin* this, int, int, int, int, int)
// callers   3   call targets   8
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00463040 0x00467190 0x005594F0 0x005BFE90 0x005C1A20 0x005C1B70 0x005EC8A0 0x00644F3A
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00466CA0
// measured tier  MISMATCH
// divergence     25
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00466ca0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_vehicles@MapWin@@QAEXHHHHH@Z  at 0x00466CA0  (1263 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
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
    void get_mouse_pos(int *, int *);
};
extern "C" int abs(int);
int __cdecl veh_at(int, int);
int __cdecl veh_draw(Buffer *, int, int, int, int, int, int);
int __cdecl veh_ready(int);
int __cdecl veh_selectable(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00689f28 = (int *)0x00689F28;
static int *const g_00689f2c = (int *)0x00689F2C;
static int *const g_00689f30 = (int *)0x00689F30;
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_006e8134 = (int *)0x006E8134;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00939288 = (int *)0x00939288;
static int *const g_0093928c = (int *)0x0093928C;
static int *const g_00939294 = (int *)0x00939294;
static int *const g_009392a8 = (int *)0x009392A8;
static int *const g_009392b0 = (int *)0x009392B0;
static int *const g_0093a938 = (int *)0x0093A938;
static int *const g_0093a940 = (int *)0x0093A940;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_0094988c = (int *)0x0094988C;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_0096c9e0 = (int *)0x0096C9E0;
static int *const g_009a6494 = (int *)0x009A6494;
static int *const g_009a649c = (int *)0x009A649C;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009b2070 = (int *)0x009B2070;

class MapWin { public:
    uint32_t vbtable_pointer_;
    void * owned_;
    uint8_t derived_tail_[0x21A6C - 0x8];
    GraphicWin virtual_base_;

    int pixel_to_tile(int, int, int *, int *);
    void draw_bases(int, int, int, int, int);
    void draw_vehicles(int, int, int, int, int);
};

extern unsigned char g_0096c9e0_arr[];
extern unsigned char g_00952828_arr[];
extern unsigned char g_009ab88c_arr[];

void MapWin::draw_vehicles(int a1, int a2, int a3, int a4, int a5) {
    char *self = reinterpret_cast<char *>(this);
    int state = 2;

    if (*g_0093a940 == 0 && *g_009b2070 == 0 &&
        (*reinterpret_cast<unsigned char *>(0x9a649c) & 0x80) == 0)
        return;

    unsigned int flags = *reinterpret_cast<unsigned int *>(self + 0x1dd70);
    if ((flags & 0x1000000) != 0 || (flags & 0x802000c0) == 0 || (flags & 0x100000) != 0)
        return;

    int mid = a1 >> 1;

    bool drewBase = false;
    if ((flags & 0x80000000u) == 0 || (flags & 0x80) != 0) {
        if ((*reinterpret_cast<unsigned char *>(0x9a64c0) & 0x80) != 0) {
            drewBase = true;
        } else {
            unsigned int rec = *reinterpret_cast<unsigned int *>(g_0096c9e0_arr + a5 * 0x20cc);
            if ((rec & 0x200) != 0) {
                drewBase = true;
            } else {
                int idx = (*g_0068faf0 * a2 + (a1 >> 1)) * 11;
                unsigned char bit = static_cast<unsigned char>(1 << a5);
                unsigned char *base = *reinterpret_cast<unsigned char **>(g_0094a30c);
                if ((base[idx * 4 + 4] & bit) != 0)
                    drewBase = true;
            }
        }
    }
    if (drewBase)
        draw_bases(a1, a2, a3, a4, a5);

    if ((*reinterpret_cast<unsigned int *>(self + 0x1dd70) & 0x80000000u) != 0)
        return;

    int target;

    if (*g_00689f30 != -1 &&
        *reinterpret_cast<short *>(g_00952828_arr + *g_00689f30 * 0x34) == a1 &&
        *reinterpret_cast<short *>(g_00952828_arr + *g_00689f30 * 0x34 + 2) == a2) {
        target = *g_00689f30;
        state = 3;
        goto finish;
    }

    if (*g_00689f28 >= 0 &&
        *reinterpret_cast<short *>(g_00952828_arr + *g_00689f28 * 0x34) == a1 &&
        *reinterpret_cast<short *>(g_00952828_arr + *g_00689f28 * 0x34 + 2) == a2) {
        target = *g_00689f28;
        int sub = *reinterpret_cast<short *>(g_00952828_arr + target * 0x34 + 0xa) * 0x34;
        if (g_009ab88c_arr[sub] == 8) {
            if (target < 0)
                return;
            target = *reinterpret_cast<short *>(g_00952828_arr + target * 0x34 + 0x30);
            if (target < 0)
                return;
        }
        goto finish;
    }

    if (*g_00689f2c >= 0 &&
        *reinterpret_cast<short *>(g_00952828_arr + *g_00689f2c * 0x34) == a1 &&
        *reinterpret_cast<short *>(g_00952828_arr + *g_00689f2c * 0x34 + 2) == a2) {
        target = *g_00689f2c;
        goto finish;
    }

    {
        bool anim = false;
        if ((*reinterpret_cast<unsigned char *>(0x9a64c0) & 0x80) != 0) {
            anim = true;
        } else {
            unsigned int rec = *reinterpret_cast<unsigned int *>(g_0096c9e0_arr + a5 * 0x20cc);
            if ((rec & 0x200) != 0) {
                anim = true;
            } else {
                int idx = (*g_0068faf0 * a2 + mid) * 11;
                unsigned char bit = static_cast<unsigned char>(1 << a5);
                unsigned char *base = *reinterpret_cast<unsigned char **>(g_0094a30c);
                if ((base[idx * 4 + 4] & bit) != 0)
                    anim = true;
            }
        }
        if (!anim)
            return;
    }

    state = 3;

    if (*reinterpret_cast<short *>(g_00952828_arr + *g_0093928c * 0x34) == a1 &&
        *reinterpret_cast<short *>(g_00952828_arr + *g_0093928c * 0x34 + 2) == a2 &&
        ((1 << *reinterpret_cast<unsigned char *>(g_00952828_arr + *g_0093928c * 0x34 + 0xe)) &
         *reinterpret_cast<unsigned char *>(0x9a64e8)) != 0 &&
        veh_selectable(*g_0093928c) != 0 &&
        (*reinterpret_cast<unsigned int *>(g_00952828_arr + *g_0093928c * 0x34 + 4) & 0x4200) == 0) {

        target = *g_0093928c;

        if (*g_00939288 != 0 && *g_009392a8 != 0 && *g_00939294 != 0 &&
            *reinterpret_cast<unsigned char *>(g_00952828_arr + target * 0x34 + 0x11) == 0 &&
            *g_009392b0 == 0 &&
            (*g_0093f660 == 0 || *g_0093a938 == 0)) {

            if ((*reinterpret_cast<unsigned int *>(0x9a6494) & 0x200) != 0)
                return;

            int mouseA, mouseB = a1;
            {
                void *vtbl = *reinterpret_cast<void **>(self);
                int adj = *reinterpret_cast<int *>(reinterpret_cast<char *>(vtbl) + 4);
                reinterpret_cast<Win *>(self + adj)->get_mouse_pos(&mouseA, &mouseB);
            }
            int tileX, tileY;
            pixel_to_tile(mouseA, mouseB, &tileX, &tileY);

            int fx = *reinterpret_cast<short *>(g_00952828_arr + target * 0x34);
            int fy = *reinterpret_cast<short *>(g_00952828_arr + target * 0x34 + 2);
            int dx = tileX - fx;
            int dy = tileY - fy;
            int adx = abs(dx);
            if ((*reinterpret_cast<unsigned char *>(0x94988c) & 1) == 0 && *g_0068faf0 < adx)
                a5 = *g_00949870 - adx;
            else
                a5 = adx;
            int ady = abs(dy);
            if (((ady + a5) & ~1) < 3)
                return;
            state = 0x403;
        }
    } else {
        target = *g_0093928c;
        if ((*reinterpret_cast<unsigned int *>(self + 0x1dd70) & 0x200040u) == 0)
            return;
        target = veh_at(a1, a2);
    }

    if ((*reinterpret_cast<unsigned int *>(self + 0x1dd70) & 0x200000u) != 0) {
        target = veh_at(a1, a3);
        if (target < 0)
            return;
        while (*reinterpret_cast<short *>(g_00952828_arr + target * 0x34 + 0x2e) != *g_006e8134) {
            target = *reinterpret_cast<short *>(g_00952828_arr + target * 0x34 + 0x30);
            if (target < 0)
                return;
        }
    }

    if (target < 0)
        return;

    if ((*reinterpret_cast<unsigned char *>(0x9a64c0) & 0x80) == 0) {
        unsigned char cl = *reinterpret_cast<unsigned char *>(g_00952828_arr + target * 0x34 + 0xe);
        unsigned short flag2 = *reinterpret_cast<unsigned short *>(g_00952828_arr + target * 0x34 + 8);
        if (cl != *g_00939284 || (flag2 & 0x440) == 0x440) {
            unsigned char al = *reinterpret_cast<unsigned char *>(g_00952828_arr + target * 0x34 + 0x27);
            if ((al & static_cast<unsigned char>(1 << (*g_00939284 & 0x1f))) == 0)
                return;
        }
    }

    {
        int idx2 = (*g_0068faf0 * a2 + (a1 >> 1)) * 11;
        unsigned char *base2 = *reinterpret_cast<unsigned char **>(g_0094a30c);
        unsigned char cl2 = base2[idx2 * 4 + 8];
        unsigned char *rec2 = base2 + idx2 * 4;
        if ((cl2 & 1) != 0) {
            int v = rec2[2] & 0xf;
            if (v < 8 && v >= 0) {
                if (*g_00939288 == 0)
                    return;
                if (target != *g_0093928c)
                    return;
                if (veh_ready(target) == 0)
                    return;
                if ((*reinterpret_cast<unsigned int *>(g_00952828_arr + target * 0x34 + 4) & 0x4200) != 0)
                    return;
                if (*g_00939294 == 0)
                    return;
            }
        }
    }

finish:
    {
        Buffer *bufferPtr;
        if (this == 0) {
            bufferPtr = 0;
        } else {
            void *vtbl = *reinterpret_cast<void **>(self);
            int adj = *reinterpret_cast<int *>(reinterpret_cast<char *>(vtbl) + 4);
            bufferPtr = reinterpret_cast<Buffer *>(adj + 0x444 + reinterpret_cast<int>(self));
        }
        veh_draw(bufferPtr, target, a3, a4, *reinterpret_cast<int *>(self + 0x1dd98), state, 1);
    }
}
