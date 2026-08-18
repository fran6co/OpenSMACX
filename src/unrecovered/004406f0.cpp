// ORIGINAL: 0x004406F0 ?release_modal@DiploPop@@QAEXXZ 0x004406F0-0x004406F5 FILE BYTE_EXACT
// working copy - scaffold materialised by --work
// size      5 bytes
// prototype void (__thiscall ?release_modal@DiploPop@@QAEXXZ)(DiploPop* this)
// callers   0   call targets   0
// kind      thunk
// flags     hidden;thunk;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?release_modal@DiploPop@@QAEXXZ  at 0x004406F0  (5 bytes)
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

class DiploPop;

class DiploPop { public:
    uint8_t pad_0_[0x1074];
    uint32_t field_1074_;

    void release_modal();
};

// Tail call to Win::release_modal() on the same `this`; a fresh minimal
// class (rather than editing the scaffold's opaque Win, which has no
// methods here) is enough to get the thiscall forwarding call compiled.
class Win4406F0 { public:
    void release_modal();
};

void DiploPop::release_modal() {
    reinterpret_cast<Win4406F0 *>(this)->release_modal();
}
