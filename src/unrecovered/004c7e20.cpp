// ORIGINAL: 0x004C7E20 BYTE_EXACT FILE
// LEVER: own vtable shim (not the scaffolded VCall) so slots 5/23 return
//        int; offset 0x40 field reached by pointer arithmetic, masked &=
// working copy - scaffold materialised by --work
// name      ?unload@Midi@@QAEHXZ
// size      89 bytes
// spans     0x004C7E20-0x004C7E79
// prototype int (__thiscall ?unload@Midi@@QAEHXZ)(Midi* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7E2F 0x004C7E3B 0x004C7E4B 0x004C7E56 0x004C7E66

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?unload@Midi@@QAEHXZ  at 0x004C7E20  (89 bytes)
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

class Midi;

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 5, 23, 32
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();  // <-- used
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
    virtual void slot023();  // <-- used
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0090db28 = (int *)0x0090DB28;
static int *const g_0090db7c = (int *)0x0090DB7C;

class Midi { public:
    uint8_t pad_0_[0x3C];
    uint32_t field_3c_;

    int unload();
};
typedef void (__cdecl *PFN_0090DB28)(int);

// Own vtable shim (see VCall's comment above: only declaration order
// matters, and the name is not pinned) so slots 5 and 23 can return int.
class MidiVCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual int slot005();
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
    virtual int slot023();
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();
};

int Midi::unload() {
    int result = 0;
    if (field_3c_ != 0) {
        int rc = reinterpret_cast<MidiVCall *>(this)->slot023();
        if (rc == 0) {
            reinterpret_cast<MidiVCall *>(field_3c_)->slot005();
            if (*g_0090db7c != 0) {
                ((PFN_0090DB28)*g_0090db28)((int)field_3c_);
            }
        } else {
            result = reinterpret_cast<MidiVCall *>(field_3c_)->slot005();
        }
        field_3c_ = 0;
    }
    reinterpret_cast<MidiVCall *>(this)->slot032();
    uint32_t *field_40 = reinterpret_cast<uint32_t *>(reinterpret_cast<char *>(this) + 0x40);
    *field_40 &= 0xFFFFFFFE;
    return result;
}
