// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004BFC70
// name           ?vehdraw_turn@@YAXPAUBuffer@@HHHH@Z
// size           394 bytes
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004bfc70/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?vehdraw_turn@@YAXPAUBuffer@@HHHH@Z  at 0x004BFC70  (394 bytes)
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
    void clear_buffers(int);
    void render_multi_table_dest(Buffer *, int, int, int, unsigned char *, int);
    void set_scaling(float);
    void set_scene_rotation(float, float, float);
};
int __cdecl vehdraw_construct_armor(int, int, int, int, int, Caviar *);
int __cdecl vehdraw_construct_vehicle(int, Caviar *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066d8d0 = (int *)0x0066D8D0;
static int *const g_0066df18 = (int *)0x0066DF18;
static int *const g_00691e6c = (int *)0x00691E6C;
static int *const g_00691e70 = (int *)0x00691E70;
static int *const g_006eef7c = (int *)0x006EEF7C;
static int *const g_008cc258 = (int *)0x008CC258;
static int *const g_008cc260 = (int *)0x008CC260;
static int *const g_008cc820 = (int *)0x008CC820;
static int *const g_008cc828 = (int *)0x008CC828;
static int *const g_008cdbf4 = (int *)0x008CDBF4;
static int *const g_008e9f40 = (int *)0x008E9F40;

// 0x00952832: rows of 0x34 bytes; a short at +0, a byte at +4.
extern unsigned char g_table1[];
// 0x009AB88C: rows of 0x34 bytes; bytes at +0, +1, +2.
extern unsigned char g_table2[];

void __cdecl vehdraw_turn(Buffer * a1, int a2, int a3, int a4, int a5) {
    float scale;
    if (*g_00691e6c > 0) {
        scale = static_cast<float>(*g_00691e6c) * *reinterpret_cast<float *>(g_0066df18)
              / static_cast<float>(*g_00691e70);
    } else {
        scale = static_cast<float>(a5 + 0x10) * *reinterpret_cast<float *>(g_0066df18)
              * *reinterpret_cast<float *>(g_0066d8d0);
    }
    int veh_index = *g_008cc820;
    reinterpret_cast<Caviar *>(g_008cc828)->set_scaling(scale);

    float rotation = static_cast<float>(a4) * *reinterpret_cast<float *>(g_008cc258)
                    + *reinterpret_cast<float *>(g_008cc260)
                    - *reinterpret_cast<float *>(g_008e9f40);
    reinterpret_cast<Caviar *>(g_008cc828)->set_scene_rotation(rotation, 0.0f, 0.0f);

    int x_offset;
    if (*g_00691e6c > 0) {
        x_offset = (*g_00691e6c * 50) / *g_00691e70;
    } else {
        x_offset = ((a5 * 5 + 0x50) * 10) / 16;
    }
    int y_offset;
    if (*g_00691e6c > 0) {
        y_offset = (*g_00691e6c * 50) / *g_00691e70;
    } else {
        y_offset = ((a5 * 5 + 0x50) * 10) / 16;
    }

    int x = x_offset + a2;
    int y = y_offset + a3;

    *g_008cdbf4 = 0;
    vehdraw_construct_vehicle(veh_index, reinterpret_cast<Caviar *>(g_008cc828));
    reinterpret_cast<Caviar *>(g_008cc828)->render(a1, x, y, 1);

    int row1 = veh_index * 0x34;
    short type_idx = *reinterpret_cast<short *>(g_table1 + row1);
    int row2 = type_idx * 0x34;

    if (g_table2[row2 + 2] == 5) {
        *g_008cdbf4 = 0;
        vehdraw_construct_armor(g_table1[row1 + 4], g_table2[row2 + 0], g_table2[row2 + 1], 5, 1,
                                 reinterpret_cast<Caviar *>(g_008cc828));
        reinterpret_cast<Caviar *>(g_008cc828)->render_multi_table_dest(
            a1, x, y, 0x18, reinterpret_cast<unsigned char *>(g_006eef7c), 0);
        return;
    }

    reinterpret_cast<Caviar *>(g_008cc828)->clear_buffers(0);
}
