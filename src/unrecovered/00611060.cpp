// ORIGINAL: 0x00611060 FILE
// RULED-OUT: `this` is offset by TWO different constants for two different
//            base sub-objects (this-0x7c for Spot::check, this-0x8c for the
//            SpriteBox proper, whose field_0_ IS *(this-0x8c)); modelled
//            with raw char*-adjusted casts rather than real inheritance.
//            `a2` is an in/out param aliased via `&a2` into check(). Gets to
//            instruction 25 of ~90 before diverging on push/mov ordering
//            around the `push ebp; push esi` callee-save point.
// working copy - scaffold materialised by --work
// name      ?on_left_click@SpriteBox@@QAEXHH@Z
// size      234 bytes
// spans     0x00611060-0x0061114A
// prototype void (__thiscall ?on_left_click@SpriteBox@@QAEXHH@Z)(SpriteBox* this, int xCoord, int yCoord)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FAB00 0x00610C50
// indirect  0x00611098 0x0061111E 0x0061113E

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_left_click@SpriteBox@@QAEXHH@Z  at 0x00611060  (234 bytes)
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

struct RECT;
class Spot;
class SpriteBox;

// ---- callees, declared and never defined (a definition would be inlined) ----
struct RECT {
    long left;
    long top;
    long right;
    long bottom;
};

class Spot { public:
    struct SpotInternal {
        RECT rect;
        int type;
        int position;
    };
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
    int check(int, int, int *, int *);
};


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
static int *const g_00669314 = (int *)0x00669314;
static int *const g_009b7b28 = (int *)0x009B7B28;

class SpriteBox { public:
    uint32_t field_0_;

    void draw_item(int);
    void on_left_click(int, int);
};
extern "C" int __stdcall SendMessageA(int, int, int, int);

void SpriteBox::on_left_click(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    int local1 = 0;
    int result = reinterpret_cast<Spot *>(self - 0x7c)->check(a1, a2, &a2, 0);
    if (result == -1) {
        return;
    }
    int *fnSlot = *reinterpret_cast<int **>(self - 0x24);
    if (fnSlot != 0) {
        typedef int (__cdecl *FnT)(int);
        FnT fn = reinterpret_cast<FnT>(fnSlot);
        local1 = fn(a2);
    }
    char *sb = self - 0x8c;
    int fld0 = *reinterpret_cast<int *>(sb + 0);
    int base = *reinterpret_cast<int *>(fld0 + 8);
    int ebpVal = *reinterpret_cast<int *>(self + base + 0x60);
    int head = *reinterpret_cast<int *>(sb + 0x2c);
    if (head != 0) {
        int count = *reinterpret_cast<int *>(sb + 0x34);
        int counter = 0;
        *reinterpret_cast<int *>(sb + 0x38) = 0;
        *reinterpret_cast<int *>(sb + 0x30) = head;
        if (count > 0) {
            do {
                int cur = *reinterpret_cast<int *>(sb + 0x30);
                if (*reinterpret_cast<int *>(cur + 4) == a2) {
                    break;
                }
                *reinterpret_cast<int *>(sb + 0x38) += 1;
                cur = *reinterpret_cast<int *>(cur + 0xc);
                counter++;
                *reinterpret_cast<int *>(sb + 0x30) = cur;
            } while (counter < count);
        }
    }
    int fld0b = *reinterpret_cast<int *>(sb + 0);
    int idx2 = *reinterpret_cast<int *>(sb + 0x38);
    int baseB = *reinterpret_cast<int *>(fld0b + 8);
    *reinterpret_cast<int *>(self + baseB + 0x60) = idx2;
    reinterpret_cast<SpriteBox *>(sb)->draw_item(ebpVal);

    int fld0c = *reinterpret_cast<int *>(sb + 0);
    int baseC = *reinterpret_cast<int *>(fld0c + 8);
    int val2 = *reinterpret_cast<int *>(self + baseC + 0x60);
    reinterpret_cast<SpriteBox *>(sb)->draw_item(val2);

    int fld0d = *reinterpret_cast<int *>(sb + 0);
    int addr = *reinterpret_cast<int *>(fld0d + 4);
    reinterpret_cast<VCall *>(addr + sb)->slot063();

    if (local1 != 0) {
        SendMessageA(*g_009b7b28, 0x100, 0xd, 0);
    }

}
