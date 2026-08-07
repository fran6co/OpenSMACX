// 0x00442560  ?on_key_down@DiploWin@@QAEHH@Z  ->  ?on_key_down@DiploWin@@QAEHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// Slot 26 at +0x68 on the editBox sub-object embedded at +0xDA7C; it takes
// one int and returns int (propagated directly as this function's result).
class EditBoxVCall {
 public:
  virtual void slot000();
  virtual void slot001();
  virtual void slot002();
  virtual void slot003();
  virtual void slot004();
  virtual void slot005();
  virtual void slot006();
  virtual void slot007();
  virtual void slot008();
  virtual void slot009();
  virtual void slot010();
  virtual void slot011();
  virtual void slot012();
  virtual void slot013();
  virtual void slot014();
  virtual void slot015();
  virtual void slot016();
  virtual void slot017();
  virtual void slot018();
  virtual void slot019();
  virtual void slot020();
  virtual void slot021();
  virtual void slot022();
  virtual void slot023();
  virtual void slot024();
  virtual void slot025();
  virtual int slot026(int);
};

int DiploWin::on_key_down(int a1) {
    if (a1 == 0xd || a1 == 0x1b) {
        return 0;
    }
    void *edit_box = reinterpret_cast<char *>(this) + 0xda7c;
    return reinterpret_cast<EditBoxVCall *>(edit_box)->slot026(a1);
}
