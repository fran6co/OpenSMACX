// ORIGINAL: 0x005CBAE0 FILE
// working copy - scaffold materialised by --work
// name      sub_5cbae0
// size      214 bytes
// spans     0x005CBAE0-0x005CBBB6
// prototype 
// callers   2   call targets   0
// calls     (none)
// indirect  0x005CBB6C 0x005CBBA0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5cbae0  at 0x005CBAE0  (214 bytes)
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
// This body dispatches through slot(s): 27, 33
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
    virtual void slot027();  // <-- used
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();
    virtual void slot033();  // <-- used
};
// PROPOSAL: signature. `mov esi, ecx` at entry (ecx consumed into a register
// that is then read at [esi+N] with no stack access) and `cmp edx, edi`
// (edi==0) with NO prior write to edx makes edx a genuine second incoming
// register argument - this is `__fastcall(void *param1, int param2)`, not
// the placeholder's zero-argument `__cdecl`. Both indirect calls also push
// their receiver ON THE STACK rather than passing it in ecx (`push ecx`
// right before `call [ebx+0x84]`, `push eax` right before `call [ecx+0x6c]`),
// which is COM's `__stdcall` vtable convention, not a C++ `__thiscall`
// vtable - so the shim below declares its used slots `__stdcall`, unlike
// the scaffold's default `VCall` (thiscall) above, which this body does not
// use.
//
// RULED-OUT / open: two locals (`device[0x64/4]-1`, `device[0x60/4]-1`,
// stored at ESP0+0x18/+0x1C - the two dwords immediately BEFORE the 14-dword
// zeroed buffer at ESP0+0x20) are computed and stored but never read again
// anywhere in this function, in the original OR in every spelling tried
// here - yet the original keeps the stores (sub/dec/mov, not eliminated).
// A standalone `int idx1, idx2;` here gets dead-store-eliminated entirely
// (confirmed: rebuilds 8 bytes short, `sub esp,0x50` not `0x58`, sourced
// zero-loop count 0xd not 0xe). They only survive as elements 0-1 of ONE
// 16-int array whose address is later taken via `&arr[2]` (passed to the
// call) - that keeps them alive, but changes the zero-init shape: rebuilds
// as `rep stosd` with ecx=0xf plus a separate explicit zero-store, 222 bytes
// against the original's clean `ecx=0xe` `rep stosd` at 214, and the
// register roles in the rect-construction block that follows (edi/ecx) come
// out swapped from the original's (eax/ecx/ebx). Also tried: zeroing only
// `arr[2..15]` via an explicit loop instead of a `= {0}` initializer on the
// whole 16 - that reintroduces the dead-store elimination of arr[0]/arr[1].
// Whatever the original's true declaration shape is (two separate locals
// that happen to land adjacent to the buffer purely by allocation luck,
// with VC6 declining to eliminate them for a reason not identified here) is
// not reproduced by any spelling tried.
class DeviceVCall {
public:
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
    virtual int __stdcall slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();
    virtual int __stdcall slot033(void *, int, void *, int, void *);
};

extern "C" int __fastcall sub_5cbae0(int *param1, int param2) {
    int arr[16] = {0};
    arr[2] = 0x38;

    int mode;
    if (param2 == 0) {
        mode = 0x200;
    } else {
        mode = 0x84000;
        arr[14] = 1;
    }

    int y = param1[6];
    int x = param1[5];
    int bottom = param1[4] + y;
    int right = param1[3] + x;

    int rect[6];
    rect[1] = y;
    rect[0] = x;
    rect[3] = bottom;
    rect[2] = right;
    rect[5] = 0;
    rect[4] = 0;

    int *device = (int *)param1[0];
    arr[0] = device[25] - 1;
    arr[1] = device[24] - 1;
    void *iface = (void *)device[7];
    int val = device[5];

    int result = ((DeviceVCall *)iface)->slot033(&rect[4], val, rect, mode, &arr[2]);
    if (result != 0) {
        if (result == (int)0x80070057) {
            param1[0x13] = 1;
            return 1;
        }
        if (result == (int)0x887601c2) {
            void *iface2 = (void *)(*(int **)param1)[7];
            ((DeviceVCall *)iface2)->slot027();
        }
        return 1;
    }
    return 0;
}
