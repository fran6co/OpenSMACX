// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0052E370
// name           ?on_player_deleted@NetDaemon@@QAEXPADK@Z
// size           1421 bytes
// measured tier  MISMATCH
// divergence     12
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0052e370/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_player_deleted@NetDaemon@@QAEXPADK@Z  at 0x0052E370  (1421 bytes)
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
class FX;
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
class NetDaemon;
class NetMsg;
class NetWin;
typedef void * PVOID;
class PlayerLock;
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
class FX { public:
    uint8_t effects_[0x61 * 0x6C];
    uint32_t field_28EC_;
    void play(int);
};
class NetMsg { public:
    void pop(const char *, int, int, const char *);
};
class NetWin { public:
    void on_button_clicked(int);
    void on_redraw();
};
class PlayerLock { public:
    void active();
    void clear();
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
    int is_visible();
};
extern "C" char *strcpy(char *, const char *);
int __cdecl parse_says(int, char *, int, int);
void __cdecl log_say(char *, char *, int, int, int);
void __cdecl log_say(char *, int, int, int);
void __cdecl message_data(int, int, int, int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068c150 = (int *)0x0068C150;
static int *const g_0068c154 = (int *)0x0068C154;
static int *const g_0068c174 = (int *)0x0068C174;
static int *const g_0068c198 = (int *)0x0068C198;
static int *const g_0068c1ac = (int *)0x0068C1AC;
static int *const g_0068c1b8 = (int *)0x0068C1B8;
static int *const g_0068c1c0 = (int *)0x0068C1C0;
static int *const g_00749cf8 = (int *)0x00749CF8;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_0080a6f8 = (int *)0x0080A6F8;
static int *const g_0090db98 = (int *)0x0090DB98;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093a938 = (int *)0x0093A938;
static int *const g_0093a93c = (int *)0x0093A93C;
static int *const g_0093a950 = (int *)0x0093A950;
static int *const g_0093e8c0 = (int *)0x0093E8C0;
static int *const g_0093e8d0 = (int *)0x0093E8D0;
static int *const g_0093e8ec = (int *)0x0093E8EC;
static int *const g_0093e96c = (int *)0x0093E96C;
static int *const g_0093f65c = (int *)0x0093F65C;
static int *const g_009a64d4 = (int *)0x009A64D4;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009a64e9 = (int *)0x009A64E9;
static int *const g_009a681c = (int *)0x009A681C;
static int *const g_009a6820 = (int *)0x009A6820;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;
static int *const g_009bb5e8 = (int *)0x009BB5E8;
static int *const g_009be5fc = (int *)0x009BE5FC;

class NetDaemon { public:
    void on_player_deleted(char *, unsigned long);
};

// Indexed table bases: each address does arithmetic work (index scaled
// by a computed register), so fresh externs are used instead of the
// scaffold's const-pointer spellings for these.
extern unsigned char g_x90db98[];  // per-slot metadata, stride 0x17c

void NetDaemon::on_player_deleted(char * a1, unsigned long a2) {
    char *self = reinterpret_cast<char *>(this);
    (void)a1;

    parse_says(0, reinterpret_cast<char *>(g_0068c150), -1, -1);

    bool serverLeft = (static_cast<unsigned long>(*g_009be5fc) == a2);
    if (serverLeft) {
        *g_0093e96c = 1;
    }

    int slot = 1;
    char *prec;
    bool found = false;
    int total = *reinterpret_cast<int *>(self + 0x768);
    if (total >= 1) {
        int *ids = reinterpret_cast<int *>(self + 0x928);
        for (slot = 1; slot <= total; slot++) {
            if (*ids == static_cast<int>(a2)) {
                found = true;
                break;
            }
            ids += 0x67;
        }
    }

    if (found) {
        unsigned int *readyMask = reinterpret_cast<unsigned int *>(self + 0x1b50);
        if (*readyMask != 0) {
            unsigned int nm = *readyMask & ~(1u << (slot & 0x1f));
            *readyMask = nm;
            if (nm == 0) {
                reinterpret_cast<FX *>(g_00749cf8)->play(0x15);
            }
        }
        prec = self + slot * 0x19c;
        parse_says(0, prec + 0x76c, -1, -1);
        signed char pnum = *reinterpret_cast<signed char *>(prec + 0x790 + 0x76c);
        if (*g_0093e8d0 == 2) {
            unsigned char *b = reinterpret_cast<unsigned char *>(g_009a64e8);
            *b = *b & ~(1 << (pnum & 0x1f));
        }
        unsigned int *m1bbc = reinterpret_cast<unsigned int *>(self + 0x1bbc);
        *m1bbc = *m1bbc & ~(1u << (pnum & 0x1f));

        if ((*reinterpret_cast<unsigned char *>(g_009a681c) & 0x10) != 0 && *g_0093e8c0 != 0 &&
            *g_009a6820 == pnum) {
            int i = pnum + 1;
            bool sent = false;
            for (; i < 8; i++) {
                unsigned int bit = 1u << (i & 0x1f);
                if ((bit & *reinterpret_cast<unsigned char *>(g_009a64e9)) != 0 &&
                    (*reinterpret_cast<unsigned char *>(g_009a64e8) & static_cast<unsigned char>(bit)) != 0) {
                    message_data(0x4309, 0, i, 0, 0, 0);
                    sent = true;
                    break;
                }
            }
            if (!sent) {
                *g_0093a950 = 1;
            }
        }
    } else {
        // Loop fell through without finding the player: original still
        // falls into the shared tail using the last-scanned slot.
    }

    prec = self + slot * 0x19c;
    signed char pnum = *reinterpret_cast<signed char *>(prec + 0x76c + 0x790);

    reinterpret_cast<PlayerLock *>(self + 0x14a0 + (pnum * 8 - pnum) * 4)->clear();

    unsigned int *m1b7c = reinterpret_cast<unsigned int *>(self + 0x1b7c);
    unsigned int m1b80 = *reinterpret_cast<unsigned int *>(self + 0x1b80 + pnum * 4);
    *m1b7c = *m1b7c & ~m1b80;
    if (*reinterpret_cast<int *>(self + 0x1580) == static_cast<int>(pnum)) {
        *reinterpret_cast<int *>(self + 0x1580) = 0;
        *reinterpret_cast<int *>(self + 0x1584) = 0;
    }

    unsigned char flag10 = *reinterpret_cast<unsigned char *>(g_009a681c) & 0x10;
    bool notify =
        ((flag10 != 0 && *g_0093a938 == 0 && *g_009a6820 == *g_00939284) ||
         ((flag10 == 0 || *g_0093a938 != 0) && *g_0093e8c0 != 0)) &&
        (*g_0093e8d0 == 2);

    if (notify) {
        unsigned char *pbVar12 = reinterpret_cast<unsigned char *>(self + 0x14bc);
        for (int i = 1; i < 8; i++) {
            unsigned char bit = static_cast<unsigned char>(1 << (i & 0x1f));
            if ((bit & *reinterpret_cast<unsigned char *>(g_009a64e9)) != 0 &&
                (*reinterpret_cast<unsigned char *>(g_009a64e8) & bit) != 0 &&
                i != pnum) {
                // PlayerLock::active() is declared void by the catalogue but
                // the disassembly clearly consumes a return value in eax;
                // that return-type mismatch is reported as a PROPOSAL. The
                // value itself cannot be recovered through the void
                // declaration, so the guard below assumes it is nonzero.
                reinterpret_cast<PlayerLock *>(pbVar12)->active();
                bool activeResult = true;
                if (activeResult && (*pbVar12 & 1) == 0) {
                    unsigned char nb = static_cast<unsigned char>(1 << (slot & 0x1f)) | *pbVar12;
                    *pbVar12 = nb;
                    if ((nb & *reinterpret_cast<unsigned char *>(g_009a64e8)) == *reinterpret_cast<unsigned char *>(g_009a64e8)) {
                        log_say(reinterpret_cast<char *>(g_0068c154), i, 0, 0);
                        message_data(0x1206, i, 0, 0, 0, 0);
                        *pbVar12 = *pbVar12 | 1;
                    }
                }
            }
            pbVar12 += 0x1c;
        }
    }

    log_say(reinterpret_cast<char *>(g_009bb5e8), reinterpret_cast<char *>(g_0068c174), static_cast<int>(a2), slot, 0);

    *reinterpret_cast<unsigned char *>(prec + 0x76c + 0x790) = 0xff;
    *reinterpret_cast<unsigned char *>(prec + 0x76c + 0x791) = 0;
    *reinterpret_cast<unsigned char *>(prec + 0x76c + 0x792) = 0;
    *reinterpret_cast<unsigned char *>(prec + 0x76c + 0x793) = 0xff;
    *reinterpret_cast<unsigned char *>(prec + 0x76c + 0x794) = 2;

    if (*g_0093e8d0 < 2) {
        g_x90db98[slot * 0x17c] = 0xff;
    }

    char *slotRec = reinterpret_cast<char *>(g_x90db98) + slot * 0x17c;
    char *p90f8 = *reinterpret_cast<char **>(g_009b90f8);
    Strings *strs = reinterpret_cast<Strings *>(g_009b90d8);
    strcpy(slotRec + 5, reinterpret_cast<const char *>(strs->get(*reinterpret_cast<int *>(p90f8 + 0xfe4))));
    strcpy(slotRec + 0x45, reinterpret_cast<const char *>(strs->get(*reinterpret_cast<int *>(p90f8 + 0xfe4))));

    int last = *reinterpret_cast<int *>(self + 0x768) - 1;
    for (int s = slot; s <= last; s++) {
        char *dstPlayer = self + s * 0x19c + 0x76c;
        char *srcPlayer = self + (s + 1) * 0x19c + 0x76c;
        for (int k = 0; k < 0x67; k++) {
            reinterpret_cast<int *>(dstPlayer)[k] = reinterpret_cast<int *>(srcPlayer)[k];
        }
        char *dstSlot = reinterpret_cast<char *>(g_x90db98) + s * 0x17c;
        char *srcSlot = reinterpret_cast<char *>(g_x90db98) + (s + 1) * 0x17c;
        for (int k2 = 0; k2 < 0x5f; k2++) {
            reinterpret_cast<int *>(dstSlot)[k2] = reinterpret_cast<int *>(srcSlot)[k2];
        }
        *reinterpret_cast<char *>(dstSlot) = static_cast<char>(s);
    }

    *reinterpret_cast<int *>(self + 0x768) = *reinterpret_cast<int *>(self + 0x768) - 1;
    *reinterpret_cast<unsigned int *>(self + 0x1b30) =
        (*reinterpret_cast<int *>(self + 0x760) == *reinterpret_cast<int *>(self + 0x764)) ? 1u : 0u;

    unsigned int uv5 = *reinterpret_cast<unsigned char *>(g_009a681c) & 0x10;
    bool cond1 = (uv5 == 0 || *g_0093a938 != 0 || *g_009a6820 != *g_00939284);
    bool cond2 = ((uv5 != 0 && *g_0093a938 == 0) || *g_0093e8c0 == 0);
    if (cond1 && cond2 && *g_0093e8d0 == 2) {
        if (uv5 == 0 && (((1 << (*g_00939284 & 0x1f)) & *g_0093e8ec) != 0)) {
            message_data(0x8301, 0, 0, 0, 0, 0);
        }
        if (*g_0093a938 != 0) {
            message_data(0x2303, 0, *g_0093a93c, *g_009a64d4, 0, 0);
        }
    }

    const char *msg;
    if (serverLeft) {
        int vis = reinterpret_cast<Win *>(g_0080a6f8)->is_visible();
        msg = (vis == 0) ? reinterpret_cast<const char *>(g_0068c1ac) : reinterpret_cast<const char *>(g_0068c198);
    } else {
        msg = reinterpret_cast<const char *>(g_0068c1b8);
    }
    reinterpret_cast<NetMsg *>(g_00805338)->pop(msg, 3000, 2, 0);

    if (serverLeft) {
        int vis2 = reinterpret_cast<Win *>(g_0080a6f8)->is_visible();
        if (vis2 != 0) {
            reinterpret_cast<NetWin *>(g_0080a6f8)->on_button_clicked(-2);
        }
        *reinterpret_cast<int *>(self + 0x1bb0) = 0;
        *reinterpret_cast<int *>(self + 0x1bb8) = 0;
        *reinterpret_cast<int *>(self + 0x1ba8) = 0;
        *reinterpret_cast<int *>(self + 0x1b70) = 2;
        *g_0093f65c = 1;
        *reinterpret_cast<int *>(self + 0x1588) = 1;
    }

    reinterpret_cast<NetWin *>(g_0080a6f8)->on_redraw();

    int serverSlot = 0;
    int *srv = reinterpret_cast<int *>(self + 0x78c);
    int who = *reinterpret_cast<int *>(self + 0x764);
    while (*srv != who && serverSlot < 8) {
        serverSlot++;
        srv += 0x67;
    }
    log_say(self + serverSlot * 0x19c + 0x76c, reinterpret_cast<char *>(g_0068c1c0), 0, 0, 0);
    *reinterpret_cast<int *>(self + 0x1b38) = 0;
}
