// ORIGINAL: 0x004C0F50 FILE
// name      ?vehdraw_scoot@@YAXPAUBuffer@@HHHH@Z
// size      838 bytes
// spans     0x004C0F50-0x004C1296
// prototype 
// callers   1   call targets   15
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004BF740 0x004C1D20 0x004C3090 0x006171A0 0x00617230 0x006182A0 0x00618370 0x006183B0 0x00618430 0x00618470 0x00627F50 0x006280E0 0x00628150 0x006281B0 0x00628220
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004C0F50
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004c0f50/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?vehdraw_scoot@@YAXPAUBuffer@@HHHH@Z  at 0x004C0F50  (838 bytes)
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

struct BITMAPINFO;
typedef int BOOL;
class Buffer;
class Caviar;
class Font;
typedef void * HBITMAP;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
typedef char * LPSTR;
typedef void * LPVOID;
struct RECT;
class Spot;
struct VOX_Matrix;
struct VOX_Vect;

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
extern "C" int __cdecl sub_627f50();
extern "C" int __cdecl sub_6280e0();
extern "C" int __cdecl sub_628150();
extern "C" int __cdecl sub_6281b0();
extern "C" int __cdecl sub_628220();
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
static int *const g_00691e6c = (int *)0x00691E6C;
static int *const g_00691e70 = (int *)0x00691E70;
static int *const g_006eef7c = (int *)0x006EEF7C;
static int *const g_006f107c = (int *)0x006F107C;
static int *const g_008cc25c = (int *)0x008CC25C;
static int *const g_008cc268 = (int *)0x008CC268;
static int *const g_008cc26c = (int *)0x008CC26C;
static int *const g_008cc270 = (int *)0x008CC270;
static int *const g_008cc278 = (int *)0x008CC278;
static int *const g_008cc27c = (int *)0x008CC27C;
static int *const g_008cc280 = (int *)0x008CC280;
static int *const g_008cc288 = (int *)0x008CC288;
static int *const g_008cc28c = (int *)0x008CC28C;
static int *const g_008cc290 = (int *)0x008CC290;
static int *const g_008cc820 = (int *)0x008CC820;
static int *const g_008cc828 = (int *)0x008CC828;
static int *const g_008cc888 = (int *)0x008CC888;
static int *const g_008cc88c = (int *)0x008CC88C;
static int *const g_008cc890 = (int *)0x008CC890;
static int *const g_008cc894 = (int *)0x008CC894;
static int *const g_008cc898 = (int *)0x008CC898;
static int *const g_008cc89c = (int *)0x008CC89C;
static int *const g_008cdbf4 = (int *)0x008CDBF4;
static int *const g_008cdcf8 = (int *)0x008CDCF8;
static int *const g_008cdd00 = (int *)0x008CDD00;
static int *const g_008cdd04 = (int *)0x008CDD04;
static int *const g_008cdd08 = (int *)0x008CDD08;
static int *const g_008e9f40 = (int *)0x008E9F40;
static int *const g_008e9f50 = (int *)0x008E9F50;

void __cdecl vehdraw_scoot(Buffer * a1, int a2, int a3, int a4, int a5) {
    int val = 0;
}
