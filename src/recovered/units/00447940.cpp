// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00447940
// name           ?general@BAmbience@@QAEXXZ
// size           361 bytes
// measured tier  MISMATCH
// divergence     18
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00447940/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?general@BAmbience@@QAEXXZ  at 0x00447940  (361 bytes)
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

class BAmbience;
class Midi;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Midi { public:
    bool is_trackset_playing(unsigned int);
};

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 67, 69
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
    virtual void slot062();
    virtual void slot063();
    virtual void slot064();
    virtual void slot065();
    virtual void slot066();
    virtual void slot067();  // <-- used
    virtual void slot068();
    virtual void slot069();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00683c18 = (int *)0x00683C18;
static int *const g_00683c1c = (int *)0x00683C1C;
static int *const g_0074dad4 = (int *)0x0074DAD4;
static int *const g_0074dad8 = (int *)0x0074DAD8;
static int *const g_0074dadc = (int *)0x0074DADC;

class BAmbience { public:
    uint8_t pad_0_[0x70];
    uint32_t field_70_;
    uint32_t field_74_;

    void general();
};

// Slot 67 at +0x10C takes one int (an id/count constant). Slot 69 at +0x114
// takes two ints, (id, field_74_) - established by ten other byte-exact
// Ambience recoveries (see 0x00447B30's NewBaseBVCall).
class GeneralVCall {
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
  virtual void slot062();
  virtual void slot063();
  virtual void slot064();
  virtual void slot065();
  virtual void slot066();
  virtual void slot067(int);
  virtual void slot068();
  virtual void slot069(int, int);
};

void BAmbience::general() {
    char *const self = reinterpret_cast<char *>(this);

    field_74_ = field_74_ + 500;
    if (static_cast<int32_t>(field_74_) > 500) {
        field_74_ = static_cast<uint32_t>(-1000);
    }

    uint8_t midi_flag = *reinterpret_cast<uint8_t *>(g_0074dad4);
    int32_t ambient_count = *g_00683c18;
    int32_t music_count = *g_0074dad8;
    int32_t track_count = *g_0074dadc;
    ambient_count++;
    music_count++;
    track_count++;
    *g_00683c18 = ambient_count;
    *g_0074dad8 = music_count;
    *g_0074dadc = track_count;

    if (midi_flag) {
        bool playing = reinterpret_cast<Midi *>(this)->is_trackset_playing(4);
        if (playing) {
            return;
        }
        *reinterpret_cast<uint8_t *>(g_0074dad4) = playing;
    }

    if (*reinterpret_cast<uint8_t *>(self + 0x6d)) {
        int32_t old_field_78 = *reinterpret_cast<int32_t *>(self + 0x78);
        *reinterpret_cast<int32_t *>(self + 0x78) = old_field_78 + 1;
        reinterpret_cast<GeneralVCall *>(this)->slot067(old_field_78);

        if (*reinterpret_cast<int32_t *>(self + 0x78) > 0x16) {
            *reinterpret_cast<int32_t *>(self + 0x78) = 0x14;
        }

        if (*g_00683c18 >= 3) {
            reinterpret_cast<GeneralVCall *>(this)->slot067(
                *reinterpret_cast<int32_t *>(self + 0x7c));
            *g_00683c18 = 0;
        }

        int32_t new_field_7c = *reinterpret_cast<int32_t *>(self + 0x7c) + 2;
        *reinterpret_cast<int32_t *>(self + 0x7c) = new_field_7c;
        if (new_field_7c == 0x15) {
            *reinterpret_cast<int32_t *>(self + 0x7c) = 0x10;
        }
        if (*reinterpret_cast<int32_t *>(self + 0x7c) == 0x16) {
            *reinterpret_cast<int32_t *>(self + 0x7c) = 0x10;
        }
    }

    if (*g_00683c18 >= 3) {
        int32_t mask = *reinterpret_cast<uint8_t *>(g_00683c1c) ? -1 : 0;
        int32_t mode = 0x19 + mask;
        reinterpret_cast<GeneralVCall *>(this)->slot069(mode, field_74_);

        if (static_cast<int32_t>(field_70_) > 1) {
            reinterpret_cast<GeneralVCall *>(this)->slot069(0xb, field_74_);
            reinterpret_cast<GeneralVCall *>(this)->slot067(0xb);
        }

        reinterpret_cast<GeneralVCall *>(this)->slot067(mode);
        *reinterpret_cast<uint8_t *>(g_00683c1c) =
            !*reinterpret_cast<uint8_t *>(g_00683c1c);
    }

    if (*g_0074dad8 >= 5) {
        reinterpret_cast<GeneralVCall *>(this)->slot069(0xd, field_74_);
        reinterpret_cast<GeneralVCall *>(this)->slot067(0xd);
        *g_0074dad8 = 0;
    }
}
