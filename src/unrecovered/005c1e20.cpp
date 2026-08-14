// ORIGINAL: 0x005C1E20 FILE
// name      ?init@Fractal@@QAEXXZ
// size      242 bytes
// spans     0x005C1E20-0x005C1F12
// prototype void (__thiscall ?init@Fractal@@QAEXXZ)(Fractal* this)
// callers   ?   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0064601D
// RULED-OUT: nothing tried beyond the direct transcription below - landed
//            at first compiling candidate given batch size. Mnemonic
//            similarity 0.80, first divergence is prologue register-save
//            scheduling (`sub esp,N` vs a `push`, #2).
// NOTE: the weighted-sum loop at 0x005C1E9A computes a `total` (an int
//       accumulator seeded from the byte at [row][col], shifted left 2,
//       then summed against 8 entries from a fixed weight table at
//       0x0066EFC0 through a second byte table at [this+0x17D]) whose
//       result is NEVER READ - `mov esi,[ebp-4]` right after the loop
//       discards it and reloads the row pointer instead. Confirmed by
//       re-tracing the disassembly by hand: Ghidra's own decompilation of
//       this block is wrong (it drops the weight-table read and the
//       accumulation entirely, leaving a no-op pointer-increment loop),
//       so the loop below is written from the raw bytes, not from the
//       Ghidra hypothesis in the brief. Kept the apparently-dead
//       computation rather than eliminating it, since VC6 did not
//       eliminate it either - the two are the same choice.

extern "C" int __cdecl rand();

class Fractal {
public:
    void init();
};

void Fractal::init() {
    char *base = reinterpret_cast<char *>(this);
    char *record = base + 0x169;
    int outer = 0x12;
    do {
        for (int i = 0; i < 0x12; ++i) {
            int r = rand();
            r = r & 0x8000000f;
            if (r < 0) {
                r = ((r - 1) | 0xfffffff0) + 1;
            }
            record[i] = (char)r;
        }
        record[0x12] = record[0];
        record += 0x13;
        outer -= 1;
    } while (outer != 0);

    char *fill = base + 0x2bf;
    do {
        *fill = fill[-0x156];
        fill += 1;
    } while ((fill - base - 0x2bf) < 0x12);

    char *rowPtr = base + 0x169;
    int row = 0;
    do {
        int col = 0;
        do {
            int total = (int)(signed char)rowPtr[col] * 4;
            const int *weight = reinterpret_cast<const int *>(0x66efc0);
            do {
                int w = *weight;
                weight += 1;
                int offset = row + w * 20 + col;
                total += (signed char)base[offset + 0x17d];
            } while (reinterpret_cast<int>(weight) <= 0x66efdc);
            int dest = row + col;
            char value = rowPtr[col];
            base[dest] = value;
            col += 1;
        } while (col < 0x11);
        row += 0x13;
        rowPtr += 0x13;
    } while (row < 0x131);

    int n = 0x11;
    char *p = base;
    do {
        p[0x10] = *p;
        p += 0x13;
        n -= 1;
    } while (n != 0);

    char *tail = base + 0x130;
    do {
        *tail = tail[-0x130];
        tail += 1;
    } while ((tail - base - 0x130) < 0x11);
}
