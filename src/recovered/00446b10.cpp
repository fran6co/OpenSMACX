// ORIGINAL: 0x00446B10 ?engine_pitch@FX@@QAEXH@Z 0x00446B10-0x00446B37 BYTE_EXACT
// size      39 bytes
// prototype void (__thiscall ?engine_pitch@FX@@QAEXH@Z)(FX* this, int)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00446B2D
// 0x00446B10  ?engine_pitch@FX@@QAEXH@Z  ->  ?engine_pitch@FX@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Set the engine pitch on the currently selected effect.
Original Offset: 00446B10
Return Value: n/a
Status: Complete
*/
// The shim must be at FILE SCOPE. A local class inside the function is
// C2599 in this dialect - every declared member needs an in-class body -
// and giving virtual slots bodies emits extra .text symbols the verifier
// then rejects.
class PitchVCall {
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
  virtual void slot038(int);
};

void FX::engine_pitch(int a1) {
    // `field_28EC_` is declared uint32_t but the original's `test eax,eax; jl`
    // is a SIGNED test, so it has to be read through a signed local; an
    // unsigned `>= 0` is trivially true and the branch disappears.
    const int32_t index = static_cast<int32_t>(field_28EC_);
    if (index >= 0) {
        reinterpret_cast<PitchVCall *>(
            reinterpret_cast<char *>(this) + index * 0x6C)->slot038(a1);
    }
}
