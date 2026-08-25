#ifndef OPENSMACX_WIN_SLOTS_H
#define OPENSMACX_WIN_SLOTS_H
// FIXED-SLOT BINDINGS, IN A HEADER ON PURPOSE.
//
// `static T *const x = (T *)0xADDR;` inside a .cpp is not a constant
// expression to VC6: it allocates storage, initialises it at runtime -
// and this build has no static initialisers, so the storage stays zero.
// A body then compiles `mov eax, [x]` where the image folds the address
// into the addressing mode, `mov [ecx*4 + 0x9b6e48], esi`. MEASURED on
// 0x005F5080: 7/29 instructions as a .cpp binding, BYTE_EXACT once the
// address reached the expression as a literal.
//
// Declared here so every use folds.
class Win;
class Buffer;
class Scroll;
class Font;

typedef int (__stdcall *GetDCFn)(void *hwnd);
typedef HRGN (__stdcall *FnCreateRectRgnIndirect)(const RECT *);
typedef int (__stdcall *FnReleaseDC)(HWND, HDC);

typedef int (__stdcall *FnSelectClipRgn)(HDC, HRGN);
typedef HDC (__stdcall *FnGetDC)(HWND);

typedef void(__cdecl *FnSetActiveWindow)(Win *);

static uint32_t * const WinStaticDefaults = (uint32_t *)0x00696D34;
static uint32_t * const WinDynamicDefaults = (uint32_t *)0x009B7AF0;
static int * const g = (int *)0x00669310;
static int * const g_009b7ae0 = (int *)0x009B7AE0;
static int * const g_009b6ef8 = (int *)0x009B6EF8;
static int * const g_009b6efc = (int *)0x009B6EFC;
static int * const g_009b7a1c = (int *)0x009B7A1C;
static int * const g_009b7a20 = (int *)0x009B7A20;
static int * const g_009b7adc = (int *)0x009B7ADC;
static int * const g_009b7ae4 = (int *)0x009B7AE4;
static int * const WinMaximizeIcon = (int *)0x009B7B04;
static int * const WinRestoreIcon = (int *)0x009B7B08;
static int * const g_0063c4e0 = (int *)0x0063C4E0;
static int * const g_00669048 = (int *)0x00669048;
static int * const g_0066904c = (int *)0x0066904C;
static int * const g_00669058 = (int *)0x00669058;
static int * const g_00669060 = (int *)0x00669060;
static int * const g_0066909c = (int *)0x0066909C;
static int * const g_006690b8 = (int *)0x006690B8;
static int * const g_006690c4 = (int *)0x006690C4;
static int * const g_00669280 = (int *)0x00669280;
static int * const g_00669288 = (int *)0x00669288;
static int * const g_0066928c = (int *)0x0066928C;
static int * const g_006692b0 = (int *)0x006692B0;
static int * const g_006692c8 = (int *)0x006692C8;
static int * const g_00669304 = (int *)0x00669304;
static int * const g_00669318 = (int *)0x00669318;
static int * const g_00669328 = (int *)0x00669328;
static int * const g_0066932c = (int *)0x0066932C;
static int * const g_00669330 = (int *)0x00669330;
static int * const g_00669334 = (int *)0x00669334;
static int * const g_00669338 = (int *)0x00669338;
static int * const g_00669354 = (int *)0x00669354;
static int * const g_00669358 = (int *)0x00669358;
static int * const g_00696d14 = (int *)0x00696D14;
static int * const g_00696d34 = (int *)0x00696D34;
static int * const g_00696d38 = (int *)0x00696D38;
static int * const g_00696d3c = (int *)0x00696D3C;
static int * const g_00696d40 = (int *)0x00696D40;
static int * const g_00696d44 = (int *)0x00696D44;
static int * const g_00696d48 = (int *)0x00696D48;
static int * const g_00696d4c = (int *)0x00696D4C;
static int * const g_00696d50 = (int *)0x00696D50;
static int * const g_00696d54 = (int *)0x00696D54;
static int * const g_00696d5c = (int *)0x00696D5C;
static int * const g_00696d60 = (int *)0x00696D60;
static int * const g_00696d80 = (int *)0x00696D80;
static int * const g_00696d94 = (int *)0x00696D94;
static int * const g_00696db4 = (int *)0x00696DB4;
static int * const g_00696dfd = (int *)0x00696DFD;
static int * const g_009b22f0 = (int *)0x009B22F0;
static int * const g_009b2300 = (int *)0x009B2300;
static int * const g_009b238c = (int *)0x009B238C;
static int * const g_009b23b4 = (int *)0x009B23B4;
static int * const g_009b2494 = (int *)0x009B2494;
static int * const g_009b26ec = (int *)0x009B26EC;
static int * const g_009b7490 = (int *)0x009B7490;
static int * const g_009b74c0 = (int *)0x009B74C0;
static int * const g_009b7a90 = (int *)0x009B7A90;
static int * const g_009b7a94 = (int *)0x009B7A94;
static int * const g_009b7af0 = (int *)0x009B7AF0;
static int * const g_009b7af4 = (int *)0x009B7AF4;
static int * const g_009b7af8 = (int *)0x009B7AF8;
static int * const g_009b7afc = (int *)0x009B7AFC;
static int * const g_009b7b18 = (int *)0x009B7B18;
static int * const g_009b7b28 = (int *)0x009B7B28;
static int * const g_009b7b48 = (int *)0x009B7B48;
static int * const g_009b7b51 = (int *)0x009B7B51;
static int * const g_009b8178 = (int *)0x009B8178;
static int * const g_009b8180 = (int *)0x009B8180;
static int * const g_009b8d7c = (int *)0x009B8D7C;
static int * const g_009b8dd4 = (int *)0x009B8DD4;
static int * const g_009bc4b0 = (int *)0x009BC4B0;
static int * const g_009be608 = (int *)0x009BE608;
static int * const g_009b3ab0 = (int *)0x009B3AB0;
static Win ** const g_zorder_list = (Win **)0x009B6E48;
static int * const g_009b6ee8 = (int *)0x009B6EE8;
static int * const g_009b6ef0 = (int *)0x009B6EF0;
static int * const g_009b6f88 = (int *)0x009B6F88;
static int * const g_009b6f8c = (int *)0x009B6F8C;
static int * const g_009b7a2c = (int *)0x009B7A2C;
static int * const g_009b7a30 = (int *)0x009B7A30;
static int * const g_009b7a34 = (int *)0x009B7A34;
static int * const g_009b7a38 = (int *)0x009B7A38;
static int * const g_009b7a3c = (int *)0x009B7A3C;
static int * const g_009b7a40 = (int *)0x009B7A40;
static int * const g_009b7a44 = (int *)0x009B7A44;
static int * const g_009b7a68 = (int *)0x009B7A68;
static int * const g_009b7a6c = (int *)0x009B7A6C;
static int * const g_009b7a70 = (int *)0x009B7A70;
static int * const g_009b7a74 = (int *)0x009B7A74;
static int * const g_009b7a78 = (int *)0x009B7A78;
static int * const g_009b7a7c = (int *)0x009B7A7C;
static int * const g_009b7aa4 = (int *)0x009B7AA4;
static int * const g_009b7ab8 = (int *)0x009B7AB8;
static int * const g_009b7abc = (int *)0x009B7ABC;
static int * const g_009b7ac0 = (int *)0x009B7AC0;
static int * const g_009b7ac4 = (int *)0x009B7AC4;
static int * const g_009b7ac8 = (int *)0x009B7AC8;
static int * const g_009b7acc = (int *)0x009B7ACC;
static int * const g_009b7ad0 = (int *)0x009B7AD0;
static int * const g_009b7ad4 = (int *)0x009B7AD4;
static int * const g_009b7ad8 = (int *)0x009B7AD8;
static int * const g_009b7b2c = (int *)0x009B7B2C;
static int * const g_009b7b30 = (int *)0x009B7B30;
static int * const g_009b7b34 = (int *)0x009B7B34;
static int * const g_009b7b38 = (int *)0x009B7B38;
static int * const g_009b7b3c = (int *)0x009B7B3C;
static int * const g_009bc494 = (int *)0x009BC494;
static int * const g_009bc498 = (int *)0x009BC498;
// `int *const`, for the same reason g_GetDC above carries that type: a
// fixed-address binding folds to its immediate only in this spelling. As
// `void **const` it compiled `mov eax, dword ptr [0]` in Win::redraw - the
// array base read from address zero - while two other bodies using the same
// name happened to fold anyway, which is what made it look like a per-body
// register-allocation quirk rather than the type. Callers cast at use.
static int * const g_win_array = (int *)0x009B6630;
// `int *const`, NOT `GetDCFn *const`, and the type is load-bearing. Spelled
// as a pointer-to-function-pointer this binding stopped folding to its
// immediate: OnQueryNewPalette compiled `mov eax, dword ptr [0]` +
// `call [eax]` where the image has a single `call dword ptr [0x66927c]`.
// The `int *` form folds, so every caller casts at the point of use - which
// is what the anonymous binding this name replaced was already doing.
static int * const g_GetDC = (int *)0x0066927C;
static FnCreateRectRgnIndirect * const g_CreateRectRgnIndirect = (FnCreateRectRgnIndirect *)0x00669078;
static FnReleaseDC * const g_ReleaseDC = (FnReleaseDC *)0x00669280;
static FnSelectClipRgn * const g_SelectClipRgn = (FnSelectClipRgn *)0x00669074;

#endif  // OPENSMACX_WIN_SLOTS_H
