// ORIGINAL: 0x0041E510 ?on_iface_button_clicked@BaseWin@@QAEXH@Z 0x0041E510-0x0041E52D BYTE_EXACT
// size      29 bytes
// prototype void (__thiscall ?on_iface_button_clicked@BaseWin@@QAEXH@Z)(BaseWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0041E523
// 0x0041E510  ?on_iface_button_clicked@BaseWin@@QAEXH@Z  ->  ?on_iface_button_clicked@BaseWin@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Forward an interface button click to the enclosing window.
Original Offset: 0041E510
Return Value: n/a
Status: Complete
*/
// `this - 0xa14` again walks OUT of this subobject into the enclosing one.
// The scaffolding's shim declares slot 44 nullary and this call passes an int.
class VCallArg1 {
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
  virtual void slot026();
  virtual void slot027();
  virtual void slot028();
  virtual void slot029();
  virtual void slot030();
  virtual void slot031();
  virtual void slot032();
  virtual void slot033();
  virtual void slot034();
  virtual void slot035();
  virtual void slot036();
  virtual void slot037();
  virtual void slot038();
  virtual void slot039();
  virtual void slot040();
  virtual void slot041();
  virtual void slot042();
  virtual void slot043();
  virtual void slot044(int);
};

void BaseWin::on_iface_button_clicked(int a1) {
    VCallArg1 *const enclosing = reinterpret_cast<VCallArg1 *>(
        reinterpret_cast<char *>(this) - 0xa14);
    enclosing->slot044(a1);
}
