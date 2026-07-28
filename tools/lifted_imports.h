#ifndef OPENSMACX_LIFTED_IMPORTS_H
#define OPENSMACX_LIFTED_IMPORTS_H

// The ABI boundary of the whole-image lift.
//
// Everywhere else in the lift a "calling convention" is a fiction: every
// lifted function is void f(State &) and the argument passing is whatever the
// original instructions did to the guest stack, so stdcall/cdecl/thiscall
// never has to be named. The 221 PE imports are the ONE place that stops
// being true, because on the far side of the boundary there is a real i686
// Windows function with a real convention, and getting it wrong corrupts the
// CALLER's stack - a failure that surfaces arbitrarily far from its cause.
//
// This header is the hand-written half of that boundary: the marshalling
// primitives and the bodies of the imports that are implemented. The other
// half - which import lives at which IAT slot, how many bytes of argument
// each one pops, and the shim that ties the two together - is DERIVED by
// tools/generate_imports.py from the PE import directory and the toolchain's
// own import libraries. Nothing about the boundary is written down twice.
//
// HOW A LIFTED CALL REACHES A REAL FUNCTION
//
// The image's IAT lives inside opensmacx_image, so `call [0x006690f4]` lowers
// to a load from the array followed by opensmacx_dispatch(value)(s). Whatever
// is in that slot has to be a value the dispatcher can turn into a callable.
// It is a SYNTHETIC GUEST ADDRESS: import k gets OpensmacxImportBase + k*16,
// a reserved range far outside the image, written into its IAT slot at
// startup by opensmacx_bind_imports(). The dispatcher recognises the range
// with one compare and indexes a dense array - no search.
//
// The alternative - storing the host address of a generated trampoline in the
// slot - fits in 32 bits on this target but loses three times over. The
// dispatcher would need to distinguish host code addresses from guest ones,
// and host code addresses occupy no range you can test cheaply or
// predictably. The value would change from run to run, so a trap could not
// name a stable address and two runs could not be compared. And it would have
// to be materialised at startup anyway, since a host function's address is
// not a link-time constant here. The synthetic range costs exactly one store
// per import at startup and buys a stable, greppable, O(1) identity.
//
// WHAT MAY CROSS THE BOUNDARY
//
// The guest's entire address space is opensmacx_image, and opensmacx_at()
// reaches it by subtracting the image base. So the rule for whether an import
// can be implemented at all is not about its convention, it is about
// pointers:
//
//   * A guest pointer going OUT becomes a host pointer via opensmacx_host(),
//     which takes the object's SIZE and refuses anything that does not fit
//     inside the image. The address alone is not enough: a pointer four bytes
//     below the end of the image is a legal address and an illegal
//     sixteen-byte structure, and Windows would write all sixteen.
//   * A host pointer coming IN must become a guest address via
//     opensmacx_guest(). That is only possible if it points INTO the image.
//     A pointer to host heap has no guest address; the guest would later
//     dereference it through opensmacx_at() and read unrelated memory.
//   * A guest CODE address going out as a callback (WndProc, thread start,
//     timer proc, enumerator) would be called by Windows as real machine
//     code. There is none at that address.
//
// The second and third cases are why so much of kernel32 is not implemented
// here and cannot be: HeapAlloc, VirtualAlloc, GetCommandLineA and
// GetEnvironmentStrings all hand the guest a host pointer. That is a MEMORY
// MODEL problem, not an ABI problem, and it has a clean fix that lives
// outside this file - see the note on opensmacx_guest() below. The imports
// that would hit it are still implemented, because then they trap at the
// exact marshalling step that is impossible instead of being absent, and
// under that fix they start working with no change to their bodies.

#include <cstdint>
#include <cstring>

#include "lifted_runtime.h"

// ---------------------------------------------------------------------------
// The synthetic import range.
//
// 0x0F000000 is above every section of the image (which ends below 0x00A10000)
// and is not a plausible value for anything else the guest computes, so a
// synthetic address showing up somewhere unexpected is self-identifying. The
// stride is 16 rather than 4 so that the address of import k is readable as
// hex at a glance and so that a slot has room to grow a per-import record if
// one is ever needed. The LIMIT is a fixed 256 KiB - 16384 imports - and NOT
// derived from the count, so the range test compiled into opensmacx_dispatch
// never has to change when the import table does.
// ---------------------------------------------------------------------------

constexpr uint32_t OpensmacxImportBase = 0x0F000000U;
constexpr uint32_t OpensmacxImportStride = 16U;
constexpr uint32_t OpensmacxImportLimit = OpensmacxImportBase + 0x40000U;

inline bool opensmacx_is_import(uint32_t address) {
    return address >= OpensmacxImportBase && address < OpensmacxImportLimit;
}

// Defined in the generated lifted_imports.cpp.
OpensmacxLiftedFunction opensmacx_import_dispatch(uint32_t address);
void opensmacx_bind_imports();
const char *opensmacx_import_name(uint32_t address);
uint32_t opensmacx_import_count();
uint32_t opensmacx_import_address(uint32_t index);
// Bytes of argument the shim at `address` pops on return, i.e. the @N of its
// decorated name. The test harness reads it to check ESP independently of the
// shim that moved it.
uint32_t opensmacx_import_arg_bytes(uint32_t address);
// The IAT slot `address` was bound into. Without this a test can only locate a
// slot by scanning the image for a synthetic value, which finds whichever slot
// happens to hold it - so a binding that pairs every import with the WRONG
// slot looks identical to a correct one. With it the pairing can be checked
// against the PE import directory itself.
uint32_t opensmacx_import_iat(uint32_t address);
// Whether the differential oracle may call this import on both sides and
// expect the two answers to agree. NOT the same question as "implemented":
// GetTickCount works perfectly and can never agree with itself. False for
// every unimplemented import, whose shim traps.
bool opensmacx_import_oracle_safe(uint32_t address);
// The half-open span of the image occupied by the IAT. The oracle's two sides
// bind their IATs to different things on purpose, so a whole-image comparison
// has to exclude exactly this range and no more.
uint32_t opensmacx_import_iat_low();
uint32_t opensmacx_import_iat_high();
// Fill the ORACLE-SAFE part of an image's IAT with the addresses of the REAL
// Windows functions, for a side that will be executed by the CPU rather than
// dispatched. `image` is the host address of guest byte OpensmacxImageBase.
// Returns the number of slots filled, so an unresolvable import is a number
// the caller can check rather than a fault later. Deliberately NOT every
// slot - see the generated definition for why binding DDRAW would cost two
// minutes a function.
uint32_t opensmacx_bind_real_imports(unsigned char *image);

// ---------------------------------------------------------------------------
// Reading the guest stack.
//
// On entry to a shim the guest stack looks exactly as it does on entry to any
// callee: ESP points at the return address the lowered CALL pushed, and
// argument k is four bytes above it. The shim reads its arguments BEFORE the
// generated wrapper moves ESP, which is why cleanup is the wrapper's job and
// never the body's.
// ---------------------------------------------------------------------------

inline uint32_t opensmacx_arg(const OpensmacxStaticRecompileState &s,
                              unsigned index) {
    return opensmacx_mem32(s.esp + 4U + 4U * index);
}

inline uint32_t opensmacx_return_address(
        const OpensmacxStaticRecompileState &s) {
    return opensmacx_mem32(s.esp);
}

// ---------------------------------------------------------------------------
// Pointer marshalling.
// ---------------------------------------------------------------------------

// A guest address to a host pointer, with the bounds check opensmacx_at()
// deliberately does not have. A null guest pointer stays null rather than
// becoming a pointer to the image base, which is a real distinction for the
// many Win32 arguments that are optional.
//
// The LENGTH is mandatory, and that is the whole point. An earlier version
// took only the address and checked only that the first byte was inside the
// image, which bounds nothing wider than a byte: GetSystemTime on
// OpensmacxImageBase + OpensmacxImageSize - 4 passed the check and then wrote
// 12 of SYSTEMTIME's 16 bytes into whatever the linker put after
// opensmacx_image. Every out-parameter in this file had the same hole, and the
// length-carrying ones were worse - ReadFile forwarded the guest's own count
// unvalidated, so a 64 KiB read four bytes below the end wrote 65,532 bytes of
// host memory. Making the size a parameter with no default means a new shim
// cannot be written without answering "how big is it".
inline void *opensmacx_host(uint32_t guest, uint32_t bytes) {
    if (guest == 0U) {
        return nullptr;
    }
    const uint32_t offset = guest - OpensmacxImageBase;
    // Written as a subtraction against the size rather than `offset + bytes <=
    // size` so that a caller-supplied count near 2^32 cannot wrap the test
    // into passing. ReadFile's count is guest data.
    if (guest < OpensmacxImageBase || offset >= OpensmacxImageSize
            || bytes > OpensmacxImageSize - offset) {
        opensmacx_trap(guest,
                       "import argument is not an object of that size inside "
                       "the image");
    }
    return opensmacx_at(guest);
}

