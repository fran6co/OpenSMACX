// CLAIM DROPPED 2026-08-16, and the body is untouched. `Buffer` became
// polymorphic - `virtual ~Buffer()` and `virtual surface_lost()` in place
// of the explicit `LPVOID vtable_` member - which is what the image has
// and what lets a slot-1 call be spelled `surface_lost()`. The scaffold
// cannot lay out a class with virtuals, so it emits `Buffer` opaque here
// and this body stops compiling: `C2065: 'surface_' : undeclared`.
//
// The body is not wrong and was byte-exact the day before. What is gone
// is the ability to CHECK it from a scaffold, so the claim goes with it -
// state is measured, not remembered. It comes back the day this body is
// promoted into the file that owns its class, which is where it belongs.
// ORIGINAL: 0x0042A260
// name      ?UNK3@Datalink@@QAEXIH@Z
// size      95 bytes
// spans     0x0042A260-0x0042A2BF
// prototype void (__thiscall ?UNK3@Datalink@@QAEXIH@Z)(Datalink* this, DatalinkID, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0042A2C0 0x00609FA0
// indirect  0x0042A2B3
// 0x0042A260  ?UNK3@Datalink@@QAEXIH@Z  ->  ?UNK3@Datalink@@QAEXIH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void Datalink::UNK3(unsigned int a1, int a2) {
    set_cat(a1, 0);
    facilityID_ = a2;
    reinterpret_cast<ListBox*>(reinterpret_cast<char*>(this) + 0xF628)->set_selected_id(
        *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x29DC) * 10000 + a2);
    reinterpret_cast<VCall*>(reinterpret_cast<char*>(this) + 0xF628 +
        *reinterpret_cast<int*>(*reinterpret_cast<int*>(
            reinterpret_cast<char*>(this) + 0xF628) + 4))->slot062();
}
