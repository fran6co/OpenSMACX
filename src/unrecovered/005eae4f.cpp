// ORIGINAL: 0x005EAE4F ?copy@Sprite@@QAEHPAUSprite@@@Z 0x005EAE4F-0x005EAF34 FILE
// RULED-OUT: register pressure gap - original keeps this/a1/zero-const/size live across 3 calls (free x2, mem_get, memcpy) in 4 callee-saved regs; straightforward member-access C body uses fewer, so the prologue push count diverges very early.
// working copy - scaffold materialised by --work
// size      229 bytes
// prototype int (__thiscall ?copy@Sprite@@QAEHPAUSprite@@@Z)(Sprite* this, Sprite*)
// callers   2   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2 0x00645930

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?copy@Sprite@@QAEHPAUSprite@@@Z  at 0x005EAE4F  (229 bytes)
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

class Sprite;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" void *memcpy(void *, const void *, unsigned int);
extern "C" void free(void *);
void * mem_get(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b6618 = (int *)0x009B6618;

class Sprite { public:
    int ppszFileName_;
    int pcBits_;
    char cTransparentIndex_;
    char pad1_;
    char pad2_;
    char pad3_;
    int iSpriteWidth2_;
    int iSpriteWidth_;
    int iSpriteHeight_;
    int iWidth_;
    int iHeight_;
    int iLeftOffset_;
    int iTopOffset_;
    int fObj1Exists_;

    int copy(Sprite *);
};
int Sprite::copy(Sprite * a1) {
    if (pcBits_ == 0) {
        return 7;
    }
    if (a1->fObj1Exists_ == 0 && a1->pcBits_ != 0) {
        *g_009b6618 = *g_009b6618 - a1->iSpriteHeight_ * a1->iSpriteWidth_;
        if (a1->pcBits_ != 0) {
            free(reinterpret_cast<void *>(a1->pcBits_));
        }
        a1->pcBits_ = 0;
    }
    if (a1->ppszFileName_ != 0) {
        free(reinterpret_cast<void *>(a1->ppszFileName_));
        a1->ppszFileName_ = 0;
    }
    a1->iSpriteWidth2_ = 0;
    a1->iSpriteWidth_ = 0;
    a1->iSpriteHeight_ = 0;
    a1->iWidth_ = 0;
    a1->iHeight_ = 0;
    a1->iLeftOffset_ = 0;
    a1->iTopOffset_ = 0;
    a1->fObj1Exists_ = 0;
    int size = iSpriteHeight_ * iSpriteWidth2_;
    void *buf = mem_get(size);
    a1->pcBits_ = reinterpret_cast<int>(buf);
    if (buf == 0) {
        return 4;
    }
    memcpy(buf, reinterpret_cast<void *>(pcBits_), size);
    a1->iSpriteWidth_ = iSpriteWidth_;
    a1->iSpriteHeight_ = iSpriteHeight_;
    a1->iWidth_ = iWidth_;
    a1->iHeight_ = iHeight_;
    a1->iLeftOffset_ = iLeftOffset_;
    a1->iTopOffset_ = iTopOffset_;
    a1->cTransparentIndex_ = cTransparentIndex_;
    a1->fObj1Exists_ = 0;
    a1->iSpriteWidth2_ = iSpriteWidth2_;
    return 0;
}
