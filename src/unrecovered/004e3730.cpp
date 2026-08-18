// ORIGINAL: 0x004E3730 sub_4e3730 0x004E3730-0x004E3888;0x0065C880-0x0065C895
// RULED-OUT: same shape as 0x004E3350's `ClassX::sub_4e3350` (own-stage list teardown at vtable 0x66eb20/0x66eb1c, then the same 0x6693d4/0x6693d0 pair the real `?close@ServiceStruct@@QAEXXZ` (0x004016c0, referenced from this function's unwind funclet at 0x0065C880) uses, on the member at `this-0x24`) - no SEH frame modelled, same precedent as 0x004C86D0/0x00538D10 and this function's sibling at 0x004E3350. 0.496 mnemonic similarity, first divergence at #2 (dropped SEH prologue).
// size      365 bytes
// prototype
// callers   3   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004E3791 0x004E37A5 0x004E37C4 0x004E3827 0x004E3839 0x004E3859

// Vtable shim: slot0 is the adjusted-`this` release call (arg 1), slot1 is
// the plain `this=self` teardown call taking the node's payload pointer.
class VCall {
public:
    virtual void slot0(int);
    virtual void slot1(void *);
};

class ClassX {
public:
    void sub_4e3730();
};

void ClassX::sub_4e3730() {
    char *const outer = reinterpret_cast<char *>(this);

    // ---- round 1: own-stage teardown ----
    {
        char *base = outer - 0x24;
        *reinterpret_cast<int *>(base) = 0x66eb20;
        {
            int *sub_vt = *reinterpret_cast<int **>(outer - 0x20);
            int off = sub_vt[1];
            *reinterpret_cast<int *>((outer - 0x20) + off) = 0x66eb1c;
        }

        int **head_slot = reinterpret_cast<int **>(base + 8);
        int **temp_slot = reinterpret_cast<int **>(base + 0xc);
        int *count_slot = reinterpret_cast<int *>(base + 0x10);

        if (*head_slot != 0) {
            if (*count_slot > 0) {
                int i = 0;
                do {
                    int *node = *head_slot;
                    *temp_slot = reinterpret_cast<int *>(node[3]);
                    void *payload = reinterpret_cast<void *>(node[2]);
                    reinterpret_cast<VCall *>(base)->slot1(payload);
                    if (payload) {
                        reinterpret_cast<VCall *>(payload)->slot0(1);
                    }
                    node[2] = 0;
                    if (*head_slot != 0) {
                        reinterpret_cast<VCall *>(*head_slot)->slot0(1);
                    }
                    *head_slot = *temp_slot;
                    ++i;
                } while (i < *count_slot);
            }
            *head_slot = 0;
            *reinterpret_cast<int *>(base + 0x14) = 0;
            *count_slot = 0;
        }
        *reinterpret_cast<int *>(base + 0x14) = 0;
    }

    // ---- round 2: base-stage teardown ----
    {
        char *base = outer - 0x24;
        *reinterpret_cast<int *>(base) = 0x6693d4;
        {
            int *sub_vt = *reinterpret_cast<int **>(outer - 0x20);
            int off = sub_vt[1];
            *reinterpret_cast<int *>((outer - 0x20) + off) = 0x6693d0;
        }

        int **head_slot = reinterpret_cast<int **>(base + 8);
        int **temp_slot = reinterpret_cast<int **>(base + 0xc);
        int *count_slot = reinterpret_cast<int *>(base + 0x10);

        if (*head_slot != 0) {
            if (*count_slot > 0) {
                int i = 0;
                do {
                    int *node = *head_slot;
                    *temp_slot = reinterpret_cast<int *>(node[3]);
                    void *payload = reinterpret_cast<void *>(node[2]);
                    reinterpret_cast<VCall *>(base)->slot1(payload);
                    if (payload) {
                        reinterpret_cast<VCall *>(payload)->slot0(1);
                    }
                    node[2] = 0;
                    if (*head_slot != 0) {
                        reinterpret_cast<VCall *>(*head_slot)->slot0(1);
                    }
                    *head_slot = *temp_slot;
                    ++i;
                } while (i < *count_slot);
            }
            *head_slot = 0;
            *reinterpret_cast<int *>(base + 0x14) = 0;
            *count_slot = 0;
        }
        *reinterpret_cast<int *>(base + 0x14) = 0;
    }
}
