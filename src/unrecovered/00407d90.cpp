// ORIGINAL: 0x00407D90 FILE
// RULED-OUT: same "iVar1<=bound-1 / abs / dec-loop" shape as 0x0063C6D0 -
//   reused the `setField` flag so the trailing store isn't shared between
//   the negative- and positive-`iVar5` branches (see that address's note).
//   `#pragma function(strlen)` was needed too (not just `abs`) or the
//   length call inlines to `repne scasb`. Flipping `if (x==0) a=0; else
//   a=...;` to `if (x!=0) a=...; else a=0;` matched the original's `jne`
//   polarity. Landed at 96.4%: the last gap is that this source's compiler
//   assigns `this` to esi and the loaded pointer to edi/ebx, a full
//   permutation from the original's ebx=this/esi=base/edi=count - not
//   forceable through statement reordering, matching what was already
//   seen on 0x00496D00's esi/edi swap.
// PROPOSAL: `void AlphaSave::on_selected(int a1)` - `a1` is read nowhere in
//   the disassembly (no `[ebp+8]`/`[esp+N]` load exists), only `ret 4`
//   proves a second thiscall argument was popped; the parameter is
//   genuinely unused, not misdetected.
// working copy - scaffold materialised by --work
// name      ?on_selected@AlphaSave@@QAEXH@Z
// size      235 bytes
// spans     0x00407D90-0x00407E7B
// prototype void (__thiscall ?on_selected@AlphaSave@@QAEXH@Z)(AlphaSave* this, int)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006151E0 0x00644F3A 0x006453E0
// indirect  0x00407E65 0x00407E6F

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_selected@AlphaSave@@QAEXH@Z  at 0x00407D90  (235 bytes)
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
class EditBox;

// ---- callees, declared and never defined (a definition would be inlined) ----
class EditBox { public:
    void set_text(char *);
};

extern "C" int abs(int);
extern "C" unsigned int strlen(const char *);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 62
class VCall { public:
    virtual void slot000();
    virtual void slot001();
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
    virtual void slot017();
    virtual void slot018();
    virtual void slot019();
    virtual void slot020();
    virtual void slot021();
    virtual void slot022();
    virtual void slot023();
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();
    virtual void slot033();
    virtual void slot034();
    virtual void slot035();
    virtual void slot036();
    virtual void slot037();
    virtual void slot038();
    virtual void slot039();
    virtual void slot040();
    virtual void slot041();
    virtual void slot042();
    virtual void slot043();
    virtual void slot044();
    virtual void slot045();
    virtual void slot046();
    virtual void slot047();
    virtual void slot048();
    virtual void slot049();
    virtual void slot050();
    virtual void slot051();
    virtual void slot052();
    virtual void slot053();
    virtual void slot054();
    virtual void slot055();
    virtual void slot056();
    virtual void slot057();
    virtual void slot058();
    virtual void slot059();
    virtual void slot060();
    virtual void slot061();
    virtual void slot062();  // <-- used
};

class AlphaSave { public:
    void on_selected(int);
};
#pragma function(abs)
#pragma function(strlen)
void AlphaSave::on_selected(int a1) {
    char *self = reinterpret_cast<char *>(this);
    char *a = *reinterpret_cast<char **>(self + 0x3280);
    char *b = *reinterpret_cast<char **>(a + 8);
    char *base = b + reinterpret_cast<int>(self);

    int *f334c = reinterpret_cast<int *>(base + 0x334c);
    int *f3344 = reinterpret_cast<int *>(base + 0x3344);
    int *f3348 = reinterpret_cast<int *>(base + 0x3348);
    int *f3350 = reinterpret_cast<int *>(base + 0x3350);

    int iVar5 = *reinterpret_cast<int *>(b + 0x336c + reinterpret_cast<int>(self));

    if (iVar5 <= *f334c - 1) {
        *f3348 = *f3344;
        int setField = 1;
        if (iVar5 < 0) {
            int iVar2 = abs(iVar5);
            if (iVar2 <= *f334c) {
                iVar2 = abs(iVar5);
                if (iVar2 > 0) {
                    do {
                        --iVar2;
                        *f3348 = *reinterpret_cast<int *>(*f3348 + 0x10);
                    } while (iVar2 != 0);
                }
                iVar5 = iVar5 + *f334c;
            } else {
                setField = 0;
            }
        } else if (iVar5 > 0) {
            int iVar2 = iVar5;
            do {
                --iVar2;
                *f3348 = *reinterpret_cast<int *>(*f3348 + 0xc);
            } while (iVar2 != 0);
        }
        if (setField) {
            *f3350 = iVar5;
        }
    }

    char *uVar3;
    if (*f3344 != 0) {
        uVar3 = *reinterpret_cast<char **>(*reinterpret_cast<int *>(*f3348 + 8) + 4);
    } else {
        uVar3 = 0;
    }

    EditBox *editBox = reinterpret_cast<EditBox *>(self + 0x270c);
    editBox->set_text(uVar3);
    *reinterpret_cast<int *>(self + 0x270c + 0xb40) = 0;
    unsigned int len = strlen(self + 0x270c + 0xa14);
    *reinterpret_cast<unsigned int *>(self + 0x270c + 0xb44) = len;
    reinterpret_cast<VCall *>(editBox)->slot062();
    reinterpret_cast<VCall *>(editBox)->slot062();
}
