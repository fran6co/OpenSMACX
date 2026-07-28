#ifndef OPENSMACX_LIFTED_ORACLE_H
#define OPENSMACX_LIFTED_ORACLE_H

// A differential oracle that tests a LIFTED function against the ORIGINAL
// MACHINE CODE of terranx.exe, for any function, with no hand-written fixture.
//
// ---------------------------------------------------------------------------
// THE IDEA
// ---------------------------------------------------------------------------
//
// The whole-image lift models one flat guest address space. Once the original
// bytes are present at their canonical addresses, the original function can
// simply be CALLED - and because the whole image is there, it calls its real
// neighbours natively, with no stubbing. The lifted function runs on a second,
// independent copy of the same memory. Give both sides identical starting CPU
// state and identical memory, run each, and compare final registers, flags and
// every byte of memory either one wrote.
//
// The original IS the expectation. Nothing here reads a disassembly and writes
// down what it thinks should happen.
//
// ---------------------------------------------------------------------------
// TWO COPIES OF MEMORY, AND WHERE THEY LIVE
// ---------------------------------------------------------------------------
//
//   pristine  - a heap buffer holding the PE as loaded: every section at its
//               virtual address, everything else zero. Never executed.
//   side A    - the ORIGINAL's memory, at its canonical 0x00400000, because
//               the code is full of absolute addresses and cannot run
//               anywhere else. Executable (see the protection note below).
//   side B    - the LIFTED side's memory: `opensmacx_image`, an ordinary
//               array wherever the linker put it. The lifted code reaches it
//               only through `opensmacx_at()`, so its host address is free.
//
// A write by one side cannot be seen by the other; that is the entire reason
// there are two. Both are restored from `pristine` before every case.
//
// ---------------------------------------------------------------------------
// GETTING 0x00400000 (this took three attempts, so the note is here)
// ---------------------------------------------------------------------------
//
// Under Wine the range is NOT free when a rebased harness starts: the loader
// has already put file mappings at 0x400000, 0x450000, 0x470000, 0x510000,
// 0x850000 and 0x860000 plus one private region at 0x490000. Unmapping them
// from inside the process appears to work and then kills the process on the
// next `fopen`, because one of them is the NLS/codepage data Wine's CRT reads
// on every locale-sensitive call. That failure is silent until it is fatal.
//
// So the harness RELAUNCHES ITSELF: the parent creates the child suspended -
// at which point only ntdll and the image are mapped - reserves the whole span
// in the child with VirtualAllocEx, and only then resumes it. The child finds
// the span already reserved, commits it, and Wine's later mappings go
// elsewhere on their own. Measured: with this in place `fopen`, `malloc`,
// stdio and CreateThread all keep working in the child.
//
// ---------------------------------------------------------------------------
// EXECUTABLE PAGES
// ---------------------------------------------------------------------------
//
// Side A is committed PAGE_EXECUTE_READWRITE for the code sections (.text and
// _SELFMOD) and PAGE_READWRITE for everything else, so a wild jump into data
// faults instead of executing whatever the seed left there. Whether Wine on
// this host actually enforces the no-execute half is reported by the harness's
// own self-test (`--selftest`) rather than assumed.
//
// ---------------------------------------------------------------------------
// SEEDING
// ---------------------------------------------------------------------------
//
// Every input is a pure function of (function address, case index), so a
// failure is reproducible from its address alone: `--only 0x004042a0` replays
// it exactly. See `oracle_seed_case` in the .cpp for the rule, and
// `oracle_seed_description` for the one-line summary the report prints.
//
// The stack and the scratch arena both live in the guest's .reloc section
// (0x009d0000..0x00a0c000). Relocations are dead weight in a module loaded at
// its preferred base, so that range is the one part of the address space no
// game code reads, and putting the seed there keeps it out of real data.
//
// ---------------------------------------------------------------------------
// WHAT CAN GO WRONG IN THE ORIGINAL, AND WHY IT IS NOT A FAILURE
// ---------------------------------------------------------------------------
//
// Fed a seeded pointer, the original may fault or loop forever. A vectored
// exception handler catches the fault and a trap-flag instruction budget
// catches the loop; both make the case INCONCLUSIVE, never FAILED. Conflating
// "the original could not run on this input" with "the lift disagrees with the
// original" is the easiest way to make a harness like this lie, in the
// direction that flatters it.
//
// The reverse asymmetry IS a failure and is reported as one: the original
// completing while the lifted side faults means the lift invented an access
// the original never made.
//
// ---------------------------------------------------------------------------
// KNOWN GAPS, measured rather than guessed
// ---------------------------------------------------------------------------
//
// 1. THE ARENA HOLDS NO NEGATIVE VALUES. Its words are arena pointers or small
//    positive integers. tools/lifted_oracle_mutate.py turned `sar` into `shr`
//    in lifted_00404ef0 and the oracle passed it, because the value shifted
//    comes from an object field and every seed makes that field non-negative,
//    where the two shifts agree. The fix is one line - a case whose arena
//    non-pointer words are negative - and it is deliberately NOT applied here
//    because it changes every number in the report and those numbers were
//    measured with this seed. It is the first thing to do next.
//
// 2. GLOBALS ARE NEVER VARIED. Both sides start from the PE's .data as loaded,
//    so a lowering that only misbehaves on a value the game computes at
//    runtime is invisible.
//
// 3. THE FPU IS RESET BUT NOT COMPARED. A divergence confined to ST(0), with
//    nothing written to memory, would pass. The count of x87-using functions
//    among the passes is printed for exactly that reason.
//
// 4. NEGATIVE ARGUMENTS AND POINTER ARGUMENTS ARE THE SAME SLOT. Case 3 puts
//    negative values in half the argument words, and this image passes
//    pointers in those words, so some of its failures are wild reads rather
//    than lowering bugs. Cases 0 to 2 keep every argument in range; a failure
//    that reproduces there is worth much more than one that only appears in
//    case 3.

