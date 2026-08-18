// ORIGINAL: 0x00529F30 ?net_end_of_turn@@YAXXZ 0x00529F30-0x0052A2D3 FILE
// size      931 bytes
// prototype 
// callers   1   call targets   21
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0041A4C0 0x00446A00 0x00479330 0x0047A7D0 0x0047A890 0x0050EF50 0x00511480 0x0051E760 0x00529C00 0x00530320 0x00530360 0x0058FFA0 0x005904A0 0x00592EE0 0x005A94F0 0x005F7E90 0x005FC700 0x00626250 0x006262F0 0x00645470 0x0064FC88
// indirect  0x0052A041 0x0052A09B
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00529F30
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00529f30/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?net_end_of_turn@@YAXXZ  at 0x00529F30  (931 bytes)
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
class BaseWin;
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
class Lock;
class Menu;
class MultiWin;
class NetDaemon;
class NetMsg;
typedef void * PVOID;
class PlayerLock;
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
class BaseWin { public:
    void exit();
};
class Console { public:
    uint8_t derived_storage_[0x23D94];
    GraphicWin virtual_base_;
    void set_view(int);
};
class FX { public:
    uint8_t effects_[0x61 * 0x6C];
    uint32_t field_28EC_;
    void play(int);
};
class Lock { public:
    int any_locks();
};
class MultiWin { public:
    void draw(int);
};
class NetDaemon { public:
    int receive();
    void net_tasks();
};
class NetMsg { public:
    void close();
    void pop(const char *, int, int, const char *);
};
class PlayerLock { public:
    void active();
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
extern "C" char *strcat(char *, const char *);
extern "C" int __cdecl _itoa();
int __cdecl save_daemon(char *);
void __cdecl do_checksums(int);
void __cdecl go_reset();
void __cdecl log_say(char *, char *, int, int, int);
void __cdecl log_say(char *, int, int, int);
void __cdecl mash_planes();
void __cdecl message_data(int, int, int, int, int, int);
void __cdecl wait_task();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669368 = (int *)0x00669368;
static int *const g_0068bd9c = (int *)0x0068BD9C;
static int *const g_0068bdb8 = (int *)0x0068BDB8;
static int *const g_0068bdc8 = (int *)0x0068BDC8;
static int *const g_0068bddc = (int *)0x0068BDDC;
static int *const g_0068bdfc = (int *)0x0068BDFC;
static int *const g_0068be18 = (int *)0x0068BE18;
static int *const g_0068be1c = (int *)0x0068BE1C;
static int *const g_0068be20 = (int *)0x0068BE20;
static int *const g_006a7628 = (int *)0x006A7628;
static int *const g_00749cf8 = (int *)0x00749CF8;
static int *const g_007fd648 = (int *)0x007FD648;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_009156b0 = (int *)0x009156B0;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00939294 = (int *)0x00939294;
static int *const g_0093a938 = (int *)0x0093A938;
static int *const g_0093a93c = (int *)0x0093A93C;
static int *const g_0093a94c = (int *)0x0093A94C;
static int *const g_0093a950 = (int *)0x0093A950;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093d4f8 = (int *)0x0093D4F8;
static int *const g_0093e230 = (int *)0x0093E230;
static int *const g_0093e24c = (int *)0x0093E24C;
static int *const g_0093e8bc = (int *)0x0093E8BC;
static int *const g_0093e8c0 = (int *)0x0093E8C0;
static int *const g_0093e8e0 = (int *)0x0093E8E0;
static int *const g_0093e8ec = (int *)0x0093E8EC;
static int *const g_0093e8f0 = (int *)0x0093E8F0;
static int *const g_0093e8f8 = (int *)0x0093E8F8;
static int *const g_0093e8fc = (int *)0x0093E8FC;
static int *const g_0093e90c = (int *)0x0093E90C;
static int *const g_009a6490 = (int *)0x009A6490;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009a64d4 = (int *)0x009A64D4;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009a681c = (int *)0x009A681C;
static int *const g_009b2068 = (int *)0x009B2068;
static int *const g_009b86a0 = (int *)0x009B86A0;

// The scaffolding's `_itoa()` takes no arguments (a fallback), so the
// real 3-argument call is declared here under a different name; the
// relocation it needs is masked by the comparison regardless of the
// symbol it names.
extern "C" char *__cdecl my_itoa(int, char *, int);

typedef unsigned long (__stdcall *FnTimeGetTime)();

// PlayerLock::active() is catalogued void (QAEXXZ) but the disasm tests
// eax right after the call - and src/playerlock.h's ground truth is
// `int active();`. Rather than redeclare the given PlayerLock class (which
// would conflict with the scaffolding's own declaration), a differently
// named opaque shell reaches the same vtable-free thiscall slot; the call
// relocation is masked by the comparison regardless of the symbol it names.
class PlayerLockRow { public:
    int active();
};

// Indexed table base: the record loop walks stride-0x1c PlayerLock rows
// starting at 0x93e24c, so the address itself does arithmetic work and the
// scaffolding's `static int *const g_0093e24c` (a plain load/store pointer)
// would have /O2 fold the literal. A differently named extern array keeps
// the relocation the indexing needs.
extern unsigned char g_0093e24c_row0[];

void __cdecl net_end_of_turn() {
    if ((*reinterpret_cast<unsigned char *>(g_009a681c) & 0x10) != 0) {
        if (*g_0093e8c0 != 0) {
            message_data(0x4301, 0, 0, 0, 0, 0);
        } else {
            while (*g_009b2068 == 0 && *g_0093a950 == 0) {
                reinterpret_cast<NetDaemon *>(g_0093cd90)->net_tasks();
            }
        }
    }

    log_say(reinterpret_cast<char *>(g_0068bd9c), *g_0093e8ec, *g_0093e8f8,
            *g_009a64c0 & 2);
    reinterpret_cast<FX *>(g_00749cf8)->play(0x22);
    *g_009a64c0 |= 2;
    *g_00939294 = 0;
    reinterpret_cast<Console *>(g_009156b0)->set_view(0);

    int shiftAmt = *g_00939284;
    if (*g_0093e8c0 == 0) {
        *g_0093e8ec |= (1 << shiftAmt);
    }
    *g_0093e8f8 |= (1 << shiftAmt);
    reinterpret_cast<MultiWin *>(g_007fd648)->draw(0);

    if ((*reinterpret_cast<unsigned char *>(g_009a681c) & 0x10) != 0) {
        goto proceed_next_turn;
    }

    message_data(0x8301, 0, 0, 0, 0, 0);
    {
        FnTimeGetTime timeGetTime = *reinterpret_cast<FnTimeGetTime *>(g_00669368);
        unsigned long startTime = timeGetTime();
        bool sentFinishing = false;
        unsigned int uVar2;
        while ((uVar2 = *g_009a64e8 & 0xff,
                ((uVar2 & *g_0093e8ec) != uVar2 ||
                 (*g_0093e8e0 & uVar2) != 0 ||
                 (*g_0093e8c0 != 0 &&
                  (reinterpret_cast<Lock *>(g_0093e230)->any_locks() != 0 ||
                   *g_0093e90c != 0)))
                && *g_009b2068 == 0
                && (*g_009a64c0 & 2) != 0)) {
            if (!sentFinishing) {
                if (timeGetTime() - startTime > 0x3e8) {
                    reinterpret_cast<NetMsg *>(g_00805338)->pop(
                        reinterpret_cast<const char *>(g_0068bdb8), 0, 0, 0);
                    sentFinishing = true;
                }
            }
            reinterpret_cast<NetDaemon *>(g_0093cd90)->net_tasks();
        }
        if (sentFinishing) {
            reinterpret_cast<NetMsg *>(g_00805338)->close();
        }
    }

    if ((*g_009a64c0 & 2) != 0) {
        if (*g_0093e8c0 == 0) {
            log_say(reinterpret_cast<char *>(g_0068bdc8), *g_0093e8ec,
                    *g_009a64e8 & 0xff, 0);
        } else {
            goto handle_93e8c0;
        }
    }
    if (*g_0093e8c0 == 0) {
        goto after_double_loop;
    }

handle_93e8c0:
    if ((*g_009a64c0 & 2) == 0) {
        return;
    }
    if (*g_009b2068 != 0) {
        goto after_double_loop;
    }
    for (;;) {
        if (*g_0093d4f8 <= 1) {
            break;
        }
        int idx = 1;
        int lockedCount = 0;
        unsigned char *rec = g_0093e24c_row0;
        do {
            int active = reinterpret_cast<PlayerLockRow *>(rec)->active();
            if (active != 0 && (*rec & 1) == 0) {
                lockedCount++;
            }
            idx++;
            rec += 0x1c;
        } while (idx < *g_0093d4f8);
        if (lockedCount == 0) {
            break;
        }
        wait_task();
        reinterpret_cast<NetDaemon *>(g_0093cd90)->receive();
    }
    message_data(0x4301, 0, 0, 0, 0, 0);
    reinterpret_cast<NetDaemon *>(g_0093cd90)->receive();

after_double_loop:
    if ((*g_009a64c0 & 2) == 0) {
        return;
    }

proceed_next_turn:
    log_say(reinterpret_cast<char *>(g_0068bddc), 0, 0, 0);
    *g_0093e8f0 = 0;
    *g_0093e8fc = 0;
    *g_0093a93c = 0;
    *g_0093a938 = 1;
    if (reinterpret_cast<Win *>(g_006a7628)->is_visible() != 0) {
        reinterpret_cast<BaseWin *>(g_006a7628)->exit();
    }
    go_reset();
    while (reinterpret_cast<NetDaemon *>(g_0093cd90)->receive() != 0) {
    }
    mash_planes();
    do_checksums(0);
    if ((*reinterpret_cast<unsigned char *>(g_009a6490) & 2) != 0 &&
        *g_0093a94c != 0) {
        *reinterpret_cast<char *>(g_009b86a0) = 0;
        strcat(reinterpret_cast<char *>(g_009b86a0),
               reinterpret_cast<const char *>(g_0068bdfc));
        strcat(reinterpret_cast<char *>(g_009b86a0),
               reinterpret_cast<const char *>(g_0068be18));
        char buf[80];
        my_itoa(*g_00939284, buf, 10);
        strcat(reinterpret_cast<char *>(g_009b86a0), buf);
        strcat(reinterpret_cast<char *>(g_009b86a0),
               reinterpret_cast<const char *>(g_0068be1c));
        my_itoa(*g_009a64d4, buf, 10);
        strcat(reinterpret_cast<char *>(g_009b86a0), buf);
        log_say(reinterpret_cast<char *>(g_0068be20),
                reinterpret_cast<char *>(g_009b86a0), 0, 0, 0);
        save_daemon(reinterpret_cast<char *>(g_009b86a0));
    }
    *g_0093e8bc += 1;
}
