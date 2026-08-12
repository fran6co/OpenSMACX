// ORIGINAL: 0x00608770 BYTE_EXACT
// 0x00608770  ?kill_entry@StringList@@QAEXPAUStringStruct@@@Z  ->  ?kill_entry@StringList@@QAEXPAUStringStruct@@@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

static inline void kill_entry_free_if_set(int32_t value) {
    if (value != 0) {
        free(reinterpret_cast<void *>(value));
    }
}

void StringList::kill_entry(StringStruct *a1) {
    if (a1->virtual_base_abi_word_ != 0 && allocator_ == 0) {
        kill_entry_free_if_set(a1->virtual_base_abi_word_);
        a1->virtual_base_abi_word_ = 0;
    }
}
