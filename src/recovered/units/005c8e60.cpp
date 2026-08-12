// ORIGINAL: 0x005C8E60 FILE
// name      ?exec@Gamma@@QAEXPAUPalette@@MH@Z
// size      1221 bytes
// spans     0x005C8E60-0x005C9325
// prototype void (__thiscall ?exec@Gamma@@QAEXPAUPalette@@MH@Z)(Gamma* this, Palette*, float, int)
// callers   2   call targets   24
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005C9520 0x005C95E0 0x005D4E40 0x005D4EF0 0x005D5250 0x005DAC70 0x005FCBB0 0x005FE500 0x00605370 0x00605840 0x006059B0 0x00605A10 0x00605A50 0x00605A90 0x00605AD0 0x00605B10 0x00605BE0 0x00605C30 0x00605D20 0x00607210 0x00607360 0x00614F30 0x00618F40 0x006458C8
// indirect  0x005C9116 0x005C915F 0x005C91BF 0x005C9219 0x005C927C 0x005C9288 0x005C929B 0x005C92AB 0x005C92B5 0x005C9301 0x005C930B
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005C8E60
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005c8e60/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?exec@Gamma@@QAEXPAUPalette@@MH@Z  at 0x005C8E60  (1221 bytes)
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
class BaseButton;
struct BorderSizing;
class Buffer;
class ButtonGroup;
class EditBox;
class Font;
class Gamma;
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
class Palette;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class Time;
typedef unsigned int UINT_PTR;
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

class ButtonGroup { public:
    BaseButton * buttons_[32];
    uint32_t count_;
    uint32_t field_84_;
    uint32_t field_88_;
    uint32_t field_8C_;
    uint32_t field_90_;
};

