// ORIGINAL: 0x0060E320 FILE
// name      ?on_mouse_move@RadioButton@@QAEXHH@Z
// size      372 bytes
// spans     0x0060E320-0x0060E494
// prototype void (__thiscall ?on_mouse_move@RadioButton@@QAEXHH@Z)(RadioButton* this, int, int)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DCAB0 0x00644F3A
// indirect  0x0060E488
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0060E320
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0060e320/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_mouse_move@RadioButton@@QAEXHH@Z  at 0x0060E320  (372 bytes)
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
class Font;
typedef void * HBITMAP;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
typedef char * LPSTR;
typedef void * LPVOID;
struct RECT;
class RadioButton;
class Spot;

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
    int text_line_height();
};
extern "C" int abs(int);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 62
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
    virtual void slot062();  // <-- used
};

class RadioButton { public:
    void on_mouse_move(int, int);
};

void RadioButton::on_mouse_move(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    int32_t eax_, ecx_, edx_, edi_, ebx_;

    ebx_ = 1;
    eax_ = *reinterpret_cast<int32_t *>(self - 0x18);
    ecx_ = *reinterpret_cast<int32_t *>(eax_ + 8);
    edx_ = *reinterpret_cast<int32_t *>(self + ecx_ + 0xb4);
    edi_ = reinterpret_cast<int32_t>(self) + ecx_ + 0xa4;
    edx_ -= 1;
    if (edx_ < -1) {
        goto L37c;
    }

    eax_ = *reinterpret_cast<int32_t *>(edi_ + 8);
    *reinterpret_cast<int32_t *>(edi_ + 0xc) = eax_;
    eax_ = abs(-1);
    ecx_ = *reinterpret_cast<int32_t *>(edi_ + 0x10);
    if (eax_ > ecx_) {
        goto L37c;
    }

    eax_ = abs(-1);
    if (eax_ <= 0) {
        goto L375;
    }

L369:
    ecx_ = *reinterpret_cast<int32_t *>(edi_ + 0xc);
    eax_ -= 1;
    edx_ = *reinterpret_cast<int32_t *>(ecx_ + 0x10);
    *reinterpret_cast<int32_t *>(edi_ + 0xc) = edx_;
    if (eax_ != 0) {
        goto L369;
    }

L375:
    eax_ = *reinterpret_cast<int32_t *>(edi_ + 0x10);
    eax_ -= 1;
    *reinterpret_cast<int32_t *>(edi_ + 0x14) = eax_;

L37c:
    eax_ = a2;
    if (eax_ <= 0) {
        goto L437;
    }

L388:
    if (ebx_ != 0) {
        goto L3a9;
    }

    eax_ = *reinterpret_cast<int32_t *>(self - 0x18);
    ecx_ = *reinterpret_cast<int32_t *>(eax_ + 8);
    edx_ = *reinterpret_cast<int32_t *>(self + ecx_ + 0xb4);
    eax_ = reinterpret_cast<int32_t>(self) + ecx_;
    edx_ -= 1;
    if (*reinterpret_cast<int32_t *>(eax_ + 0xb8) == edx_) {
        goto L48e;
    }

L3a9:
    eax_ = *reinterpret_cast<int32_t *>(self - 0x18);
    ebx_ = 0;
    ecx_ = *reinterpret_cast<int32_t *>(eax_ + 8);
    eax_ = reinterpret_cast<int32_t>(self) + ecx_ + 0xa4;
    ecx_ = *reinterpret_cast<int32_t *>(self + ecx_ + 0xac);
    if (ecx_ == 0) {
        goto L3dd;
    }

    edx_ = *reinterpret_cast<int32_t *>(eax_ + 0xc);
    edi_ = *reinterpret_cast<int32_t *>(eax_ + 0x10);
    edx_ = *reinterpret_cast<int32_t *>(edx_ + 0xc);
    *reinterpret_cast<int32_t *>(eax_ + 0xc) = edx_;
    edx_ = *reinterpret_cast<int32_t *>(eax_ + 0x14);
    edx_ += 1;
    *reinterpret_cast<int32_t *>(eax_ + 0x14) = edx_;
    if (edx_ != edi_) {
        goto L3dd;
    }
    *reinterpret_cast<int32_t *>(eax_ + 0x14) = ebx_;

L3dd:
    eax_ = *reinterpret_cast<int32_t *>(eax_ + 0xc);
    if (eax_ == 0) {
        goto L3f7;
    }
    if (ecx_ == 0) {
        goto L3f0;
    }

    eax_ = *reinterpret_cast<int32_t *>(eax_ + 8);
    edi_ = *reinterpret_cast<int32_t *>(eax_ + 8);
    goto L3f9;

L3f0:
    eax_ = 0;
    edi_ = *reinterpret_cast<int32_t *>(eax_ + 8);
    goto L3f9;

L3f7:
    edi_ = 0;

L3f9:
    eax_ = *reinterpret_cast<int32_t *>(self - 0x18);
    ecx_ = *reinterpret_cast<int32_t *>(eax_ + 4);
    ecx_ = ecx_ + reinterpret_cast<int32_t>(self) + 0x42c;
    eax_ = reinterpret_cast<Buffer *>(ecx_)->text_line_height();
    eax_ *= edi_;
    edx_ = *reinterpret_cast<int32_t *>(self - 0x18);
    ecx_ = *reinterpret_cast<int32_t *>(edx_ + 8);
    edx_ = *reinterpret_cast<int32_t *>(self + ecx_ + 0x50);
    ecx_ = ecx_ + reinterpret_cast<int32_t>(self);
    ecx_ = *reinterpret_cast<int32_t *>(ecx_ + 0x2c);
    ecx_ = ecx_ + edx_;
    if (ecx_ > eax_) {
        eax_ = ecx_;
    }

    ecx_ = a2;
    ecx_ = ecx_ - eax_;
    a2 = ecx_;
    if (ecx_ > 0) {
        goto L388;
    }

L437:
    edx_ = *reinterpret_cast<int32_t *>(self - 0x18);
    eax_ = *reinterpret_cast<int32_t *>(edx_ + 8);
    edi_ = *reinterpret_cast<int32_t *>(self + eax_ + 0x4c);
    ebx_ = *reinterpret_cast<int32_t *>(self + eax_ + 0x2c);
    ecx_ = eax_ + reinterpret_cast<int32_t>(self);
    eax_ = a1;
    edi_ += ebx_;
    eax_ /= edi_;
    edi_ = *reinterpret_cast<int32_t *>(ecx_ + 0xb8);
    edx_ = *reinterpret_cast<int32_t *>(ecx_ + 0xd4);
    eax_ *= *reinterpret_cast<int32_t *>(ecx_ + 0x40);
    eax_ += edi_;
    if (eax_ != edx_) {
        goto L474;
    }

    edx_ = *reinterpret_cast<int32_t *>(ecx_ + 0xd8);
    eax_ = -1;
    if (edx_ == eax_) {
        goto L48e;
    }

L474:
    *reinterpret_cast<int32_t *>(ecx_ + 0xd8) = eax_;
    ecx_ = *reinterpret_cast<int32_t *>(self - 0x18);
    edx_ = *reinterpret_cast<int32_t *>(ecx_ + 4);
    ecx_ = edx_ + reinterpret_cast<int32_t>(self) - 0x18;
    reinterpret_cast<VCall *>(ecx_)->slot062();

L48e:
    return;
}
