// ORIGINAL: 0x00633010 sub_633010 0x00633010-0x00633152;0x00663730-0x006637AF
// TRIED: the tail at 0x00663730-0x006637AF is a chain of EH unwind funclets (push -1 / __CxxFrameHandler prologue) protecting the partial destruction of several embedded members if an earlier member's destructor throws. Reproducing that would need this class's exact multiple/virtual-inheritance layout (a ListBox member whose Dialog and GraphicWin virtual bases are destroyed separately by the most-derived class, matching how src/stringstruct.cpp's StringList::destroy documents the same shape for its own virtual base) modelled with real inheritance so the compiler regenerates the funclets itself; landing the flat, unprotected call sequence instead; the string-list member at +0x36a8 is StringStruct::close_with_tables (0x00401060) inlined at the call site with two different literal table pairs, remove_all (0x00402970) left as a real call - matches src/stringstruct.cpp's close_with_tables/remove_all pair.
// size      449 bytes
// prototype
// callers   1   call targets   8
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00402970 0x005D45B0 0x005D4DD0 0x00608E10 0x00609EC0 0x00616200 0x0062C010 0x0063BD10
// PROPOSAL: extern "C" int __cdecl sub_633010() -> a __thiscall member, no
//           free params - evidence: `mov esi, ecx` at entry with every
//           access reached from esi thereafter. Expressed via an introduced
//           `Sub633010Shim` class, as 0x005CC710 does.

class Sub633010Shim {
 public:
  void run();
};

void Sub633010Shim::run() {
  char *base = reinterpret_cast<char *>(this);

  // Install this object's own vtable(s) before tearing members down.
  *reinterpret_cast<unsigned int *>(base + 0x0) = 0x670ddc;
  *reinterpret_cast<unsigned int *>(base + 0x444) = 0x670dd4;

  typedef void(__fastcall * ThisOnlyFn)(void *);
  ((ThisOnlyFn)sub_63bd10)(this);

  // Embedded list-like member at +0x36a8: close under its derived tables,
  // then under its own base tables (StringStruct::close_with_tables,
  // inlined at the call site, with remove_all left as a real call).
  char *stringObj = base + 0x36a8;
  *reinterpret_cast<unsigned int *>(stringObj) = 0x66eae8;
  {
    unsigned int *vbtable = *reinterpret_cast<unsigned int **>(stringObj + 4);
    unsigned int disp = vbtable[1];
    *reinterpret_cast<unsigned int *>(stringObj + 4 + disp) = 0x66eae4;
  }
  ((ThisOnlyFn)fn_00402970)(stringObj);
  *reinterpret_cast<unsigned int *>(stringObj + 0x14) = 0;

  *reinterpret_cast<unsigned int *>(stringObj) = 0x669408;
  {
    unsigned int *vbtable = *reinterpret_cast<unsigned int **>(stringObj + 4);
    unsigned int disp = vbtable[1];
    *reinterpret_cast<unsigned int *>(stringObj + 4 + disp) = 0x669404;
  }
  ((ThisOnlyFn)fn_00402970)(stringObj);
  *reinterpret_cast<unsigned int *>(stringObj + 0x14) = 0;

  // The list's virtual base, further along the object: hand it back its own
  // vtable and republish the pending-allocation owner (matches
  // StringList::destroy's tail in src/stringstruct.cpp).
  char *virtualBase = base + 0x36e0;
  unsigned int owner = *reinterpret_cast<unsigned int *>(virtualBase + 4);
  *reinterpret_cast<unsigned int *>(virtualBase) = 0x6693ac;
  *reinterpret_cast<unsigned int *>(0x9b3374) = owner;

  reinterpret_cast<PushButton *>(base + 0x2ba4)->~PushButton();
  reinterpret_cast<PushButton *>(base + 0x20a4)->~PushButton();
  reinterpret_cast<PushButton *>(base + 0x15a4)->~PushButton();

  reinterpret_cast<ListBox *>(base + 0xa98)->~ListBox();
  reinterpret_cast<Dialog *>(base + 0x14b0)->~Dialog();
  reinterpret_cast<GraphicWin *>(base + 0xa98)->~GraphicWin();

  reinterpret_cast<Heap *>(base + 0xa3c)->shutdown();
  reinterpret_cast<Time *>(base + 0xa14)->~Time();

  reinterpret_cast<GraphicWin *>(base)->~GraphicWin();
}
