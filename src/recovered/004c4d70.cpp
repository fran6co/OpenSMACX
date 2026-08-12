// ORIGINAL: 0x004C4D70 BYTE_EXACT
// name      sub_4c4d70
// size      54 bytes
// spans     0x004C4D70-0x004C4DA6
// prototype 
// callers   0   call targets   1
// 0x004C4D70  sub_4c4d70  ->  _sub_4c4d70
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// SIGNATURE CHANGED FROM SCAFFOLDING - see PROPOSALS in report. The
// disassembly opens `push esi; mov esi, ecx` with no stack-parameter
// reads at all: `ecx` on entry is `this`, not garbage, so the fixed
// `extern "C" int __cdecl sub_4c4d70()` head cannot be reached and this
// is a __thiscall constructor-shaped member instead. `eax = esi` before
// `ret` is the ABI's implicit `this`-return, not a written statement.
class Record4C4D70 {
    void *vtable;
    char field4[4];
    int field8;
    int fieldC;
    int field10;
    int field14;
    int field18;
    int field1C;
public:
    Record4C4D70();
};

Record4C4D70::Record4C4D70() {
    vtable = g_0066e098;
    memset(field4, 0, sizeof(field4));
    fieldC = 0;
    field18 = 0;
    field1C = 0;
    field10 = 0;
    field14 = 0;
    field8 = 0x7f;
}
