// ORIGINAL: 0x00446A00 FILE
// name      ?play@FX@@QAEXH@Z
// size      142 bytes
// spans     0x00446A00-0x00446A8E
// prototype void (__thiscall ?play@FX@@QAEXH@Z)(FX* this, int)
// callers   24   call targets   0
// kind      
// flags     
// calls     (none)
// indirect  0x00446A2E 0x00446A3C 0x00446A4E 0x00446A5C 0x00446A7B 0x00446A85
// working copy - scaffold materialised by --work

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?play@FX@@QAEXH@Z  at 0x00446A00  (142 bytes)
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

class FX;

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 7, 23, 35, 38, 82
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();  // <-- used
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
    virtual void slot023();  // <-- used
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
    virtual void slot035();  // <-- used
    virtual void slot036();
    virtual void slot037();
    virtual void slot038();  // <-- used
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
    virtual void slot082();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0074daa0 = (int *)0x0074DAA0;
static int *const g_009a6490 = (int *)0x009A6490;

class FX { public:
    uint8_t effects_[0x61 * 0x6C];
    uint32_t field_28EC_;

    void play(int);
};

// Second vtable shim, distinct from VCall above: slot023 here needs an int
// RETURN (tested with `test eax,eax` after the call) and slot038 needs an
// int PARAMETER (a value is pushed before both of its call sites). VCall's
// slots are fixed nullary/void by the generated scaffolding and cannot be
// edited in place without colliding with it (C2011), so this is a second,
// differently-shaped shim over the same vtable, matching the precedent in
// src/recovered/units/004d9970.cpp (VCallShim alongside VCall).
class FxShim { public:
    virtual void slot00();
    virtual void slot01();
    virtual void slot02();
    virtual void slot03();
    virtual void slot04();
    virtual void slot05();
    virtual void slot06();
    virtual void slot07();
    virtual void slot08();
    virtual void slot09();
    virtual void slot10();
    virtual void slot11();
    virtual void slot12();
    virtual void slot13();
    virtual void slot14();
    virtual void slot15();
    virtual void slot16();
    virtual void slot17();
    virtual void slot18();
    virtual void slot19();
    virtual void slot20();
    virtual void slot21();
    virtual void slot22();
    virtual int slot23();
    virtual void slot24();
    virtual void slot25();
    virtual void slot26();
    virtual void slot27();
    virtual void slot28();
    virtual void slot29();
    virtual void slot30();
    virtual void slot31();
    virtual void slot32();
    virtual void slot33();
    virtual void slot34();
    virtual void slot35();
    virtual void slot36();
    virtual void slot37();
    virtual void slot38(int);
};

void FX::play(int a1) {
    // RULED-OUT: 141/142 bytes match (edit distance 1). Only the bit-4 test
    // near the end differs: original is `shr ecx,4 / test cl,1`, every
    // rephrasing tried here (direct mask `flags & 0x10`, a named shifted
    // temp, byte-truncating the shift result, `% 2`, `/ 16`, unsigned flags,
    // inlining the load into the condition) still lowers to a single fused
    // `test` against the shifted-away mask. Whatever source form produced
    // the two-instruction shr+test was not found in ~11 tries.
    if ((*g_009a6490 & 0x400) != 0) {
        if (a1 == 1 || a1 == 0 || a1 == 2) {
            int obj = *g_0074daa0;
            if (obj != 0) {
                reinterpret_cast<VCall *>(obj)->slot082();
            }
            if (a1 == 0) {
                if (reinterpret_cast<FxShim *>(this)->slot23() != 0) {
                    int param = *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x58) + 0x64;
                    reinterpret_cast<FxShim *>(this)->slot38(param);
                } else {
                    reinterpret_cast<FxShim *>(this)->slot38(0);
                }
            }
        }
        int index = a1 * 27;
        int flags = *reinterpret_cast<int *>(effects_ + index * 4 + 0x54);
        unsigned char *entry = effects_ + index * 4;
        if ((flags >> 4) & 1) {
            reinterpret_cast<VCall *>(entry)->slot035();
        }
        reinterpret_cast<VCall *>(entry)->slot007();
    }
}
