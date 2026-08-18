// ORIGINAL: 0x004C8910 sub_4c8910 0x004C8910-0x004C891F FILE
// RULED-OUT: the tail-called vtable slot37 matches exactly (mov/test/je/mov/jmp, 5/5 mnemonics), so the divergence is entirely the null branch: the original's `return -1` JUMPS OUT of this function into a shared tail block at 0x4C891F, four bytes past its own 15, and no C++ spelling reaches another function's interior. This body inlines `or eax,-1; ret` instead and is 4 bytes long. Not a lever - it names why the remainder cannot close from here.
// working copy - scaffold materialised by --work
// size      15 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_4c8910  at 0x004C8910  (15 bytes)
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

// Vtable shim for the field-0x3C object's virtual call at slot 37
// (0x94 / 4). Mirrors the pattern already landed for sibling releasers
// (e.g. VoiceTx::start / 004c8eb0.cpp): field_3c_ IS the vtable-having
// object, not a pointer to one.
class VCall4C8910 { public:
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
    virtual int slot037();  // <-- used
};

// `ecx+0x3c` with no stack access -> __thiscall member, not a free
// __cdecl function; the receiver is opaque here (no catalogued class).
class Obj4C8910 { public:
    void * pad_0_[0xF];
    void * field_3c_;

    int release();
};

int Obj4C8910::release() {
    if (field_3c_) {
        return reinterpret_cast<VCall4C8910 *>(field_3c_)->slot037();
    }
    return -1;
}
