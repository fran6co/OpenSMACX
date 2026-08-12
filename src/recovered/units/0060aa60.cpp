// ORIGINAL: 0x0060AA60 FILE
// name      ?on_left_down@ListBox@@QAEXHH@Z
// size      1316 bytes
// spans     0x0060AA60-0x0060AF84
// prototype void (__thiscall ?on_left_down@ListBox@@QAEXHH@Z)(ListBox* this, int xCoord, int yCoord)
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00402530 0x0060A020 0x00644F3A
// indirect  0x0060AC00 0x0060ACB3 0x0060AF77
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0060AA60
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0060aa60/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_left_down@ListBox@@QAEXHH@Z  at 0x0060AA60  (1316 bytes)
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
class ListBox;
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

class Heap { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
};

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
    int current_entry();
};
extern "C" int abs(int);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 44
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
    virtual void slot044();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669330 = (int *)0x00669330;
static int *const g_009b7ab8 = (int *)0x009B7AB8;

class ListBox { public:
    uint32_t vbtable_pointer_;
    uint32_t field_4_;
    uint32_t field_8_;
    uint32_t field_C_;
    uint32_t field_10_;
    uint32_t field_14_;
    uint32_t field_18_;
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
    uint32_t graphic_vbase_adjust_;
    GraphicWin virtual_base_;
    uint32_t dialog_vbase_adjust_;
    Dialog dialog_;

    void set_selected_pos(int, int);
    void on_left_down(int, int);
};

typedef short(__stdcall *KeyStateFn)(int);

class VCall2 { public:
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
    virtual void slot044(int);  // <-- used
};

