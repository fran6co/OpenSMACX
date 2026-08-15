// ORIGINAL: 0x005C1F20 FILE
// name      ?UNK1@Fractal@@QAEHHH@Z
// size      153 bytes
// spans     0x005C1F20-0x005C1FB9
// prototype int (__thiscall ?UNK1@Fractal@@QAEHHH@Z)(Fractal* this, int, int)
// callers   3   call targets   0
// kind      
// flags     
// calls     (none)
// working copy - scaffold materialised by --work

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?UNK1@Fractal@@QAEHHH@Z  at 0x005C1F20  (153 bytes)
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

class Fractal;

class Fractal { public:
    int UNK1(int, int);
};
int Fractal::UNK1(int a1, int a2) {
    // Bilinear sample of the terrain byte grid the object opens with (stride
    // 19 bytes/row). a1 selects the row (coarse index + 5-bit fraction), a2
    // selects the column the same way; the 4 taps around (row,col) are
    // blended by fraction and the accumulated product is renormalised >> 5.
    char *self = reinterpret_cast<char *>(this);

    int rowIdx  = ((a1 - 0x80) >> 8) & 0xf;
    int colIdx  = ((a2 - 0x80) >> 8) & 0xf;
    int colFrac = ((a2 - 0x80) >> 3) & 0x1f;
    int topOffset = colIdx + rowIdx * 19;
    char *topRow = self + topOffset;
    int rowFrac = ((a1 - 0x80) >> 3) & 0x1f;

    int col0 = self[colIdx + (rowIdx + 1) * 19] * rowFrac + topRow[0] * (32 - rowFrac);
    int col1 = topRow[1] * (32 - rowFrac) + topRow[20] * rowFrac;

    return (col0 * (32 - colFrac) + col1 * colFrac) >> 5;
}
