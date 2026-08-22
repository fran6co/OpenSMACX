// ORIGINAL: 0x004E3350 sub_4e3350 0x004E3350-0x004E34A8;0x0065C7F0-0x0065C805
// TRIED: no SEH-frame modelling attempted - the function installs a real fs:[0] frame (push -1; push 0x65c7fb; ...) whose unwind funclet at 0x0065C7F0 calls the real `SessionStruct::close` (0x00401ce0, see src/alphanet.cpp) on `this-0x1c`, matching the precedent at 0x004C86D0/0x00538D10: none of the calls in the body throws in practice, so the frame is unreachable and is dropped. `ecx` does real work (`lea edi,[ecx-0x38]`, no stack access) so this is __thiscall, written as `ClassX::sub_4e3350()` per the 0x004E3300 precedent, which already calls `sub->sub_4e3350()` on the same class. The body is TWO back-to-back rounds of the same list-teardown loop (own-stage vtable 0x66eae8/0x66eae4 first, then the same 0x669408/0x669404 pair `SessionStruct::close` itself uses), both operating on the SAME member at `this-0x38`; reproduced as two literal blocks (not a loop, matching the original's duplication) via a `VCall` shim for the two indirect calls per node (slot1 with `this=self,payload`, slot0 with an adjusted `this` and `arg=1`), simplified to skip the raw adjustor-thunk arithmetic (same simplification already accepted at 0x0062E010's `Net::close`). 0.496 mnemonic similarity - the dropped SEH prologue alone accounts for most of the gap - not chased further.
// size      365 bytes
// prototype
// callers   3   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004E33B1 0x004E33C5 0x004E33E4 0x004E3447 0x004E3459 0x004E3479

// Vtable shim: slot0 is the adjusted-`this` release call (arg 1), slot1 is
// the plain `this=self` teardown call taking the node's payload pointer.
class VCall {
public:
    virtual void slot0(int);
    virtual void slot1(void *);
};

class ClassX {
public:
    void sub_4e3350();
};

void ClassX::sub_4e3350() {
    char *const outer = reinterpret_cast<char *>(this);

    // ---- round 1: own-stage teardown ----
    {
        char *base = outer - 0x38;
        *reinterpret_cast<int *>(base) = 0x66eae8;
        {
            int *sub_vt = *reinterpret_cast<int **>(outer - 0x34);
            int off = sub_vt[1];
            *reinterpret_cast<int *>((outer - 0x34) + off) = 0x66eae4;
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
        char *base = outer - 0x38;
        *reinterpret_cast<int *>(base) = 0x669408;
        {
            int *sub_vt = *reinterpret_cast<int **>(outer - 0x34);
            int off = sub_vt[1];
            *reinterpret_cast<int *>((outer - 0x34) + off) = 0x669404;
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
