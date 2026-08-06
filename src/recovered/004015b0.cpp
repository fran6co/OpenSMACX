// 0x004015B0  ?seek_pos@StringStruct@@QAEHH@Z  ->  ?seek_pos@StringStruct@@QAEHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

typedef int (__cdecl *AbsFnT)(int);

int StringStruct::seek_pos(int a1) {
    AbsFnT abs_fn = reinterpret_cast<AbsFnT>(&abs);
    if (a1 > entry_count_ - 1) {
        return 0;
    }
    current_ = head_;
    if (a1 < 0) {
        if (abs_fn(a1) > entry_count_) {
            return 0;
        }
        int i = abs_fn(a1);
        if (i > 0) {
            do {
                i--;
                current_ = reinterpret_cast<StringStructEntry *>(reinterpret_cast<int *>(current_)[4]);
            } while (i != 0);
        }
        current_position_ = a1 + entry_count_;
        return 1;
    } else {
        if (a1 > 0) {
            int i = a1;
            do {
                i--;
                current_ = reinterpret_cast<StringStructEntry *>(reinterpret_cast<int *>(current_)[3]);
            } while (i != 0);
        }
        current_position_ = a1;
        return 1;
    }
}
