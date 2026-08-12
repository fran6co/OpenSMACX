// ORIGINAL: 0x004457C0 BYTE_EXACT
// name      ?input_callback@@YAXXZ
// size      19 bytes
// spans     0x004457C0-0x004457D3
// prototype 
// callers   0   call targets   0
// 0x004457C0  ?input_callback@@YAXXZ  ->  ?input_callback@@YAXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Dispatch the input callback to the live object's virtual slot 79,
         when there is one.
Original Offset: 004457C0
Return Value: n/a
Status: Complete
*/
// A local vtable shim. The emitted scaffolding declares one, but its slots
// are all nullary and this dispatch is not, and its slot count is derived
// from `call` sites while this reaches the vtable another way. Declaring a
// differently-named class with the right arity is the technique the emitter
// itself uses (emit_translation_unit.py:516); only the CALLING SHAPE is
// compared, because the callee's displacement is a relocation and is masked.
class InputVCall {
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
  virtual void slot067();
  virtual void slot068();
  virtual void slot069();
  virtual void slot070();
  virtual void slot071();
  virtual void slot072();
  virtual void slot073();
  virtual void slot074();
  virtual void slot075();
  virtual void slot076();
  virtual void slot077();
  virtual void slot078();
  virtual void slot079();
};

void __cdecl input_callback() {
    const int object = *g_0074daa0;
    if (object != 0) {
        reinterpret_cast<InputVCall *>(object)->slot079();
    }
}