// The common case: the size comes from the type, so it cannot be mistyped and
// cannot drift when the structure changes.
template <typename T>
inline T *opensmacx_host_as(uint32_t guest) {
    return static_cast<T *>(opensmacx_host(guest, sizeof(T)));
}

// A caller-supplied count, named so that the shims where the guest chooses the
// length read differently from the ones where the type does. Identical
// behaviour; the distinction is for the reader, because these are the
// dangerous ones.
inline void *opensmacx_host_buffer(uint32_t guest, uint32_t bytes) {
    return opensmacx_host(guest, bytes);
}

// An array of COUNT elements where the guest chose the count.
//
// The multiplication is the whole reason this exists. opensmacx_host() takes a
// uint32_t, so a shim that writes `count * sizeof(T)` computes the product in
// 32 bits and a guest count of 0x40000001 with a four-byte element wraps to 4:
// the bounds check then approves four bytes and AnimatePalette writes a
// gigabyte. Done in 64 bits here, once, so no call site can get it wrong. Any
// product larger than the image is refused before it is narrowed - it could
// not have been a legal object anyway, since the image is the whole address
// space the guest has.
template <typename T>
inline T *opensmacx_host_array(uint32_t guest, uint32_t count) {
    const uint64_t bytes = static_cast<uint64_t>(count) * sizeof(T);
    if (bytes > OpensmacxImageSize) {
        opensmacx_trap(guest,
                       "import array argument is larger than the whole image");
    }
    return static_cast<T *>(
        opensmacx_host(guest, static_cast<uint32_t>(bytes)));
}

// A NUL-terminated string. Its length is not known up front, so the bound is
// that the terminator itself must be found inside the image: an unterminated
// string would otherwise send the host's strlen walking off the end of the
// array, which is the same defect as an unbounded structure with no size to
// check against.
inline char *opensmacx_host_str_mutable(uint32_t guest) {
    if (guest == 0U) {
        return nullptr;
    }
    char *const text = static_cast<char *>(opensmacx_host(guest, 1U));
    const uint32_t remaining =
        OpensmacxImageSize - (guest - OpensmacxImageBase);
    if (std::memchr(text, '\0', remaining) == nullptr) {
        opensmacx_trap(guest,
                       "import string argument has no terminator before the "
                       "end of the image");
    }
    return text;
}

inline const char *opensmacx_host_str(uint32_t guest) {
    return opensmacx_host_str_mutable(guest);
}

// A host pointer back to a guest address.
//
// This is the one direction that can be impossible, and the trap here is the
// single most informative thing in this file. It fires when a real Windows
// function hands back memory that is not in the image - HeapAlloc's block,
// GetCommandLineA's string - which the guest cannot address at all.
//
// The fix is not in this layer. It is to give the guest image its CANONICAL
// host address, so that guest and host addresses coincide and this function
// becomes the identity: link the lifted executable at a base that is not
// 0x00400000 (`-Wl,--image-base=0x20000000`) and reserve
// 0x00400000 + OpensmacxImageSize with VirtualAlloc at startup instead of
// using a static array. Every shim in this file is written through
// opensmacx_host()/opensmacx_guest() precisely so that it needs no edit when
// that happens; the two functions collapse to nothing and the whole
// pointer-marshalling problem disappears with them.
inline uint32_t opensmacx_guest(const void *host) {
    if (host == nullptr) {
        return 0U;
    }
    const unsigned char *const bytes =
        static_cast<const unsigned char *>(host);
    if (bytes < opensmacx_image
            || bytes >= opensmacx_image + OpensmacxImageSize) {
        opensmacx_trap(
            OpensmacxImageBase,
            "import returned host memory the guest cannot address; the image "
            "needs its canonical base (see opensmacx_guest)");
    }
    return OpensmacxImageBase
        + static_cast<uint32_t>(bytes - opensmacx_image);
}

// A HANDLE is opaque: the guest stores it and passes it back, never
// dereferences it. So it crosses as a bare 32-bit value with no translation,
// and these two names exist only to say at each use site that the opacity was
// considered rather than forgotten.
inline void *opensmacx_handle(uint32_t value) {
    return reinterpret_cast<void *>(static_cast<uintptr_t>(value));
}

inline uint32_t opensmacx_from_handle(void *value) {
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(value));
}

// ---------------------------------------------------------------------------
// The implemented imports.
//
// Each one is a pair the generator reads:
//
//   #define OPENSMACX_ARGS_<DLL>_<Name>   <argument count>
//   uint32_t opensmacx_shim_<DLL>_<Name>(State &s)   returning EAX
//
// The count is INDEPENDENT knowledge - it comes from the prototype written
// here - while the generator's cleanup comes from the @N decoration in the
// toolchain's import library. The generated shim static_asserts that
// 4 * count == @N, so the two sources must agree or the build stops. That is
// the check that makes a wrong cleanup a compile error rather than a caller
// corrupted at run time, and it is why the body does not touch ESP: cleanup
// belongs to the derived side, which has the authoritative number.
//
// An import with no pair here gets a shim that traps naming itself. That is
// deliberate for this phase - a named trap is a work item, a wrong shim is a
// bug hunt.
//
// THE THIRD FACT: WHETHER IT MAY BE CALLED TWICE
//
//   #define OPENSMACX_ORACLE_<DLL>_<Name>   safe | unsafe   // why
//
// The differential oracle runs the ORIGINAL machine code and the LIFTED code
// separately and compares everything either one wrote. When both reach the same
// import, that is TWO calls to the real Windows function, and the comparison is
// only meaningful if the second call can agree with the first. Most cannot:
// GetTickCount has advanced, HeapAlloc returns a different pointer, TlsAlloc
// has consumed a slot, CreateFileA has made a file that was not there. Every
// one of those imports is implemented and correct; none of them can be
// compared. Marking them "works, therefore testable" would manufacture
// failures that look exactly like lowering bugs, in the direction that makes
// the harness look productive - the failure mode this project has already been
// bitten by three times.
//
// So implementation and comparability are stated separately, and
// generate_imports.py refuses to run if a shim states only the first. The
// planner reads the answer out of the generated manifest, so it never carries
// its own opinion about which imports work.
//
// The reasons that make an import safe are worth naming, because they are
// narrower than they look:
//   * pure computation on the arguments (the rectangle family, CharUpperA);
//   * a property of the machine or the process that nothing here changes
//     (GetSystemMetrics, GetACP, GetModuleFileNameA);
//   * a DEFINED failure on a handle the guest does not own, with no side
//     effect - which is what makes the whole window and DC family testable
//     even though no window exists.
// ---------------------------------------------------------------------------

#define OPENSMACX_IMPORT_WINDOWS_HEADERS 1
#include <windows.h>
#undef OPENSMACX_IMPORT_WINDOWS_HEADERS

// --- kernel32: time, identity, error state ---------------------------------

#define OPENSMACX_ARGS_KERNEL32_GetTickCount 0
#define OPENSMACX_ORACLE_KERNEL32_GetTickCount unsafe  // the counter advances between the two sides
inline uint32_t opensmacx_shim_KERNEL32_GetTickCount(
        OpensmacxStaticRecompileState &) {
    return GetTickCount();
}

#define OPENSMACX_ARGS_KERNEL32_GetVersion 0
#define OPENSMACX_ORACLE_KERNEL32_GetVersion safe  // constant for the life of the process
inline uint32_t opensmacx_shim_KERNEL32_GetVersion(
        OpensmacxStaticRecompileState &) {
    return GetVersion();
}

#define OPENSMACX_ARGS_KERNEL32_GetLastError 0
#define OPENSMACX_ORACLE_KERNEL32_GetLastError safe  // the TEB is not part of the compared image
inline uint32_t opensmacx_shim_KERNEL32_GetLastError(
        OpensmacxStaticRecompileState &) {
    return GetLastError();
}

