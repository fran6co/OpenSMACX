// ORIGINAL: 0x006345E0 sub_6345e0 0x006345E0-0x00634641 FILE
// TRIED: signature is `__thiscall`, not the placeholder's `__stdcall` free function - `[ecx+N]` reads with no matching stack access, reinterpreted as `Vec3 *cross(Vec3 *out, Vec3 *b)` (3-float vectors, this=A, stack args=out,B). Named-temp form (rx/ry/rz locals before the stores) forces no closer a match than direct member-store form; both compile but diverge at #0 (sub vs mov/push) - original spills all three FP products to stack temps between fld/fmul/fsubp triplets, this form keeps more in registers under /O1
// working copy - scaffold materialised by --work
// size      97 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_6345e0  at 0x006345E0  (97 bytes)
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

class Vec3 { public:
    float x_;
    float y_;
    float z_;

    Vec3 * cross(Vec3 *out, Vec3 *b);
};

Vec3 * Vec3::cross(Vec3 *out, Vec3 *b) {
    out->x_ = y_ * b->z_ - z_ * b->y_;
    out->y_ = z_ * b->x_ - x_ * b->z_;
    out->z_ = x_ * b->y_ - y_ * b->x_;
    return out;
}
