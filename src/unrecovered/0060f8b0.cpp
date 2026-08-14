// ORIGINAL: 0x0060F8B0
// name      ?on_key_down@CheckBox@@QAEHH@Z
// size      360 bytes
// spans     0x0060F8B0-0x0060FA18
// prototype int (__thiscall ?on_key_down@CheckBox@@QAEHH@Z)(CheckBox* this, int)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D5890 0x0060F030
// indirect  0x0060F9D1
// RULED-OUT: same shape as sibling 0x0060E020 (RadioButton::on_key_down):
//            `this` arrives adjusted to the GraphicWin virtual base
//            (`this-0x1c` recovers CheckBox's own start), and field access
//            goes through an adjustor offset read from CheckBox's own
//            vtable slots 1/2. Unlike RadioButton, the "current bit index"
//            field (`this-0x18`) and the callback pointer (`this-0x14`)
//            are DIRECT fields needing no adjustor, while the flags dword
//            at `off2+0xec` is addressed off `edi` (the true CheckBox base)
//            and the count/page-size fields at `off2+0xb0`/`off2+0x3c` are
//            addressed off `esi` (the adjusted GraphicWin pointer) - kept
//            that distinction since it is visible in the disassembly. The
//            key-range switch (VK_SPACE..VK_NUMPAD8) case labels (1, 3,
//            0x50, 0x52, 0x54) are a guess at the mapping, same caveat as
//            the RadioButton sibling - the jump/case-index table bytes are
//            not in the brief. 0.89 mnemonic similarity, first divergence
//            at #24.

static int *const g_009b7ab8 = (int *)0x009B7AB8;

class CheckBox {
public:
    int on_key_down(int a1);
    void draw_item(int, int, int);
};

class GraphicWinX {
public:
    void soft_update();
};

int CheckBox::on_key_down(int a1) {
    char *esi = reinterpret_cast<char *>(this);
    char *edi = esi - 0x1c;

    int *vt = *reinterpret_cast<int **>(edi);
    int off1 = vt[1];
    int cached = *reinterpret_cast<int *>(esi + off1 + 0xa8);
    *g_009b7ab8 = cached;

    int *vt0 = *reinterpret_cast<int **>(edi);
    int bitIndex = *reinterpret_cast<int *>(esi - 0x18);
    int off2 = vt0[2];
    int flagBit = (1 << bitIndex) & *reinterpret_cast<int *>(edi + off2 + 0xec);
    reinterpret_cast<CheckBox *>(edi)->draw_item(bitIndex, flagBit, 0);

    int idx = a1 - 0x20;

    if (static_cast<unsigned int>(idx) <= 0x48) {
        switch (idx) {
            case 1: {
                int v = *reinterpret_cast<int *>(esi - 0x18) - 1;
                if (v <= 0) {
                    v = 0;
                }
                *reinterpret_cast<int *>(esi - 0x18) = v;
                break;
            }
            case 3: {
                int v = *reinterpret_cast<int *>(esi - 0x18) + 1;
                int *vtA = *reinterpret_cast<int **>(edi);
                int off2a = vtA[2];
                int count = *reinterpret_cast<int *>(esi + off2a + 0xb0);
                if (v >= count) {
                    v = count - 1;
                }
                *reinterpret_cast<int *>(esi - 0x18) = v;
                break;
            }
            case 0x50: {
                int *vtA = *reinterpret_cast<int **>(edi);
                int off2a = vtA[2];
                int pageSize = *reinterpret_cast<int *>(esi + off2a + 0x3c);
                int v = *reinterpret_cast<int *>(esi - 0x18) + pageSize;
                *reinterpret_cast<int *>(esi - 0x18) = v;
                int *vtB = *reinterpret_cast<int **>(edi);
                int off2b = vtB[2];
                int count = *reinterpret_cast<int *>(esi + off2b + 0xb0);
                if (v >= count) {
                    v -= *reinterpret_cast<int *>(esi + off2b + 0x3c);
                    *reinterpret_cast<int *>(esi - 0x18) = v;
                }
                break;
            }
            case 0x52: {
                int *vtA = *reinterpret_cast<int **>(edi);
                int off2a = vtA[2];
                int pageSize = *reinterpret_cast<int *>(esi + off2a + 0x3c);
                int v = *reinterpret_cast<int *>(esi - 0x18) - pageSize;
                *reinterpret_cast<int *>(esi - 0x18) = v;
                if (v < 0) {
                    int *vtB = *reinterpret_cast<int **>(edi);
                    int off2b = vtB[2];
                    v += *reinterpret_cast<int *>(esi + off2b + 0x3c);
                    *reinterpret_cast<int *>(esi - 0x18) = v;
                }
                break;
            }
            case 0x54: {
                int *vtA = *reinterpret_cast<int **>(edi);
                int off2a = vtA[2];
                int bit = *reinterpret_cast<int *>(esi - 0x18);
                int *flags = reinterpret_cast<int *>(edi + off2a + 0xec);
                int mask = 1 << bit;
                if (*flags & mask) {
                    *flags = *flags & ~mask;
                } else {
                    *flags = *flags | mask;
                }
                break;
            }
            default:
                break;
        }
    }

    typedef void (__cdecl *Callback)(int);
    Callback cb = *reinterpret_cast<Callback *>(esi - 0x14);
    if (cb) {
        int *vtC = *reinterpret_cast<int **>(edi);
        int off2c = vtC[2];
        int flags = *reinterpret_cast<int *>(edi + off2c + 0xec);
        cb(flags);
    }

    int *vtD = *reinterpret_cast<int **>(edi);
    int bitIndex2 = *reinterpret_cast<int *>(esi - 0x18);
    int off2d = vtD[2];
    int flagBit2 = (1 << bitIndex2) & *reinterpret_cast<int *>(edi + off2d + 0xec);
    reinterpret_cast<CheckBox *>(edi)->draw_item(bitIndex2, flagBit2, 1);

    int *vtE = *reinterpret_cast<int **>(edi);
    int off1e = vtE[1];
    reinterpret_cast<GraphicWinX *>(off1e + edi)->soft_update();

    return 1;
}