#define OPENSMACX_ARGS_KERNEL32_SetLastError 1
#define OPENSMACX_ORACLE_KERNEL32_SetLastError safe  // the TEB is not part of the compared image
inline uint32_t opensmacx_shim_KERNEL32_SetLastError(
        OpensmacxStaticRecompileState &s) {
    SetLastError(opensmacx_arg(s, 0));
    return 0U;
}

#define OPENSMACX_ARGS_KERNEL32_GetCurrentThreadId 0
#define OPENSMACX_ORACLE_KERNEL32_GetCurrentThreadId safe  // constant within one thread
inline uint32_t opensmacx_shim_KERNEL32_GetCurrentThreadId(
        OpensmacxStaticRecompileState &) {
    return GetCurrentThreadId();
}

#define OPENSMACX_ARGS_KERNEL32_GetCurrentThread 0
#define OPENSMACX_ORACLE_KERNEL32_GetCurrentThread safe  // a constant pseudo-handle
inline uint32_t opensmacx_shim_KERNEL32_GetCurrentThread(
        OpensmacxStaticRecompileState &) {
    return opensmacx_from_handle(GetCurrentThread());
}

#define OPENSMACX_ARGS_KERNEL32_GetCurrentProcess 0
#define OPENSMACX_ORACLE_KERNEL32_GetCurrentProcess safe  // a constant pseudo-handle
inline uint32_t opensmacx_shim_KERNEL32_GetCurrentProcess(
        OpensmacxStaticRecompileState &) {
    return opensmacx_from_handle(GetCurrentProcess());
}

#define OPENSMACX_ARGS_KERNEL32_Sleep 1
#define OPENSMACX_ORACLE_KERNEL32_Sleep unsafe  // the guest chooses the duration and it is unbounded
inline uint32_t opensmacx_shim_KERNEL32_Sleep(
        OpensmacxStaticRecompileState &s) {
    Sleep(opensmacx_arg(s, 0));
    return 0U;
}

#define OPENSMACX_ARGS_KERNEL32_GetSystemTime 1
#define OPENSMACX_ORACLE_KERNEL32_GetSystemTime unsafe  // the clock advances between the two sides
inline uint32_t opensmacx_shim_KERNEL32_GetSystemTime(
        OpensmacxStaticRecompileState &s) {
    // SYSTEMTIME is eight WORDs and holds no pointers, so it can be written
    // straight into guest memory.
    GetSystemTime(opensmacx_host_as<SYSTEMTIME>(opensmacx_arg(s, 0)));
    return 0U;
}

#define OPENSMACX_ARGS_KERNEL32_OutputDebugStringA 1
#define OPENSMACX_ORACLE_KERNEL32_OutputDebugStringA safe  // output only; no result and no memory effect
inline uint32_t opensmacx_shim_KERNEL32_OutputDebugStringA(
        OpensmacxStaticRecompileState &s) {
    OutputDebugStringA(opensmacx_host_str(opensmacx_arg(s, 0)));
    return 0U;
}

#define OPENSMACX_ARGS_KERNEL32_ExitProcess 1
#define OPENSMACX_ORACLE_KERNEL32_ExitProcess unsafe  // ends the run
inline uint32_t opensmacx_shim_KERNEL32_ExitProcess(
        OpensmacxStaticRecompileState &s) {
    ExitProcess(opensmacx_arg(s, 0));
    return 0U;
}

// --- kernel32: code page and locale ----------------------------------------

#define OPENSMACX_ARGS_KERNEL32_GetACP 0
#define OPENSMACX_ORACLE_KERNEL32_GetACP safe  // constant for the life of the process
inline uint32_t opensmacx_shim_KERNEL32_GetACP(
        OpensmacxStaticRecompileState &) {
    return GetACP();
}

#define OPENSMACX_ARGS_KERNEL32_GetOEMCP 0
#define OPENSMACX_ORACLE_KERNEL32_GetOEMCP safe  // constant for the life of the process
inline uint32_t opensmacx_shim_KERNEL32_GetOEMCP(
        OpensmacxStaticRecompileState &) {
    return GetOEMCP();
}

#define OPENSMACX_ARGS_KERNEL32_GetCPInfo 2
#define OPENSMACX_ORACLE_KERNEL32_GetCPInfo safe  // a fixed table for a fixed code page
inline uint32_t opensmacx_shim_KERNEL32_GetCPInfo(
        OpensmacxStaticRecompileState &s) {
    // CPINFO is a DWORD and two byte arrays - no pointers.
    return GetCPInfo(opensmacx_arg(s, 0),
                     opensmacx_host_as<CPINFO>(opensmacx_arg(s, 1)));
}

// --- kernel32: interlocked -------------------------------------------------

#define OPENSMACX_ARGS_KERNEL32_InterlockedIncrement 1
#define OPENSMACX_ORACLE_KERNEL32_InterlockedIncrement safe  // arithmetic on guest memory
inline uint32_t opensmacx_shim_KERNEL32_InterlockedIncrement(
        OpensmacxStaticRecompileState &s) {
    return static_cast<uint32_t>(InterlockedIncrement(
        opensmacx_host_as<LONG>(opensmacx_arg(s, 0))));
}

#define OPENSMACX_ARGS_KERNEL32_InterlockedDecrement 1
#define OPENSMACX_ORACLE_KERNEL32_InterlockedDecrement safe  // arithmetic on guest memory
inline uint32_t opensmacx_shim_KERNEL32_InterlockedDecrement(
        OpensmacxStaticRecompileState &s) {
    return static_cast<uint32_t>(InterlockedDecrement(
        opensmacx_host_as<LONG>(opensmacx_arg(s, 0))));
}

#define OPENSMACX_ARGS_KERNEL32_InterlockedExchange 2
#define OPENSMACX_ORACLE_KERNEL32_InterlockedExchange safe  // arithmetic on guest memory
inline uint32_t opensmacx_shim_KERNEL32_InterlockedExchange(
        OpensmacxStaticRecompileState &s) {
    return static_cast<uint32_t>(InterlockedExchange(
        opensmacx_host_as<LONG>(opensmacx_arg(s, 0)),
        static_cast<LONG>(opensmacx_arg(s, 1))));
}

// --- kernel32: critical sections -------------------------------------------
//
// CRITICAL_SECTION lives in guest memory and DOES contain host pointers
// (DebugInfo, OwningThread) once initialised. That is safe here and only
// here, because the guest never reads those fields: it passes the address of
// the whole structure back to the same three functions, which are the only
// readers. A struct the guest treats as opaque may hold host values; one it
// walks may not.

#define OPENSMACX_ARGS_KERNEL32_InitializeCriticalSection 1
#define OPENSMACX_ORACLE_KERNEL32_InitializeCriticalSection unsafe  // writes a host DebugInfo pointer into guest memory
inline uint32_t opensmacx_shim_KERNEL32_InitializeCriticalSection(
        OpensmacxStaticRecompileState &s) {
    InitializeCriticalSection(opensmacx_host_as<CRITICAL_SECTION>(opensmacx_arg(s, 0)));
    return 0U;
}

#define OPENSMACX_ARGS_KERNEL32_DeleteCriticalSection 1
#define OPENSMACX_ORACLE_KERNEL32_DeleteCriticalSection unsafe  // pairs with a section only one side initialised
inline uint32_t opensmacx_shim_KERNEL32_DeleteCriticalSection(
        OpensmacxStaticRecompileState &s) {
    DeleteCriticalSection(opensmacx_host_as<CRITICAL_SECTION>(opensmacx_arg(s, 0)));
    return 0U;
}

#define OPENSMACX_ARGS_KERNEL32_EnterCriticalSection 1
#define OPENSMACX_ORACLE_KERNEL32_EnterCriticalSection unsafe  // pairs with a section only one side initialised
inline uint32_t opensmacx_shim_KERNEL32_EnterCriticalSection(
        OpensmacxStaticRecompileState &s) {
    EnterCriticalSection(opensmacx_host_as<CRITICAL_SECTION>(opensmacx_arg(s, 0)));
    return 0U;
}

