// ORIGINAL: 0x00407910 BYTE_EXACT FILE
// LEVER: thiscall shim - a synthetic GraphicWin class exposes close() as a
//        member so reinterpret_cast<GraphicWin*>(this)->close() folds into
//        the original's bare 5-byte jmp (same shim shape as 0x004210F0)
// working copy - scaffold materialised by --work
// name      ?close@AlphaSave@@QAEXXZ
// size      5 bytes
// spans     0x00407910-0x00407915
// prototype void (__thiscall ?close@AlphaSave@@QAEXXZ)(AlphaSave* this)
// callers   3   call targets   0
// kind      thunk
// flags     hidden;thunk;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?close@AlphaSave@@QAEXXZ  at 0x00407910  (5 bytes)
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

class AlphaSave;

class GraphicWin { public:
    void close();
};

class AlphaSave { public:
    void close();
};
void AlphaSave::close() {
    reinterpret_cast<GraphicWin *>(this)->close();
}
