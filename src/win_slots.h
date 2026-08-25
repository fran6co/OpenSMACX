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

// `g` IS SetCursorPos, and the one-letter name is why three bodies reached
// for it meaning three different APIs. Named now; see g_GetCursorPos and
// g_BeginPaint below for the two that were calling the wrong import
// entirely - invisible to BYTE_EXACT, because a call target is a relocation
// and asm.py masks those out of the comparison.
static int * const g_SetCursorPos = (int *)0x00669310;
static int * const g_GetCursorPos = (int *)0x00669284;
static int * const g_BeginPaint = (int *)0x006692B8;
static int * const g_EndPaint = (int *)0x006692B4;
static int * const g_ShowWindow = (int *)0x00669320;
static int * const g_SetRect = (int *)0x00669274;
static int * const WinFocusStack = (int *)0x009B7A1C;
static int * const WinFocusTop = (int *)0x009B7ADC;
static int * const WinModalDepth = (int *)0x009B7AE4;
static int * const WinMaximizeIcon = (int *)0x009B7B04;
static int * const WinRestoreIcon = (int *)0x009B7B08;
static int * const g_MoveToEx = (int *)0x00669048;
static int * const g_CreatePenIndirect = (int *)0x0066904C;
static int * const g_DeleteObject = (int *)0x00669058;
static int * const g_SelectObject = (int *)0x00669060;
static int * const g_RealizePalette = (int *)0x0066909C;
static int * const g_SelectPalette = (int *)0x006690B8;
static int * const g_LineTo = (int *)0x006690C4;
static int * const g_SetCursor = (int *)0x00669288;
static int * const g_LoadCursorA = (int *)0x0066928C;
static int * const g_DefWindowProcA = (int *)0x006692B0;
static int * const g_GetWindowLongA = (int *)0x0066934C;
static int * const g_IsWindow = (int *)0x006692C8;
static int * const g_InvalidateRect = (int *)0x00669304;
static int * const g_MessageBoxA = (int *)0x00669318;
static int * const g_UnionRect = (int *)0x00669328;
static int * const g_GetKeyState = (int *)0x0066932C;
static int * const g_GetAsyncKeyState = (int *)0x00669330;
static int * const g_GetSystemMetrics = (int *)0x00669334;
static int * const g_IntersectRect = (int *)0x00669338;
static int * const g_ShowCursor = (int *)0x00669354;
static int * const g_PeekMessageA = (int *)0x00669358;
static int * const WinFillColour = (int *)0x00696D14;
static int * const WinKeyRingCursor = (int *)0x00696D5C;
static int * const WinMsgIncreaseMaxChildren = (int *)0x00696D60;
static int * const WinMsgTooManyChildren = (int *)0x00696D80;
static int * const WinMsgIncreaseMaxParents = (int *)0x00696D94;
static int * const WinMsgTooManyParents = (int *)0x00696DB4;
static int * const WinMdebugCodeEnd = (int *)0x00696DFD;
static int * const WinBubbleWindow = (int *)0x009B22F0;
static int * const WinTopDialog = (int *)0x009B2300;
static int * const WinDrawFlags = (int *)0x009B238C;
static int * const WinPopupWindow = (int *)0x009B23B4;
static int * const WinDialogList = (int *)0x009B2494;
static int * const WinPopupCount = (int *)0x009B26EC;
static int * const WinScreenClipRect = (int *)0x009B74C0;
static int * const WinGlobalCallback = (int *)0x009B7A90;
static int * const WinMouseCallback = (int *)0x009B7A94;
static int * const WinKeyModifiers = (int *)0x009B7B18;
static int * const WinMainHwnd = (int *)0x009B7B28;
static int * const WinKeyRingStart = (int *)0x009B7B48;
static int * const WinKeyRingEnd = (int *)0x009B7B51;
static int * const WinPalette = (int *)0x009B8180;
static int * const WinModalFocus = (int *)0x009B8D7C;
static int * const WinTitleBarHeight = (int *)0x009B8DD4;
static int * const WinNetBuffer = (int *)0x009BE608;
static int * const WinScreenDCDepth = (int *)0x009B3AB0;
static int * const WinDirtyRect = (int *)0x009B6EE8;
static int * const WinModalResult = (int *)0x009B6EF0;
static int * const WinSavedAreaWidth = (int *)0x009B6F88;
static int * const WinSavedAreaHeight = (int *)0x009B6F8C;
static int * const WinSavedFocus = (int *)0x009B7A2C;
static int * const WinClipLeft = (int *)0x009B7A30;
static int * const WinClipTop = (int *)0x009B7A34;
static int * const WinDragOffsetX = (int *)0x009B7A38;
static int * const WinDragOffsetY = (int *)0x009B7A3C;
static int * const WinClipWidth = (int *)0x009B7A40;
static int * const WinClipHeight = (int *)0x009B7A44;
static int * const WinBackBuffer = (int *)0x009B7A68;
static int * const WinHighlighted = (int *)0x009B7A6C;
static int * const WinViewOriginX = (int *)0x009B7A70;
static int * const WinViewOriginY = (int *)0x009B7A74;
static int * const WinZOrderDirty = (int *)0x009B7A78;
static int * const WinDoubleClickFlag = (int *)0x009B7AA4;
// TRIED 2026-08-25 and REFUTED BY MEASUREMENT: retyping these three to
// `Win **const` - the honest type, and the spelling win.h's
// WinBubbleCompanion uses - cost show_maximize and maximize their claims.
// The binding stopped folding to its immediate (`mov eax, dword ptr [eax]`
// where the image has the address inline) and VC6 also stopped sharing one
// zeroed register across the neighbouring stores, so the image's
// `xor edi,edi` + three `mov [addr], edi` became three immediate stores.
// Spelling the null `0` rather than `nullptr` did not bring it back. The
// `reinterpret_cast<int>(this)` at the use sites is therefore LOAD-BEARING,
// not laziness - it is raw-self-access debt the ratchet has to keep.
// See the recorded lesson `binding-type-decides-folding`.
static int * const WinCallbackWindow = (int *)0x009B7AB8;
static int * const WinActiveDialog = (int *)0x009B7ABC;
static int * const WinLastActive = (int *)0x009B7AC0;
static int * const WinInputFocus = (int *)0x009B7AC4;
static int * const WinActiveWindow = (int *)0x009B7AC8;
static int * const WinFocusPrimary = (int *)0x009B7ACC;
static int * const WinFocusSecondary = (int *)0x009B7AD0;
static int * const WinSizingFlag = (int *)0x009B7AD4;
static int * const WinScreenDC = (int *)0x009B7B2C;
static int * const WinZOrderListCount = (int *)0x009B7B34;
static int * const WinPendingFocus = (int *)0x009B7B38;
static int * const WinKeyState = (int *)0x009B7B3C;
static int * const WinDDSurface = (int *)0x009BC498;
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
static FnSelectClipRgn * const g_SelectClipRgn = (FnSelectClipRgn *)0x00669074;
static uint32_t * const WinStaticDefaults = (uint32_t *)0x00696D34;
static uint32_t * const WinDynamicDefaults = (uint32_t *)0x009B7AF0;
static Win ** const g_zorder_list = (Win **)0x009B6E48;
// `int *const`, for the reason g_GetDC above records: a fixed-address
// binding spelled as a pointer-to-function-pointer stops folding to its
// immediate. As `FnReleaseDC *const` this compiled `mov edx, dword ptr [0]`
// + `call [edx]` in OnQueryNewPalette against the image's single
// `call dword ptr [0x669280]`.
static int * const g_ReleaseDC = (int *)0x00669280;

#endif  // OPENSMACX_WIN_SLOTS_H
// 0x009B6628, read by get_mouse_window_recurse when it restores the
// screen position it was called with; its neighbour 0x009B662C is the Y.
static int * const WinMouseScreenXSaved = (int *)0x009B6628;
static int * const WinMouseScreenYSaved = (int *)0x009B662C;
// USER32 imports the message pump reaches, named by tools/iat_names.py
// from the PE import table rather than inferred from their use.
static int * const g_TranslateMessage = (int *)0x0066935C;
static int * const g_DispatchMessageA = (int *)0x00669340;
