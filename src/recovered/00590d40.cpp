// ORIGINAL: 0x00590D40 BYTE_EXACT
// name      sub_590d40
// size      28 bytes
// spans     0x00590D40-0x00590D5C
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// 0x00590D40  sub_590d40  ->  _sub_590d40
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

typedef void (__cdecl *Handler)(int);
struct Slot { Handler fn; uint32_t reserved; };

class S_00590D40 { public:
    uint8_t pad0_[2];
    uint8_t index_;
    uint8_t arg_;

    void Dispatch();
};

void S_00590D40::Dispatch() {
    Slot *entry = reinterpret_cast<Slot *>(g_00945a60) + index_;
    if (entry->fn) {
        entry->fn(arg_);
    }
}
