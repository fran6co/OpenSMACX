// ORIGINAL: 0x005E15F0 FILE
// name      ?line@Buffer@@QAEXHHHHH@Z
// size      1167 bytes
// spans     0x005E15F0-0x005E1A7F
// prototype void (__thiscall ?line@Buffer@@QAEXHHHHH@Z)(Buffer* this, int, int, int, int, int)
// callers   5   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E1A80 0x005E1BF0 0x00644F3A
// indirect  0x005E16AB 0x005E1A68
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005E15F0
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005e15f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?line@Buffer@@QAEXHHHHH@Z  at 0x005E15F0  (1167 bytes)
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
extern "C" int abs(int);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 25, 32
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
    virtual void slot025();  // <-- used
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();  // <-- used
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

    void hline(int, int, int, int);
    void vline(int, int, int, int);
    void line(int, int, int, int, int);
};

// The buffer's `field_58_` holds a DirectDraw surface interface pointer.
// `Lock` on that COM vtable is __stdcall with an EXPLICIT `this` (the COM
// ABI, not the compiler's own thiscall), so it is called through a plain
// function pointer read out of the vtable rather than the VCall shim (which
// only ever produces a thiscall dispatch).
typedef int32_t(__stdcall *DDLockFn)(void *, void *, void *, int32_t, void *);

void Buffer::line(int x1, int y1, int x2, int y2, int color) {
    char *self = reinterpret_cast<char *>(this);

    if (*reinterpret_cast<int32_t *>(self + 0x54) == 0 &&
        *reinterpret_cast<int32_t *>(self + 0x58) == 0)
        return;

    if (x1 == x2) {
        vline(x1, y1, y2, color);
        return;
    }
    if (y1 == y2) {
        hline(x1, x2, y1, color);
        return;
    }

    if (x2 < x1) {
        int t;
        t = x1;
        x1 = x2;
        x2 = t;
        t = y1;
        y1 = y2;
        y2 = t;
    }

    void *surface = *reinterpret_cast<void **>(self + 0x58);
    int32_t bufferBase;
    if (surface != 0) {
        bufferBase = *reinterpret_cast<int32_t *>(self + 0x50);
        if (bufferBase == 0) {
            int32_t desc[27];
            desc[0] = 0x6c;
            void *vtbl = *reinterpret_cast<void **>(surface);
            DDLockFn lock = *reinterpret_cast<DDLockFn *>(
                reinterpret_cast<char *>(vtbl) + 0x64);
            int32_t hr = lock(surface, 0, desc, 1, 0);
            if (hr != 0)
                return;
            *reinterpret_cast<int32_t *>(self + 0x6c) += 1;
            bufferBase = desc[5];  // [ebp-0x5c]: the returned lpSurface
            *reinterpret_cast<int32_t *>(self + 0x4a8) = desc[19];  // [ebp-0x70]: lPitch
            *reinterpret_cast<int32_t *>(self + 0x50) = bufferBase;
        } else {
            *reinterpret_cast<int32_t *>(self + 0x6c) += 1;
        }
    } else {
        bufferBase = *reinterpret_cast<int32_t *>(self + 0x54);
        *reinterpret_cast<int32_t *>(self + 0x50) = bufferBase;
        if (bufferBase == 0)
            return;
        *reinterpret_cast<int32_t *>(self + 0x6c) += 1;
    }
    if (bufferBase == 0)
        return;

    int32_t *clip = reinterpret_cast<int32_t *>(self + 0x20);
    int32_t clipLeft = clip[0];
    int32_t clipTop = clip[1];
    int32_t clipRight = clip[2];
    int32_t clipBottom = clip[3];

    int32_t stride = *reinterpret_cast<int32_t *>(self + 0x4a8);
    uint8_t *base = reinterpret_cast<uint8_t *>(*reinterpret_cast<int32_t *>(self + 0x50));

    // General Bresenham line with clipping to [clipLeft,clipRight] x
    // [clipTop,clipBottom]. The original spells four near-duplicate
    // octant-specific loops sharing heavily reused registers; this is the
    // faithful ALGORITHM the disassembly implements, not a transcription of
    // those four loops instruction-by-instruction.
    int32_t dx = x2 - x1;
    int32_t dy = y2 - y1;
    if (dx >= abs(dy)) {
        if (x1 > clipRight || x2 < clipLeft || (y1 < clipTop && y2 < clipTop) ||
            (y1 > clipBottom && y2 > clipBottom))
            return;
        int32_t step = dy >= 0 ? 1 : -1;
        int32_t err = 2 * (dy >= 0 ? dy : -dy) - dx;
        int32_t y = y1;
        for (int32_t x = x1; x <= x2; x++) {
            if (x >= clipLeft && x <= clipRight && y >= clipTop && y <= clipBottom)
                base[stride * y + x] = static_cast<uint8_t>(color);
            if (err > 0) {
                y += step;
                err += 2 * (-dx);
            }
            err += 2 * (dy >= 0 ? dy : -dy);
        }
    } else {
        if (y1 > clipBottom || y2 < clipTop || (x1 < clipLeft && x2 < clipLeft) ||
            (x1 > clipRight && x2 > clipRight))
            return;
        int32_t step = dx >= 0 ? 1 : -1;
        int32_t absDy = dy >= 0 ? dy : -dy;
        int32_t err = 2 * (dx >= 0 ? dx : -dx) - absDy;
        int32_t x = x1;
        int32_t yStart = y1 < y2 ? y1 : y2;
        int32_t yEnd = y1 < y2 ? y2 : y1;
        for (int32_t y = yStart; y <= yEnd; y++) {
            if (x >= clipLeft && x <= clipRight && y >= clipTop && y <= clipBottom)
                base[stride * y + x] = static_cast<uint8_t>(color);
            if (err > 0) {
                x += step;
                err += 2 * (-absDy);
            }
            err += 2 * (dx >= 0 ? dx : -dx);
        }
    }

    if (surface != 0) {
        int32_t count = *reinterpret_cast<int32_t *>(self + 0x6c) - 1;
        *reinterpret_cast<int32_t *>(self + 0x6c) = count;
        if (count <= 0) {
            *reinterpret_cast<int32_t *>(self + 0x50) = 0;
            *reinterpret_cast<int32_t *>(self + 0x6c) = 0;
        }
    }
}
