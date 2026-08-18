// ORIGINAL: 0x00538EC0 ??1JackalVoiceRx@@QAE@XZ 0x00538EC0-0x00538F2E FILE
// size      110 bytes
// prototype void (__thiscall ??1JackalVoiceRx@@QAE@XZ)(JackalVoiceRx* this)
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00538EE2 0x00538EEF 0x00538F0F
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00538EC0
// measured tier  MISMATCH
// divergence     9
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00538ec0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??1JackalVoiceRx@@QAE@XZ  at 0x00538EC0  (110 bytes)
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

class JackalVoiceRx;

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 0, 1, 2
class VCall { public:
    virtual void slot000(int);  // <-- used
    virtual void slot001(void *);  // <-- used
    virtual void slot002(int);  // <-- used
};

class JackalVoiceRx { public:
    ~JackalVoiceRx();
};

// NOT BYTE_EXACT. Closest reached: 108/110 bytes, mnemonic similarity
// 0.918, edit_count 4 (verify_recovered_function.py --json / byte_match.py).
// The loop body's logic and every virtual-dispatch shape match:
// `slot001` on `this` fed by an adjustor-free vtable read, `slot002`
// on the freed sub-object, and the vcall-thunk-style adjustor call
// (`off = *(*node + 4); node[off]->slot000(1)`) for the third all
// reproduce the original's exact instruction sequence and operands.
// Divergence is REGISTER ALLOCATION, not logic: the original keeps
// everything in registers with NO stack frame at all - eax doubles
// as the reusable "0" constant AND scratch, ebx is dedicated to the
// loop counter, esi is `this`, edi is the freed sub-object pointer;
// four roles packed into three callee-saved registers (ebx/esi/edi)
// plus one caller-saved (eax) reused across the whole body. Every
// source spelling tried here (naming vs. inlining the freed-node and
// adjusted-node reads, `!= 0` vs. plain truthy guards, reordering the
// two reads sharing one base pointer) has the compiler give the loop
// counter its OWN stack slot instead of reusing a register the way
// the original's allocator did, adding a `push ecx` alignment filler
// and an EBP frame the original does not have. Ruled out as a
// register-allocation wall like the one documented on 0x006339E0's
// NetFifo destructor: mnemonics and operands agree wherever the
// register choice does not force it, and no restructuring tried
// moved the counter off the stack.
JackalVoiceRx::~JackalVoiceRx() {
    char *self = reinterpret_cast<char *>(this);
    if (*reinterpret_cast<void **>(self + 8) != 0) {
        if (*reinterpret_cast<int *>(self + 0x10) > 0) {
            int i = 0;
            do {
                *reinterpret_cast<char **>(self + 0xc) =
                    *reinterpret_cast<char **>(*reinterpret_cast<char **>(self + 8) + 0xc);
                void *sub = *reinterpret_cast<void **>(*reinterpret_cast<char **>(self + 8) + 8);
                reinterpret_cast<VCall *>(self)->slot001(sub);
                if (sub) {
                    reinterpret_cast<VCall *>(sub)->slot002(1);
                }
                *reinterpret_cast<void **>(*reinterpret_cast<char **>(self + 8) + 8) = 0;
                if (*reinterpret_cast<void **>(self + 8)) {
                    char *n2 = *reinterpret_cast<char **>(self + 8);
                    reinterpret_cast<VCall *>(
                        n2 + *reinterpret_cast<int *>(*reinterpret_cast<char **>(n2) + 4))
                        ->slot000(1);
                }
                ++i;
                *reinterpret_cast<char **>(self + 8) = *reinterpret_cast<char **>(self + 0xc);
            } while (i < *reinterpret_cast<int *>(self + 0x10));
        }
        *reinterpret_cast<void **>(self + 8) = 0;
        *reinterpret_cast<void **>(self + 0x14) = 0;
        *reinterpret_cast<int *>(self + 0x10) = 0;
    }
}

