// ORIGINAL: 0x005E3890 FILE
// working copy - scaffold materialised by --work
// name      ?init@Sprite@@QAEHPADHH@Z
// size      195 bytes
// spans     0x005E3890-0x005E3953
// prototype int (__thiscall ?init@Sprite@@QAEHPADHH@Z)(Sprite* this, int8*, int, int)
// callers   15   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2 0x006453E0 0x00645470

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?init@Sprite@@QAEHPADHH@Z  at 0x005E3890  (195 bytes)
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
extern "C" char *strcat(char *, const char *);
extern "C" unsigned int strlen(const char *);
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

    int init(char *, int, int);
};
int Sprite::init(char * a1, int a2, int a3) {
    if (a1 == 0) {
        return 0x10;
    }
    if (fObj1Exists_ == 0 && pcBits_ != 0) {
        *g_009b6618 -= iSpriteHeight_ * iSpriteWidth_;
        if (pcBits_ != 0) {
            free(reinterpret_cast<void *>(pcBits_));
        }
        pcBits_ = 0;
    }
    if (ppszFileName_ != 0) {
        free(reinterpret_cast<void *>(ppszFileName_));
        ppszFileName_ = 0;
    }
    iSpriteWidth2_ = 0;
    iSpriteWidth_ = 0;
    iSpriteHeight_ = 0;
    iWidth_ = 0;
    iHeight_ = 0;
    iLeftOffset_ = 0;
    iTopOffset_ = 0;
    fObj1Exists_ = 0;
    unsigned int len = strlen(a1);
    void *mem = mem_get(len + 1);
    ppszFileName_ = reinterpret_cast<int>(mem);
    if (mem == 0) {
        return 4;
    }
    *reinterpret_cast<char *>(mem) = 0;
    strcat(reinterpret_cast<char *>(ppszFileName_), a1);
    iSpriteWidth2_ = a2;
    iSpriteWidth_ = a2;
    iSpriteHeight_ = a3;
    iWidth_ = a2;
    iHeight_ = a3;
    return 0;
}
