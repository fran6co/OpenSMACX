// ORIGINAL: 0x004C12A0 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004C12A0
// name           ?vehdraw_draw@@YAXPAUBuffer@@HHHHHH@Z
// size           1170 bytes
// measured tier  MISMATCH
// divergence     5
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004c12a0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?vehdraw_draw@@YAXPAUBuffer@@HHHHHH@Z  at 0x004C12A0  (1170 bytes)
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
class Caviar;
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
class MapWin;
class Menu;
typedef void * PVOID;
struct RECT;
class Scroll;
class Spot;
class Sprite;
struct VOX_Matrix;
struct VOX_Vect;
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
class Caviar { public:
    float scene_scale_;
    uint8_t unmapped_04_[0x2C - 0x04];
    int32_t field_2C_;
    int32_t field_30_;
    int32_t field_34_;
    uint8_t unmapped_38_[0x108 - 0x38];
    int32_t field_108_;
    uint8_t unmapped_10C_[0x13D0 - 0x10C];
    int render(Buffer *, int, int, int);
    int set_shadow_table(unsigned char *);
    void clear_buffers(int);
    void render_multi_table_dest(Buffer *, int, int, int, unsigned char *, int);
    void set_camera_direct(VOX_Vect *, VOX_Matrix *);
    void set_scaling(float);
    void set_scene_rotation(float, float, float);
};
class MapWin { public:
    uint32_t vbtable_pointer_;
    void * owned_;
    uint8_t derived_tail_[0x21A6C - 0x8];
    GraphicWin virtual_base_;
    int get_relative_alt(int, int, int);
};
extern "C" int __cdecl sub_627f50();
extern "C" int __cdecl sub_6280e0();
extern "C" int __cdecl sub_628150();
extern "C" int __cdecl sub_628220();
int __cdecl pick_top_veh(int);
int __cdecl vehdraw_construct_armor(int, int, int, int, int, Caviar *);
int __cdecl vehdraw_construct_vehicle(int, Caviar *);
void __cdecl compute_camera(VOX_Vect *, VOX_Matrix *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066aa40 = (int *)0x0066AA40;
static int *const g_0066d8d0 = (int *)0x0066D8D0;
static int *const g_0066df14 = (int *)0x0066DF14;
static int *const g_0066df18 = (int *)0x0066DF18;
static int *const g_00687ad8 = (int *)0x00687AD8;
static int *const g_00687adc = (int *)0x00687ADC;
static int *const g_00687ae0 = (int *)0x00687AE0;
static int *const g_0068e564 = (int *)0x0068E564;
static int *const g_0068e568 = (int *)0x0068E568;
static int *const g_0068e56c = (int *)0x0068E56C;
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_00691e6c = (int *)0x00691E6C;
static int *const g_00691e70 = (int *)0x00691E70;
static int *const g_006eef7c = (int *)0x006EEF7C;
static int *const g_006f107c = (int *)0x006F107C;
static int *const g_008cc828 = (int *)0x008CC828;
static int *const g_008cc888 = (int *)0x008CC888;
static int *const g_008cc88c = (int *)0x008CC88C;
static int *const g_008cc890 = (int *)0x008CC890;
static int *const g_008cc894 = (int *)0x008CC894;
static int *const g_008cc898 = (int *)0x008CC898;
static int *const g_008cc89c = (int *)0x008CC89C;
static int *const g_008cdbf4 = (int *)0x008CDBF4;
static int *const g_008cdd10 = (int *)0x008CDD10;
static int *const g_008e9f40 = (int *)0x008E9F40;
static int *const g_009156b0 = (int *)0x009156B0;
static int *const g_0094a30c = (int *)0x0094A30C;

extern int16_t g_00952828[];
extern int16_t g_0095282a[];
extern int8_t g_00952851[];
extern int16_t g_00952832[];
extern uint8_t g_009ab88c[];
extern uint8_t g_009ab88d[];
extern uint8_t g_009ab88e[];
extern uint8_t g_0094ae6a[];
extern uint8_t g_00952836[];
extern float g_008d3adc[];
extern float g_008dab68[];
extern float g_008e1bf4[];

typedef void (__cdecl *Vec3CombineFn)(float *, float *, float *);
typedef void (__cdecl *Vec3NormalizeFn)(float *, float *);
typedef void (__cdecl *MatTransformFn)(void *, void *);
typedef void (__cdecl *MatVecFn)(void *, void *, void *);

void __cdecl vehdraw_draw(Buffer * a1, int a2, int a3, int a4, int a5, int a6, int a7) {
    uint8_t frame[0xb0];
    char *fp = reinterpret_cast<char *>(frame) + 0xb0;

    *reinterpret_cast<uint32_t *>(fp - 0x48) = 0xbf13cd36;
    *reinterpret_cast<uint32_t *>(fp - 0x44) = 0x3f13cd36;
    *reinterpret_cast<uint32_t *>(fp - 0x40) = 0x3f13cd36;

    if (a1 == 0) {
        return;
    }

    *reinterpret_cast<int32_t *>(fp - 0x2c) = *g_00687ad8;
    *reinterpret_cast<int32_t *>(fp - 0x28) = *g_00687adc;
    *reinterpret_cast<int32_t *>(fp - 0x24) = *g_00687ae0;

    if (a5 & 1) {
        a2 = pick_top_veh(a2);
    }

    int32_t alt1, alt2;
    if (*g_0068e564 == -1 || *g_0068e56c != a2) {
        alt1 = g_00952828[a2 * 0x1a];
        alt2 = g_0095282a[a2 * 0x1a];
    } else {
        alt1 = *g_0068e564;
        alt2 = *g_0068e568;
    }

    int32_t protoOff = a2 * 0x34;
    int32_t moraleOrPad = g_00952851[protoOff];
    if (moraleOrPad == -1) {
        moraleOrPad = 0;
    }

    int16_t chassisIdx = g_00952832[a2 * 0x1a];
    uint8_t abVal = g_009ab88c[chassisIdx * 0x34];
    uint8_t abVal2 = g_009ab88d[chassisIdx * 0x34];

    bool skipTerrain =
        a7 == 0 ||
        (reinterpret_cast<uint8_t *>(g_0094a30c)[(*g_0068faf0 * alt2 + (alt1 >> 1)) * 0x2c] & 0xe0) <
            0x60 ||
        (abVal == 0 && g_0094ae6a[abVal2 * 0x10] < 3);

    if (skipTerrain) {
        *reinterpret_cast<int32_t *>(fp - 0x1c) = 0;
        *reinterpret_cast<int32_t *>(fp - 0x18) = 0;
        *reinterpret_cast<uint32_t *>(fp - 0x14) = 0x3f800000;
    } else {
        int32_t iv2 = reinterpret_cast<MapWin *>(0x9156b0)->get_relative_alt(alt1, alt2, 1);
        int32_t iv3 = reinterpret_cast<MapWin *>(0x9156b0)->get_relative_alt(alt1, alt2, 2);
        int32_t iv34 = reinterpret_cast<MapWin *>(0x9156b0)->get_relative_alt(alt1, alt2, 3);
        int32_t iv4 = reinterpret_cast<MapWin *>(0x9156b0)->get_relative_alt(alt1, alt2, 4);
        iv4 = iv4 + ((iv2 * 7 + iv3) * 7 + iv34) * 7;
        int32_t idx3 = iv4 * 3 - 0x4b0;
        int32_t idx12 = iv4 * 0xc;

        float *lightDir = reinterpret_cast<float *>(fp - 0x10);
        lightDir[0] = reinterpret_cast<float *>(g_008cdd10)[idx3];
        lightDir[1] = reinterpret_cast<float *>(g_008cdd10)[idx3 + 1];
        lightDir[2] = reinterpret_cast<float *>(g_008cdd10)[idx3 + 2];

        float *row1 = reinterpret_cast<float *>(fp - 0x64);
        row1[0] = g_008d3adc[idx12 + 0];
        row1[1] = g_008d3adc[idx12 + 1];
        row1[2] = g_008d3adc[idx12 + 2];
        float *row2 = reinterpret_cast<float *>(fp - 0x58);
        row2[0] = g_008dab68[idx12 + 0];
        row2[1] = g_008dab68[idx12 + 1];
        row2[2] = g_008dab68[idx12 + 2];
        float *row3 = reinterpret_cast<float *>(fp - 0x40);
        row3[0] = g_008e1bf4[idx12 + 0];
        row3[1] = g_008e1bf4[idx12 + 1];
        row3[2] = g_008e1bf4[idx12 + 2];

        float *vecOut = reinterpret_cast<float *>(fp - 0x1c);
        reinterpret_cast<Vec3CombineFn>(sub_628150)(lightDir, row1, vecOut);
        reinterpret_cast<Vec3CombineFn>(sub_628150)(vecOut, row2, vecOut);
        reinterpret_cast<Vec3CombineFn>(sub_628150)(vecOut, row3, vecOut);
        reinterpret_cast<Vec3NormalizeFn>(sub_628220)(vecOut, vecOut);
    }

    void *vecPtr = fp - 0x1c;
    void *matOutPtr = fp - 0x84;
    compute_camera(reinterpret_cast<VOX_Vect *>(vecPtr), reinterpret_cast<VOX_Matrix *>(matOutPtr));

    *reinterpret_cast<int32_t *>(g_008cc888) = *reinterpret_cast<int32_t *>(fp - 0x2c);
    *reinterpret_cast<int32_t *>(g_008cc88c) = *reinterpret_cast<int32_t *>(fp - 0x28);
    *reinterpret_cast<int32_t *>(g_008cc890) = *reinterpret_cast<int32_t *>(fp - 0x24);

    void *matA = fp - 0xa8;
    reinterpret_cast<MatTransformFn>(sub_627f50)(matOutPtr, matA);

    float *outVec = reinterpret_cast<float *>(fp - 0xc);
    reinterpret_cast<MatVecFn>(sub_6280e0)(matA, fp - 0x48, outVec);

    *reinterpret_cast<float *>(g_008cc898) = outVec[1];
    *reinterpret_cast<float *>(g_008cc894) = outVec[0];
    *reinterpret_cast<float *>(g_008cc89c) = outVec[2];

    reinterpret_cast<Caviar *>(g_008cc828)->set_shadow_table(reinterpret_cast<unsigned char *>(g_006f107c));

    float *shade = reinterpret_cast<float *>(fp - 0xc);
    float *light = reinterpret_cast<float *>(fp - 0x70);
    shade[0] = light[0] * *reinterpret_cast<float *>(g_0066aa40);
    shade[1] = light[1] * *reinterpret_cast<float *>(g_0066aa40);
    shade[2] = light[2] * *reinterpret_cast<float *>(g_0066aa40);

    float scale;
    if (*g_00691e6c < 1) {
        scale = static_cast<float>(a6 + 0x10) * *reinterpret_cast<float *>(g_0066df18) *
                *reinterpret_cast<float *>(g_0066d8d0);
    } else {
        scale = (static_cast<float>(*g_00691e6c) * *reinterpret_cast<float *>(g_0066df18)) /
                static_cast<float>(*g_00691e70);
    }
    reinterpret_cast<Caviar *>(g_008cc828)->set_scaling(scale);
    reinterpret_cast<Caviar *>(g_008cc828)
        ->set_camera_direct(reinterpret_cast<VOX_Vect *>(fp - 0xc), reinterpret_cast<VOX_Matrix *>(fp - 0x84));

    vehdraw_construct_vehicle(a2, reinterpret_cast<Caviar *>(g_008cc828));

    int32_t rotIdx = (moraleOrPad - 3) & 7;
    reinterpret_cast<Caviar *>(g_008cc828)
        ->set_scene_rotation(static_cast<float>(rotIdx) * *reinterpret_cast<float *>(g_0066df14) -
                                  *reinterpret_cast<float *>(g_008e9f40),
                              0, 0);

    int32_t offX;
    if (*g_00691e6c > 0) {
        offX = (*g_00691e6c * 50) / *g_00691e70;
    } else {
        int32_t t = (a6 * 5 + 0x50) * 10;
        offX = (t + ((t >> 31) & 0xf)) >> 4;
    }
    int32_t offY;
    if (*g_00691e6c > 0) {
        offY = (*g_00691e6c * 50) / *g_00691e70;
    } else {
        int32_t t = (a6 * 5 + 0x50) * 10;
        offY = (t + ((t >> 31) & 0xf)) >> 4;
    }

    int32_t drawX = offX + a3;
    int32_t drawY = offY + a4;
    int32_t renderFlags = ((static_cast<uint32_t>(a5) >> 8) & 0x20 | 0x10) >> 4;
    reinterpret_cast<Caviar *>(g_008cc828)->render(a1, drawX, drawY, renderFlags);

    int32_t armorIdx = g_00952832[protoOff / 0x34 * 0x1a] * 0x34;
    if (g_009ab88e[armorIdx] == 5) {
        *reinterpret_cast<int32_t *>(g_008cdbf4) = 0;
        vehdraw_construct_armor(g_00952836[protoOff], g_009ab88c[armorIdx], g_009ab88d[armorIdx], 5,
                                 1, reinterpret_cast<Caviar *>(g_008cc828));
        reinterpret_cast<Caviar *>(g_008cc828)
            ->render_multi_table_dest(a1, drawX, drawY, 0x18,
                                       reinterpret_cast<unsigned char *>(g_006eef7c),
                                       (static_cast<uint32_t>(a5) >> 0xc) & 2);
        return;
    }

    reinterpret_cast<Caviar *>(g_008cc828)->clear_buffers(0);
}
