// ORIGINAL: 0x00478150 FILE
// name      ?on_right_click@MultiWin@@QAEXHH@Z
// size      1215 bytes
// spans     0x00478150-0x004785FA;0x006563E0-0x006563F5
// prototype void (__thiscall ?on_right_click@MultiWin@@QAEXHH@Z)(MultiWin* this, int xCoord, int yCoord)
// callers   0   call targets   20
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00479330 0x0047A890 0x004ACBD0 0x00515770 0x0052C880 0x0053E4C0 0x0053E5C0 0x00558C60 0x005ED240 0x005F8800 0x005F88A0 0x005F8920 0x005F89D0 0x005F8BE0 0x005F9170 0x005F9F40 0x005FAB00 0x006169A0 0x00625EC0 0x00645470
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00478150
// measured tier  NO_COMPILE
// refusal        u00478150.cpp(147) : error C2079: 'spot_' uses undefined class 'Spot'
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00478150/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_right_click@MultiWin@@QAEXHH@Z  at 0x00478150  (1215 bytes)
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
class MultiWin;
class NetMsg;
typedef void * PVOID;
class PullDown;
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

struct RECT {
    long left;
    long top;
    long right;
    long bottom;
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
class Console { public:
    uint8_t derived_storage_[0x23D94];
    GraphicWin virtual_base_;
    void chat(int);
};
class NetMsg { public:
    void pop(const char *, int, int, const char *);
};
class PullDown { public:
    PullDown();
    int add_item(int, char *);
    int add_separator(int);
    int get_selected();
    int show(int, int, int (__cdecl *)());
    void init(Menu *);
    ~PullDown();
};
class Spot { public:
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
    int check(int, int, int *, int *);
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
    void client_to_screen(int *, int *);
};
extern "C" char *strcat(char *, const char *);
int __cdecl parse_says(int, char *, int, int);
void __cdecl call_council(int);
void __cdecl commlink_attempt(int);
void __cdecl demands_withdrawal(int, int);
void __cdecl renounce_pact(int, int);
void __cdecl report_intel(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_004785fc = (int *)0x004785FC;
static int *const g_00539920 = (int *)0x00539920;
static int *const g_006563eb = (int *)0x006563EB;
static int *const g_00676190 = (int *)0x00676190;
static int *const g_00682820 = (int *)0x00682820;
static int *const g_0068677c = (int *)0x0068677C;
static int *const g_007fff74 = (int *)0x007FFF74;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_009156b0 = (int *)0x009156B0;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093e960 = (int *)0x0093E960;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_0096c9f8 = (int *)0x0096C9F8;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009a64e9 = (int *)0x009A64E9;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

class MultiWin { public:
    void draw(int);
    void on_right_click(int, int);
};

extern char g_00946a50[];
extern char g_00946a84[];
extern char g_00946a9c[];
extern char g_00946d34[];
extern char g_00946d4c[];
extern char g_00946d50[];

void MultiWin::on_right_click(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    PullDown menu;

    int local14, local18;
    int rc = reinterpret_cast<Spot *>(self + 0x28ac)->check(a1, a2, &local14, &local18);
    if (rc < 0 || local14 < 0 || local18 != 1) {
        return;
    }

    reinterpret_cast<Win *>(this)->client_to_screen(&a1, &a2);
    menu.init(0);

    char *msg = reinterpret_cast<char *>(g_009b86a0);
    *msg = 0;

    strcat(msg, reinterpret_cast<char *>(reinterpret_cast<Strings *>(0x9b90d8)->get(
                    *reinterpret_cast<int32_t *>(*reinterpret_cast<char **>(g_009b90f8) + 0x9f0))));
    strcat(msg, reinterpret_cast<char *>(g_00682820));
    strcat(msg, g_00946a9c + local14 * 0x59c);
    strcat(msg, reinterpret_cast<char *>(g_00682820));
    strcat(msg, g_00946a84 + local14 * 0x59c);
    menu.add_item(1, msg);

    menu.add_item(2, reinterpret_cast<char *>(reinterpret_cast<Strings *>(0x9b90d8)->get(
                          *reinterpret_cast<int32_t *>(*reinterpret_cast<char **>(g_009b90f8) + 0x9f4))));

    uint8_t flags = *reinterpret_cast<uint8_t *>(g_0096c9f8 + local14 * 4 + *g_00939284 * 0x20cc);
    if (flags & 7) {
        menu.add_separator(0x63);
        if (flags & 1) {
            menu.add_item(8, reinterpret_cast<char *>(reinterpret_cast<Strings *>(0x9b90d8)->get(
                                  *reinterpret_cast<int32_t *>(*reinterpret_cast<char **>(g_009b90f8) + 0xe20))));
        } else {
            menu.add_item(9, reinterpret_cast<char *>(reinterpret_cast<Strings *>(0x9b90d8)->get(
                                  *reinterpret_cast<int32_t *>(*reinterpret_cast<char **>(g_009b90f8) + 0xe24))));
        }
    }

    menu.add_separator(0x63);
    menu.add_item(3, reinterpret_cast<char *>(reinterpret_cast<Strings *>(0x9b90d8)->get(
                         *reinterpret_cast<int32_t *>(*reinterpret_cast<char **>(g_009b90f8) + 0x9f8))));

    if (*g_0093f660 != 0) {
        menu.add_separator(0x63);
        menu.add_item(0xa, reinterpret_cast<char *>(reinterpret_cast<Strings *>(0x9b90d8)->get(
                                *reinterpret_cast<int32_t *>(*reinterpret_cast<char **>(g_009b90f8) + 0x9fc))));
        menu.add_item(0xb, reinterpret_cast<char *>(reinterpret_cast<Strings *>(0x9b90d8)->get(
                                *reinterpret_cast<int32_t *>(*reinterpret_cast<char **>(g_009b90f8) + 0xa00))));
    }

    menu.show(a1, a2, reinterpret_cast<int (__cdecl *)()>(g_00539920));
    if (*g_009a64c0 & 0x800) {
        return;
    }

    int sel = menu.get_selected();
    if (sel >= 0x40) {
        return;
    }

    int val = *reinterpret_cast<int32_t *>(reinterpret_cast<char *>(&a1) + sel * 20 - 0x534 + 8);
    if (val < 0) {
        return;
    }

    if (!(*reinterpret_cast<uint8_t *>(g_009a64e9) & (1 << local14))) {
        parse_says(0, g_00946a9c + local14 * 0x59c, -1, -1);
        parse_says(1, g_00946a84 + local14 * 0x59c, -1, -1);
        parse_says(2, g_00946d34 + local14 * 0x59c, *reinterpret_cast<int32_t *>(g_00946d4c + local14 * 0x59c),
                    *reinterpret_cast<int32_t *>(g_00946d50 + local14 * 0x59c));
        reinterpret_cast<NetMsg *>(g_00805338)->pop(reinterpret_cast<const char *>(g_0068677c), 0x1388, 0, 0);
        return;
    }

    switch (val) {
        case 1:
            commlink_attempt(local14);
            break;
        case 2:
            report_intel(local14);
            break;
        case 3:
            call_council(*g_00939284);
            break;
        case 4:
            renounce_pact(*g_00939284, local14);
            break;
        case 5:
            demands_withdrawal(local14, *g_00939284);
            break;
        case 6: {
            uint32_t bit = 1u << local14;
            uint32_t x = *reinterpret_cast<uint32_t *>(g_007fff74);
            if (bit & x) {
                *reinterpret_cast<uint32_t *>(g_007fff74) = x & ~bit;
                *reinterpret_cast<uint32_t *>(g_0093e960) |= bit;
                draw(0);
            } else {
                uint32_t y = *reinterpret_cast<uint32_t *>(g_0093e960);
                if (bit & y) {
                    *reinterpret_cast<uint32_t *>(g_0093e960) = y & ~bit;
                    draw(0);
                } else {
                    *reinterpret_cast<uint32_t *>(g_007fff74) |= bit;
                    draw(0);
                }
            }
            break;
        }
        case 9:
            reinterpret_cast<Console *>(0x9156b0)->chat(local14);
            break;
        default:
            break;
    }
}
