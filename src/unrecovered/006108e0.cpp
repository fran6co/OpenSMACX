// ORIGINAL: 0x006108E0 ?sprite@SpriteBox@@QAEHPAUSprite@@PADH@Z 0x006108E0-0x00610965 FILE
// TRIED: base = self+0x24 to hint a shared-base store (matches ecx=lea); still 3 bytes short (one lea/mov collapsed) at the a1/0/0 store
// working copy - scaffold materialised by --work
// size      133 bytes
// prototype int (__thiscall ?sprite@SpriteBox@@QAEHPAUSprite@@PADH@Z)(SpriteBox* this, Sprite*, int8*, int)
// callers   22   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00401100 0x00402550

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?sprite@SpriteBox@@QAEHPAUSprite@@PADH@Z  at 0x006108E0  (133 bytes)
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
class SpriteBox;
class StringStruct;
struct StringStructEntry;

// ---- callees, declared and never defined (a definition would be inlined) ----
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
};

class StringStruct { public:
    uint32_t primary_abi_word_;
    uint32_t virtual_base_abi_word_;
    StringStructEntry * head_;
    StringStructEntry * current_;
    int entry_count_;
    int current_position_;
    void * allocator_;
    uint32_t allocation_base_abi_word_;
    void * allocation_owner_;
    int add(int);
};

struct StringStructEntry {
    uint32_t abi_word;
    int id;
    int payload;
    StringStructEntry * next;
    StringStructEntry * previous;
    uint32_t secondary_abi_word;
    void * allocation_owner;
};

extern "C" int __stdcall fn_00402550(int);

class SpriteBox { public:
    uint32_t field_0_;

    int sprite(Sprite *, char *, int);
};
int SpriteBox::sprite(Sprite * a1, char * a2, int a3) {
    if (a1 == 0) {
        return 3;
    }
    char *self = reinterpret_cast<char *>(this);
    char *base = self + 0x24;
    *reinterpret_cast<Sprite **>(base + 0x1c) = a1;
    *reinterpret_cast<int *>(base + 0x20) = 0;
    *reinterpret_cast<int *>(base + 0x24) = 0;
    if (fn_00402550(a3) != 0) {
        return 4;
    }
    if (a2 != 0) {
        int disp = (*reinterpret_cast<int **>(self))[2];
        *reinterpret_cast<char **>(self + disp + 0xd8) = a2;
        *reinterpret_cast<int *>(self + disp + 0xdc) = 0;
        *reinterpret_cast<int *>(self + disp + 0xe0) = 0;
        if (reinterpret_cast<StringStruct *>(self + disp + 0xbc)->add(a3) != 0) {
            return 4;
        }
    }
    return 0;
}
