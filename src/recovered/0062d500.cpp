// ORIGINAL: 0x0062D500 BYTE_EXACT
// name      ?jackal_close@@YAXXZ
// size      98 bytes
// spans     0x0062D500-0x0062D562
// prototype 
// callers   1   call targets   15
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DF580 0x005F04E0 0x005FECF0 0x00604680 0x0060E5D0 0x0060FD60 0x00614E30 0x00616890 0x00616950 0x00619610 0x0062D100 0x006339B0 0x00635750 0x0063B930 0x0063CEF0
// 0x0062D500  ?jackal_close@@YAXXZ  ->  ?jackal_close@@YAXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// Every `close_class`/`close_*_class` callee below is invoked at its real
// call site with NO `this` set up at all - no push, no `mov ecx` - even
// though each is a public non-virtual __cdecl member (confirmed: compiling
// a real `((Time*)addr)->close_class();` call through this exact prototype
// emits `push addr` before the call, which the target bytes do not have).
// So they are modeled here as plain zero-argument externs instead of calls
// through the context's class shells, which is the only form that
// reproduces the bare `call` the target actually contains.
extern "C" void __cdecl close_time_stub();
extern "C" void __cdecl close_checkbutton_stub();
extern "C" void __cdecl close_buffer_stub();
extern "C" void __cdecl close_checkbox_stub();
extern "C" void __cdecl close_radiobutton_stub();
extern "C" void __cdecl close_font_stub();
extern "C" void __cdecl close_filewin_stub();
extern "C" void __cdecl close_basepop_stub();
extern "C" void __cdecl close_cursor_stub();
extern "C" void __cdecl close_win_stub();
extern "C" void __cdecl close_palette_stub();

// sub_635750 IS called through ecx (`mov ecx, 0x9be618` precedes it), so
// unlike the close_class family above it is a genuine thiscall receiver
// call - modeled as a member of a dummy opaque class bound to that fixed
// address.
class Unk9be618 { public:
    int unk_call();
};

void __cdecl jackal_close() {
    sub_62d100();
    close_time_stub();
    reinterpret_cast<Unk9be618 *>(g_009be618)->unk_call();
    close_checkbutton_stub();
    close_buffer_stub();
    close_checkbox_stub();
    close_radiobutton_stub();
    close_font_stub();
    sub_63cef0();
    close_filewin_stub();
    close_basepop_stub();
    close_cursor_stub();
    close_win_stub();
    close_palette_stub();
    reinterpret_cast<Strings *>(g_009b90d8)->shutdown();
    *g_009bc4b0 = *g_009bc4b0 & ~1;
}
