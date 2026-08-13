// ORIGINAL: 0x00616860 FILE
// RULED-OUT: (int)this; pointer-cast this; address-of-first-member
//            (&unk_tgl_, == this); named local; unsigned casts; omitting
//            the convention keyword. All 9 compile identically - see the
//            note above the definition below.
// working copy - scaffold materialised by --work
// name      ?set_modal@Time@@QAAXXZ
// size      7 bytes
// spans     0x00616860-0x00616867
// prototype 
// callers   2   call targets   0
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned method

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?set_modal@Time@@QAAXXZ  at 0x00616860  (7 bytes)
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

class Time;
typedef unsigned int UINT_PTR;

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b90d0 = (int *)0x009B90D0;

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

    void __cdecl set_modal();
};
// RULED-OUT: 9 body spellings, all MISMATCH the same way (extra `mov`
// where the original has `ret`) - (int)this, pointer-cast this,
// address-of-first-member (&unk_tgl_, algebraically == this), a named
// local, unsigned casts, and omitting the convention keyword entirely.
// Every one compiles to `mov eax,[esp+4] / mov [addr],eax / ret` (10
// bytes) because VC6 genuinely loads `this` from the stack for a member
// function whose IN-CLASS declaration says __cdecl - confirmed by direct
// disassembly of the compiled object, not inferred. The original's `mov
// [addr],ecx / ret` (7 bytes) needs `this` already resident in ecx at
// entry, which VC6 only does for __thiscall/__fastcall - and VC6 refuses
// explicit `__thiscall` in source (C4234), so the only way to get there
// is to let the definition inherit thiscall from a class that declares
// it that way. The scaffold's class here is generated from the
// catalogue's convention (__cdecl, docs/recovery/derived-prototypes.csv
// row for this address) and is prepended unconditionally by
// writeback.build_unit, so no body-only candidate can change it. See
// PROPOSAL below.
void __cdecl Time::set_modal() {
    *(Time **)g_009b90d0 = this;
}
