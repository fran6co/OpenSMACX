// ORIGINAL: 0x005F9570 BYTE_EXACT
// name      ?on_right_up@PullDown@@QAEXHH@Z
// size      18 bytes
// spans     0x005F9570-0x005F9582
// prototype void (__thiscall ?on_right_up@PullDown@@QAEXHH@Z)(PullDown* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005F957C
// 0x005F9570  ?on_right_up@PullDown@@QAEXHH@Z  ->  ?on_right_up@PullDown@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Pass a right-button release straight through to virtual slot 21.
Original Offset: 005F9570
Return Value: n/a
Status: Complete
*/
// A local vtable shim. The emitted scaffolding declares one, but its slots
// are all nullary and this dispatch is not, and its slot count is derived
// from `call` sites while this reaches the vtable another way. Declaring a
// differently-named class with the right arity is the technique the emitter
// itself uses (emit_translation_unit.py:516); only the CALLING SHAPE is
// compared, because the callee's displacement is a relocation and is masked.
class RightVCall {
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
  virtual void slot021(int, int);
};

void PullDown::on_right_up(int a1, int a2) {
    reinterpret_cast<RightVCall *>(this)->slot021(a1, a2);
}
