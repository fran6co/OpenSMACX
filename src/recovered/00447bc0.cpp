// ORIGINAL: 0x00447BC0 BYTE_EXACT
// name      ?begin@GAmbience@@QAEXXZ
// size      29 bytes
// spans     0x00447BC0-0x00447BDD
// prototype void (__thiscall ?begin@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447BC5 0x00447BD5
// 0x00447BC0  ?begin@GAmbience@@QAEXXZ  ->  ?begin@GAmbience@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Start the geological ambience: run the shared slot 7 step, record
         the mode on the object, and announce it through slot 67.
Original Offset: 00447BC0
Return Value: n/a
Status: Complete
*/
// Ambience's slot 67 at +0x10C takes one int - a per-subclass mode id.
// The scaffolding's own shim declares EVERY slot nullary, so a local one
// with the right arity is needed.
class BeginVCall {
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
  virtual void slot044();
  virtual void slot045();
  virtual void slot046();
  virtual void slot047();
  virtual void slot048();
  virtual void slot049();
  virtual void slot050();
  virtual void slot051();
  virtual void slot052();
  virtual void slot053();
  virtual void slot054();
  virtual void slot055();
  virtual void slot056();
  virtual void slot057();
  virtual void slot058();
  virtual void slot059();
  virtual void slot060();
  virtual void slot061();
  virtual void slot062();
  virtual void slot063();
  virtual void slot064();
  virtual void slot065();
  virtual void slot066();
  virtual void slot067(int);
};

void GAmbience::begin() {
    reinterpret_cast<BeginVCall *>(this)->slot007();
    // Alone among the six begin() overrides, this one also stores its mode.
    field_68_ = 0x18;
    reinterpret_cast<BeginVCall *>(this)->slot067(0x18);
}
