// ORIGINAL: 0x00532940 FILE
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00532940
// name           ?send_message@NetDaemon@@QAEXPADKH@Z
// size           260 bytes
// measured tier  NO_COMPILE
// refusal        u00532940.cpp(46) : error C2653: 'NetDaemon' : is not a class or namespace name u00532940.cpp(47) : error C2065: 'g_0093f660' : undeclared identifier u00532940.
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00532940/unit.cpp
// and score it with tools/agent_brief.py.
// 0x00532940  ?send_message@NetDaemon@@QAEXPADKH@Z  (260 bytes)
//
// A four-way dispatch on bits of `*(short*)a1`: 0x4000 (timestamp+send),
// 0x8000 (send with a possibly-substituted value), 0xa000 (an extra
// who_2_pid path, falling back to the same substitution as 0x8000), and a
// default (send with `a3` or `who_2_pid(a3)`). `Net::send` and
// `AlphaNet::who_2_pid` are both called through `reinterpret_cast<Net*>` /
// `reinterpret_cast<AlphaNet*>(this)`: NetDaemon is opaque and neither base
// is declared as a real base class in this unit.
//
// `timeGetTime` is called through its IAT slot (g_00669368), same pattern
// as UNK2's Win32 calls (0x00609bf0) - a `__stdcall` function-pointer
// typedef dereferenced off the fixed-address global, not a named extern.
//
// THREE occurrences of "value = (global==1) ? a3 : fallback" (two computing
// `this->field_764` for the same 0x764 offset, one computing
// `who_2_pid(a3)`) each needed the SAME two source-form fixes to reproduce
// their original instruction order, none of them obvious from a plain
// if/else:
//
//   1. `if (cond) { return_early_no_assignment; }` (no else) let the
//      compiler duplicate the ENTIRE trailing `send(...)` call and epilogue
//      into both arms instead of merging on one shared call site - a
//      single-branch if with no else, immediately followed by the
//      unconditional `send`, is NOT enough; it needs an explicit
//      `if (..) { value = A; } else { value = B; }` with a dedicated local,
//      merging back to ONE assignment before the shared call.
//   2. Even with that, a REUSED PARAMETER (`if (x!=1) a3 = fallback;`)
//      compiled the load-and-compare in the opposite order from a fresh
//      local (`int value; if (x==1) value=a3; else value=fallback; a3=value;`)
//      - original loads the flag/condition FIRST, then the "keep a3" arm,
//      matching the fresh-local form, not the reused-parameter form.
//   3. A trivial `a3 = a3;` self-assignment (tried for the third,
//      who_2_pid, occurrence, to force the same shape without a spare
//      local) gets optimized away entirely, losing the explicit
//      reload+jmp the original has - only the real `int value; if (){ }
//      else { }` form survives with the merge point intact.
//
// The 0xa000 branch also reads `*g_0093e8d0` ONCE into a local (`counter`)
// BEFORE its four-way `&&` guard, and the branch's own fallback re-tests
// that SAME local rather than re-reading the global - the original never
// reloads `[0x93e8d0]` a second time inside this branch (one `mov`, two
// `cmp`s against the same register).
typedef unsigned long(__stdcall *TimeGetTimeFn)();

void NetDaemon::send_message(char *a1, unsigned long a2, int a3) {
    if (*g_0093f660) {
        unsigned short flags = *reinterpret_cast<unsigned short *>(a1);
        if (flags & 0x4000) {
            *reinterpret_cast<unsigned long *>(a1 + 8) =
                (*reinterpret_cast<TimeGetTimeFn *>(g_00669368))();
            reinterpret_cast<Net *>(this)->send(a1, a2, 0, 1);
            return;
        }
        if (flags & 0x8000) {
            int value;
            if (*g_0093e8d0 == 1) {
                value = a3;
            } else {
                value = *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x764);
            }
            a3 = value;
            reinterpret_cast<Net *>(this)->send(a1, a2, a3, 1);
            return;
        }
        if (flags & 0xa000) {
            int counter = *g_0093e8d0;
            if ((*reinterpret_cast<unsigned char *>(g_009a681c) & 0x10) &&
                *g_0093a938 == 0 && *g_009a6820 > 0 && counter > 1) {
                reinterpret_cast<Net *>(this)->send(
                    a1, a2, reinterpret_cast<AlphaNet *>(this)->who_2_pid(*g_009a6820), 1);
                return;
            }
            int value;
            if (counter == 1) {
                value = a3;
            } else {
                value = *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x764);
            }
            a3 = value;
            reinterpret_cast<Net *>(this)->send(a1, a2, a3, 1);
            return;
        }
        int value;
        if (*g_0093e8d0 == 1) {
            value = a3;
        } else {
            value = reinterpret_cast<AlphaNet *>(this)->who_2_pid(a3);
        }
        a3 = value;
        reinterpret_cast<Net *>(this)->send(a1, a2, a3, 1);
    }
}