class Heap { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
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

class Time { public:
    int unk_tgl_;
    UINT_PTR id_event_;
    void * callback1_;
    void * callback2_;
    int cb_param2_;
    int cb_param1_;
    uint32_t count_;
    int unk_1_;
    uint32_t resolution_;
    int unk_2_;
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
    int set_font(Font *, Font *, Font *, Font *);
};
class BaseButton { public:
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
    uint32_t field_A14_;
    uint32_t field_A18_;
    Time time1_;
    uint32_t field_A44_;
    uint32_t field_A48_;
    Time time2_;
    uint32_t field_A74_;
    uint32_t field_A78_;
    LPSTR name_;
    LPSTR bubble_text_;
    uint32_t color_;
    uint32_t bevel_upper_;
    uint32_t bevel_lower_;
    uint32_t bevel_thickness_;
    uint32_t field_A94_;
    uint32_t field_A98_;
    uint32_t field_A9C_;
    uint32_t field_AA0_;
    uint32_t field_AA4_;
    ButtonGroup * group_;
    uint32_t field_AAC_;
    uint32_t field_AB0_;
    uint32_t field_AB4_;
    int init(char *, int, int, int, int, int, Win *, int);
    void set_text_color(int, int, int, int);
};
class EditBox { public:
    void close();
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
    int init(char *, int, int);
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
    void close();
    void fill(int);
    void init(int, int, int, int, char *, int, Win *, Menu *, BorderSizing *);
};
class Palette { public:
    void close();
};
class Scroll { public:
    int init(RECT *, Win *, int, int);
    void close();
    void set_bevel_lower(int);
    void set_bevel_thickness(int);
    void set_bevel_upper(int);
    void set_border_color(int);
    void set_button_color(int);
    void set_pos(int);
    void set_range(int, int);
    void set_sprite_left(Sprite *, Sprite *, Sprite *);
    void set_sprite_right(Sprite *, Sprite *, Sprite *);
};
extern "C" int __cdecl _ftol();
void __cdecl do_all_draws();

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 1, 2, 57, 62, 90
class VCall { public:
    virtual void slot000();
    virtual void slot001();  // <-- used
    virtual void slot002();  // <-- used
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
    virtual void slot044();
    virtual void slot045();
    virtual void slot046();
    virtual void slot047();
    virtual void slot048();
    virtual void slot049();
    virtual void slot050();
    virtual void slot051();
    virtual void slot052();
    virtual void slot053();
    virtual void slot054();
    virtual void slot055();
    virtual void slot056();
    virtual void slot057();  // <-- used
    virtual void slot058();
    virtual void slot059();
    virtual void slot060();
    virtual void slot061();
    virtual void slot062();  // <-- used
    virtual void slot063();
    virtual void slot064();
    virtual void slot065();
    virtual void slot066();
    virtual void slot067();
    virtual void slot068();
    virtual void slot069();
    virtual void slot070();
    virtual void slot071();
    virtual void slot072();
    virtual void slot073();
    virtual void slot074();
    virtual void slot075();
    virtual void slot076();
    virtual void slot077();
    virtual void slot078();
    virtual void slot079();
    virtual void slot080();
    virtual void slot081();
    virtual void slot082();
    virtual void slot083();
    virtual void slot084();
    virtual void slot085();
    virtual void slot086();
    virtual void slot087();
    virtual void slot088();
    virtual void slot089();
    virtual void slot090();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066d58c = (int *)0x0066D58C;
static int *const g_00691e74 = (int *)0x00691E74;
static int *const g_009b7b1c = (int *)0x009B7B1C;
static int *const g_009b7b20 = (int *)0x009B7B20;
static int *const g_009b8d80 = (int *)0x009B8D80;
static int *const g_009b8d84 = (int *)0x009B8D84;

class Gamma { public:
    void adjust_palette();
    void adjust_pal(double);
    void exec(Palette *, float, int);
};

// Slot 1 needs an int argument at both call sites here (the emitted `VCall`
// declares it nullary), so it gets its own shim per RULES item 4 rather than
// editing the auto-generated class.
class VCallInt { public:
    virtual void slot000();
    virtual void slot001(int);  // <-- used
};

// Slot 57 needs three int arguments here; same reasoning as VCallInt, but
// slot 57 needs the filler run out that far since only declaration order
// (not signature) fixes a slot's index.
class VCall57 { public:
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
    virtual void slot044();
    virtual void slot045();
    virtual void slot046();
    virtual void slot047();
    virtual void slot048();
    virtual void slot049();
    virtual void slot050();
    virtual void slot051();
    virtual void slot052();
    virtual void slot053();
    virtual void slot054();
    virtual void slot055();
    virtual void slot056();
    virtual void slot057(int, int, int);  // <-- used
};

void Gamma::exec(Palette * a1, float a2, int a3) {
    char *self = reinterpret_cast<char *>(this);
    int *p = reinterpret_cast<int *>(this);

    *reinterpret_cast<Palette **>(self + 0xa64) = a1;
    *reinterpret_cast<float *>(self + 0xee4) = a2;

    if (a3 != 0) {
        adjust_pal(static_cast<double>(a2));
        return;
    }

    int iVar4 = *g_009b7b20;
    int iVar3 = *g_009b7b1c;

    p[0x285] = 0;
    p[0x286] = 0;
    p[0x287] = 0xec;
    p[0x288] = 0x20;
    int iVar5 = ((iVar3 - p[0x287]) + p[0x285]) / 2;
    p[0x285] += iVar5;
    p[0x287] += iVar5;
    iVar5 = ((p[0x286] - p[0x288]) + iVar4) / 2;
    p[0x286] += iVar5;
    p[0x288] += iVar5;

    p[0x289] = p[0x285];
    p[0x28a] = p[0x286];
    p[0x28b] = p[0x287];
    p[0x28c] = p[0x288];
    iVar5 = (p[0x28c] - p[0x28a]) + 3;
    p[0x28a] += iVar5;
    p[0x28c] += iVar5;

    p[0x28d] = p[0x285];
    p[0x28e] = p[0x286];
    p[0x28f] = p[0x287];
    p[0x290] = p[0x288];
    iVar5 = (p[0x288] - p[0x286]) + 3;
    p[0x28e] += iVar5;
    p[0x290] += iVar5;
    p[0x290] -= 4;
    p[0x28e] += 4;

    p[0x291] = p[0x28d];
    p[0x292] = p[0x28e];
    p[0x293] = p[0x28f];
    p[0x294] = p[0x290];
    iVar5 = (p[0x294] - p[0x292]) + 3;
    p[0x292] += iVar5;
    p[0x294] += iVar5;
    p[0x294] = p[0x292] + 0x14;
    p[0x293] = ((p[0x293] - p[0x291]) + 3) / 2 - 3 + p[0x291];

    p[0x295] = p[0x291];
    p[0x296] = p[0x292];
    p[0x297] = p[0x293];
    p[0x298] = p[0x294];
    iVar5 = (p[0x297] - p[0x295]) + 3;
    p[0x295] += iVar5;
    p[0x297] += iVar5;

    reinterpret_cast<Font *>(self + 0xebc)->init(reinterpret_cast<char *>(g_00691e74), 0xe, 1);
    reinterpret_cast<GraphicWin *>(self)->init(0, 0, iVar3, iVar4, static_cast<char *>(0), 0,
                                                static_cast<Win *>(0), static_cast<Menu *>(0),
                                                static_cast<BorderSizing *>(0));
    reinterpret_cast<GraphicWin *>(self)->fill(0xf8);

    Scroll *scroll = reinterpret_cast<Scroll *>(self + 0xef0);
    scroll->init(reinterpret_cast<RECT *>(self + 0xa34), reinterpret_cast<Win *>(self), 0, 0);
    scroll->set_border_color(0);
    *reinterpret_cast<int *>(self + 0x1954) = 0xf8;
    scroll->set_button_color(7);
    *reinterpret_cast<int *>(self + 0x1938) = 7;
    scroll->set_bevel_thickness(2);
    scroll->set_bevel_upper(7);
    scroll->set_bevel_lower(0);
    *reinterpret_cast<int *>(self + 0x1930) = 0xa;
    *reinterpret_cast<int *>(self + 0x1964) = 0;
    scroll->set_sprite_left(0, 0, 0);
    scroll->set_sprite_right(0, 0, 0);
    scroll->set_range(1, 0x190);
    int pos = static_cast<int>(a2 * *reinterpret_cast<float *>(g_0066d58c));
    scroll->set_pos(pos);
    reinterpret_cast<VCallInt *>(scroll)->slot001(0);

    int left = p[0x28d], top = p[0x28e], right = p[0x28f], bottom = p[0x290];
    int height = bottom - top;
    int width = right - left;
    int icon1 = *g_009b8d80;
    BaseButton *bb1 = reinterpret_cast<BaseButton *>(self + 0x3bb0);
    bb1->init(reinterpret_cast<char *>(icon1), -1, left, top, width, height,
              reinterpret_cast<Win *>(self), 0);
    reinterpret_cast<Buffer *>(reinterpret_cast<char *>(bb1) + 0x444)
        ->set_font(reinterpret_cast<Font *>(self + 0xebc), 0, 0, 0);
    reinterpret_cast<VCall *>(bb1)->slot062();
    bb1->set_text_color(0, -1, 1, 1);
    *reinterpret_cast<int *>(self + 0x4634) = 0xf8;
    *reinterpret_cast<int *>(self + 0x4638) = 7;
    *reinterpret_cast<int *>(self + 0x463c) = 0;
    *reinterpret_cast<int *>(self + 0x4640) = 2;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(bb1) + 0xac8) = 0;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(bb1) + 0xaec) = 0;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(bb1) + 0xac0) = 0;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(bb1) + 0xae4) = 0;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(bb1) + 0xacc) = 0;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(bb1) + 0xaf0) = 0;
    reinterpret_cast<VCall *>(bb1)->slot062();

    left = p[0x295];
    top = p[0x296];
    right = p[0x297];
    bottom = p[0x298];
    height = bottom - top;
    width = right - left;
    int icon2 = *g_009b8d84;
    BaseButton *bb2 = reinterpret_cast<BaseButton *>(self + 0x46b0);
    bb2->init(reinterpret_cast<char *>(icon2), -2, left, top, width, height,
              reinterpret_cast<Win *>(self), 0);
    reinterpret_cast<Buffer *>(reinterpret_cast<char *>(bb2) + 0x444)
        ->set_font(reinterpret_cast<Font *>(self + 0xebc), 0, 0, 0);
    reinterpret_cast<VCall *>(bb2)->slot062();
    bb2->set_text_color(0, -1, 1, 1);
    *reinterpret_cast<int *>(self + 0x5134) = 0xf8;
    *reinterpret_cast<int *>(self + 0x5138) = 7;
    *reinterpret_cast<int *>(self + 0x513c) = 0;
    *reinterpret_cast<int *>(self + 0x5140) = 2;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(bb2) + 0xac8) = 0;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(bb2) + 0xaec) = 0;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(bb2) + 0xac0) = 0;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(bb2) + 0xae4) = 0;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(bb2) + 0xacc) = 0;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(bb2) + 0xaf0) = 0;
    reinterpret_cast<VCall *>(bb2)->slot062();

    reinterpret_cast<VCallInt *>(self)->slot001(1);
    do_all_draws();
    adjust_palette();
    reinterpret_cast<VCall *>(self)->slot062();
    reinterpret_cast<VCall57 *>(self)->slot057(0, 0, 0);
    reinterpret_cast<VCall *>(self)->slot002();
    do_all_draws();
    adjust_pal(*reinterpret_cast<double *>(self + 0xee8));

    *reinterpret_cast<Palette **>(self + 0xa64) = 0;
    reinterpret_cast<Palette *>(self + 0xa68)->close();
    scroll->close();
    reinterpret_cast<EditBox *>(self + 0x303c)->close();
    reinterpret_cast<VCall *>(bb1)->slot090();
    reinterpret_cast<VCall *>(bb2)->slot090();
    reinterpret_cast<GraphicWin *>(self)->close();
}