#define OPENSMACX_ARGS_KERNEL32_LeaveCriticalSection 1
#define OPENSMACX_ORACLE_KERNEL32_LeaveCriticalSection unsafe  // pairs with a section only one side initialised
inline uint32_t opensmacx_shim_KERNEL32_LeaveCriticalSection(
        OpensmacxStaticRecompileState &s) {
    LeaveCriticalSection(opensmacx_host_as<CRITICAL_SECTION>(opensmacx_arg(s, 0)));
    return 0U;
}

// --- kernel32: pointer validation ------------------------------------------
//
// These take guest pointers and answer about guest memory, so they are
// answered against the image directly rather than forwarded. Forwarding would
// ask about the HOST mapping of the translated pointer, which is a different
// question with a coincidentally similar answer.

#define OPENSMACX_ARGS_KERNEL32_IsBadReadPtr 2
#define OPENSMACX_ORACLE_KERNEL32_IsBadReadPtr safe  // a probe; changes nothing
inline uint32_t opensmacx_shim_KERNEL32_IsBadReadPtr(
        OpensmacxStaticRecompileState &s) {
    const uint32_t base = opensmacx_arg(s, 0);
    const uint32_t length = opensmacx_arg(s, 1);
    if (length == 0U) {
        return 0U;  // A zero-length range is always readable.
    }
    const uint32_t end = OpensmacxImageBase + OpensmacxImageSize;
    const bool inside = base >= OpensmacxImageBase && base < end
        && length <= end - base;
    return inside ? 0U : 1U;
}

#define OPENSMACX_ARGS_KERNEL32_IsBadWritePtr 2
#define OPENSMACX_ORACLE_KERNEL32_IsBadWritePtr safe  // a probe; changes nothing
inline uint32_t opensmacx_shim_KERNEL32_IsBadWritePtr(
        OpensmacxStaticRecompileState &s) {
    // The guest image is one flat writable array, so readable implies
    // writable. If the image is ever mapped with real section protections
    // this has to consult them.
    return opensmacx_shim_KERNEL32_IsBadReadPtr(s);
}

#define OPENSMACX_ARGS_KERNEL32_IsBadCodePtr 1
#define OPENSMACX_ORACLE_KERNEL32_IsBadCodePtr safe  // a probe; changes nothing
inline uint32_t opensmacx_shim_KERNEL32_IsBadCodePtr(
        OpensmacxStaticRecompileState &s) {
    const uint32_t address = opensmacx_arg(s, 0);
    return (address >= OpensmacxImageBase
            && address < OpensmacxImageBase + OpensmacxImageSize) ? 0U : 1U;
}

// --- kernel32: files -------------------------------------------------------

#define OPENSMACX_ARGS_KERNEL32_CreateFileA 7
#define OPENSMACX_ORACLE_KERNEL32_CreateFileA unsafe  // a fresh handle each call, and it can create a file
inline uint32_t opensmacx_shim_KERNEL32_CreateFileA(
        OpensmacxStaticRecompileState &s) {
    return opensmacx_from_handle(CreateFileA(
        opensmacx_host_str(opensmacx_arg(s, 0)),
        opensmacx_arg(s, 1),
        opensmacx_arg(s, 2),
        opensmacx_host_as<SECURITY_ATTRIBUTES>(opensmacx_arg(s, 3)),
        opensmacx_arg(s, 4),
        opensmacx_arg(s, 5),
        opensmacx_handle(opensmacx_arg(s, 6))));
}

#define OPENSMACX_ARGS_KERNEL32_CloseHandle 1
#define OPENSMACX_ORACLE_KERNEL32_CloseHandle unsafe  // the result depends on what the other side already closed
inline uint32_t opensmacx_shim_KERNEL32_CloseHandle(
        OpensmacxStaticRecompileState &s) {
    return CloseHandle(opensmacx_handle(opensmacx_arg(s, 0)));
}

#define OPENSMACX_ARGS_KERNEL32_ReadFile 5
#define OPENSMACX_ORACLE_KERNEL32_ReadFile unsafe  // moves a shared file pointer
inline uint32_t opensmacx_shim_KERNEL32_ReadFile(
        OpensmacxStaticRecompileState &s) {
    // The buffer's length is the guest's own third argument, so it is
    // checked against the image rather than trusted: an unchecked count here
    // let a 64 KiB read four bytes below the image end write 65,532 bytes of
    // host memory.
    return ReadFile(opensmacx_handle(opensmacx_arg(s, 0)),
                    opensmacx_host_buffer(opensmacx_arg(s, 1),
                                          opensmacx_arg(s, 2)),
                    opensmacx_arg(s, 2),
                    opensmacx_host_as<DWORD>(opensmacx_arg(s, 3)),
                    opensmacx_host_as<OVERLAPPED>(opensmacx_arg(s, 4)));
}

#define OPENSMACX_ARGS_KERNEL32_WriteFile 5
#define OPENSMACX_ORACLE_KERNEL32_WriteFile unsafe  // writes a file
inline uint32_t opensmacx_shim_KERNEL32_WriteFile(
        OpensmacxStaticRecompileState &s) {
    return WriteFile(opensmacx_handle(opensmacx_arg(s, 0)),
                     opensmacx_host_buffer(opensmacx_arg(s, 1),
                                           opensmacx_arg(s, 2)),
                     opensmacx_arg(s, 2),
                     opensmacx_host_as<DWORD>(opensmacx_arg(s, 3)),
                     opensmacx_host_as<OVERLAPPED>(opensmacx_arg(s, 4)));
}

#define OPENSMACX_ARGS_KERNEL32_GetFileSize 2
#define OPENSMACX_ORACLE_KERNEL32_GetFileSize unsafe  // the size depends on the writes the other side made
inline uint32_t opensmacx_shim_KERNEL32_GetFileSize(
        OpensmacxStaticRecompileState &s) {
    return GetFileSize(opensmacx_handle(opensmacx_arg(s, 0)),
                       opensmacx_host_as<DWORD>(opensmacx_arg(s, 1)));
}

#define OPENSMACX_ARGS_KERNEL32_SetFilePointer 4
#define OPENSMACX_ORACLE_KERNEL32_SetFilePointer unsafe  // moves a shared file pointer
inline uint32_t opensmacx_shim_KERNEL32_SetFilePointer(
        OpensmacxStaticRecompileState &s) {
    return SetFilePointer(opensmacx_handle(opensmacx_arg(s, 0)),
                          static_cast<LONG>(opensmacx_arg(s, 1)),
                          opensmacx_host_as<LONG>(opensmacx_arg(s, 2)),
                          opensmacx_arg(s, 3));
}

#define OPENSMACX_ARGS_KERNEL32_SetEndOfFile 1
#define OPENSMACX_ORACLE_KERNEL32_SetEndOfFile unsafe  // truncates a file
inline uint32_t opensmacx_shim_KERNEL32_SetEndOfFile(
        OpensmacxStaticRecompileState &s) {
    return SetEndOfFile(opensmacx_handle(opensmacx_arg(s, 0)));
}

#define OPENSMACX_ARGS_KERNEL32_FlushFileBuffers 1
#define OPENSMACX_ORACLE_KERNEL32_FlushFileBuffers unsafe  // writes a file
inline uint32_t opensmacx_shim_KERNEL32_FlushFileBuffers(
        OpensmacxStaticRecompileState &s) {
    return FlushFileBuffers(opensmacx_handle(opensmacx_arg(s, 0)));
}

#define OPENSMACX_ARGS_KERNEL32_GetFileType 1
#define OPENSMACX_ORACLE_KERNEL32_GetFileType safe  // a property of a handle, not of time
inline uint32_t opensmacx_shim_KERNEL32_GetFileType(
        OpensmacxStaticRecompileState &s) {
    return GetFileType(opensmacx_handle(opensmacx_arg(s, 0)));
}

#define OPENSMACX_ARGS_KERNEL32_GetStdHandle 1
#define OPENSMACX_ORACLE_KERNEL32_GetStdHandle safe  // the same three handles all run
inline uint32_t opensmacx_shim_KERNEL32_GetStdHandle(
        OpensmacxStaticRecompileState &s) {
    return opensmacx_from_handle(GetStdHandle(opensmacx_arg(s, 0)));
}

#define OPENSMACX_ARGS_KERNEL32_SetHandleCount 1
#define OPENSMACX_ORACLE_KERNEL32_SetHandleCount safe  // a no-op on NT that returns its argument
inline uint32_t opensmacx_shim_KERNEL32_SetHandleCount(
        OpensmacxStaticRecompileState &s) {
    return SetHandleCount(opensmacx_arg(s, 0));
}

