// ORIGINAL: 0x005C1AB0 FILE
// working copy - scaffold materialised by --work
// name      ?veh_unmoved@@YAHH@Z
// size      191 bytes
// spans     0x005C1AB0-0x005C1B6F
// prototype int (__cdecl ?veh_unmoved@@YAHH@Z)(int vehID)
// callers   1   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005C1540 0x005C1C40

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?veh_unmoved@@YAHH@Z  at 0x005C1AB0  (191 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
int speed(int, int);
int veh_jail(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;
struct VehRecord {
    short field0_;                        // 0x00 - vs g_00949870
    short field2_;                        // 0x02 - vs g_00949874
    int field4_;                          // 0x04 - flag 0x4000000
    unsigned short field8_;               // 0x08 - flag word 0x40/0x400
    unsigned char pad_0A_[0x0E - 0x0A];
    unsigned char field0E_;               // 0x0E - vs g_00939284
    unsigned char pad_0F_[0x11 - 0x0F];
    signed char field11_;                 // 0x11 - stage byte
    unsigned char pad_12_[0x28 - 0x12];
    unsigned char field28_;               // 0x28 - speed threshold
    unsigned char pad_29_[0x34 - 0x29];
};

extern VehRecord g_veh_table[];

int __cdecl veh_unmoved(int a1) {
    if (a1 < 0) {
        return 0;
    }
    VehRecord *rec = &g_veh_table[a1];
    if (rec->field2_ < 0) {
        return 0;
    }
    if (rec->field2_ >= *g_00949874) {
        return 0;
    }
    if (rec->field0_ < 0) {
        return 0;
    }
    if (rec->field0_ >= *g_00949870) {
        return 0;
    }
    if ((unsigned char)rec->field0E_ != *g_00939284) {
        return 0;
    }
    if ((rec->field8_ & 0x40) != 0 && (rec->field8_ & 0x400) != 0) {
        return 0;
    }
    if (veh_jail(a1) != 0) {
        return 0;
    }
    if ((rec->field4_ & 0x4000000) == 0 && rec->field11_ >= 4 && rec->field11_ < 0x18) {
        return 1;
    }
    int spd = speed(a1, 0);
    int diff = spd - rec->field28_;
    if (diff < 0) {
        return 0;
    }
    if (diff > 0x3e7) {
        return 1;
    }
    if (diff != 0) {
        return 1;
    }
    return 0;
}
