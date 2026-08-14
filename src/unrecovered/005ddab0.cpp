// ORIGINAL: 0x005DDAB0 FILE
// name      ?wrap_cent@Buffer@@QAEHPADPAURECT@@@Z
// size      246 bytes
// spans     0x005DDAB0-0x005DDBA6
// prototype int (__thiscall ?wrap_cent@Buffer@@QAEHPADPAURECT@@@Z)(Buffer* this, char*, RECT*)
// callers   ?   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005DC410 0x005DD020 0x006453E0
// RULED-OUT: local `int lineWidth = width;` passed by address to
//            find_line_break_l, mirroring the Ghidra `local_4` exactly -
//            compiles and is a faithful transcription (mnemonic similarity
//            0.68, first divergence in the prologue's push/sub register
//            save order) but was not iterated further given the size of
//            this batch; a byte-exact prologue shape is left for a later
//            pass.

// GENERATED SKELETON (hand-written, following tools/emit_translation_unit.py
// conventions) - a VERIFICATION ARTIFACT, not product source: classes are
// opaque and globals are bound to fixed addresses.

struct RECT {
    int left;
    int top;
    int right;
    int bottom;
};

class Buffer {
public:
    int find_line_break_l(char *, int *, int);
    int write_cent_l(char *, int, int, int, int);
    int wrap_cent(char *, RECT *);
};

extern "C" unsigned int strlen(const char *);

extern int g_9b3a9c;
extern int g_9bb484;

int Buffer::wrap_cent(char * a1, RECT * a2) {
    if (a1 == 0 || a2 == 0) {
        return a2->top;
    }
    g_9b3a9c = 0;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x18) = 0;
    int left = a2->left;
    int width = a2->right - left;
    char *str = a1;
    int yPos = a2->top;
    char *nextStr;
    for (;;) {
        if (str == 0) {
            nextStr = 0;
        } else {
            g_9b3a9c = 0;
            int lineWidth = width;
            unsigned int fullLen = strlen(str);
            nextStr = reinterpret_cast<char *>(
                find_line_break_l(str, &lineWidth, static_cast<int>(fullLen)));
            unsigned int chunkLen;
            if (nextStr == 0) {
                chunkLen = strlen(str);
            } else {
                chunkLen = static_cast<unsigned int>(nextStr - str);
            }
            write_cent_l(str, left, yPos, width, static_cast<int>(chunkLen));
        }
        int *lineCount = reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x18);
        *lineCount = *lineCount + 1;
        int **fontSlot = reinterpret_cast<int **>(reinterpret_cast<char *>(this) + 0x52c);
        if (*fontSlot == 0) {
            *fontSlot = reinterpret_cast<int *>(g_9bb484);
        }
        int *font = *fontSlot;
        int advance;
        if (*font < 0) {
            advance = font[3];
        } else {
            advance = font[4] + *font;
        }
        yPos = yPos + advance;
        str = nextStr;
        if (nextStr == 0) {
            break;
        }
    }
    return yPos;
}
