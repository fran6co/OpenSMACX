// ORIGINAL: 0x00609BF0 FILE
// name      ?UNK2@Dialog@@QAEXHHHH@Z
// size      112 bytes
// spans     0x00609BF0-0x00609C60
// prototype void (__thiscall ?UNK2@Dialog@@QAEXHHHH@Z)(Dialog* this, HWND hWndParent, UINT Msg, HWND hWnd, LPARAM lParam)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00609BF0
// measured tier  NO_COMPILE
// refusal        u00609bf0.cpp(39) : error C2653: 'Dialog' : is not a class or namespace name u00609bf0.cpp(42) : error C2065: 'g_006692dc' : undeclared identifier u00609bf0.cpp
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00609bf0/unit.cpp
// and score it with tools/agent_brief.py.
// 0x00609BF0  ?UNK2@Dialog@@QAEXHHHH@Z  (112 bytes)  -- MISMATCH, KEPT
//
// A WndProc-shaped function: no `ecx`/`this` read anywhere, all four params
// come off the stack, and `ret 0x10` pops exactly 4 dwords. The catalogue's
// `void Dialog::UNK2(int,int,int,int)` head still binds fine (an unused
// implicit `this` costs nothing here), so no signature change is proposed.
//
// `a2` is compared with `jb`/`ja` (unsigned), not `jl`/`jg` - matches
// `uint param_2` in the Ghidra guess, not the catalogue's signed `int`. The
// class declaration fixes all four params as plain `int`, so the parameter
// stays `int a2` in this definition (an out-of-line definition must match
// the declared types exactly, C2511 otherwise) and is copied into an
// `unsigned int` local for the range test instead.
//
// `IsChild`/`GetWindowLongA`/`CallWindowProcA` are never declared as named
// externs by the emitter for this address - only their IAT slots
// (g_006692dc, g_0066934c, g_006692ac) show up as "fixed globals". Each is
// called through a `__stdcall` function-pointer typedef dereferenced off
// that slot, matching `call dword ptr [addr]`.
//
// RULED OUT, not fixed: the original has TWO physically separate
// `pop edi; pop esi; pop ebx; ret 0x10` epilogues (one for the early
// `if (GetWindowLongA(...) == 0) return;`, one for the common tail after
// `CallWindowProcA`), 112 bytes total. Every source form tried here -
// `if (p==0) return;` early, `if (p!=0) {...} else return;` inverted,
// - compiles to ONE shared epilogue with a forward jump into it (106
// bytes, similarity 0.9286, single edit at the early-return branch: original
// `jne`+3×`pop`+`ret` vs rebuilt one `je`). This is the compiler's own
// cross-jump/tail-merging pass recognizing the two epilogues are byte-
// identical and folding them; it runs on the compiled code's shape, not the
// source spelling that produced it, so no source restructuring reached it.
// Tried across all 4 measured flag combinations via `byte_match.py
// --source`, not just the default; all four either keep the frame pointer
// (wrong prologue) or hit this exact merge.
typedef int (__stdcall *IsChildFn)(void *, void *);
typedef int *(__stdcall *GetWindowLongFn)(void *, int);
typedef int (__stdcall *CallWindowProcFn)(void *, void *, unsigned int, int, int);

void Dialog::UNK2(int a1, int a2, int a3, int a4) {
    unsigned int msg = a2;
    if (msg >= 7 && msg <= 8) {
        if (!(*reinterpret_cast<IsChildFn *>(g_006692dc))(
                reinterpret_cast<void *>(a1), reinterpret_cast<void *>(a3))) {
            int *p = (*reinterpret_cast<GetWindowLongFn *>(g_0066934c))(
                reinterpret_cast<void *>(a1), -0x15);
            if (p == 0) {
                return;
            }
            if ((p[0x26] & 0x1000) == 0 && *g_009b7b28 != 0) {
                reinterpret_cast<VCall *>(p)->slot062();
            }
        }
    }
    (*reinterpret_cast<CallWindowProcFn *>(g_006692ac))(
        g_005d5c50, reinterpret_cast<void *>(a1), a2, a3, a4);
}
