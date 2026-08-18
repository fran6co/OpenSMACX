// ORIGINAL: 0x005169F0 ?end_my_turn@Console@@QAEXXZ 0x005169F0-0x00516C4D FILE
// size      605 bytes
// prototype void (__thiscall ?end_my_turn@Console@@QAEXXZ)(Console* this)
// callers   5   call targets   13
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00446A00 0x0047A890 0x004B8970 0x004BDDE0 0x004E1F40 0x004E2660 0x00510CE0 0x00511520 0x00592EE0 0x005BF310 0x005C1B70 0x00625EC0 0x00631A20
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005169F0
// measured tier  MISMATCH
// divergence     35
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005169f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?end_my_turn@Console@@QAEXXZ  at 0x005169F0  (605 bytes)
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

class AlphaNet;
class AutoSound;
struct BITMAPINFO;
typedef int BOOL;
class Buffer;
class Console;
class FX;
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
class Net;
class NetMsg;
typedef void * PVOID;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class StatusWin;
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
class AlphaNet { public:
    uint32_t alignment_;
    uint8_t data_[0x149C];
    int who_2_pid(int);
};
class FX { public:
    uint8_t effects_[0x61 * 0x6C];
    uint32_t field_28EC_;
    void play(int);
};
class Net { public:
    char * get_player_name(unsigned long);
};
class NetMsg { public:
    void pop(const char *, int, int, const char *);
};
class StatusWin { public:
    void reset();
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
    int reset();
};
int __cdecl X_pop(const char *, int (__cdecl *)());
int __cdecl parse_says(int, char *, int, int);
int __cdecl veh_ready(int);
void __cdecl message_data(int, int, int, int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068a9d8 = (int *)0x0068A9D8;
static int *const g_0068a9e4 = (int *)0x0068A9E4;
static int *const g_0068a9f0 = (int *)0x0068A9F0;
static int *const g_0068aa00 = (int *)0x0068AA00;
static int *const g_006e8150 = (int *)0x006E8150;
static int *const g_00749cf8 = (int *)0x00749CF8;
static int *const g_007ad348 = (int *)0x007AD348;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_008c5568 = (int *)0x008C5568;
static int *const g_008c6e68 = (int *)0x008C6E68;
static int *const g_009156b0 = (int *)0x009156B0;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00939288 = (int *)0x00939288;
static int *const g_0093a938 = (int *)0x0093A938;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093e8ec = (int *)0x0093E8EC;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009a64d4 = (int *)0x009A64D4;
static int *const g_009a681c = (int *)0x009A681C;
static int *const g_009a6820 = (int *)0x009A6820;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

class Console { public:
    uint8_t derived_storage_[0x23D94];
    GraphicWin virtual_base_;

    int edit_lock();
    int ready_search(int);
    void set_move(int);
    void end_my_turn();
};

// Indexed table bases (record stride 0x59C, the faction-record stride seen
// elsewhere in this wave): a plain-pointer global lets /O2 fold the
// `idx * 0x59C` scale-and-add into a constant, losing the addressing mode
// the original used.
extern int g_00946a50[];
extern char g_00946a9c[];
extern char g_00946a84[];
extern int g_00946d4c[];
extern int g_00946d50[];
extern char g_00946d34[];

void Console::end_my_turn() {
    char *self = reinterpret_cast<char *>(this);

    *g_006e8150 = 0;
    *g_007ad348 = 0;
    reinterpret_cast<TutWin *>(g_008c6e68)->reset();
    reinterpret_cast<StatusWin *>(g_008c5568)->reset();
    reinterpret_cast<FX *>(g_00749cf8)->play(0xf);

    if (*g_0093a938 != 0)
        return;

    if (edit_lock() != 0) {
        reinterpret_cast<NetMsg *>(g_00805338)
            ->pop(reinterpret_cast<const char *>(g_0068a9d8), 0x1388, 0, 0);
        return;
    }

    unsigned char flagByte = *reinterpret_cast<unsigned char *>(g_009a64c0);

    if (*g_0093f660 != 0) {
        if ((*g_009a681c & 0x10) != 0 && *g_009a6820 != *g_00939284) {
            int idx = *g_009a6820;
            *g_009bbff0 = 0;
            *g_009bbfec = g_00946a50[idx * 0x167];
            parse_says(0, g_00946a9c + idx * 0x59c, -1, -1);

            idx = *g_009a6820;
            *g_009bbff0 = 0;
            *g_009bbfec = g_00946a50[idx * 0x167];
            parse_says(1, g_00946a84 + idx * 0x59c, -1, -1);

            idx = *g_009a6820;
            *g_009bbfec = g_00946d4c[idx * 0x167];
            *g_009bbff0 = g_00946d50[idx * 0x167];
            parse_says(2, g_00946d34 + idx * 0x59c, -1, -1);

            int pid = reinterpret_cast<AlphaNet *>(g_0093cd90)->who_2_pid(*g_009a6820);
            char *name = reinterpret_cast<Net *>(g_0093cd90)->get_player_name(pid);
            parse_says(3, name, -1, -1);

            reinterpret_cast<NetMsg *>(g_00805338)
                ->pop(reinterpret_cast<const char *>(g_0068a9e4), 0x1388, 0, 0);
            return;
        }
        if ((flagByte & 2) != 0 && (*g_009a681c & 0x10) == 0) {
            int mask = 1 << *g_00939284;
            if ((*g_0093e8ec & mask) != 0 && *g_0093e8ec != 0xff) {
                reinterpret_cast<NetMsg *>(g_00805338)
                    ->pop(reinterpret_cast<const char *>(g_0068a9f0), 0x1388, 0, 0);
                message_data(0x2302, 0, *g_009a64d4, 0, 0, 0);
                return;
            }
        }
    }

    if (*reinterpret_cast<int *>(self + 0x23be8) == 0)
        return;

    if ((flagByte & 2) != 0)
        goto setFlags;

    set_move(1);
    if (*g_00939288 == 0)
        goto setFlags;

    {
        int vehId = *reinterpret_cast<int *>(self + 0x23bdc);
        if (vehId < 0)
            goto setFlags;

        if (veh_ready(vehId) == 0) {
            int rs = reinterpret_cast<Console *>(g_009156b0)
                         ->ready_search(*reinterpret_cast<int *>(self + 0x23bdc));
            if (rs < 0)
                goto setFlags;
        }
    }

    if (X_pop(reinterpret_cast<const char *>(g_0068aa00), 0) == 0)
        return;

setFlags:
    *reinterpret_cast<unsigned char *>(g_009a64c0) =
        *reinterpret_cast<unsigned char *>(g_009a64c0) | 2;
    {
        int newVeh = *reinterpret_cast<int *>(self + 0x23bdc);
        *reinterpret_cast<int *>(self + 0x23be4) = 0;
        *reinterpret_cast<int *>(self + 0x23be8) = 0;
        *reinterpret_cast<int *>(self + 0x23be0) = newVeh;
    }
}