#define OPENSMACX_ARGS_KERNEL32_DeleteFileA 1
#define OPENSMACX_ORACLE_KERNEL32_DeleteFileA unsafe  // deletes a file; the second call finds it gone
inline uint32_t opensmacx_shim_KERNEL32_DeleteFileA(
        OpensmacxStaticRecompileState &s) {
    return DeleteFileA(opensmacx_host_str(opensmacx_arg(s, 0)));
}

#define OPENSMACX_ARGS_KERNEL32_MoveFileA 2
#define OPENSMACX_ORACLE_KERNEL32_MoveFileA unsafe  // renames a file; the second call finds it gone
inline uint32_t opensmacx_shim_KERNEL32_MoveFileA(
        OpensmacxStaticRecompileState &s) {
    return MoveFileA(opensmacx_host_str(opensmacx_arg(s, 0)),
                     opensmacx_host_str(opensmacx_arg(s, 1)));
}

#define OPENSMACX_ARGS_KERNEL32_CreateDirectoryA 2
#define OPENSMACX_ORACLE_KERNEL32_CreateDirectoryA unsafe  // creates a directory; the second call finds it there
inline uint32_t opensmacx_shim_KERNEL32_CreateDirectoryA(
        OpensmacxStaticRecompileState &s) {
    return CreateDirectoryA(
        opensmacx_host_str(opensmacx_arg(s, 0)),
        opensmacx_host_as<SECURITY_ATTRIBUTES>(opensmacx_arg(s, 1)));
}

#define OPENSMACX_ARGS_KERNEL32_SetCurrentDirectoryA 1
#define OPENSMACX_ORACLE_KERNEL32_SetCurrentDirectoryA unsafe  // changes process state the other side then sees
inline uint32_t opensmacx_shim_KERNEL32_SetCurrentDirectoryA(
        OpensmacxStaticRecompileState &s) {
    return SetCurrentDirectoryA(opensmacx_host_str(opensmacx_arg(s, 0)));
}

#define OPENSMACX_ARGS_KERNEL32_GetCurrentDirectoryA 2
#define OPENSMACX_ORACLE_KERNEL32_GetCurrentDirectoryA safe  // reads process state nothing here changes
inline uint32_t opensmacx_shim_KERNEL32_GetCurrentDirectoryA(
        OpensmacxStaticRecompileState &s) {
    return GetCurrentDirectoryA(
        opensmacx_arg(s, 0),
        static_cast<char *>(opensmacx_host_buffer(opensmacx_arg(s, 1),
                                                  opensmacx_arg(s, 0))));
}

#define OPENSMACX_ARGS_KERNEL32_GetDriveTypeA 1
#define OPENSMACX_ORACLE_KERNEL32_GetDriveTypeA safe  // a property of the machine
inline uint32_t opensmacx_shim_KERNEL32_GetDriveTypeA(
        OpensmacxStaticRecompileState &s) {
    return GetDriveTypeA(opensmacx_host_str(opensmacx_arg(s, 0)));
}

#define OPENSMACX_ARGS_KERNEL32_FindFirstFileA 2
#define OPENSMACX_ORACLE_KERNEL32_FindFirstFileA unsafe  // a fresh handle each call
inline uint32_t opensmacx_shim_KERNEL32_FindFirstFileA(
        OpensmacxStaticRecompileState &s) {
    // WIN32_FIND_DATAA is DWORDs, FILETIMEs and two char arrays - no
    // pointers - so it can be filled in place in guest memory.
    return opensmacx_from_handle(FindFirstFileA(
        opensmacx_host_str(opensmacx_arg(s, 0)),
        opensmacx_host_as<WIN32_FIND_DATAA>(opensmacx_arg(s, 1))));
}

#define OPENSMACX_ARGS_KERNEL32_FindNextFileA 2
#define OPENSMACX_ORACLE_KERNEL32_FindNextFileA unsafe  // advances a shared enumeration
inline uint32_t opensmacx_shim_KERNEL32_FindNextFileA(
        OpensmacxStaticRecompileState &s) {
    return FindNextFileA(
        opensmacx_handle(opensmacx_arg(s, 0)),
        opensmacx_host_as<WIN32_FIND_DATAA>(opensmacx_arg(s, 1)));
}

#define OPENSMACX_ARGS_KERNEL32_FindClose 1
#define OPENSMACX_ORACLE_KERNEL32_FindClose unsafe  // the result depends on what the other side already closed
inline uint32_t opensmacx_shim_KERNEL32_FindClose(
        OpensmacxStaticRecompileState &s) {
    return FindClose(opensmacx_handle(opensmacx_arg(s, 0)));
}

#define OPENSMACX_ARGS_KERNEL32_GetModuleFileNameA 3
#define OPENSMACX_ORACLE_KERNEL32_GetModuleFileNameA safe  // constant for the life of the process
inline uint32_t opensmacx_shim_KERNEL32_GetModuleFileNameA(
        OpensmacxStaticRecompileState &s) {
    return GetModuleFileNameA(
        static_cast<HMODULE>(opensmacx_handle(opensmacx_arg(s, 0))),
        static_cast<char *>(opensmacx_host_buffer(opensmacx_arg(s, 1),
                                                  opensmacx_arg(s, 2))),
        opensmacx_arg(s, 2));
}

// --- kernel32: profile (.ini) files ----------------------------------------

#define OPENSMACX_ARGS_KERNEL32_GetPrivateProfileIntA 4
#define OPENSMACX_ORACLE_KERNEL32_GetPrivateProfileIntA safe  // reads a file it does not write
inline uint32_t opensmacx_shim_KERNEL32_GetPrivateProfileIntA(
        OpensmacxStaticRecompileState &s) {
    return GetPrivateProfileIntA(opensmacx_host_str(opensmacx_arg(s, 0)),
                                 opensmacx_host_str(opensmacx_arg(s, 1)),
                                 static_cast<INT>(opensmacx_arg(s, 2)),
                                 opensmacx_host_str(opensmacx_arg(s, 3)));
}

#define OPENSMACX_ARGS_KERNEL32_GetPrivateProfileStringA 6
#define OPENSMACX_ORACLE_KERNEL32_GetPrivateProfileStringA safe  // reads a file it does not write
inline uint32_t opensmacx_shim_KERNEL32_GetPrivateProfileStringA(
        OpensmacxStaticRecompileState &s) {
    return GetPrivateProfileStringA(
        opensmacx_host_str(opensmacx_arg(s, 0)),
        opensmacx_host_str(opensmacx_arg(s, 1)),
        opensmacx_host_str(opensmacx_arg(s, 2)),
        static_cast<char *>(opensmacx_host_buffer(opensmacx_arg(s, 3),
                                                  opensmacx_arg(s, 4))),
        opensmacx_arg(s, 4),
        opensmacx_host_str(opensmacx_arg(s, 5)));
}

#define OPENSMACX_ARGS_KERNEL32_WritePrivateProfileStringA 4
#define OPENSMACX_ORACLE_KERNEL32_WritePrivateProfileStringA unsafe  // writes a file
inline uint32_t opensmacx_shim_KERNEL32_WritePrivateProfileStringA(
        OpensmacxStaticRecompileState &s) {
    return WritePrivateProfileStringA(
        opensmacx_host_str(opensmacx_arg(s, 0)),
        opensmacx_host_str(opensmacx_arg(s, 1)),
        opensmacx_host_str(opensmacx_arg(s, 2)),
        opensmacx_host_str(opensmacx_arg(s, 3)));
}

// --- kernel32: heap --------------------------------------------------------
//
// These are implemented rather than left to the generic trap even though two
// of them cannot yet succeed, because the difference matters. HeapCreate and
// HeapDestroy work today: a HANDLE is opaque. HeapAlloc and HeapReAlloc run
// the real allocation and then fail inside opensmacx_guest(), naming the one
// thing that is wrong - the block has no guest address - instead of naming
// the function, which is not wrong at all. Give the image its canonical base
// and these four become correct with no edit.

#define OPENSMACX_ARGS_KERNEL32_HeapCreate 3
#define OPENSMACX_ORACLE_KERNEL32_HeapCreate unsafe  // a fresh handle each call
inline uint32_t opensmacx_shim_KERNEL32_HeapCreate(
        OpensmacxStaticRecompileState &s) {
    return opensmacx_from_handle(HeapCreate(opensmacx_arg(s, 0),
                                            opensmacx_arg(s, 1),
                                            opensmacx_arg(s, 2)));
}

