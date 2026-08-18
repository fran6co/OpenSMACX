// ORIGINAL: 0x005EAD9F ?change_color@Sprite@@QAEHHH@Z 0x005EAD9F-0x005EAE4E FILE
// working copy - scaffold materialised by --work
// size      175 bytes
// prototype int (__thiscall ?change_color@Sprite@@QAEHHH@Z)(Sprite* this, int, int)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?change_color@Sprite@@QAEHHH@Z  at 0x005EAD9F  (175 bytes)
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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_005eadfc = (int *)0x005EADFC;
static int *const g_005eae15 = (int *)0x005EAE15;
static int *const g_005eae3b = (int *)0x005EAE3B;

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

    int change_color(int, int);
};
int Sprite::change_color(int a1, int a2) {
    char *data = reinterpret_cast<char *>(pcBits_);
    if (data == 0) {
        return 7;
    }
    int total = iSpriteHeight_ * iSpriteWidth2_;
    unsigned char findColor = static_cast<unsigned char>(a1 & 0xff);
    unsigned char replColor = static_cast<unsigned char>(a2 & 0xff);
    for (int i = 0; i < total; i++) {
        if (static_cast<unsigned char>(data[i]) == findColor) {
            data[i] = static_cast<char>(replColor);
        }
    }
    return 0;
}
