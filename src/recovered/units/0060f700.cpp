// ORIGINAL: 0x0060F700 FILE
// name      ?on_left_down@CheckBox@@QAEXHH@Z
// size      432 bytes
// spans     0x0060F700-0x0060F8B0
// prototype void (__thiscall ?on_left_down@CheckBox@@QAEXHH@Z)(CheckBox* this, int xCoord, int yCoord)
// callers   1   call targets   6
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00401560 0x005D5890 0x00609AF0 0x00609B50 0x0060EB80 0x0060F030
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0060F700
// measured tier  MISMATCH
// divergence     12
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0060f700/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_left_down@CheckBox@@QAEXHH@Z  at 0x0060F700  (432 bytes)
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
class CheckBox;
class Dialog;
struct DialogEntry;
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
typedef void * PVOID;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class StringStruct;
struct StringStructEntry;
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

struct DialogEntry {
    uint32_t vtable;
    int id;
    void * payload;
    DialogEntry * next;
    DialogEntry * previous;
    uint32_t secondary_vtable;
    void * heap;
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

struct StringStructEntry {
    uint32_t abi_word;
    int id;
    int payload;
    StringStructEntry * next;
    StringStructEntry * previous;
    uint32_t secondary_abi_word;
    void * allocation_owner;
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
class Dialog { public:
    LPVOID vtable_;
    Heap heap_;
    Heap * heap_ptr_;
    uint32_t field_1C_;
    uint32_t field_20_;
    uint32_t field_24_;
    uint32_t field_28_;
    uint32_t field_2C_;
    uint32_t field_30_;
    uint32_t field_34_;
    uint32_t field_38_;
    uint32_t field_3C_;
    uint32_t field_40_;
    uint32_t field_44_;
    uint32_t field_48_;
    uint32_t field_4C_;
    uint32_t field_50_;
    uint32_t field_54_;
    uint32_t field_58_;
    uint32_t field_5C_;
    uint32_t field_60_;
    uint32_t field_64_;
    uint32_t field_68_;
    uint32_t field_6C_;
    Font * font1_;
    Font * font2_;
    Font * font3_;
    uint32_t text_color_a_;
    uint32_t text_color_2a_;
    uint32_t text_color_3a_;
    uint32_t text_color_b_;
    uint32_t text_color_2b_;
    uint32_t text_color_3b_;
    uint32_t text_color_c_;
    uint32_t text_color_2c_;
    uint32_t text_color_3c_;
    uint32_t text_color_d_;
    uint32_t text_color_2d_;
    uint32_t text_color_3d_;
    uint32_t field_AC_;
    uint32_t field_B0_;
    uint32_t field_B4_;
    uint32_t field_B8_;
    uint32_t field_BC_;
    uint32_t field_C0_;
    DialogEntry * entry_head_;
    DialogEntry * current_entry_;
    int entry_count_;
    int entry_position_;
    uint32_t field_D4_;
    uint32_t field_D8_;
    uint32_t field_DC_;
    uint32_t field_E0_;
    uint32_t field_E4_;
    uint32_t field_E8_;
    int selected_position_;
    uint32_t field_F0_;
    int id_to_pos(int);
    int pos_to_id(int);
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
    void soft_update();
};
class StringStruct { public:
    uint32_t primary_abi_word_;
    uint32_t virtual_base_abi_word_;
    StringStructEntry * head_;
    StringStructEntry * current_;
    int entry_count_;
    int current_position_;
    void * allocator_;
    uint32_t allocation_base_abi_word_;
    void * allocation_owner_;
    int seek_id(int);
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b7ab8 = (int *)0x009B7AB8;

class CheckBox { public:
    void set_state_id(int, int);
    void draw_item(int, int, int);
    void on_left_down(int, int);
};

void CheckBox::on_left_down(int a1, int a2) {
    char *edi = reinterpret_cast<char *>(this);
    char *self = edi - 0x1c;

    int off1 = *reinterpret_cast<int *>(*reinterpret_cast<int **>(self) + 4);
    *g_009b7ab8 = *reinterpret_cast<int *>(edi + off1 + 0xa8);

    int off2 = *reinterpret_cast<int *>(*reinterpret_cast<int **>(self) + 8);
    char *base1 = edi + off2;
    int field28 = *reinterpret_cast<int *>(base1 + 0x28);
    int field48 = *reinterpret_cast<int *>(base1 + 0x48);
    int denom1 = field48 + field28;
    char *base1adj = base1 - 0x1c;
    int q1 = a1 / denom1;
    int field68 = *reinterpret_cast<int *>(base1adj + 0x68);
    int denom2 = field68 + field28;
    int field58 = *reinterpret_cast<int *>(base1adj + 0x58);
    int posArg = q1 * field58 + a2 / denom2;

    int id = reinterpret_cast<Dialog *>(base1adj)->pos_to_id(posArg);

    int off2b = *reinterpret_cast<int *>(*reinterpret_cast<int **>(self) + 8);
    reinterpret_cast<StringStruct *>(self + off2b + 0xbc)->seek_id(id);

    int off2c = *reinterpret_cast<int *>(*reinterpret_cast<int **>(self) + 8);
    char *fieldBase = self + off2c;
    int bitIndex = *reinterpret_cast<int *>(fieldBase + 0xd0);
    unsigned int bit = 1u << bitIndex;

    if ((*reinterpret_cast<unsigned int *>(fieldBase + 0xec) & bit) != 0) {
        reinterpret_cast<StringStruct *>(fieldBase + 0xbc)->seek_id(id);

        int off2d = *reinterpret_cast<int *>(*reinterpret_cast<int **>(self) + 8);
        char *fieldBase2 = self + off2d;
        int bitIndex2 = *reinterpret_cast<int *>(fieldBase2 + 0xd0);
        unsigned int bit2 = 1u << bitIndex2;
        unsigned int flags = *reinterpret_cast<unsigned int *>(fieldBase2 + 0xec);
        flags &= ~bit2;
        *reinterpret_cast<unsigned int *>(fieldBase2 + 0xec) = flags;
    } else {
        this->set_state_id(id, 1);
    }

    int cachedPos = *reinterpret_cast<int *>(edi - 0x18);
    int off2e = *reinterpret_cast<int *>(*reinterpret_cast<int **>(self) + 8);
    int posId = reinterpret_cast<Dialog *>(edi + off2e - 0x1c)->pos_to_id(cachedPos);

    if (posId != id) {
        int off2f = *reinterpret_cast<int *>(*reinterpret_cast<int **>(self) + 8);
        char *fieldBase3 = self + off2f;
        unsigned int flagsBit = *reinterpret_cast<unsigned int *>(fieldBase3 + 0xec) &
                                 (1u << cachedPos);
        this->draw_item(cachedPos, flagsBit, 0);

        int off2g = *reinterpret_cast<int *>(*reinterpret_cast<int **>(self) + 8);
        *reinterpret_cast<int *>(edi - 0x18) =
            reinterpret_cast<Dialog *>(edi + off2g - 0x1c)->id_to_pos(id);
    }

    void (__cdecl *callback)(int) = *reinterpret_cast<void(__cdecl **)(int)>(edi - 0x14);
    if (callback != 0) {
        int off2h = *reinterpret_cast<int *>(*reinterpret_cast<int **>(self) + 8);
        int flagsVal = *reinterpret_cast<int *>(self + off2h + 0xec);
        callback(flagsVal);
    }

    int off2i = *reinterpret_cast<int *>(*reinterpret_cast<int **>(self) + 8);
    char *fieldBase4 = self + off2i + 0xbc;
    int count = *reinterpret_cast<int *>(self + off2i + 0xc4);
    if (count != 0) {
        int maxCount = *reinterpret_cast<int *>(fieldBase4 + 0x10);
        *reinterpret_cast<int *>(fieldBase4 + 0x14) = 0;
        *reinterpret_cast<int *>(fieldBase4 + 0xc) = count;
        if (maxCount > 0) {
            int i = 0;
            do {
                int *node = *reinterpret_cast<int **>(fieldBase4 + 0xc);
                if (node[1] == id) {
                    break;
                }
                (*reinterpret_cast<int *>(fieldBase4 + 0x14))++;
                node = reinterpret_cast<int *>(node[3]);
                i++;
                *reinterpret_cast<int **>(fieldBase4 + 0xc) = node;
            } while (i < maxCount);
        }
    }

    int off2j = *reinterpret_cast<int *>(*reinterpret_cast<int **>(self) + 8);
    char *fieldBase5 = self + off2j;
    int bitIndex5 = *reinterpret_cast<int *>(fieldBase5 + 0xd0);
    unsigned int flags5 = *reinterpret_cast<unsigned int *>(fieldBase5 + 0xec) & (1u << bitIndex5);

    int off2k = *reinterpret_cast<int *>(*reinterpret_cast<int **>(self) + 8);
    int pos5 = reinterpret_cast<Dialog *>(edi + off2k - 0x1c)->id_to_pos(id);
    this->draw_item(pos5, flags5, 1);

    int off1b = *reinterpret_cast<int *>(*reinterpret_cast<int **>(self) + 4);
    reinterpret_cast<GraphicWin *>(edi + off1b - 0x1c)->soft_update();
}
