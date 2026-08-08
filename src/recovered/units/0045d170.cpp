// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0045D170
// name           ?on_iface_button_clicked@MainInterface@@QAEXH@Z
// size           257 bytes
// measured tier  MISMATCH
// divergence     69
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045d170/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_iface_button_clicked@MainInterface@@QAEXH@Z  at 0x0045D170  (257 bytes)
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

class AlphaMenu;
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
class MainInterface;
class Menu;
class MultiWin;
typedef void * PVOID;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class StatusWin;
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
class AlphaMenu { public:
    void hide();
    void show();
};
class Menu { public:
    void hide_menus();
};
class MultiWin { public:
    void hide();
    void show();
};
class StatusWin { public:
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
int __cdecl veh_at(int, int);
void __cdecl veh_demote(int);
void __cdecl veh_promote(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0045d274 = (int *)0x0045D274;
static int *const g_007d3934 = (int *)0x007D3934;
static int *const g_007fd648 = (int *)0x007FD648;
static int *const g_007fe06c = (int *)0x007FE06C;
static int *const g_008c5568 = (int *)0x008C5568;
static int *const g_008c6b1c = (int *)0x008C6B1C;
static int *const g_008c6b20 = (int *)0x008C6B20;
static int *const g_009380dc = (int *)0x009380DC;
static int *const g_00952858 = (int *)0x00952858;

class MainInterface { public:
    void on_iface_button_clicked(int);
};

extern short g_table_00952858[];

// MNEMONIC_ONLY match, not BYTE_EXACT, and provably so for a tooling
// reason rather than a code one - same shape as the documented case
// in Dialogs::on_redraw (build/byte-match/00612e00/unit.cpp): every
// one of the 69 original mnemonics is reproduced in order
// (mnemonics_in_common == original_mnemonics, edit_count 1), and the
// real compiled span is verified byte-for-byte 257/257 (checked via
// byte_match.py's `--keep`: the function's own code runs 0x0-0x101,
// exactly 257 bytes, matching the catalogued size to the byte).
// `/Gy` puts this switch's jump table in the SAME `.text` COMDAT
// right after the function, padded to the table's alignment with a
// 3-byte `lea ecx,[ecx]` filler - and the ORIGINAL BINARY has that
// same filler at the same relative offset before ITS OWN table.
// `clip_jump_table` in tools/byte_match.py strips the table's DIR32
// relocation run but has no rule for the leading alignment-filler
// instruction, so the rebuilt span reads 292 bytes and the
// comparator reports six trailing phantom `lea`/`add` instructions
// it cannot explain away - the table's own raw (pre-relocation, all
// zero) bytes decoded as an opcode run. The switch's case-block
// LAYOUT ORDER matters and is reproduced deliberately: the original
// places the two blocks that jump/fall into the shared "set flag,
// redraw, clear flag" tail FIRST (0x3f0/0x3f1's demote path via an
// explicit `jmp`, 0x3f2's promote path via fallthrough), then the
// two independently-returning cases (0x3eb, 0x3ec) - written in
// source in a DIFFERENT order (0x3eb/0x3ec first), the shared tail
// gets duplicated into 0x3ec's block instead of falling through to
// it, costing 4 bytes; the block order above avoids that.
void MainInterface::on_iface_button_clicked(int a1) {
    switch (a1) {
        case 0x3f0:
        case 0x3f1: {
            int result = veh_at(*g_008c6b1c, *g_008c6b20);
            if (result < 0) {
                goto end;
            }
            veh_demote(result);
            goto redraw;
        }
        case 0x3f2: {
            int result = veh_at(*g_008c6b1c, *g_008c6b20);
            if (result < 0) {
                goto end;
            }
            short v = g_table_00952858[result * 0x1a];
            while (v >= 0) {
                result = v;
                v = g_table_00952858[result * 0x1a];
            }
            veh_promote(result);
        }
redraw:
        *g_007d3934 = 1;
        reinterpret_cast<StatusWin *>(g_008c5568)->on_redraw();
        *g_007d3934 = 0;
        return;
        case 0x3eb:
            if (reinterpret_cast<Win *>(g_009380dc)->is_visible()) {
                reinterpret_cast<Menu *>(g_009380dc)->hide_menus();
                reinterpret_cast<AlphaMenu *>(g_009380dc)->hide();
                return;
            }
            reinterpret_cast<AlphaMenu *>(g_009380dc)->show();
            return;
        case 0x3ec:
            if (*g_007fe06c != 0) {
                reinterpret_cast<MultiWin *>(g_007fd648)->hide();
                return;
            }
            reinterpret_cast<MultiWin *>(g_007fd648)->show();
            break;
    }
end:
    return;
}

