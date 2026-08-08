// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0048AF30
// name           ?on_redraw@PlanWin@@QAEXXZ
// size           1159 bytes
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0048af30/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_redraw@PlanWin@@QAEXXZ  at 0x0048AF30  (1159 bytes)
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
class PlanWin;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class Win;

// ---- callees, declared and never defined (a definition would be inlined) ----
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
    int copy(Buffer *, int, int, int, int);
    void line(int, int, int, int, int, int, int, int, int);
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

class MapWin { public:
    uint32_t vbtable_pointer_;
    void * owned_;
    uint8_t derived_tail_[0x21A6C - 0x8];
    GraphicWin virtual_base_;
    void tile_to_pixel(int, int, int *, int *);
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

int __cdecl veh_draw(Buffer *, int, int, int, int, int, int);
int __cdecl veh_fake(int, int);
void __cdecl base_draw(Buffer *, int, int, int, int, int);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 63
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
    virtual void slot057();
    virtual void slot058();
    virtual void slot059();
    virtual void slot060();
    virtual void slot061();
    virtual void slot062();
    virtual void slot063();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00939fd4 = (int *)0x00939FD4;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_0096c9e0 = (int *)0x0096C9E0;
static int *const g_0097d042 = (int *)0x0097D042;
static int *const g_0097d044 = (int *)0x0097D044;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009a64cc = (int *)0x009A64CC;

class PlanWin { public:
    void on_redraw();
};

void PlanWin::on_redraw() {
    char *self = reinterpret_cast<char *>(this);
    int eax, ebx, ecx, edx, esi;
    int local_4, local_8, local_c, local_10, local_14;
    int local_18, local_1c, local_20, local_24, local_28, local_2c;

    eax = *reinterpret_cast<int *>(self - 0x5E8);
    if (eax == 0) {
        return;
    }
    eax = reinterpret_cast<int>(self) - 0x22050;
    if (eax != 0) {
        eax = *reinterpret_cast<int *>(self - 0x22050);
        ecx = *reinterpret_cast<int *>(eax + 4);
        ecx = ecx + reinterpret_cast<int>(self) - 0x21C0C;
    } else {
        ecx = 0;
    }
    edx = *reinterpret_cast<int *>(self - 0x22050);
    eax = *reinterpret_cast<int *>(edx + 4);
    edx = *reinterpret_cast<int *>(eax + reinterpret_cast<int>(self) - 0x21B88);
    eax = eax + reinterpret_cast<int>(self);
    edx = -edx;
    eax = *reinterpret_cast<int *>(eax - 0x21B8C);
    reinterpret_cast<Buffer *>(self - 0x5E0)->copy(
        reinterpret_cast<Buffer *>(ecx), 0, 0, eax, edx);

    eax = *g_009a64cc;
    local_4 = 0;
    if (eax <= 0) {
        esi = *g_00939284;
        goto L_48b10e;
    }
    esi = *g_00939284;
    ebx = 0x97D042;

L_48afb2:
    if ((*reinterpret_cast<unsigned char *>(g_009a64c0) & 0x80) != 0) {
        goto L_48b023;
    }
    ecx = esi;
    ecx = (ecx << 6) + ecx;
    edx = esi + ecx * 2;
    eax = esi + edx * 8;
    ecx = esi + eax * 2;
    eax = *reinterpret_cast<int *>(0x96C9E0 + ecx * 4);
    if (((eax >> 8) & 2) != 0) {
        goto L_48b008;
    }
    edx = *reinterpret_cast<short *>(ebx);
    edx = edx * (*g_0068faf0);
    eax = *reinterpret_cast<short *>(ebx - 2);
    eax = eax >> 1;
    eax = eax + edx;
    ecx = eax + eax * 4;
    edx = eax + ecx * 2;
    eax = 1;
    ecx = esi;
    eax = eax << (ecx & 0x1f);
    ecx = *g_0094a30c;
    if ((*reinterpret_cast<unsigned char *>(ecx + edx * 4 + 4) & eax) != 0) {
        goto L_48b008;
    }
    goto L_48b0eb;

L_48b008:
    edx = 0;
    edx = (edx & ~0xff) | *reinterpret_cast<unsigned char *>(ebx + 2);
    if (edx == esi) {
        goto L_48b023;
    }
    eax = 1;
    ecx = esi;
    eax = eax << (ecx & 0x1f);
    if ((*reinterpret_cast<unsigned char *>(ebx + 8) & eax) == 0) {
        goto L_48b0eb;
    }

L_48b023:
    eax = *reinterpret_cast<int *>(self - 0x5E4);
    local_c = 0;
    if (eax == 0) {
        goto L_48b06f;
    }
    ecx = *reinterpret_cast<int *>(self - 0x58);
    if (ecx <= 0) {
        goto L_48b06f;
    }
    esi = *reinterpret_cast<short *>(ebx - 2);
    eax = reinterpret_cast<int>(self) - 0x40;
L_48b042:
    if (esi != *reinterpret_cast<int *>(eax - 4)) {
        goto L_48b055;
    }
    edx = *reinterpret_cast<short *>(ebx);
    if (edx != *reinterpret_cast<int *>(eax)) {
        goto L_48b055;
    }
    local_c = 1;
L_48b055:
    if (esi != *reinterpret_cast<int *>(eax + 4)) {
        goto L_48b069;
    }
    edx = *reinterpret_cast<short *>(ebx);
    if (edx != *reinterpret_cast<int *>(eax + 8)) {
        goto L_48b069;
    }
    local_c = 1;
L_48b069:
    eax += 0x10;
    --ecx;
    if (ecx != 0) {
        goto L_48b042;
    }

L_48b06f:
    edx = *reinterpret_cast<short *>(ebx);
    eax = *reinterpret_cast<short *>(ebx - 2);
    esi = reinterpret_cast<int>(self) - 0x22050;
    reinterpret_cast<MapWin *>(esi)->tile_to_pixel(eax, edx, &local_8, &local_18);

    eax = *reinterpret_cast<int *>(self - 0x5E4);
    if (eax == 0) {
        goto L_48b0a5;
    }
    eax = local_c;
    if (eax == 0) {
        goto L_48b0a5;
    }
    eax = 0x200;
    goto L_48b0a7;
L_48b0a5:
    eax = 0;
L_48b0a7:
    ecx = reinterpret_cast<int>(self) - 0x22050;
    if (ecx == 0) {
        goto L_48b0bd;
    }
    ecx = *reinterpret_cast<int *>(esi);
    edx = *reinterpret_cast<int *>(ecx + 4);
    ecx = edx + reinterpret_cast<int>(self) - 0x21C0C;
L_48b0bd:
    edx = local_8;
    eax = (eax & ~0xff00) | (((eax >> 8) | 1) << 8);
    edx = edx - *reinterpret_cast<int *>(self - 0x4290);
    eax = *reinterpret_cast<int *>(self - 0x42B8);
    reinterpret_cast<Buffer *>(ecx)->line(
        eax, local_4, edx, local_18, 0, 0, 0, 0, eax);
    esi = *g_00939284;

L_48b0eb:
    eax = local_4;
    ecx = *g_009a64cc;
    ++eax;
    ebx += 0x134;
    local_4 = eax;
    if (eax < ecx) {
        goto L_48afb2;
    }
    goto L_48b10e;

L_48b108:
    esi = *g_00939284;
L_48b10e:
    eax = *reinterpret_cast<int *>(self - 0x58);
    local_c = 0;
    if (eax <= 0) {
        goto L_48b2bb;
    }
    local_1c = reinterpret_cast<int>(self) - 0x40;
    ecx = 0;
    local_20 = ecx;
    goto L_48b130;

L_48b12d:
    eax = local_1c;
L_48b130:
    if (ecx == 0) {
        goto L_48b13e;
    }
    ecx = *reinterpret_cast<int *>(eax - 4);
    edx = *reinterpret_cast<int *>(eax);
    local_14 = ecx;
    goto L_48b147;
L_48b13e:
    ecx = *reinterpret_cast<int *>(eax + 4);
    edx = *reinterpret_cast<int *>(eax + 8);
    local_14 = ecx;
L_48b147:
    eax = *g_009a64cc;
    local_10 = edx;
    local_4 = 0;
    if (eax <= 0) {
        goto L_48b20a;
    }
    eax = *reinterpret_cast<unsigned char *>(g_009a64c0);
    edx = 0x97D044;
    eax = eax & 0x80;
    local_24 = eax;
    goto L_48b175;

L_48b172:
    eax = local_24;
L_48b175:
    if (eax != 0) {
        goto L_48b1d9;
    }
    eax = esi;
    eax = (eax << 6) + esi;
    ecx = esi + eax * 2;
    eax = esi + ecx * 8;
    ecx = esi + eax * 2;
    eax = *reinterpret_cast<int *>(0x96C9E0 + ecx * 4);
    if (((eax >> 8) & 2) != 0) {
        goto L_48b1c3;
    }
    eax = *reinterpret_cast<short *>(edx - 2);
    eax = eax * (*g_0068faf0);
    ecx = *reinterpret_cast<short *>(edx - 4);
    ecx = ecx >> 1;
    eax = eax + ecx;
    ebx = 1;
    ecx = eax + eax * 4;
    eax = eax + ecx * 2;
    ecx = esi;
    ebx = ebx << (ecx & 0x1f);
    ecx = *g_0094a30c;
    if ((*reinterpret_cast<unsigned char *>(ecx + eax * 4 + 4) & ebx) != 0) {
        goto L_48b1d9;
    }
    goto L_48b1ef;

L_48b1c3:
    eax = 0;
    eax = (eax & ~0xff) | *reinterpret_cast<unsigned char *>(edx);
    if (eax == esi) {
        goto L_48b1d9;
    }
    eax = 1;
    ecx = esi;
    eax = eax << (ecx & 0x1f);
    if ((*reinterpret_cast<unsigned char *>(edx + 6) & eax) == 0) {
        goto L_48b1ef;
    }

L_48b1d9:
    ecx = *reinterpret_cast<short *>(edx - 4);
    if (ecx != local_14) {
        goto L_48b1ef;
    }
    eax = *reinterpret_cast<short *>(edx - 2);
    if (eax == local_10) {
        goto L_48b290;
    }

L_48b1ef:
    eax = local_4;
    ecx = *g_009a64cc;
    ++eax;
    edx += 0x134;
    local_4 = eax;
    if (eax < ecx) {
        goto L_48b172;
    }

L_48b20a:
    eax = local_10;
    ecx = reinterpret_cast<int>(self) - 0x8;
    local_c;
    {
        int outA, outB;
        eax = local_10;
        ecx = local_14;
        esi = reinterpret_cast<int>(self) - 0x22050;
        reinterpret_cast<MapWin *>(esi)->tile_to_pixel(ecx, eax, &outA, &outB);
        local_8 = outA;

        edx = *reinterpret_cast<int *>(self - 0x4290);
        ebx = outA;
        eax = *g_00939284;
        ebx = ebx - edx;
        local_8 = ebx;
        veh_fake(2, eax);
    }

    ecx = reinterpret_cast<int>(self) - 0x22050;
    if (ecx == 0) {
        goto L_48b25b;
    }
    ecx = *reinterpret_cast<int *>(esi);
    edx = *reinterpret_cast<int *>(ecx + 4);
    ecx = edx + reinterpret_cast<int>(self) - 0x21C0C;
L_48b25b:
    edx = *reinterpret_cast<int *>(self - 0x5E4);
    edx = -edx;
    ecx = ecx;
    edx = -edx;
    edx = edx & 0x200;
    edx = (edx & ~0xff00) | (((edx >> 8) | 1) << 8);
    eax = *reinterpret_cast<int *>(self - 0x42B8);
    veh_draw(reinterpret_cast<Buffer *>(ecx), edx, eax, local_18, local_8, 1, 1);
    esi = *g_00939284;

L_48b290:
    ecx = local_20;
    ++ecx;
    local_20 = ecx;
    if (ecx < 2) {
        goto L_48b12d;
    }
    ecx = local_c;
    eax = local_1c;
    edx = *reinterpret_cast<int *>(self - 0x58);
    ++ecx;
    eax += 0x10;
    local_c = ecx;
    local_1c = eax;
    if (ecx < edx) {
        goto L_48b126_actual;
    }
    goto L_48b2bb;

L_48b126_actual:
    goto L_48b12d;

L_48b2bb:
    eax = *reinterpret_cast<int *>(self - 0x58);
    local_c = 0;
    if (eax <= 0) {
        goto L_48b393;
    }
    eax = reinterpret_cast<int>(self) - 0x54;
    esi = reinterpret_cast<int>(self) - 0x3C;
    local_10 = eax;

L_48b2d6:
    ecx = local_10;
    eax = *reinterpret_cast<int *>(esi + 4);
    local_24 = eax;
    edx = *reinterpret_cast<int *>(ecx);
    ecx = *reinterpret_cast<int *>(esi);
    local_2c = edx;
    local_28 = ecx;
    ecx = *reinterpret_cast<int *>(esi - 4);
    edx = *reinterpret_cast<int *>(esi - 8);
    ebx = reinterpret_cast<int>(self) - 0x22050;
    reinterpret_cast<MapWin *>(ebx)->tile_to_pixel(edx, ecx, &local_1c, &local_14);

    edx = local_24;
    eax = local_28;
    reinterpret_cast<MapWin *>(ebx)->tile_to_pixel(eax, edx, &local_20, &local_4);

    eax = *reinterpret_cast<int *>(ebx + 0x1DDBC);
    ecx = local_14;
    edx = local_1c;
    ecx = ecx + eax;
    local_14 = ecx;
    ecx = *reinterpret_cast<int *>(ebx + 0x1DDC0);
    edx = edx + ecx;
    local_1c = edx;
    edx = local_20;
    edx = edx + eax;
    eax = local_4;
    eax = eax + ecx;
    ecx = *g_00939fd4;
    local_20 = edx;
    local_4 = eax;
    eax = local_14;
    edx = local_1c;
    ecx = *reinterpret_cast<int *>(ebx);
    edx = *reinterpret_cast<int *>(ecx + 4);
    ecx = edx + ebx + 0x444;
    reinterpret_cast<Buffer *>(ecx)->line(
        local_2c, 4, 4, *g_00939fd4, local_4, local_20, local_1c, local_14, 0);

    eax = local_c;
    ebx = local_10;
    ecx = *reinterpret_cast<int *>(self - 0x58);
    ++eax;
    ebx += 4;
    esi += 0x10;
    local_c = eax;
    local_10 = ebx;
    if (eax < ecx) {
        goto L_48b2d6;
    }

L_48b393:
    eax = *reinterpret_cast<int *>(self - 0x22050);
    ecx = *reinterpret_cast<int *>(eax + 4);
    edx = *reinterpret_cast<int *>(ecx + reinterpret_cast<int>(self) - 0x22050);
    ecx = ecx + reinterpret_cast<int>(self) - 0x22050;
    reinterpret_cast<VCall *>(ecx)->slot063();
}