void ListBox::on_left_down(int a1, int a2) {
    int self = reinterpret_cast<int>(this);
    int ebx;
    int edi2 = 0;

    int vb1 = *reinterpret_cast<int *>(self - 0x48);
    int d1 = *reinterpret_cast<int *>(vb1 + 4);
    *g_009b7ab8 = *reinterpret_cast<int *>(d1 + self + 0x7c);

    int vb2 = *reinterpret_cast<int *>(self - 0x48);
    int d2 = *reinterpret_cast<int *>(vb2 + 8);
    int colHave = *reinterpret_cast<int *>(d2 + self + 0x84);
    int col = d2 + self;
    if (colHave == 0)
        goto L60af7d;

    ebx = a2;
    ebx -= *reinterpret_cast<int *>(col - 0x10);
    if (ebx < 0)
        goto L60af7d;

    if (*reinterpret_cast<int *>(col + 0xc) == 1) {
        int edi = *reinterpret_cast<int *>(self - 0x30);
        int edx = *reinterpret_cast<int *>(col + 0x84);
        edi -= 1;
        edx -= 1;
        if (edi <= edx) {
            *reinterpret_cast<int *>(col + 0x80) = *reinterpret_cast<int *>(col + 0x7c);
            if (edi < 0) {
                int a = abs(edi);
                int cnt = *reinterpret_cast<int *>(col + 0x84);
                if (a <= cnt) {
                    int steps = abs(edi);
                    if (steps > 0) {
                        do {
                            int cur = *reinterpret_cast<int *>(col + 0x80);
                            steps -= 1;
                            *reinterpret_cast<int *>(col + 0x80) = *reinterpret_cast<int *>(cur + 0x10);
                        } while (steps != 0);
                    }
                    edi += *reinterpret_cast<int *>(col + 0x84);
                    *reinterpret_cast<int *>(col + 0x88) = edi;
                }
            } else {
                if (edi > 0) {
                    int steps = edi;
                    do {
                        int cur = *reinterpret_cast<int *>(col + 0x80);
                        steps -= 1;
                        *reinterpret_cast<int *>(col + 0x80) = *reinterpret_cast<int *>(cur + 0xc);
                    } while (steps != 0);
                }
                *reinterpret_cast<int *>(col + 0x88) = edi;
            }
        }
    }

    {
        int vb3 = *reinterpret_cast<int *>(self - 0x48);
        int colc = *reinterpret_cast<int *>(vb3 + 8) + self;
        int rows = *reinterpret_cast<int *>(colc + 0x84);
        if (rows <= 0)
            goto L60af7d;
        if (ebx < 0)
            goto L60abb2;
        do {
            int rowFlag = *reinterpret_cast<int *>(colc + 0x7c);
            colc = colc + 0x74;
            edi2 += 1;
            if (rowFlag != 0) {
                int p1 = *reinterpret_cast<int *>(colc + 0xc);
                int lim = *reinterpret_cast<int *>(colc + 0x10);
                *reinterpret_cast<int *>(colc + 0xc) = *reinterpret_cast<int *>(p1 + 0xc);
                int cnt2 = *reinterpret_cast<int *>(colc + 0x14) + 1;
                *reinterpret_cast<int *>(colc + 0x14) = cnt2;
                if (cnt2 == lim)
                    *reinterpret_cast<int *>(colc + 0x14) = 0;
            }
            int entryVal;
            int p2 = *reinterpret_cast<int *>(colc + 0xc);
            if (p2 != 0) {
                int r = reinterpret_cast<StringStruct *>(colc)->current_entry();
                entryVal = *reinterpret_cast<int *>(r + 0xc);
            } else {
                entryVal = 0;
            }
            int vb4 = *reinterpret_cast<int *>(self - 0x48);
            int d4 = *reinterpret_cast<int *>(vb4 + 8);
            int base4 = d4 + self;
            int width = *reinterpret_cast<int *>(d4 + self + 0x20);
            width += *reinterpret_cast<int *>(base4 - 4);
            width *= entryVal;
            ebx -= width;
            rows -= 1;
        } while (rows > 0);
    }

L60abb2:
    if (edi2 == 0)
        goto L60af7d;
    edi2 -= 1;
    if (edi2 >= *reinterpret_cast<int *>(self - 0x2c))
        goto L60af7d;

    {
        int vb5 = *reinterpret_cast<int *>(self - 0x48);
        ebx = *reinterpret_cast<int *>(self - 0x30) + edi2;
        int d5 = *reinterpret_cast<int *>(vb5 + 8);
        int base5 = d5 + self;
        int rowCount = *reinterpret_cast<int *>(d5 + self + 0x84);
        if (ebx >= rowCount)
            goto L60af7d;

        if (*reinterpret_cast<int *>(self - 0x44) == 0)
            goto L60af4f;
        if (ebx == *reinterpret_cast<int *>(base5 + 0xa4))
            goto L60af4f;

        KeyStateFn getKeyState = *reinterpret_cast<KeyStateFn *>(0x669330);
        bool ctrlDown = (getKeyState(0x11) & 0x8000) != 0;
        if (!ctrlDown) {
            int vb6 = *reinterpret_cast<int *>(self - 0x48);
            int d6 = *reinterpret_cast<int *>(vb6 + 8);
            int base6 = d6 + self + 0x74;
            int cnt6 = *reinterpret_cast<int *>(d6 + self + 0x84) - 1;
            if (cnt6 >= -1) {
                *reinterpret_cast<int *>(base6 + 0xc) = *reinterpret_cast<int *>(base6 + 8);
                int a6 = abs(-1);
                int lim6 = *reinterpret_cast<int *>(base6 + 0x10);
                if (a6 <= lim6) {
                    int steps6 = abs(-1);
                    if (steps6 > 0) {
                        do {
                            int cur6 = *reinterpret_cast<int *>(base6 + 0xc);
                            steps6 -= 1;
                            *reinterpret_cast<int *>(base6 + 0xc) = *reinterpret_cast<int *>(cur6 + 0x10);
                        } while (steps6 != 0);
                    }
                    *reinterpret_cast<int *>(base6 + 0x14) = *reinterpret_cast<int *>(base6 + 0x10) - 1;
                }
            }

            int vb7 = *reinterpret_cast<int *>(self - 0x48);
            int d7 = *reinterpret_cast<int *>(vb7 + 8);
            int rows7 = *reinterpret_cast<int *>(d7 + self + 0x84);
            if (rows7 > 0) {
                int cnt7 = rows7;
                do {
                    int vb8 = *reinterpret_cast<int *>(self - 0x48);
                    int d8 = *reinterpret_cast<int *>(vb8 + 8);
                    int base8 = d8 + self + 0x74;
                    int flag8 = *reinterpret_cast<int *>(d8 + self + 0x7c);
                    int out8;
                    if (flag8 != 0) {
                        int p8 = *reinterpret_cast<int *>(base8 + 0xc);
                        int lim8 = *reinterpret_cast<int *>(base8 + 0x10);
                        *reinterpret_cast<int *>(base8 + 0xc) = *reinterpret_cast<int *>(p8 + 0xc);
                        int c8 = *reinterpret_cast<int *>(base8 + 0x14) + 1;
                        *reinterpret_cast<int *>(base8 + 0x14) = c8;
                        if (c8 == lim8)
                            *reinterpret_cast<int *>(base8 + 0x14) = 0;
                        out8 = *reinterpret_cast<int *>(base8 + 0xc);
                        out8 = *reinterpret_cast<int *>(out8 + 8);
                    } else {
                        out8 = 0;
                    }
                    cnt7 -= 1;
                    *reinterpret_cast<int *>(out8 + 8) = 0;
                } while (cnt7 != 0);
            }
        }

        bool shiftDown = (getKeyState(0x10) & 0x8000) != 0;
        if (shiftDown) {
            int vb9 = *reinterpret_cast<int *>(self - 0x48);
            int d9 = *reinterpret_cast<int *>(vb9 + 8);
            int lim9 = *reinterpret_cast<int *>(d9 + self + 0xa4);
            int base9 = d9 + self;
            if (ebx <= lim9) {
                int edi9 = lim9 - 1;
                int rows9 = *reinterpret_cast<int *>(base9 + 0x84) - 1;
                if (edi9 <= rows9) {
                    *reinterpret_cast<int *>(base9 + 0x80) = *reinterpret_cast<int *>(base9 + 0x7c);
                    if (edi9 < 0) {
                        int a9 = abs(edi9);
                        int cnt9 = *reinterpret_cast<int *>(base9 + 0x84);
                        if (a9 <= cnt9) {
                            int steps9 = abs(edi9);
                            if (steps9 > 0) {
                                do {
                                    int cur9 = *reinterpret_cast<int *>(base9 + 0x80);
                                    steps9 -= 1;
                                    *reinterpret_cast<int *>(base9 + 0x80) = *reinterpret_cast<int *>(cur9 + 0x10);
                                } while (steps9 != 0);
                            }
                            edi9 += *reinterpret_cast<int *>(base9 + 0x84);
                            *reinterpret_cast<int *>(base9 + 0x88) = edi9;
                        }
                    } else {
                        if (edi9 > 0) {
                            int steps9b = edi9;
                            do {
                                int cur9b = *reinterpret_cast<int *>(base9 + 0x80);
                                steps9b -= 1;
                                *reinterpret_cast<int *>(base9 + 0x80) = *reinterpret_cast<int *>(cur9b + 0xc);
                            } while (steps9b != 0);
                        }
                        *reinterpret_cast<int *>(base9 + 0x88) = edi9;
                    }
                }
            }

            int vb10 = *reinterpret_cast<int *>(self - 0x48);
            int d10 = *reinterpret_cast<int *>(vb10 + 8);
            int lim10 = *reinterpret_cast<int *>(d10 + self + 0xa4);
            if (lim10 <= ebx)
                goto L60af4f;

            int edx10 = ebx - lim10;
            edx10 += 1;
            int cnt10 = edx10;
            int one10 = 1;
            do {
                int vb11 = *reinterpret_cast<int *>(self - 0x48);
                int d11 = *reinterpret_cast<int *>(vb11 + 8);
                int base11 = d11 + self + 0x74;
                int flag11 = *reinterpret_cast<int *>(d11 + self + 0x7c);
                int out11;
                if (flag11 != 0) {
                    int p11 = *reinterpret_cast<int *>(base11 + 0xc);
                    int lim11 = *reinterpret_cast<int *>(base11 + 0x10);
                    *reinterpret_cast<int *>(base11 + 0xc) = *reinterpret_cast<int *>(p11 + 0xc);
                    int c11 = *reinterpret_cast<int *>(base11 + 0x14) + 1;
                    *reinterpret_cast<int *>(base11 + 0x14) = c11;
                    if (c11 == lim11)
                        *reinterpret_cast<int *>(base11 + 0x14) = 0;
                    out11 = *reinterpret_cast<int *>(base11 + 0xc);
                    out11 = *reinterpret_cast<int *>(out11 + 8);
                } else {
                    out11 = 0;
                }
                cnt10 -= 1;
                *reinterpret_cast<int *>(out11 + 8) = one10;
            } while (cnt10 != 0);
            goto L60af4f;
        } else {
            int vb12 = *reinterpret_cast<int *>(self - 0x48);
            int d12 = *reinterpret_cast<int *>(vb12 + 8);
            int base12 = d12 + self;
            int lim12 = *reinterpret_cast<int *>(d12 + self + 0xa4);
            if (ebx > lim12)
                goto L60af4f;

            int edi12 = ebx - 1;
            int rows12 = *reinterpret_cast<int *>(base12 + 0x84) - 1;
            if (edi12 <= rows12) {
                *reinterpret_cast<int *>(base12 + 0x80) = *reinterpret_cast<int *>(base12 + 0x7c);
                if (edi12 < 0) {
                    int a12 = abs(edi12);
                    int cnt12 = *reinterpret_cast<int *>(base12 + 0x84);
                    if (a12 <= cnt12) {
                        int steps12 = abs(edi12);
                        if (steps12 > 0) {
                            do {
                                int cur12 = *reinterpret_cast<int *>(base12 + 0x80);
                                steps12 -= 1;
                                *reinterpret_cast<int *>(base12 + 0x80) = *reinterpret_cast<int *>(cur12 + 0x10);
                            } while (steps12 != 0);
                        }
                        edi12 += *reinterpret_cast<int *>(base12 + 0x84);
                        *reinterpret_cast<int *>(base12 + 0x88) = edi12;
                    }
                } else {
                    if (edi12 > 0) {
                        int steps12b = edi12;
                        do {
                            int cur12b = *reinterpret_cast<int *>(base12 + 0x80);
                            steps12b -= 1;
                            *reinterpret_cast<int *>(base12 + 0x80) = *reinterpret_cast<int *>(cur12b + 0xc);
                        } while (steps12b != 0);
                    }
                    *reinterpret_cast<int *>(base12 + 0x88) = edi12;
                }
            }

            int vb13 = *reinterpret_cast<int *>(self - 0x48);
            int d13 = *reinterpret_cast<int *>(vb13 + 8);
            int base13 = d13 + self;
            int lim13 = *reinterpret_cast<int *>(d13 + self + 0xa4);
            if (ebx > lim13)
                goto L60af4f;

            int esi13 = 1;
            int edi13 = 0;
            int rowBase13 = base13;
            for (int ecx13 = ebx;;) {
                int flag13 = *reinterpret_cast<int *>(rowBase13 + 0x7c);
                int outv13;
                if (flag13 != edi13) {
                    int cur13 = *reinterpret_cast<int *>(rowBase13 + 0x80);
                    cur13 = *reinterpret_cast<int *>(cur13 + 0xc);
                    *reinterpret_cast<int *>(rowBase13 + 0x80) = cur13;
                    int c13 = *reinterpret_cast<int *>(rowBase13 + 0x88) + 1;
                    if (c13 == *reinterpret_cast<int *>(rowBase13 + 0x84))
                        c13 = edi13;
                    *reinterpret_cast<int *>(rowBase13 + 0x88) = c13;
                    outv13 = *reinterpret_cast<int *>(rowBase13 + 0x80);
                    outv13 = *reinterpret_cast<int *>(outv13 + 8);
                } else {
                    outv13 = 0;
                }
                *reinterpret_cast<int *>(outv13 + 8) = esi13;

                int vb14 = *reinterpret_cast<int *>(self - 0x48);
                int d14 = *reinterpret_cast<int *>(vb14 + 8);
                int lim14 = *reinterpret_cast<int *>(d14 + self + 0xa4);
                rowBase13 = d14 + self;
                ecx13 += 1;
                if (ecx13 > lim14)
                    break;
            }
        }
    }

L60af4f:
    {
        int esiF = self - 0x48;
        reinterpret_cast<ListBox *>(esiF)->set_selected_pos(ebx, 1);
        int eaxF = *reinterpret_cast<int *>(esiF);
        int edxF = *reinterpret_cast<int *>(eaxF + 8);
        if ((*reinterpret_cast<unsigned char *>(edxF + self - 0x28) & 4) != 0) {
            int eax2F = *reinterpret_cast<int *>(eaxF + 4);
            int ecxF = *reinterpret_cast<int *>(eax2F + self + 0x7c);
            if (ecxF != 0) {
                reinterpret_cast<VCall2 *>(ecxF)->slot044(-1);
            }
        }
    }

L60af7d:
    return;
}
