// 0x005FCDA0  sub_5fcda0  ->  _sub_5fcda0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

struct MSG {
    void *hwnd;
    unsigned int message;
    unsigned int wParam;
    long lParam;
    unsigned long time;
    long pt_x;
    long pt_y;
};

typedef int (__stdcall *PeekMessageAFn)(MSG *, void *, unsigned int, unsigned int, unsigned int);
typedef int (__stdcall *TranslateMessageFn)(const MSG *);
typedef long (__stdcall *DispatchMessageAFn)(const MSG *);

extern "C" int __cdecl sub_5fcda0() {
    MSG msg;
    if ((*reinterpret_cast<PeekMessageAFn *>(g_00669358))(&msg, 0, 0x200, 0x200, 1)) {
        (*reinterpret_cast<TranslateMessageFn *>(g_0066935c))(&msg);
        (*reinterpret_cast<DispatchMessageAFn *>(g_00669340))(&msg);
        return 1;
    }
    return 0;
}
