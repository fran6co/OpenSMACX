// ORIGINAL: 0x00609300 ?init@Dialog@@QAEHHHHHH@Z 0x00609300-0x00609465
// TRIED: the two branches (`a5 != 0` / `a5 == 0`) each inline the SAME list-teardown loop already seen standalone as `SessionStruct::close` (0x00401CE0, src/alphanet.cpp) over the member at `this+0xbc`, reproduced with the same `VCall`-shim idiom as 0x004E3350/0x004E3730 rather than calling a shared helper (no helper functions allowed). The field at `list+0x18` (`this+0xd4`) is set to the register `edi` un-conditionally after the loop in the disassembly, which holds either `a1` (re-read from the stack, only when the loop actually ran) or the dead heap-pointer value from an earlier step (when the loop was skipped) - a register-reuse artifact, not a real value; written here as plain `a1` in the `a5 != 0` branch (matching Ghidra) since reproducing the stale-register case is not expressible from source. 0.89 mnemonic similarity, first divergence at #22.
// size      357 bytes
// prototype int (__thiscall ?init@Dialog@@QAEHHHHHH@Z)(Dialog* this, int, int, int, int, int)
// callers   3   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4620 0x00608F50
// indirect  0x0060938A 0x0060939C 0x006093B8 0x0060940E 0x00609422 0x0060943D

class VCall {
public:
    virtual void slot0(int);
    virtual void slot1(void *);
};

class Heap {
public:
    bool init(int);
};

class Dialog {
public:
    void close();
    int init(int a1, int a2, int a3, int a4, int a5);
};

int Dialog::init(int a1, int a2, int a3, int a4, int a5) {
    char *self = reinterpret_cast<char *>(this);
    close();

    *reinterpret_cast<int *>(self + 0x24) = a1;
    *reinterpret_cast<int *>(self + 0x2c) = a3;
    *reinterpret_cast<int *>(self + 0x30) = a4;
    *reinterpret_cast<int *>(self + 0x28) = a2;
    *reinterpret_cast<unsigned int *>(self + 0x20) |= 3;
    *reinterpret_cast<int *>(self + 0x1c) = a5;

    if (a5 != 0) {
        Heap *heap = reinterpret_cast<Heap *>(self + 4);
        if (!heap->init(a5)) {
            return 4;
        }

        char *list = self + 0xbc;
        if (*reinterpret_cast<int *>(self + 0xc4) != 0) {
            int count = *reinterpret_cast<int *>(list + 0x10);
            if (count > 0) {
                int i = 0;
                do {
                    int *node = *reinterpret_cast<int **>(list + 8);
                    *reinterpret_cast<int **>(list + 0xc) =
                        reinterpret_cast<int *>(node[3]);
                    void *payload = reinterpret_cast<void *>(node[2]);
                    reinterpret_cast<VCall *>(list)->slot1(payload);
                    if (payload) {
                        reinterpret_cast<VCall *>(payload)->slot0(1);
                    }
                    reinterpret_cast<int *>(*reinterpret_cast<int **>(list + 8))[2] = 0;
                    if (*reinterpret_cast<int *>(list + 8) != 0) {
                        reinterpret_cast<VCall *>(*reinterpret_cast<void **>(list + 8))->slot0(1);
                    }
                    *reinterpret_cast<int **>(list + 8) = *reinterpret_cast<int **>(list + 0xc);
                    ++i;
                } while (i < *reinterpret_cast<int *>(list + 0x10));
            }
            *reinterpret_cast<int *>(list + 8) = 0;
            *reinterpret_cast<int *>(list + 0x14) = 0;
            *reinterpret_cast<int *>(list + 0x10) = 0;
        }
        *reinterpret_cast<int *>(list + 0x18) = a1;
        *reinterpret_cast<int *>(list + 0x14) = 0;
        return 0;
    }

    char *list = self + 0xbc;
    if (*reinterpret_cast<int *>(self + 0xc4) != 0) {
        int count = *reinterpret_cast<int *>(list + 0x10);
        if (count > 0) {
            int i = 0;
            do {
                int *node = *reinterpret_cast<int **>(list + 8);
                *reinterpret_cast<int **>(list + 0xc) = reinterpret_cast<int *>(node[3]);
                void *payload = reinterpret_cast<void *>(node[2]);
                reinterpret_cast<VCall *>(list)->slot1(payload);
                if (payload) {
                    reinterpret_cast<VCall *>(payload)->slot0(1);
                }
                reinterpret_cast<int *>(*reinterpret_cast<int **>(list + 8))[2] = 0;
                if (*reinterpret_cast<int *>(list + 8) != 0) {
                    reinterpret_cast<VCall *>(*reinterpret_cast<void **>(list + 8))->slot0(1);
                }
                *reinterpret_cast<int **>(list + 8) = *reinterpret_cast<int **>(list + 0xc);
                ++i;
            } while (i < *reinterpret_cast<int *>(list + 0x10));
        }
        *reinterpret_cast<int *>(list + 8) = 0;
        *reinterpret_cast<int *>(list + 0x14) = 0;
        *reinterpret_cast<int *>(list + 0x10) = 0;
    }
    *reinterpret_cast<int *>(list + 0x14) = 0;
    *reinterpret_cast<int *>(list + 0x18) = 0;
    return 0;
}
