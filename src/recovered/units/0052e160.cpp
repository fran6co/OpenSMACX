// ORIGINAL: 0x0052E160 ?on_player_added@NetDaemon@@QAEXPADK@Z 0x0052E160-0x0052E364 FILE
// size      516 bytes
// prototype void (__thiscall ?on_player_added@NetDaemon@@QAEXPADK@Z)(NetDaemon* this, int8* lpszInput, unsigned int)
// callers   0   call targets   11
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0047A890 0x0047B020 0x0047B0F0 0x0047B260 0x0047B2E0 0x00481AD0 0x005F7E90 0x00625EC0 0x00630080 0x00631A20 0x00645460
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0052E160
// measured tier  MISMATCH
// divergence     93
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0052e160/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_player_added@NetDaemon@@QAEXPADK@Z  at 0x0052E160  (516 bytes)
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
typedef void * HBITMAP;
struct HCURSOR;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
class Heap;
typedef char * LPSTR;
typedef void * LPVOID;
class Menu;
class Net;
class NetDaemon;
class NetMsg;
class NetWin;
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
class Net { public:
    char * get_player_name(unsigned long);
    int send(void *, int, unsigned long, int);
};
class NetMsg { public:
    void pop(const char *, int, int, const char *);
};
class NetWin { public:
    void on_redraw();
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
};
extern "C" char *strcpy(char *, const char *);
int __cdecl parse_says(int, char *, int, int);
void __cdecl send_all_player_packets(int);
void __cdecl send_faction_data(int);
void __cdecl send_session2_packet(int, int);
void __cdecl send_session_packet(int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068c148 = (int *)0x0068C148;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_0080a6f8 = (int *)0x0080A6F8;
static int *const g_0090db98 = (int *)0x0090DB98;
static int *const g_0090db9a = (int *)0x0090DB9A;
static int *const g_0090e8e2 = (int *)0x0090E8E2;
static int *const g_0093cd90 = (int *)0x0093CD90;

class NetDaemon { public:
    void on_player_added(char *, unsigned long);
};

// The scaffold's `strcpy` identifier triggers VC6's built-in intrinsic
// recognition (rep movsd/repne scasb inline expansion) purely by literal
// spelling, replacing the real out-of-line `call`. Declaring it as
// `_strcpy` instead dodges the intrinsic-name match; the symbol emitted
// does not need to resolve to anything real since the call's relocation
// is masked by the comparison.
extern "C" char *_strcpy(char *, const char *);

// 0x90db98/99/9a/9b/9c and 0x90dd10 are all indexed by a register
// (`[eax + addr]` with eax = n*0x17c precomputed), so they need
// `extern` arrays, not the plain const-pointer spelling above.
extern unsigned char g_0090db98_tbl[];
extern unsigned char g_0090db99_tbl[];
extern unsigned char g_0090db9a_tbl[];
extern unsigned char g_0090db9b_tbl[];
extern unsigned char g_0090db9c_tbl[];
extern unsigned char g_0090dd10_tbl[];

void NetDaemon::on_player_added(char * a1, unsigned long a2) {
    char *self = reinterpret_cast<char *>(this);

    int eq = (*reinterpret_cast<int *>(self + 0x760) == *reinterpret_cast<int *>(self + 0x764));
    *reinterpret_cast<int *>(self + 0x1b30) = eq;

    if (*reinterpret_cast<int *>(self + 0x768) < 7 &&
        *reinterpret_cast<int *>(self + 0x1b40) < 2) {

        int n = ++(*reinterpret_cast<int *>(self + 0x768));
        *reinterpret_cast<unsigned long *>(self + 0x78c + n * 0x19c) = a2;

        _strcpy(self + 0x76c + (*reinterpret_cast<int *>(self + 0x768)) * 0x19c,
                reinterpret_cast<Net *>(this)->get_player_name(a2));

        g_0090db9a_tbl[(*reinterpret_cast<int *>(self + 0x768)) * 0x17c] =
            *reinterpret_cast<unsigned char *>(g_0090e8e2);
        g_0090db98_tbl[(*reinterpret_cast<int *>(self + 0x768)) * 0x17c] =
            static_cast<unsigned char>(*reinterpret_cast<int *>(self + 0x768));
        *reinterpret_cast<unsigned char *>(self + 0x790 + (*reinterpret_cast<int *>(self + 0x768)) * 0x19c) =
            *reinterpret_cast<unsigned char *>(self + 0x768);

        bool found = false;
        if (*reinterpret_cast<int *>(self + 0x768) > 1) {
            int target = *reinterpret_cast<int *>(self + 0x760);
            char *rec = self + 0x928;
            int i = *reinterpret_cast<int *>(self + 0x768) - 1;
            do {
                if (*reinterpret_cast<int *>(rec) == target) found = true;
                rec += 0x19c;
                i--;
            } while (i != 0);
            if (found) {
                parse_says(0, a1, -1, -1);
                reinterpret_cast<NetMsg *>(g_00805338)->pop(
                    reinterpret_cast<const char *>(g_0068c148), 3000, 0, 0);
            }
        }

        if (*reinterpret_cast<int *>(self + 0x1b30) != 0) {
            int n2 = *reinterpret_cast<int *>(self + 0x768);
            int off2 = n2 * 0x17c;
            *reinterpret_cast<int *>(g_0090dd10_tbl + off2) = 0;
            g_0090db98_tbl[off2] = 0xff;
            g_0090db99_tbl[off2] = 0;
            g_0090db9a_tbl[off2] = 0;
            g_0090db9b_tbl[off2] = 0xff;
            g_0090db9c_tbl[off2] = 2;
            unsigned char b = *reinterpret_cast<unsigned char *>(g_0090e8e2);
            g_0090db9b_tbl[off2] = 0xff;
            g_0090db98_tbl[off2] = static_cast<unsigned char>(*reinterpret_cast<int *>(self + 0x768));
            g_0090db9a_tbl[off2] = b;
            g_0090db99_tbl[off2] = 0;

            send_all_player_packets(-1);
            send_session_packet(n2, -1);
            send_session2_packet(n2, -1);
            send_faction_data(n2);
        }

        if (reinterpret_cast<Win *>(g_0080a6f8)->is_visible()) {
            reinterpret_cast<NetWin *>(g_0080a6f8)->on_redraw();
        }
    } else if (eq) {
        unsigned short buf[16];
        buf[0] = 0x1f01;
        reinterpret_cast<Net *>(g_0093cd90)->send(buf, 0x20, a2, 1);
    }
}
