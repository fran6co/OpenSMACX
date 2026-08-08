// 0x004C38E0  sub_4c38e0  ->  _sub_4c38e0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

class Sub4c38e0VCall {
 public:
  virtual void slot000();
  virtual void slot001(int);
};

// Named for this address rather than `Win`. The emitter defines the real
// `Win` with its layout in this unit, so a second definition here is a
// C2011 redefinition - and the emitter's `Win` has no `hide()`, because the
// call was always spelled through this shim and so was never catalogued as
// one of its methods. The class name is masked at comparison, so the call
// encodes identically either way.
class Sub4c38e0Win {
 public:
  void hide();
};

extern "C" void __cdecl sub_4c38e0() {
    reinterpret_cast<ButtonGroup *>(g_007cd12c)->set(0x3ef, 0);
    char *obj = *reinterpret_cast<char **>(g_007d3c40);
    reinterpret_cast<Sub4c38e0VCall *>(
        obj + *reinterpret_cast<int *>(*reinterpret_cast<char **>(obj) + 4))
        ->slot001(0);
    reinterpret_cast<Sub4c38e0Win *>(g_008e9f60)->hide();
}