#define OPENSMACX_ARGS_KERNEL32_HeapDestroy 1
#define OPENSMACX_ORACLE_KERNEL32_HeapDestroy unsafe  // the result depends on what the other side already destroyed
inline uint32_t opensmacx_shim_KERNEL32_HeapDestroy(
        OpensmacxStaticRecompileState &s) {
    return HeapDestroy(opensmacx_handle(opensmacx_arg(s, 0)));
}

#define OPENSMACX_ARGS_KERNEL32_HeapAlloc 3
#define OPENSMACX_ORACLE_KERNEL32_HeapAlloc unsafe  // a fresh host pointer each call
inline uint32_t opensmacx_shim_KERNEL32_HeapAlloc(
        OpensmacxStaticRecompileState &s) {
    return opensmacx_guest(HeapAlloc(opensmacx_handle(opensmacx_arg(s, 0)),
                                     opensmacx_arg(s, 1),
                                     opensmacx_arg(s, 2)));
}

#define OPENSMACX_ARGS_KERNEL32_HeapFree 3
#define OPENSMACX_ORACLE_KERNEL32_HeapFree unsafe  // the result depends on what the other side already freed
inline uint32_t opensmacx_shim_KERNEL32_HeapFree(
        OpensmacxStaticRecompileState &s) {
    return HeapFree(opensmacx_handle(opensmacx_arg(s, 0)),
                    opensmacx_arg(s, 1),
                    // The block's size is not an argument, so one byte is all
                    // that can honestly be bounded here. Unreachable in
                    // practice: the pointer can only have come from HeapAlloc,
                    // which traps in opensmacx_guest() before returning one.
                    opensmacx_host(opensmacx_arg(s, 2), 1U));
}

#define OPENSMACX_ARGS_KERNEL32_HeapSize 3
#define OPENSMACX_ORACLE_KERNEL32_HeapSize unsafe  // asks about a block only one side allocated
inline uint32_t opensmacx_shim_KERNEL32_HeapSize(
        OpensmacxStaticRecompileState &s) {
    return HeapSize(opensmacx_handle(opensmacx_arg(s, 0)),
                    opensmacx_arg(s, 1),
                    // The block's size is not an argument, so one byte is all
                    // that can honestly be bounded here. Unreachable in
                    // practice: the pointer can only have come from HeapAlloc,
                    // which traps in opensmacx_guest() before returning one.
                    opensmacx_host(opensmacx_arg(s, 2), 1U));
}

// --- kernel32: thread-local storage ----------------------------------------
//
// TLS slots hold plain 32-bit values that the guest chooses, so they need no
// translation. The slot INDEX is host-allocated and opaque.

#define OPENSMACX_ARGS_KERNEL32_TlsAlloc 0
#define OPENSMACX_ORACLE_KERNEL32_TlsAlloc unsafe  // consumes a slot, so the second call gets a different index
inline uint32_t opensmacx_shim_KERNEL32_TlsAlloc(
        OpensmacxStaticRecompileState &) {
    return TlsAlloc();
}

#define OPENSMACX_ARGS_KERNEL32_TlsGetValue 1
#define OPENSMACX_ORACLE_KERNEL32_TlsGetValue unsafe  // reads a slot the other side wrote
inline uint32_t opensmacx_shim_KERNEL32_TlsGetValue(
        OpensmacxStaticRecompileState &s) {
    return reinterpret_cast<uintptr_t>(TlsGetValue(opensmacx_arg(s, 0)));
}

#define OPENSMACX_ARGS_KERNEL32_TlsSetValue 2
#define OPENSMACX_ORACLE_KERNEL32_TlsSetValue unsafe  // writes a slot the other side then reads
inline uint32_t opensmacx_shim_KERNEL32_TlsSetValue(
        OpensmacxStaticRecompileState &s) {
    return TlsSetValue(opensmacx_arg(s, 0),
                       opensmacx_handle(opensmacx_arg(s, 1)));
}

// --- winmm -----------------------------------------------------------------

#define OPENSMACX_ARGS_WINMM_timeGetTime 0
#define OPENSMACX_ORACLE_WINMM_timeGetTime unsafe  // the counter advances between the two sides
inline uint32_t opensmacx_shim_WINMM_timeGetTime(
        OpensmacxStaticRecompileState &) {
    return timeGetTime();
}

// --- user32 ----------------------------------------------------------------
//
// Only the pure ones. Everything that needs an HWND needs a window, which
// needs RegisterClassA with a guest WndProc, which is the callback problem.

#define OPENSMACX_ARGS_USER32_GetSystemMetrics 1
#define OPENSMACX_ORACLE_USER32_GetSystemMetrics safe  // a property of the desktop
inline uint32_t opensmacx_shim_USER32_GetSystemMetrics(
        OpensmacxStaticRecompileState &s) {
    return static_cast<uint32_t>(GetSystemMetrics(
        static_cast<int>(opensmacx_arg(s, 0))));
}

#define OPENSMACX_ARGS_USER32_SetRect 5
#define OPENSMACX_ORACLE_USER32_SetRect safe  // rectangle arithmetic
inline uint32_t opensmacx_shim_USER32_SetRect(
        OpensmacxStaticRecompileState &s) {
    return SetRect(opensmacx_host_as<RECT>(opensmacx_arg(s, 0)),
                   static_cast<int>(opensmacx_arg(s, 1)),
                   static_cast<int>(opensmacx_arg(s, 2)),
                   static_cast<int>(opensmacx_arg(s, 3)),
                   static_cast<int>(opensmacx_arg(s, 4)));
}

#define OPENSMACX_ARGS_USER32_PtInRect 3
#define OPENSMACX_ORACLE_USER32_PtInRect safe  // rectangle arithmetic
inline uint32_t opensmacx_shim_USER32_PtInRect(
        OpensmacxStaticRecompileState &s) {
    // POINT is passed BY VALUE, so it occupies two stack slots rather than
    // one. That is invisible in the @12 decoration - three dwords is three
    // dwords - and is exactly the kind of detail the argument COUNT here
    // exists to keep honest against.
    POINT point;
    point.x = static_cast<LONG>(opensmacx_arg(s, 1));
    point.y = static_cast<LONG>(opensmacx_arg(s, 2));
    return PtInRect(
        opensmacx_host_as<const RECT>(opensmacx_arg(s, 0)),
        point);
}

#define OPENSMACX_ARGS_USER32_IntersectRect 3
#define OPENSMACX_ORACLE_USER32_IntersectRect safe  // rectangle arithmetic
inline uint32_t opensmacx_shim_USER32_IntersectRect(
        OpensmacxStaticRecompileState &s) {
    return IntersectRect(
        opensmacx_host_as<RECT>(opensmacx_arg(s, 0)),
        opensmacx_host_as<const RECT>(opensmacx_arg(s, 1)),
        opensmacx_host_as<const RECT>(opensmacx_arg(s, 2)));
}

#define OPENSMACX_ARGS_USER32_UnionRect 3
#define OPENSMACX_ORACLE_USER32_UnionRect safe  // rectangle arithmetic
inline uint32_t opensmacx_shim_USER32_UnionRect(
        OpensmacxStaticRecompileState &s) {
    return UnionRect(
        opensmacx_host_as<RECT>(opensmacx_arg(s, 0)),
        opensmacx_host_as<const RECT>(opensmacx_arg(s, 1)),
        opensmacx_host_as<const RECT>(opensmacx_arg(s, 2)));
}

#define OPENSMACX_ARGS_USER32_EqualRect 2
#define OPENSMACX_ORACLE_USER32_EqualRect safe  // rectangle arithmetic
inline uint32_t opensmacx_shim_USER32_EqualRect(
        OpensmacxStaticRecompileState &s) {
    return EqualRect(
        opensmacx_host_as<const RECT>(opensmacx_arg(s, 0)),
        opensmacx_host_as<const RECT>(opensmacx_arg(s, 1)));
}

#define OPENSMACX_ARGS_USER32_IsRectEmpty 1
#define OPENSMACX_ORACLE_USER32_IsRectEmpty safe  // rectangle arithmetic
inline uint32_t opensmacx_shim_USER32_IsRectEmpty(
        OpensmacxStaticRecompileState &s) {
    return IsRectEmpty(
        opensmacx_host_as<const RECT>(opensmacx_arg(s, 0)));
}

