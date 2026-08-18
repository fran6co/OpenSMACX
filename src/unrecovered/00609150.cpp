// ORIGINAL: 0x00609150 ?init@Dialog@@QAEHXZ 0x00609150-0x006092FB
// RULED-OUT: MISMATCH #1 mov/push - the three inlined entry-list clears (originally StringStruct::remove_all-shaped) written out by hand with a locally redefined `VCall` shim (slot000(int), slot001(void*)) for the notify/destroy virtual calls; stack frame layout differs from instruction 1 on.
// size      427 bytes
// prototype int (__thiscall ?init@Dialog@@QAEHXZ)(Dialog* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4620
// indirect  0x00609197 0x006091A9 0x006091C5 0x0060922D 0x00609241 0x0060925C 0x006092A6 0x006092B8 0x006092D4
// To start: tools/decomp_status.py --work 0x00609150

class VCall { public:
    virtual void slot000(int);
    virtual void slot001(void *);
};

int Dialog::init() {
    if (static_cast<int32_t>(field_1C_) < 0) {
        return 1;
    }

    if (entry_head_ != 0) {
        int index = 0;
        if (0 < entry_count_) {
            do {
                DialogEntry *const entry = entry_head_;
                current_entry_ = entry->next;
                void *const payload = entry->payload;
                reinterpret_cast<VCall *>(&field_BC_)->slot001(payload);
                if (payload) {
                    uint32_t *const payload_vtable = *reinterpret_cast<uint32_t *const *>(payload);
                    void *const subobject = reinterpret_cast<char *>(payload) + payload_vtable[1];
                    reinterpret_cast<VCall *>(subobject)->slot000(1);
                }
                entry->payload = 0;
                DialogEntry *const head = entry_head_;
                if (head) {
                    uint32_t *const head_vtable = *reinterpret_cast<uint32_t *const *>(head);
                    void *const subobject = reinterpret_cast<char *>(head) + head_vtable[1];
                    reinterpret_cast<VCall *>(subobject)->slot000(1);
                }
                ++index;
                entry_head_ = current_entry_;
            } while (index < entry_count_);
        }
        entry_head_ = 0;
        entry_position_ = 0;
        entry_count_ = 0;
    }
    entry_position_ = 0;

    if (field_1C_ == 0) {
        if (entry_head_ != 0) {
            int index = 0;
            if (0 < entry_count_) {
                do {
                    DialogEntry *const entry = entry_head_;
                    current_entry_ = entry->next;
                    void *const payload = entry->payload;
                    reinterpret_cast<VCall *>(&field_BC_)->slot001(payload);
                    if (payload) {
                        uint32_t *const payload_vtable = *reinterpret_cast<uint32_t *const *>(payload);
                        void *const subobject = reinterpret_cast<char *>(payload) + payload_vtable[1];
                        reinterpret_cast<VCall *>(subobject)->slot000(1);
                    }
                    entry->payload = 0;
                    DialogEntry *const head = entry_head_;
                    if (head) {
                        uint32_t *const head_vtable = *reinterpret_cast<uint32_t *const *>(head);
                        void *const subobject = reinterpret_cast<char *>(head) + head_vtable[1];
                        reinterpret_cast<VCall *>(subobject)->slot000(1);
                    }
                    ++index;
                    entry_head_ = current_entry_;
                } while (index < entry_count_);
            }
            entry_head_ = 0;
            entry_position_ = 0;
            entry_count_ = 0;
        }
        entry_position_ = 0;
        field_D4_ = 0;
        return 0;
    }

    if (heap_.init(static_cast<int>(field_1C_))) {
        return 4;
    }

    if (entry_head_ != 0) {
        int index = 0;
        if (0 < entry_count_) {
            do {
                DialogEntry *const entry = entry_head_;
                current_entry_ = entry->next;
                void *const payload = entry->payload;
                reinterpret_cast<VCall *>(&field_BC_)->slot001(payload);
                if (payload) {
                    uint32_t *const payload_vtable = *reinterpret_cast<uint32_t *const *>(payload);
                    void *const subobject = reinterpret_cast<char *>(payload) + payload_vtable[1];
                    reinterpret_cast<VCall *>(subobject)->slot000(1);
                }
                entry->payload = 0;
                DialogEntry *const head = entry_head_;
                if (head) {
                    uint32_t *const head_vtable = *reinterpret_cast<uint32_t *const *>(head);
                    void *const subobject = reinterpret_cast<char *>(head) + head_vtable[1];
                    reinterpret_cast<VCall *>(subobject)->slot000(1);
                }
                ++index;
                entry_head_ = current_entry_;
            } while (index < entry_count_);
        }
        entry_head_ = 0;
        entry_position_ = 0;
        entry_count_ = 0;
    }
    field_D4_ = reinterpret_cast<uint32_t>(&heap_);
    entry_position_ = 0;
    return 0;
}
