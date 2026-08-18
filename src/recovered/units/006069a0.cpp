// ORIGINAL: 0x006069A0 ?on_button_pressed@Scroll@@QAEXH@Z 0x006069A0-0x00606B14 FILE
// size      372 bytes
// prototype void (__thiscall ?on_button_pressed@Scroll@@QAEXH@Z)(Scroll* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x006069BA 0x006069C8 0x006069DE 0x00606A4F 0x00606A7F 0x00606AAD 0x00606AC6 0x00606AD1 0x00606AE2 0x00606AE8 0x00606B02
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x006069A0
// measured tier  MISMATCH
// divergence     19
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/006069a0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_button_pressed@Scroll@@QAEXH@Z  at 0x006069A0  (372 bytes)
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

class Scroll;

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 7, 23, 49, 62
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
    virtual void slot049();  // <-- used
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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066912c = (int *)0x0066912C;
static int *const g_00669368 = (int *)0x00669368;
static int *const g_009b7ab8 = (int *)0x009B7AB8;
static int *const g_009b8ddc = (int *)0x009B8DDC;

class Scroll { public:
    void on_button_pressed(int);
};

typedef unsigned long (__stdcall *FnTimeGetTime)();
typedef void (__stdcall *FnSleep)(unsigned long);

// The 4 dispatched slots (7, 23, 49, 62) reach three different objects
// (an inherited `target` at +0xc4, its own +0x84 sub-object, and `this`)
// through the same shim type; only 23 (returns int, checked against 0) and
// 49 (takes the two ints pushed before it) need a real signature.
class ScrollVCall {
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
    virtual void slot049(int, int);
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
    virtual void slot062();
};

void Scroll::on_button_pressed(int a1) {
    int *p = reinterpret_cast<int *>(this);

    if (p[0x31] == 0) {
        return;
    }

    FnTimeGetTime timeGetTime = *reinterpret_cast<FnTimeGetTime *>(g_00669368);
    unsigned long startTime = timeGetTime();
    int lastTime = p[0x28e];
    if (lastTime != 0) {
        unsigned long now = timeGetTime();
        if (static_cast<int>(now - lastTime) < p[0x28c]) {
            return;
        }
        // A re-read, not the cached `lastTime`: the call above is opaque to
        // the compiler, so it cannot prove this redundant guard is dead the
        // way it can when the same local is re-tested. Matches the
        // original's un-eliminated `test edi,edi; jne`.
        if (p[0x28e] == 0) {
            p[0x28e] = timeGetTime();
        }
    } else {
        p[0x28e] = timeGetTime();
    }

    *g_009b8ddc = 0;
    int pos = p[0x28b];
    if (a1 == 1) {
        if (p[0x288] < pos) {
            p[0x28b] = pos - 1;
        }
    } else if (a1 == 2 && pos < p[0x289]) {
        p[0x28b] = pos + 1;
    }

    *g_009b7ab8 = p[0x31];
    int newPos = p[0x28b];
    if (pos != newPos) {
        ScrollVCall *target = reinterpret_cast<ScrollVCall *>(p[0x31]);
        int cbArg;
        if (p[0x28a] == 0) {
            target->slot049(p[0x286], newPos);
            cbArg = p[0x28b];
        } else {
            target->slot049(p[0x286], (p[0x288] - newPos) + p[0x289]);
            cbArg = (p[0x288] - p[0x28b]) + p[0x289];
        }
        void (__cdecl *cb)(int, int) =
            reinterpret_cast<void (__cdecl *)(int, int)>(p[0x852]);
        if (cb) {
            cb(p[0x286], cbArg);
        }
    }

    ScrollVCall *obj2 = *reinterpret_cast<ScrollVCall **>(p[0x31] + 0x84);
    if (obj2) {
        if (obj2->slot023() == 0) {
            obj2->slot007();
        }
    }

    p[0x28f] = -1;
    reinterpret_cast<ScrollVCall *>(this)->slot062();

    unsigned long endTime = timeGetTime();
    unsigned long elapsed = endTime - startTime;
    unsigned long duration = 1000 / static_cast<unsigned int>(p[0x28d]);
    if (elapsed < duration) {
        (*reinterpret_cast<FnSleep *>(g_0066912c))(duration - elapsed);
    }
}
