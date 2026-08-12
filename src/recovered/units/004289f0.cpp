// ORIGINAL: 0x004289F0 FILE
// name      ?exec@Credits@@QAEXXZ
// size      514 bytes
// spans     0x004289F0-0x00428BF2
// prototype void (__thiscall ?exec@Credits@@QAEXXZ)(Credits* this)
// callers   1   call targets   15
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00428880 0x004C6920 0x004C6EA0 0x005D4E40 0x005D4EF0 0x005D5250 0x005D5500 0x005DAC70 0x00616350 0x00616780 0x00616860 0x00616870 0x00618F40 0x00644EF2 0x00645470
// indirect  0x00428A1C 0x00428AD7 0x00428B6F 0x00428B7F 0x00428BE5
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004289F0
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004289f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?exec@Credits@@QAEXXZ  at 0x004289F0  (514 bytes)
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
struct BorderSizing;
class Buffer;
class Credits;
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
class Palette;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class Time;
typedef unsigned int UINT_PTR;
class Wave;
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
    int load_pcx(char *, Palette *, int, int);
    void close();
    void fill(int);
    void init(int, int, int, int, char *, int, Win *, Menu *, BorderSizing *);
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
    void __cdecl release_modal();
    void __cdecl set_modal();
    void close();
    void start(void (__cdecl *)(int), int, int, int);
};
class Wave { public:
    uint32_t vtable_storage_;
    uint32_t volume_;
    uint32_t field_8_;
    uint8_t memset_region_[0x24];
    uint32_t field_30_;
    uint32_t field_34_;
    uint32_t field_38_;
    void * device_;
    uint32_t field_40_;
    Wave * chain_prev_;
    Wave * chain_next_;
    void * fname_;
    uint32_t field_50_;
    uint8_t flags_54_;
    uint8_t pad_55_[3];
    int32_t pitch_;
    float reverb_mix_;
    int32_t ms_length_;
    uint32_t start_time_;
    uint32_t group_slot_;
    int play();
    void unload();
};
extern "C" char *strcat(char *, const char *);
extern "C" void free(void *);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 1, 57, 90
class VCall { public:
    virtual void slot000();
    virtual void slot001();  // <-- used
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
    virtual void slot057();  // <-- used
    virtual void slot058();
    virtual void slot059();
    virtual void slot060();
    virtual void slot061();
    virtual void slot062();
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
static int *const g_00428dd0 = (int *)0x00428DD0;
static int *const g_005398e0 = (int *)0x005398E0;
static int *const g_006832b0 = (int *)0x006832B0;
static int *const g_006832bc = (int *)0x006832BC;
static int *const g_00691b28 = (int *)0x00691B28;
static int *const g_006f127c = (int *)0x006F127C;
static int *const g_00703e30 = (int *)0x00703E30;
static int *const g_009a6490 = (int *)0x009A6490;
static int *const g_009b7b1c = (int *)0x009B7B1C;
static int *const g_009b7b20 = (int *)0x009B7B20;
static int *const g_009b86a0 = (int *)0x009B86A0;

class Credits { public:
    uint8_t pad_0_[0xA14];
    uint32_t field_a14_;

    void load();
    void exec();
};

// Extra vtable shims: the emitter's VCall declares every slot nullary; these
// give the two slots this body needs (1 and 57) the argument lists their
// calls actually push. Slot 90 stays nullary and reuses the emitted VCall.
class VCall1 { public:
    virtual void slot000();
    virtual void slot001(int);  // <-- used
};
class VCall58 { public:
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

void Credits::exec() {
    // Object embedded at +0x21F8 has a vtable; slot 90 is called with no
    // args, both here and again at the end - the compiler spills its
    // address to the stack (matches the frame's one local: `push ecx`).
    void *vobj0 = reinterpret_cast<char *>(this) + 0x21F8;
    Time *timeObj = reinterpret_cast<Time *>(reinterpret_cast<char *>(this) + 0x2D44);

    timeObj->close();
    reinterpret_cast<GraphicWin *>(this)->close();

    reinterpret_cast<VCall *>(vobj0)->slot090();

    reinterpret_cast<GraphicWin *>(this)->init(
        0, 0, *g_009b7b1c, *g_009b7b20, 0, 0, 0, 0, 0);

    *reinterpret_cast<char *>(g_009b86a0) = 0;
    strcat(reinterpret_cast<char *>(g_009b86a0),
           (*g_009b7b1c == 0x320) ? reinterpret_cast<char *>(g_006832b0)
                                   : reinterpret_cast<char *>(g_006832bc));

    reinterpret_cast<GraphicWin *>(this)->load_pcx(
        reinterpret_cast<char *>(g_009b86a0), 0, 10, 0xEC);

    void *swing = reinterpret_cast<char *>(this) + 0x1234;
    reinterpret_cast<GraphicWin *>(swing)->init(
        0x1E, (*g_009b7b1c == 0x320) ? 100 : 200, 400, 400, 0, 0x1000020,
        reinterpret_cast<Win *>(this), 0, 0);
    reinterpret_cast<GraphicWin *>(swing)->fill(9);

    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x12D8) =
        reinterpret_cast<int>(g_006f127c);
    reinterpret_cast<VCall1 *>(swing)->slot001(0);

    field_a14_ = 0;

    swing = reinterpret_cast<char *>(this) + 0x1C48;
    reinterpret_cast<Font *>(swing)->init(*reinterpret_cast<char **>(g_00691b28), 0xE, 1);
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x122C) =
        (reinterpret_cast<Font *>(swing)->unk_1_ < 0)
            ? reinterpret_cast<Font *>(swing)->line_height_
            : reinterpret_cast<Font *>(swing)->height_ + reinterpret_cast<Font *>(swing)->unk_1_;

    load();

    reinterpret_cast<Buffer *>(reinterpret_cast<char *>(this) + 0x1678)
        ->set_font(reinterpret_cast<Font *>(swing), 0, 0, 0);
    reinterpret_cast<Buffer *>(reinterpret_cast<char *>(this) + 0x444)
        ->set_font(reinterpret_cast<Font *>(swing), 0, 0, 0);

    timeObj->set_modal();
    timeObj->start(reinterpret_cast<void(__cdecl *)(int)>(g_00428dd0),
                    reinterpret_cast<int>(this), 0x1E, 5);

    if ((*g_009a6490 & 0x800) != 0) {
        reinterpret_cast<Wave *>(g_00703e30)->play();
    }

    reinterpret_cast<VCall1 *>(this)->slot001(0);
    reinterpret_cast<VCall58 *>(this)->slot057(
        0, reinterpret_cast<int>(g_005398e0), 0);

    // ecx is already `this` here (from the vtable calls just above) and
    // release_modal never gets a fresh `mov ecx` in the disassembly, so the
    // callee does not read its incoming `this` at all.
    reinterpret_cast<Time *>(this)->release_modal();

    void **p = reinterpret_cast<void **>(reinterpret_cast<char *>(this) + 0xA1C);
    int n = 0x200;
    do {
        if (*p != 0) {
            free(*p);
            *p = 0;
        }
        ++p;
    } while (--n);

    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0xA18) = 0;
    field_a14_ = 0;
    reinterpret_cast<Wave *>(g_00703e30)->unload();

    reinterpret_cast<Time *>(reinterpret_cast<char *>(this) + 0x2D44)->close();
    reinterpret_cast<GraphicWin *>(this)->close();
    reinterpret_cast<VCall *>(vobj0)->slot090();
}
