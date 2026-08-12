// ORIGINAL: 0x00439700 FILE
// name      ?on_right_click@DesignWin@@QAEXHH@Z
// size      584 bytes
// spans     0x00439700-0x00439948
// prototype void (__thiscall ?on_right_click@DesignWin@@QAEXHH@Z)(Win* this, int xCoord, int yCoord)
// callers   0   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004BDDE0 0x005ED240 0x005F83D0 0x005FAB00 0x006169A0
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00439700
// measured tier  NO_COMPILE
// refusal        u00439700.cpp(144) : error C2079: 'spot_' uses undefined class 'Spot'
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00439700/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_right_click@DesignWin@@QAEXHH@Z  at 0x00439700  (584 bytes)
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
class DesignWin;
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
typedef void * PVOID;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class Strings;
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
    void set_bubble_text(char *, RECT *);
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00439948 = (int *)0x00439948;
static int *const g_008c6e68 = (int *)0x008C6E68;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class DesignWin { public:
    uint8_t pad_0_[0x12764];
    Spot spot_;

    void on_right_click(int, int);
};

void DesignWin::on_right_click(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    reinterpret_cast<TutWin *>(g_008c6e68)->reset();
    int origA2 = a2;
    int origA1 = a1;

    int category, unused;
    int rc = reinterpret_cast<Spot *>(self + 0x12764)->check(origA1, origA2, &unused, &category);
    if (rc == -1 || static_cast<unsigned int>(category) > 5) {
        return;
    }

    switch (category) {
    case 0: {
        int msgId = *reinterpret_cast<int *>(*g_009b90f8 + 0xcd4);
        char *msg = reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(msgId));
        a1 = origA2;
        a2 = origA1;
        reinterpret_cast<Win *>(self)->client_to_screen(&a2, &a1);
        RECT r;
        r.left = a2;
        r.top = a1;
        r.right = a2;
        r.bottom = a1;
        reinterpret_cast<Win *>(self)->set_bubble_text(msg, &r);
        return;
    }
    case 1: {
        int msgId = *reinterpret_cast<int *>(*g_009b90f8 + 0xcd8);
        char *msg = reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(msgId));
        a1 = origA2;
        a2 = origA1;
        reinterpret_cast<Win *>(self)->client_to_screen(&a2, &a1);
        RECT r;
        r.left = a2;
        r.top = a1;
        r.right = a2;
        r.bottom = a1;
        reinterpret_cast<Win *>(self)->set_bubble_text(msg, &r);
        return;
    }
    case 2: {
        int msgId = *reinterpret_cast<int *>(*g_009b90f8 + 0xcdc);
        char *msg = reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(msgId));
        a1 = origA2;
        a2 = origA1;
        reinterpret_cast<Win *>(self)->client_to_screen(&a2, &a1);
        RECT r;
        r.left = a2;
        r.top = a1;
        r.right = a2;
        r.bottom = a1;
        reinterpret_cast<Win *>(self)->set_bubble_text(msg, &r);
        return;
    }
    case 3: {
        int msgId = *reinterpret_cast<int *>(*g_009b90f8 + 0xce0);
        char *msg = reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(msgId));
        a1 = origA2;
        a2 = origA1;
        reinterpret_cast<Win *>(self)->client_to_screen(&a2, &a1);
        RECT r;
        r.left = a2;
        r.top = a1;
        r.right = a2;
        r.bottom = a1;
        reinterpret_cast<Win *>(self)->set_bubble_text(msg, &r);
        return;
    }
    case 4: {
        int msgId = *reinterpret_cast<int *>(*g_009b90f8 + 0xce4);
        char *msg = reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(msgId));
        a1 = origA2;
        a2 = origA1;
        reinterpret_cast<Win *>(self)->client_to_screen(&a2, &a1);
        RECT r;
        r.left = a2;
        r.top = a1;
        r.right = a2;
        r.bottom = a1;
        reinterpret_cast<Win *>(self)->set_bubble_text(msg, &r);
        return;
    }
    case 5: {
        int msgId = *reinterpret_cast<int *>(*g_009b90f8 + 0xce4);
        char *msg = reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(msgId));
        a1 = origA2;
        a2 = origA1;
        reinterpret_cast<Win *>(self)->client_to_screen(&a2, &a1);
        RECT r;
        r.left = a2;
        r.top = a1;
        r.right = a2;
        r.bottom = a1;
        reinterpret_cast<Win *>(self)->set_bubble_text(msg, &r);
        break;
    }
    }
}
