// ORIGINAL: 0x00632290 sub_632290 0x00632290-0x006322FC FILE
// working copy - scaffold materialised by --work
// size      108 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x006322BD 0x006322F2

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_632290  at 0x00632290  (108 bytes)
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
// This body dispatches through slot(s): 20, 29
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
    virtual void slot020();  // <-- used
    virtual void slot021();
    virtual void slot022();
    virtual void slot023();
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009be600 = (int *)0x009BE600;
// Not a genuine virtual dispatch: `ecx` holds the dereferenced vtable
// pointer at call time and the receiver (`eax`) is pushed EXPLICITLY as
// the call's first stack argument, so the VCall shim (real thiscall) does
// not reproduce it. Modelled as a raw function-pointer table instead.
class Obj632290 {
 public:
  int sub_632290(int a1);
};

int Obj632290::sub_632290(int a1) {
    char *self = reinterpret_cast<char *>(this);
    int *iface = *(int **)g_009be600;
    if (iface != 0) {
        typedef int (__stdcall *VCall1Fn)(void *, int, char *, int *, int);
        typedef void (__stdcall *VCall2Fn)(void *, int, char *, int);

        char flag_byte;
        int flag_int = 1;
        int field1 = *(int *)(self + 0x760);
        VCall1Fn call1 = (VCall1Fn)(*(void ***)iface)[0x50 / 4];
        int result = call1(iface, field1, &flag_byte, &flag_int, 0);
        if (result == 0) {
            char al = flag_byte;
            if (a1 != 0) {
                al = al | 2;
            } else {
                al = al & 0xfd;
            }
            flag_byte = al;
            int field2 = *(int *)(self + 0x760);
            VCall2Fn call2 = (VCall2Fn)(*(void ***)iface)[0x74 / 4];
            call2(iface, field2, &flag_byte, 1);
        }
    }
    return 0;
}
