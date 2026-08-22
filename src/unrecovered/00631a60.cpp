// ORIGINAL: 0x00631A60 ?process_voice@Net@@QAEXXZ 0x00631A60-0x00631B2E FILE
// TRIED: single-level reinterpret_cast<VCall*>(self+0x58)->slotN() for the embedded subobject at +0x58, recomputed at each call site instead of cached in a local (0.73 -> 0.96 mnemonic similarity); slots 34/35/37 needed non-void signatures (int(), void(int), unsigned int()) since the shared VCall shim declares every slot void(); the outer `if (flags & mask)` condition polarity is INVERTED versus the ghidra hypothesis (`!= 0` first, `== 0` in the else) to match which branch the original falls into vs jumps to; the two call sites (sub_62f390/send_packet_type) each re-read `*(int*)(packet+0x20) - offset` inline rather than hoisting a shared `length` local, since the original recomputes `sub edx,eax` once per branch.
// TRIED: hoisting length/chan into locals shared by both branches (dropped the ebx push/pop and one sub entirely).
// working copy - scaffold materialised by --work
// size      206 bytes
// prototype void (__thiscall ?process_voice@Net@@QAEXXZ)(Net* this)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FD2B0 0x0062F390 0x0062F8A0
// indirect  0x00631A7F 0x00631AAB 0x00631AC8 0x00631AD3 0x00631B0A 0x00631B24

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?process_voice@Net@@QAEXXZ  at 0x00631A60  (206 bytes)
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

class Net;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" int __stdcall sub_62f390(int, int, int, int, int);
int do_sound();

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 8, 34, 35, 37
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();  // <-- used
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
    virtual int slot034();  // <-- used, returns a pointer-as-int
    virtual void slot035(int);  // <-- used, takes one int arg
    virtual void slot036();
    virtual unsigned int slot037();  // <-- used, returns a level
};

class Net { public:
    int send_packet_type(void *, int, unsigned long, int, int);
    void process_voice();
};
void Net::process_voice() {
    char *self = reinterpret_cast<char *>(this);

    do_sound();

    unsigned int flags = *reinterpret_cast<unsigned int *>(self + 0xd8);
    if ((flags & 0x40000000) != 0) {
        unsigned int level = reinterpret_cast<VCall *>(self + 0x58)->slot037();
        if (level < 0x32) {
            *reinterpret_cast<unsigned int *>(self + 0xd8) =
                *reinterpret_cast<unsigned int *>(self + 0xd8) & 0xbfffffff;
        }
    } else {
        unsigned int level = reinterpret_cast<VCall *>(self + 0x58)->slot037();
        if (level > 100) {
            *reinterpret_cast<unsigned int *>(self + 0xd8) =
                *reinterpret_cast<unsigned int *>(self + 0xd8) | 0x40000000;
            reinterpret_cast<VCall *>(self + 0x58)->slot008();
            return;
        }
        int packet = reinterpret_cast<VCall *>(self + 0x58)->slot034();
        if (packet != 0) {
            int offset = *reinterpret_cast<int *>(packet + 0x18);
            unsigned long field_d4 = *reinterpret_cast<unsigned long *>(self + 0xd4);
            if (*reinterpret_cast<int *>(self + 0x54) != 0) {
                sub_62f390(offset, *reinterpret_cast<int *>(packet + 0x20) - offset, field_d4, 0x100, 0);
            } else {
                send_packet_type(reinterpret_cast<void *>(offset), *reinterpret_cast<int *>(packet + 0x20) - offset, field_d4, 0x100, 0);
            }
            reinterpret_cast<VCall *>(self + 0x58)->slot035(packet);
        }
    }
}