#define OPENSMACX_ARGS_USER32_CharUpperA 1
#define OPENSMACX_ORACLE_USER32_CharUpperA safe  // a fixed case table for a fixed locale
inline uint32_t opensmacx_shim_USER32_CharUpperA(
        OpensmacxStaticRecompileState &s) {
    // CharUpperA is overloaded on its argument: a value below 0x10000 is a
    // character, anything else is a string pointer edited in place. Both
    // return the same kind of thing they were given, so the character case
    // must NOT go through opensmacx_guest().
    const uint32_t argument = opensmacx_arg(s, 0);
    if (argument < 0x10000U) {
        return reinterpret_cast<uintptr_t>(CharUpperA(
            reinterpret_cast<char *>(static_cast<uintptr_t>(argument))));
    }
    CharUpperA(opensmacx_host_str_mutable(argument));
    return argument;
}

#define OPENSMACX_ARGS_USER32_CharLowerA 1
#define OPENSMACX_ORACLE_USER32_CharLowerA safe  // a fixed case table for a fixed locale
inline uint32_t opensmacx_shim_USER32_CharLowerA(
        OpensmacxStaticRecompileState &s) {
    const uint32_t argument = opensmacx_arg(s, 0);
    if (argument < 0x10000U) {
        return reinterpret_cast<uintptr_t>(CharLowerA(
            reinterpret_cast<char *>(static_cast<uintptr_t>(argument))));
    }
    CharLowerA(opensmacx_host_str_mutable(argument));
    return argument;
}

#define OPENSMACX_ARGS_USER32_MessageBeep 1
#define OPENSMACX_ORACLE_USER32_MessageBeep safe  // sound only; no result difference and no memory effect
inline uint32_t opensmacx_shim_USER32_MessageBeep(
        OpensmacxStaticRecompileState &s) {
    return MessageBeep(opensmacx_arg(s, 0));
}

#define OPENSMACX_ARGS_USER32_GetKeyState 1
#define OPENSMACX_ORACLE_USER32_GetKeyState unsafe  // reads the real keyboard at the instant of the call
inline uint32_t opensmacx_shim_USER32_GetKeyState(
        OpensmacxStaticRecompileState &s) {
    return static_cast<uint32_t>(static_cast<int32_t>(
        GetKeyState(static_cast<int>(opensmacx_arg(s, 0)))));
}

#define OPENSMACX_ARGS_USER32_GetAsyncKeyState 1
#define OPENSMACX_ORACLE_USER32_GetAsyncKeyState unsafe  // clears the since-last-call bit, so the second call differs
inline uint32_t opensmacx_shim_USER32_GetAsyncKeyState(
        OpensmacxStaticRecompileState &s) {
    return static_cast<uint32_t>(static_cast<int32_t>(
        GetAsyncKeyState(static_cast<int>(opensmacx_arg(s, 0)))));
}

#define OPENSMACX_ARGS_USER32_MapVirtualKeyA 2
#define OPENSMACX_ORACLE_USER32_MapVirtualKeyA safe  // a property of the keyboard layout
inline uint32_t opensmacx_shim_USER32_MapVirtualKeyA(
        OpensmacxStaticRecompileState &s) {
    return MapVirtualKeyA(opensmacx_arg(s, 0), opensmacx_arg(s, 1));
}

#define OPENSMACX_ARGS_USER32_GetCursorPos 1
#define OPENSMACX_ORACLE_USER32_GetCursorPos unsafe  // reads the real pointer at the instant of the call
inline uint32_t opensmacx_shim_USER32_GetCursorPos(
        OpensmacxStaticRecompileState &s) {
    return GetCursorPos(
        opensmacx_host_as<POINT>(opensmacx_arg(s, 0)));
}

#define OPENSMACX_ARGS_USER32_SetCursorPos 2
#define OPENSMACX_ORACLE_USER32_SetCursorPos unsafe  // moves the real pointer
inline uint32_t opensmacx_shim_USER32_SetCursorPos(
        OpensmacxStaticRecompileState &s) {
    return SetCursorPos(static_cast<int>(opensmacx_arg(s, 0)),
                        static_cast<int>(opensmacx_arg(s, 1)));
}

#define OPENSMACX_ARGS_USER32_ShowCursor 1
#define OPENSMACX_ORACLE_USER32_ShowCursor unsafe  // increments a global display counter
inline uint32_t opensmacx_shim_USER32_ShowCursor(
        OpensmacxStaticRecompileState &s) {
    return static_cast<uint32_t>(ShowCursor(opensmacx_arg(s, 0) != 0U));
}

// --- user32: the window functions ------------------------------------------
//
// These take an HWND the guest never legitimately owns yet, because creating a
// window needs RegisterClassA with a guest WndProc and that is the callback
// problem. They are implemented anyway, and the reason is the interesting one:
// an HWND is OPAQUE, so nothing here has to marshal it, and Win32's documented
// behaviour for an invalid window handle is a DEFINED failure - FALSE, 0, or
// CLR_INVALID - with SetLastError(ERROR_INVALID_WINDOW_HANDLE) and no side
// effect. So calling the real function with the guest's real argument is both
// faithful and repeatable, which is exactly what the differential oracle needs:
// the original and the lifted side make the same call and get the same
// defined failure. That is why they are marked `safe` below while
// CreateSolidBrush - which succeeds, and succeeds DIFFERENTLY each time - is
// not. Nothing here fabricates a result; if a real window ever exists these
// keep working unchanged.

#define OPENSMACX_ARGS_USER32_PostMessageA 4
#define OPENSMACX_ORACLE_USER32_PostMessageA safe  // no image memory changes
inline uint32_t opensmacx_shim_USER32_PostMessageA(
        OpensmacxStaticRecompileState &s) {
    return PostMessageA(
        static_cast<HWND>(opensmacx_handle(opensmacx_arg(s, 0))),
        opensmacx_arg(s, 1),
        static_cast<WPARAM>(opensmacx_arg(s, 2)),
        static_cast<LPARAM>(opensmacx_arg(s, 3)));
}

#define OPENSMACX_ARGS_USER32_GetWindowLongA 2
#define OPENSMACX_ORACLE_USER32_GetWindowLongA safe  // reads window state only
inline uint32_t opensmacx_shim_USER32_GetWindowLongA(
        OpensmacxStaticRecompileState &s) {
    return static_cast<uint32_t>(GetWindowLongA(
        static_cast<HWND>(opensmacx_handle(opensmacx_arg(s, 0))),
        static_cast<int>(opensmacx_arg(s, 1))));
}

#define OPENSMACX_ARGS_USER32_IsZoomed 1
#define OPENSMACX_ORACLE_USER32_IsZoomed safe  // reads window state only
inline uint32_t opensmacx_shim_USER32_IsZoomed(
        OpensmacxStaticRecompileState &s) {
    return IsZoomed(static_cast<HWND>(opensmacx_handle(opensmacx_arg(s, 0))));
}

#define OPENSMACX_ARGS_USER32_ShowWindow 2
#define OPENSMACX_ORACLE_USER32_ShowWindow safe  // no image memory changes
inline uint32_t opensmacx_shim_USER32_ShowWindow(
        OpensmacxStaticRecompileState &s) {
    return ShowWindow(
        static_cast<HWND>(opensmacx_handle(opensmacx_arg(s, 0))),
        static_cast<int>(opensmacx_arg(s, 1)));
}

#define OPENSMACX_ARGS_USER32_DestroyWindow 1
#define OPENSMACX_ORACLE_USER32_DestroyWindow safe  // no image memory changes
inline uint32_t opensmacx_shim_USER32_DestroyWindow(
        OpensmacxStaticRecompileState &s) {
    return DestroyWindow(
        static_cast<HWND>(opensmacx_handle(opensmacx_arg(s, 0))));
}

#define OPENSMACX_ARGS_USER32_GetWindowRect 2
#define OPENSMACX_ORACLE_USER32_GetWindowRect safe  // reads window state only
inline uint32_t opensmacx_shim_USER32_GetWindowRect(
        OpensmacxStaticRecompileState &s) {
    return GetWindowRect(
        static_cast<HWND>(opensmacx_handle(opensmacx_arg(s, 0))),
        opensmacx_host_as<RECT>(opensmacx_arg(s, 1)));
}

