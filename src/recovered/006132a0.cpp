// ORIGINAL: 0x006132A0
// 0x006132A0  ?calculate_dimensions@Dialogs@@QAEHXZ  ->  ?calculate_dimensions@Dialogs@@QAEHXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

class RadioButton { public: int calculate_dimensions(); };
class ListBox { public: int calculate_dimensions(); };
class CheckBox { public: int calculate_dimensions(); };
class SpriteBox { public: int calculate_dimensions(); };
class EditGroup { public: int calculate_dimensions(); };

int Dialogs::calculate_dimensions() {
    char *self = reinterpret_cast<char *>(this);
    switch (*reinterpret_cast<int *>(self + 0x180)) {
    case 0x10:
        return reinterpret_cast<RadioButton *>(self + 0x44)->calculate_dimensions();
    case 2:
        return reinterpret_cast<ListBox *>(self)->calculate_dimensions();
    case 1:
        return reinterpret_cast<CheckBox *>(self + 0x58)->calculate_dimensions();
    case 8:
        return reinterpret_cast<SpriteBox *>(self + 0x70)->calculate_dimensions();
    case 4:
        return reinterpret_cast<EditGroup *>(self + 0xf8)->calculate_dimensions();
    default:
        *reinterpret_cast<int *>(
            self + 0x5c + *reinterpret_cast<int *>(*reinterpret_cast<int *>(self) + 8)) = 0;
        *reinterpret_cast<int *>(
            self + 0x60 + *reinterpret_cast<int *>(*reinterpret_cast<int *>(self) + 8)) = 0;
        return 0;
    }
}
