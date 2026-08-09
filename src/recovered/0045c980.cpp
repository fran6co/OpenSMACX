// ORIGINAL: 0x0045C980
// 0x0045C980  ?on_mouse_move@MainInterface@@QAEXHH@Z  ->  ?on_mouse_move@MainInterface@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

class OnMouseMoveVCall {
 public:
  virtual void slot000();
  virtual void slot001();
  virtual void slot002();
  virtual void slot003();
  virtual void slot004();
  virtual void slot005();
  virtual void slot006();
  virtual void slot007();
  virtual void slot008(int, int);
};

void MainInterface::on_mouse_move(int a1, int a2) {
    if (*g_0068f21c != 1) {
        int mid = g_007ae778[*g_007d392c];
        int obj = g_007ae7f0[mid];
        if (obj != 0) {
            reinterpret_cast<OnMouseMoveVCall *>(obj)->slot008(a1, a2);
        }
    }
}
