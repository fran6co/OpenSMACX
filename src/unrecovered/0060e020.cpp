// ORIGINAL: 0x0060E020
// name      ?on_key_down@RadioButton@@QAEHH@Z
// size      355 bytes
// spans     0x0060E020-0x0060E183
// prototype int (__thiscall ?on_key_down@RadioButton@@QAEHH@Z)(RadioButton* this, int)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D5890 0x0060D750
// indirect  0x0060E145
// RULED-OUT: `this` arrives already adjusted to the GraphicWin virtual-base
//            subobject (the function computes `this-0x18` for RadioButton's
//            own start, matching src/radiobutton.h's documented vbtable
//            {0,0x18,0xA30}), so field access goes through an adjustor
//            offset read out of RadioButton's own vtable slots 1 and 2
//            (`*(vt+4)`/`*(vt+8)`) rather than a fixed member offset -
//            modelled with raw `char*`/`int*` arithmetic, not a real class
//            layout. The `[0x25,0x68]` key-code switch (VK_LEFT through
//            VK_NUMPAD8) has only 4 distinct bodies out of 0x44 case
//            values; the jump/case-index TABLE BYTES themselves are not in
//            the brief, so the case labels used here (1, 3, 0x50, 0x52) are
//            a guess at which key maps to which body, not a fact - the
//            bodies themselves (decrement-clamp-to-0, increment-clamp-to-
//            count-1, add-a-page-then-clamp-down, subtract-a-page-then-
//            clamp-up) are transcribed directly from the disassembly.
//            0.80 mnemonic similarity, first divergence at #2.

static int *const g_009b7ab8 = (int *)0x009B7AB8;

class RadioButton {
public:
    int on_key_down(int a1);
    void draw_item(int, int);
};

class GraphicWinX {
public:
    void soft_update();
};

int RadioButton::on_key_down(int a1) {
    char *esi = reinterpret_cast<char *>(this);
    char *edi = esi - 0x18;

    {
        int *vt = *reinterpret_cast<int **>(edi);
        int off1 = vt[1];
        int cached = *reinterpret_cast<int *>(esi + off1 + 0xac);
        *g_009b7ab8 = cached;
    }

    int *vt2 = *reinterpret_cast<int **>(edi);
    int off2 = vt2[2];
    int cur = *reinterpret_cast<int *>(esi + off2 + 0xd4);
    int prev = cur;
    int idx = a1 - 0x25;

    if (static_cast<unsigned int>(idx) > 0x43) {
        return 0;
    }

    switch (idx) {
        case 1: {
            int newVal = cur - 1;
            *reinterpret_cast<int *>(esi + off2 + 0xd4) = newVal;
            int *vtA = *reinterpret_cast<int **>(edi);
            int off2a = vtA[2];
            int v = *reinterpret_cast<int *>(esi + off2a + 0xd4);
            if (v <= 0) {
                v = 0;
            }
            *reinterpret_cast<int *>(esi + off2a + 0xd4) = v;
            break;
        }
        case 3: {
            int newVal = cur + 1;
            *reinterpret_cast<int *>(esi + off2 + 0xd4) = newVal;
            int *vtA = *reinterpret_cast<int **>(edi);
            int off2a = vtA[2];
            int v = *reinterpret_cast<int *>(esi + off2a + 0xd4);
            int count = *reinterpret_cast<int *>(esi + off2a + 0xb4);
            if (v >= count - 1) {
                v = count - 1;
            }
            *reinterpret_cast<int *>(esi + off2a + 0xd4) = v;
            break;
        }
        case 0x50: {
            int pageSize = *reinterpret_cast<int *>(esi + off2 + 0x40);
            int newVal = cur + pageSize;
            *reinterpret_cast<int *>(esi + off2 + 0xd4) = newVal;
            int *vtA = *reinterpret_cast<int **>(edi);
            int off2a = vtA[2];
            int count = *reinterpret_cast<int *>(esi + off2a + 0xb4);
            int v = *reinterpret_cast<int *>(esi + off2a + 0xd4);
            if (v >= count) {
                v -= *reinterpret_cast<int *>(esi + off2a + 0x40);
                *reinterpret_cast<int *>(esi + off2a + 0xd4) = v;
            }
            break;
        }
        case 0x52: {
            int pageSize = *reinterpret_cast<int *>(esi + off2 + 0x40);
            int newVal = cur - pageSize;
            *reinterpret_cast<int *>(esi + off2 + 0xd4) = newVal;
            int *vtA = *reinterpret_cast<int **>(edi);
            int off2a = vtA[2];
            int v = *reinterpret_cast<int *>(esi + off2a + 0xd4);
            if (v < 0) {
                v += *reinterpret_cast<int *>(esi + off2a + 0x40);
                *reinterpret_cast<int *>(esi + off2a + 0xd4) = v;
            }
            break;
        }
        default:
            break;
    }

    {
        int *vtB = *reinterpret_cast<int **>(edi);
        int off2b = vtB[2];
        *reinterpret_cast<int *>(esi + off2b + 0xd8) = -1;
        reinterpret_cast<RadioButton *>(edi)->draw_item(prev, 0);
    }

    typedef void (__cdecl *Callback)(int);
    Callback cb = *reinterpret_cast<Callback *>(esi - 0xc);
    if (cb) {
        int *vtC = *reinterpret_cast<int **>(edi);
        int off2c = vtC[2];
        int newVal = *reinterpret_cast<int *>(esi + off2c + 0xd4);
        cb(newVal);
    }

    {
        int *vtD = *reinterpret_cast<int **>(edi);
        int off2d = vtD[2];
        int newVal = *reinterpret_cast<int *>(esi + off2d + 0xd4);
        reinterpret_cast<RadioButton *>(edi)->draw_item(newVal, 1);
    }

    {
        int *vtE = *reinterpret_cast<int **>(edi);
        int off1e = vtE[1];
        reinterpret_cast<GraphicWinX *>(off1e + edi)->soft_update();
    }

    return 1;
}