#include <cstdint>
#include <cstddef>

// --------------------------------------------------------------------------
// Guest layout
// --------------------------------------------------------------------------

constexpr uint32_t OracleImageBase = 0x00400000U;
constexpr uint32_t OracleImageSize = 0x0060c000U;
// Rounded up to the 64 KiB allocation granularity so the reservation and the
// commit inside it agree about where the span ends.
constexpr uint32_t OracleSpanReservation = 0x00610000U;

// Everything free below this is reserved address space so that a wild guest
// write lands on nothing instead of on the host's heap. See the comment at the
// wall in oracle_bootstrap - this is the difference between a case being
// INCONCLUSIVE and the harness dying four printf calls later.
constexpr uint32_t OracleWallTop = 0x08000000U;

// .reloc, 0x009d0000..0x00a0c000: dead in a module at its preferred base.
constexpr uint32_t OracleArenaBase = 0x009d0000U;
constexpr uint32_t OracleArenaSize = 0x00020000U;
constexpr uint32_t OracleStackLow = 0x009f0000U;
constexpr uint32_t OracleStackTop = 0x009fe000U;   // initial ESP
constexpr uint32_t OracleScratchEnd = 0x00a0b000U;
constexpr uint32_t OracleSeedArgWords = 12;        // args written above ESP

constexpr int OracleCasesPerFunction = 4;

// --------------------------------------------------------------------------
// Verdicts
// --------------------------------------------------------------------------

enum OracleVerdict {
    OracleUnrun = 0,
    OraclePass,               // both ran, registers + flags + memory agree
    OracleFail,               // both ran, something disagrees
    OracleFailLiftedFault,    // original completed, lifted faulted
    OracleSkipTrap,           // lifted hit opensmacx_trap: not lowered yet
    OracleInconclusiveFault,  // the original itself faulted
    OracleInconclusiveBudget, // the original blew the instruction budget
    OracleInconclusiveLiftedBudget,
    // The lifted side dereferenced outside `opensmacx_image`. That is provably
    // a guest address the lift's model does not cover: `opensmacx_at` is a
    // subtraction with no bounds check, so anything outside the 6.3 MB span
    // reads host memory. The original meanwhile reads the real address, which
    // under Wine on this host does not even fault near 4 GiB - it returns zero.
    // Neither side is running the program, so the case cannot be judged.
    //
    // This bucket is reported separately and NOT folded into the pass rate,
    // because it can also hide a real bug: a lowering that computes the wrong
    // address lands here too, and this harness cannot tell the two apart.
    OracleInconclusiveOutOfSpan,
};

const char *oracle_verdict_name(OracleVerdict verdict);

// --------------------------------------------------------------------------
// One case
// --------------------------------------------------------------------------

struct OracleRegisters {
    uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi, eflags;
};

