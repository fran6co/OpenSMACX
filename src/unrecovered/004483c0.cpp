// ORIGINAL: 0x004483C0 FILE
// UNRECOVERABLE: a five-byte linker thunk, `jmp 0x4C7670` (e9 ab f2 07 00)
//                and nothing else. No C++ source form emits a bare jump under
//                a decorated destructor name - the linker synthesises these
//                when a call crosses a section it wants a stub for - and the
//                emitter refuses it for the same reason from the other side:
//                "the emitted C++ symbol would name a synthesised class".
//                The jump target 0x004C7670 is the real Ambience destructor
//                and is catalogued separately, so nothing is lost by leaving
//                this uncovered.
// name      j_??1Ambience@@QAE@XZ
// size      5 bytes
// spans     0x004483C0-0x004483C5
// prototype void (__thiscall j_??1Ambience@@QAE@XZ)(Ambience* this)
// callers   1   call targets   0
// kind      thunk
// flags     hidden;thunk;sp_ready;purged_ok
// calls     (none)
// placeholder - not yet decompiled
// To start: tools/decomp_status.py --work 0x004483C0

// BODY GOES HERE.