#define OPENSMACX_ARGS_USER32_InvalidateRect 3
#define OPENSMACX_ORACLE_USER32_InvalidateRect safe  // no image memory changes
inline uint32_t opensmacx_shim_USER32_InvalidateRect(
        OpensmacxStaticRecompileState &s) {
    // The RECT is optional - NULL means the whole client area - so it goes
    // through opensmacx_host_as, which maps guest 0 to nullptr rather than to
    // the image base.
    return InvalidateRect(
        static_cast<HWND>(opensmacx_handle(opensmacx_arg(s, 0))),
        opensmacx_host_as<const RECT>(opensmacx_arg(s, 1)),
        opensmacx_arg(s, 2) != 0);
}

#define OPENSMACX_ARGS_USER32_DefWindowProcA 4
#define OPENSMACX_ORACLE_USER32_DefWindowProcA safe  // no image memory changes
inline uint32_t opensmacx_shim_USER32_DefWindowProcA(
        OpensmacxStaticRecompileState &s) {
    // The LPARAM of several messages is a pointer, and it is NOT translated
    // here: with no real window the message is rejected before the parameter
    // is read. If a real window ever backs this, every pointer-carrying
    // message needs its own marshalling and this comment is the work item.
    return static_cast<uint32_t>(DefWindowProcA(
        static_cast<HWND>(opensmacx_handle(opensmacx_arg(s, 0))),
        opensmacx_arg(s, 1),
        static_cast<WPARAM>(opensmacx_arg(s, 2)),
        static_cast<LPARAM>(opensmacx_arg(s, 3))));
}

#define OPENSMACX_ARGS_USER32_UnregisterClassA 2
#define OPENSMACX_ORACLE_USER32_UnregisterClassA safe  // no image memory changes
inline uint32_t opensmacx_shim_USER32_UnregisterClassA(
        OpensmacxStaticRecompileState &s) {
    // Overloaded on its first argument exactly as CharUpperA is: a value below
    // 0x10000 is an ATOM, not a pointer, and must not be marshalled. MAKEINTATOM
    // is what the caller used to build it.
    const uint32_t name = opensmacx_arg(s, 0);
    const HINSTANCE instance =
        static_cast<HINSTANCE>(opensmacx_handle(opensmacx_arg(s, 1)));
    if (name < 0x10000U) {
        return UnregisterClassA(
            reinterpret_cast<const char *>(static_cast<uintptr_t>(name)),
            instance);
    }
    return UnregisterClassA(opensmacx_host_str(name), instance);
}

// --- gdi32 -----------------------------------------------------------------

#define OPENSMACX_ARGS_GDI32_GetStockObject 1
#define OPENSMACX_ORACLE_GDI32_GetStockObject safe  // the stock handles are the same every call
inline uint32_t opensmacx_shim_GDI32_GetStockObject(
        OpensmacxStaticRecompileState &s) {
    return opensmacx_from_handle(GetStockObject(
        static_cast<int>(opensmacx_arg(s, 0))));
}

#define OPENSMACX_ARGS_GDI32_DeleteObject 1
#define OPENSMACX_ORACLE_GDI32_DeleteObject unsafe  // the result depends on what the other side already deleted
inline uint32_t opensmacx_shim_GDI32_DeleteObject(
        OpensmacxStaticRecompileState &s) {
    return DeleteObject(opensmacx_handle(opensmacx_arg(s, 0)));
}

#define OPENSMACX_ARGS_GDI32_GetDeviceCaps 2
#define OPENSMACX_ORACLE_GDI32_GetDeviceCaps safe  // a property of a device; changes nothing
inline uint32_t opensmacx_shim_GDI32_GetDeviceCaps(
        OpensmacxStaticRecompileState &s) {
    return static_cast<uint32_t>(GetDeviceCaps(
        static_cast<HDC>(opensmacx_handle(opensmacx_arg(s, 0))),
        static_cast<int>(opensmacx_arg(s, 1))));
}

#define OPENSMACX_ARGS_GDI32_GdiFlush 0
#define OPENSMACX_ORACLE_GDI32_GdiFlush safe  // flushes a batch this process never fills
inline uint32_t opensmacx_shim_GDI32_GdiFlush(
        OpensmacxStaticRecompileState &) {
    return GdiFlush();
}

#define OPENSMACX_ARGS_GDI32_CreateSolidBrush 1
#define OPENSMACX_ORACLE_GDI32_CreateSolidBrush unsafe  // a fresh handle each call
inline uint32_t opensmacx_shim_GDI32_CreateSolidBrush(
        OpensmacxStaticRecompileState &s) {
    return opensmacx_from_handle(CreateSolidBrush(opensmacx_arg(s, 0)));
}

// The same argument as for the window functions: an HDC the guest does not own
// is opaque, and every one of these has a DEFINED failure for an invalid one -
// FALSE, 0, or CLR_INVALID - with nothing written. The guest's real argument
// goes to the real function.

#define OPENSMACX_ARGS_GDI32_DeleteDC 1
#define OPENSMACX_ORACLE_GDI32_DeleteDC safe  // no image memory changes
inline uint32_t opensmacx_shim_GDI32_DeleteDC(
        OpensmacxStaticRecompileState &s) {
    return DeleteDC(static_cast<HDC>(opensmacx_handle(opensmacx_arg(s, 0))));
}

#define OPENSMACX_ARGS_GDI32_SetTextColor 2
#define OPENSMACX_ORACLE_GDI32_SetTextColor safe  // no image memory changes
inline uint32_t opensmacx_shim_GDI32_SetTextColor(
        OpensmacxStaticRecompileState &s) {
    return SetTextColor(
        static_cast<HDC>(opensmacx_handle(opensmacx_arg(s, 0))),
        static_cast<COLORREF>(opensmacx_arg(s, 1)));
}

#define OPENSMACX_ARGS_GDI32_StretchBlt 11
#define OPENSMACX_ORACLE_GDI32_StretchBlt safe  // no image memory changes
inline uint32_t opensmacx_shim_GDI32_StretchBlt(
        OpensmacxStaticRecompileState &s) {
    return StretchBlt(
        static_cast<HDC>(opensmacx_handle(opensmacx_arg(s, 0))),
        static_cast<int>(opensmacx_arg(s, 1)),
        static_cast<int>(opensmacx_arg(s, 2)),
        static_cast<int>(opensmacx_arg(s, 3)),
        static_cast<int>(opensmacx_arg(s, 4)),
        static_cast<HDC>(opensmacx_handle(opensmacx_arg(s, 5))),
        static_cast<int>(opensmacx_arg(s, 6)),
        static_cast<int>(opensmacx_arg(s, 7)),
        static_cast<int>(opensmacx_arg(s, 8)),
        static_cast<int>(opensmacx_arg(s, 9)),
        static_cast<DWORD>(opensmacx_arg(s, 10)));
}

#define OPENSMACX_ARGS_GDI32_SetDIBColorTable 4
#define OPENSMACX_ORACLE_GDI32_SetDIBColorTable safe  // no image memory changes
inline uint32_t opensmacx_shim_GDI32_SetDIBColorTable(
        OpensmacxStaticRecompileState &s) {
    // The colour table is `entries` RGBQUADs chosen by the guest, so the bound
    // is the PRODUCT and it is computed in 64 bits - see opensmacx_host_array.
    const uint32_t entries = opensmacx_arg(s, 2);
    return SetDIBColorTable(
        static_cast<HDC>(opensmacx_handle(opensmacx_arg(s, 0))),
        opensmacx_arg(s, 1), entries,
        opensmacx_host_array<const RGBQUAD>(opensmacx_arg(s, 3), entries));
}

#define OPENSMACX_ARGS_GDI32_AnimatePalette 4
#define OPENSMACX_ORACLE_GDI32_AnimatePalette safe  // no image memory changes
inline uint32_t opensmacx_shim_GDI32_AnimatePalette(
        OpensmacxStaticRecompileState &s) {
    const uint32_t entries = opensmacx_arg(s, 2);
    return AnimatePalette(
        static_cast<HPALETTE>(opensmacx_handle(opensmacx_arg(s, 0))),
        opensmacx_arg(s, 1), entries,
        opensmacx_host_array<const PALETTEENTRY>(opensmacx_arg(s, 3), entries));
}

#endif  // OPENSMACX_LIFTED_IMPORTS_H
