// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x006343E0
// name           sub_6343e0
// size           75 bytes
// measured tier  MISMATCH
// divergence     8
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/006343e0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_6343e0  at 0x006343E0  (75 bytes)
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

// NOT BYTE_EXACT. Closest reached: 73/75 bytes, mnemonic similarity 0.93,
// edit_count 3 (verify_recovered_function.py --json). Divergence is at
// mnemonic #8: original interleaves "mov edx,[stack]" (reading back the
// first sum's bit pattern) between the second sum's fadd/fstp and the
// third operand's fld, i.e. it schedules the int-register round-trip
// eagerly; every source-form tried here (direct field stores, a temp
// struct + aggregate assignment, an out-of-line operator+, an extra
// volatile local to reproduce the leading `mov [esp],0`) gets the SAME
// instructions in a different order, never that interleaving - this
// reads as the INSTRUCTION SCHEDULING wall (see the source-form rules),
// not a wrong body. This is the closest of ~8 tried variants and is kept
// as the base for the next pass.
//
// Ruled out:
//   - direct `dest->x = src->x + self->x;` per field: no temp at all,
//     35 bytes (missing the whole stack-temp / int-register round trip).
//   - `*dest = *src + *self;` via an out-of-line `operator+`: not
//     inlined at this optimization level, becomes a real call (52 bytes).
//   - `Vec3 tmp; ...; *dest = tmp;`: right shape (65/75), but missing the
//     leading `mov dword ptr [esp], 0` dead store entirely.
//   - `Vec3 tmp = {0};` for that dead store: matches byte count (75) but
//     compiles as `xor eax,eax; mov [esp],eax` instead of the original's
//     single immediate `mov [esp], 0`.
//   - a separate `volatile int pad = 0;` local: reproduces the immediate
//     `mov [esp], 0` exactly (this file) - which is what gets the
//     leading bytes to line up - but does not change the later
//     interleaving.
struct Vec3 { float x, y, z; };

class VecAdder { public:
    void add(Vec3 *dest, Vec3 *src);
};

void VecAdder::add(Vec3 *dest, Vec3 *src) {
    Vec3 *self = reinterpret_cast<Vec3 *>(this);
    volatile int pad = 0;
    Vec3 tmp;
    tmp.x = src->x + self->x;
    tmp.y = src->y + self->y;
    tmp.z = src->z + self->z;
    *dest = tmp;
}

