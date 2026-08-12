// ORIGINAL: 0x004C5780 BYTE_EXACT
// name      ??1Midi_Device@@QAE@XZ
// size      19 bytes
// spans     0x004C5780-0x004C5793
// prototype void (__thiscall ??1Midi_Device@@QAE@XZ)(Midi_Device* this)
// callers   0   call targets   0
// 0x004C5780  ??1Midi_Device@@QAE@XZ  ->  ??1Midi_Device@@QAE@XZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Re-install the shared device vtable and tear down the owned
         sub-object at +0x14 through its own virtual slot 4.
Original Offset: 004C5780
Return Value: n/a
Status: Complete
*/
class Midi_DeviceOwned {
 public:
  virtual void slot000();
  virtual void slot001();
  virtual void slot002();
  virtual void slot003();
  virtual void slot004();
};

Midi_Device::~Midi_Device() {
    *reinterpret_cast<int **>(this) = g_0066e098;
    Midi_DeviceOwned *const owned = reinterpret_cast<Midi_DeviceOwned *>(field_14_);
    if (owned) {
        owned->slot004();
    }
}