struct OracleResult {
    OracleVerdict verdict;
    uint32_t address;
    int case_index;
    OracleRegisters before;
    OracleRegisters original;
    OracleRegisters lifted;
    uint32_t register_mask;    // bit i set: register i disagrees (index order)
    uint32_t flag_mask;        // OracleFlag* bits that disagree
    uint32_t memory_diffs;     // number of differing 4-byte words
    uint32_t first_diff;       // guest address of the first differing word
    uint32_t first_diff_original;
    uint32_t first_diff_lifted;
    uint32_t first_diff_pristine;
    uint32_t original_steps;   // only counted when the budget is armed
    uint32_t fault_code;
    uint32_t fault_address;
    uint32_t fault_data;      // the address the faulting access named
    uint32_t blamed_eip;       // instruction that produced the divergence
    const char *trap_reason;
};

// The seven flags the lift models. AF is carried separately in the report
// because the manuals leave it undefined after several of the instructions
// this image uses, so an AF-only divergence is not the same class of finding.
//
// DF is here because leaving it out was a false-PASS hole, not a nicety: the
// lift models it (OpensmacxDirectionFlag) and 305 string sites read it, so a
// lowering that leaves DF set would reverse every later rep movs/stos and
// still PASS. It cannot be read with lahf, which is why it was missing; it is
// captured with pushfl on the HOST stack instead.
enum OracleFlagBit : uint32_t {
    OracleFlagCF = 1U << 0,
    OracleFlagPF = 1U << 1,
    OracleFlagAF = 1U << 2,
    OracleFlagZF = 1U << 3,
    OracleFlagSF = 1U << 4,
    OracleFlagOF = 1U << 5,
    OracleFlagDF = 1U << 6,
};

// --------------------------------------------------------------------------
// API
// --------------------------------------------------------------------------

// Parent/child bootstrap. Returns 1 when this process was the parent (it has
// already run the child to completion; `*exit_code` is the child's status and
// the caller must exit with it), 0 when this process is the child and should
// carry on. Any other value is a bootstrap failure.
int oracle_bootstrap(int argc, char **argv, int *exit_code);

// Loads the user's own terranx.exe into `pristine` and stamps side A and B.
// Returns an error string, or nullptr on success. Nothing from the executable
// is ever written back out.
const char *oracle_load_image(const char *path);

// Arms the vectored handler and the budget. `budget` of 0 disables the
// trap-flag instruction budget (much faster, no runaway protection beyond the
// watchdog); anything else is a hard cap on instructions executed by the
// original.
void oracle_arm(uint32_t budget, uint32_t watchdog_ms);

// Runs one case. Deterministic in (address, case_index).
OracleResult oracle_run_case(uint32_t address, int case_index);

// --------------------------------------------------------------------------
// Proving the comparison itself is live.
//
// The mutation harness breaks the LIFTED side and checks the oracle notices.
// It cannot notice anything about the ORACLE: deleting the whole-image memory
// comparison, or comparing only EAX, or excusing ESP, each left that harness
// reporting the identical score, because its mutants all landed on four tiny
// functions that touch no memory and answer in EAX.
//
// So each comparison is made to prove itself directly. After a case that
// PASSES, the lifted result is perturbed one field at a time and the verdict
// must become FAIL every time. A comparison that has been deleted, narrowed or
// excused cannot survive that, and it costs one extra run rather than a
// rebuild.
//
// `oracle_perturb` selects what to damage between the lifted body returning
// and the comparison running: 0 none, 1..8 a register, 9..15 a flag bit,
// 16 one byte of guest memory.
extern int oracle_perturb;
constexpr int OraclePerturbNone = 0;
constexpr int OraclePerturbRegisterFirst = 1;   // .. 8
constexpr int OraclePerturbFlagFirst = 9;       // .. 15
constexpr int OraclePerturbMemory = 16;
constexpr int OraclePerturbLast = 16;
const char *oracle_perturb_name(int which);

// Re-runs the original single-stepped to find the instruction that last wrote
// `watch_address` (a guest address) or, when `watch_register` is >= 0, the one
// that last changed that register. Returns 0 when nothing was attributed.
uint32_t oracle_blame(uint32_t address, int case_index,
                      uint32_t watch_address, int watch_register);

// Reserves every free region of the address space, so that a guest pointer the
// lift cannot model faults instead of reading host memory. Returns MiB sealed.
// Call it after all of the harness's own allocation is done.
uint32_t oracle_seal_address_space();

const char *oracle_seed_description(int case_index);
const char *oracle_register_name(int index);

// Statistics the caller reports so the cost of the design is a measurement.
struct OracleCost {
    double reset_seconds;
    double original_seconds;
    double lifted_seconds;
    double compare_seconds;
    uint64_t cases;
};
extern OracleCost oracle_cost;

// True when the last lifted run ended in opensmacx_trap.
extern bool oracle_lifted_trapped;

// Prints a line per phase of every case, for when a run stops making progress.
extern bool oracle_trace;

#endif
