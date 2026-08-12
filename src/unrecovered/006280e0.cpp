// ORIGINAL: 0x006280E0 FILE
// name      sub_6280e0
// size      107 bytes
// spans     0x006280E0-0x0062814B
// prototype 
// callers   14   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// RULED-OUT: term order (all 6 permutations); operand order (mat*vec vs vec*mat, per-term and uniform); association ((A+B)+C, A+(B+C), fully parenthesised); product temps declared in every order; partial-sum temp; += accumulation; vec components bound to locals (adds a frame); const-qualified params; *p vs p[0]. All twenty-four spellings compile to BYTE-IDENTICAL code - VC6 canonicalises the commutative tree - and the flags do not move it either (/O1 /O2 /Ox /Od /Og /Oi /Op /Oa /Ow /Os /Ot /Ob0 x /G3 /G4 /G5 /G6 /GB all identical here). The rebuilt body has every load, multiply, add and store right and is short by exactly the twelve `fxch` (24 bytes, 83 vs 107) of an interleaved schedule VC6 12.00.8168 only emits for a value that is genuinely live across another computation; this function has no such value. Same wall, same idiom, same module as 0x00628290.
// working copy - scaffold materialised by --work

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_6280e0  at 0x006280E0  (107 bytes)
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
// A 3x3 matrix times a 3-vector, row by row: `mat` is nine floats at
// [esp+4], `vec` three at [esp+8], `out` three at [esp+0xc], every access
// `dword ptr` (single precision, never promoted), plain `ret` - so cdecl,
// three pointers, no return value, not the placeholder `int()`.
//
// The parentheses are the ORIGINAL's addition tree, read off the two
// `faddp` in each row rather than assumed: the vec[0] column term is
// added LAST in all three rows, and row 2 pairs mat[8]*vec[2] with
// mat[7]*vec[1] the other way round from rows 0 and 1. Floating-point
// addition is not associative, so that grouping is part of the function.
// VC6 reassociates it away again here - see the RULED-OUT line - but the
// source should still say what the shipped code computes.
extern "C" void __cdecl sub_6280e0(float *mat, float *vec, float *out) {
    out[0] = (mat[1] * vec[1] + mat[2] * vec[2]) + vec[0] * mat[0];
    out[1] = (mat[4] * vec[1] + mat[5] * vec[2]) + mat[3] * vec[0];
    out[2] = (mat[8] * vec[2] + mat[7] * vec[1]) + mat[6] * vec[0];
}
