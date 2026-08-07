// 0x004429C0  ?done_deal@DiploWin@@QAEXXZ  ->  ?done_deal@DiploWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Conclude the deal: set both parties to state 3, clear the two
         pending slots, and repaint.
Original Offset: 004429C0
Return Value: n/a
Status: Complete
*/
// The shim must be at FILE SCOPE. A local class inside the function is
// C2599 in this dialect - every declared member needs an in-class body -
// and giving virtual slots bodies emits extra .text symbols the verifier
// then rejects.
class DoneVCall {
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
};

void DiploWin::done_deal() {
    volatile uint32_t *const fields = reinterpret_cast<volatile uint32_t *>(this);
    fields[0xa28 / 4] = 3;
    fields[0xa24 / 4] = 3;
    fields[0xa20 / 4] = 0;
    fields[0xa1c / 4] = 0;
    // Last statement, so VC6 folds the virtual call and the implicit return
    // into the original's tail `jmp dword ptr [eax+0xf8]`.
    reinterpret_cast<DoneVCall *>(this)->slot062();
}
