// ORIGINAL: 0x005EE6B0 sub_5ee6b0 0x005EE6B0-0x005EE74E FILE BYTE_EXACT
// LEVER: void return (eax never set before ret) + own VCall2 shim for slot023 (int return, tested) and slots 19/32 ((int,int)), not the file's nullary VCall. Same shape as sibling 0x005EE610, offsets 0x414/0x20/0x50 in place of that one's 0x410/0x48/0x6c.
// working copy - scaffold materialised by --work
// size      158 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EE6F0 0x005EE6FB 0x005EE70B 0x005EE716 0x005EE72B 0x005EE73C 0x005EE747

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5ee6b0  at 0x005EE6B0  (158 bytes)
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
// This body dispatches through slot(s): 7, 19, 23, 32
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();  // <-- used
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
    virtual void slot019();  // <-- used
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
static int *const g_009b7ab8 = (int *)0x009B7AB8;

// Second vtable shim: slot023 returns int (its result is tested), and
// slots 19/32 take (int, int); the file's own VCall types every slot
// void(). Declaration order pins the offset, so this carries 0..32 in
// order even though only 7/19/23/32 differ from the default.
class VCall2 { public:
    virtual void slot000(); virtual void slot001(); virtual void slot002();
    virtual void slot003(); virtual void slot004(); virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008(); virtual void slot009(); virtual void slot010();
    virtual void slot011(); virtual void slot012(); virtual void slot013();
    virtual void slot014(); virtual void slot015(); virtual void slot016();
    virtual void slot017(); virtual void slot018();
    virtual void slot019(int, int);
    virtual void slot020(); virtual void slot021(); virtual void slot022();
    virtual int slot023();
    virtual void slot024(); virtual void slot025(); virtual void slot026();
    virtual void slot027(); virtual void slot028(); virtual void slot029();
    virtual void slot030(); virtual void slot031();
    virtual void slot032(int, int);
};

typedef void (__cdecl *cb2_t)(int, int);

// No stack slot matches the values read at [ecx+N]; this is a
// __thiscall receiver, not the stdcall(int,int,int) the given head
// guessed, and eax is never set before any `ret` -> void, not int.
// See PROPOSALS in the report.
class Sub5ee6b0Recv { public:
    void sub_5ee6b0(int a1, int a2, int a3);
};

void Sub5ee6b0Recv::sub_5ee6b0(int a1, int a2, int a3) {
    char *self = (char *)this;

    if ((*(unsigned int *)(self + 0x98) & 0x200000) != 0) {
        return;
    }
    if ((*(unsigned char *)(self + 0x9c) & 8) != 0) {
        return;
    }

    if (a3 == 0) {
        *g_009b7ab8 = (int)self;
        cb2_t cb = *(cb2_t *)(self + 0x414);
        if (cb != 0) {
            cb(a1, a2);
        }
        ((VCall2 *)self)->slot019(a1, a2);

        char *child = *(char **)(self + 0x20);
        if (child == 0) {
            return;
        }
        if (((VCall2 *)child)->slot023() != 0) {
            return;
        }
        ((VCall2 *)child)->slot007();
        return;
    }

    ((VCall2 *)self)->slot032(a1, a2);

    char *child2 = *(char **)(self + 0x50);
    if (child2 == 0) {
        return;
    }
    if (((VCall2 *)child2)->slot023() != 0) {
        return;
    }
    ((VCall2 *)child2)->slot007();
}
