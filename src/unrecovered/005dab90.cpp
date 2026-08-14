// ORIGINAL: 0x005DAB90 FILE
// RULED-OUT: this is a real __thiscall Buffer member (ecx=this pervasively);
//        slot001 (self+4) is a genuine virtual thiscall dispatch via the
//        scaffold's VCall class, but slots at offset 0x44/0x68 on the
//        secondary object (field_58_) push that object as an explicit
//        stack arg while ecx holds the vtable pointer - not a real
//        thiscall, so those two use raw `int**`/function-pointer casts
//        instead of VCall. field_50_/0x58/0x60/0x64/0x68/0x50C offsets
//        match Buffer's known layout from the wrap_cent brief.
// RULED-OUT: tracking a single `iVar3` local mirroring ghidra's variable
//            (should instead re-read *piVar1 fresh at the "iVar3 != 0"
//            gate, since the original reloads memory there); several
//            statement-order and branch-order permutations for the
//            piVar2==0 vs *piVar1==0 arms, none reached better than
//            ~0.81 mnemonic similarity.
// working copy - scaffold materialised by --work
// name      sub_5dab90
// size      213 bytes
// spans     0x005DAB90-0x005DAC65
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005DAB9D 0x005DABC7 0x005DABD2 0x005DAC03 0x005DAC45 0x005DAC50

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5dab90  at 0x005DAB90  (213 bytes)
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

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 1, 17, 26
class VCall { public:
    virtual void slot000();
    virtual void slot001();  // <-- used
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
    virtual void slot017();  // <-- used
    virtual void slot018();
    virtual void slot019();
    virtual void slot020();
    virtual void slot021();
    virtual void slot022();
    virtual void slot023();
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006690b0 = (int *)0x006690B0;

typedef unsigned int (__stdcall *SetTextColorFn)(void *, unsigned int);
typedef int (__stdcall *Slot44Fn)(void *, void *);
typedef int (__stdcall *Slot68Fn)(void *, void *);

class Buffer { public:
    void run_5dab90(unsigned int param_2);
};

void Buffer::run_5dab90(unsigned int param_2) {
    char *self = reinterpret_cast<char *>(this);

    if (*reinterpret_cast<int *>(self + 0x50) != 0) {
        reinterpret_cast<VCall *>(this)->slot001();
    }

    int *piVar2 = *reinterpret_cast<int **>(self + 0x58);
    int *piVar1 = reinterpret_cast<int *>(self + 0x60);

    if (piVar2 == 0) {
        *piVar1 = *reinterpret_cast<int *>(self + 0x64);
        *reinterpret_cast<int *>(self + 0x68) = *reinterpret_cast<int *>(self + 0x68) + 1;
    } else if (*piVar1 == 0) {
        int *vtbl = *reinterpret_cast<int **>(piVar2);
        Slot44Fn fn = reinterpret_cast<Slot44Fn>(vtbl[0x44 / 4]);
        int result = fn(piVar2, piVar1);
        if (result != 0) {
            reinterpret_cast<VCall *>(this)->slot001();
        }
        *reinterpret_cast<int *>(self + 0x68) = *reinterpret_cast<int *>(self + 0x68) + 1;
    } else {
        *reinterpret_cast<int *>(self + 0x68) = *reinterpret_cast<int *>(self + 0x68) + 1;
    }

    if (*piVar1 != 0) {
        if (param_2 != *reinterpret_cast<unsigned int *>(self + 0x50c)) {
            (*reinterpret_cast<SetTextColorFn *>(g_006690b0))(reinterpret_cast<void *>(*piVar1), (param_2 & 0xffff) | 0x10ff0000);
            *reinterpret_cast<unsigned int *>(self + 0x50c) = param_2;
        }
        piVar2 = *reinterpret_cast<int **>(self + 0x58);
        if (piVar2 == 0) {
            int count = *reinterpret_cast<int *>(self + 0x68);
            *reinterpret_cast<int *>(self + 0x68) = count - 1;
            if (count - 1 < 1) {
                *piVar1 = 0;
                *reinterpret_cast<int *>(self + 0x68) = 0;
                return;
            }
        } else {
            int count = *reinterpret_cast<int *>(self + 0x68);
            *reinterpret_cast<int *>(self + 0x68) = count - 1;
            if (*piVar1 != 0 && count - 1 < 1) {
                int *vtbl2 = *reinterpret_cast<int **>(piVar2);
                Slot68Fn fn2 = reinterpret_cast<Slot68Fn>(vtbl2[0x68 / 4]);
                int result = fn2(piVar2, reinterpret_cast<void *>(*piVar1));
                if (result != 0) {
                    reinterpret_cast<VCall *>(this)->slot001();
                }
                *reinterpret_cast<int *>(self + 0x68) = 0;
                *piVar1 = 0;
            }
        }
    }
}
