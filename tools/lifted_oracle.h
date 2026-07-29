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
// it exactly. `--dump-seed <case>` prints the actual registers and the arena
// around `this`, which is what an argument about a failure needs; the
// generator's one-line description of a case is a summary, not evidence.
//
// Cases 0..3 are the four seeds the first whole-image sweep used, byte for
// byte. Cases 4..15 are DRAWN: pointer density, value class, the words below
// `this`, the stack fill, the register and argument mixes and all seven entry
// flags are each chosen from the same (address, case) hash. The four fixed
// seeds were the diagnosed weakness - they never produced a negative arena
// word, never set a flag other than DF, and never put a boundary value
// anywhere - so a mutation drawn at random from the PASS set was missed as
// often as it was caught.
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
// 1. CLOSED. The arena held no negative values, and `sar` -> `shr` in
//    lifted_00404ef0 survived because of it. Cases 4 and up now draw arena
//    words from four value classes, one of which is small-signed and one of
//    which is a boundary table containing -1, INT_MIN and both signs of every
//    width boundary.
//
// 2. GLOBALS ARE NEVER VARIED. Both sides start from the PE's .data as loaded,
//    so a lowering that only misbehaves on a value the game computes at
//    runtime is invisible. This one is still open.
//
// 3. CLOSED. The x87 register file is compared: stack depth, the control word,
//    the four condition codes, and the 80-bit encoding of every LIVE ST(i).
//    Empty slots are excluded because nothing defines them. `--selfcheck`
//    proves all four of those comparisons fail when damaged.
//
// 4. NEGATIVE ARGUMENTS AND POINTER ARGUMENTS ARE THE SAME SLOT. Still true,
//    and now deliberate: a draw chooses a pointer weight per case, so some
//    cases pass structures and others pass numbers into the same slots. What
//    changed is that the harness no longer MISREADS the result - see gap 5.
//
// 5. THE TOP 64 KiB IS UNMODELLABLE, and it is where every small negative
//    pointer lands. 0xFFFF0000..0xFFFFFFFF is committed read/write memory on
//    this host; VirtualQuery refuses the address, so the seal cannot cover it,
//    and PAGE_NOACCESS makes it fault and then kills the process. The original
//    reads it for real; the lifted side computes `opensmacx_image + 0xFFBFxxxx`
//    and wraps onto the harness. A failing case is therefore arbitrated: the
//    original is re-run twice with two different fills of that page, and if
//    its answer moves the verdict is INCONCLUSIVE-original-top-page, never
//    FAIL. The arbitration is one-sided - it cannot rescue a PASS that was
//    reached the same way, and that remains a caveat on every PASS.
//
// 6. OUT-OF-SPAN READS THAT DO NOT FAULT ARE STILL POSSIBLE ABOVE 128 MiB.
//    The wall covers the low 128 MiB and the seal covers every FREE region
//    above it, but Wine's own mappings are neither. A guest pointer landing in
//    one of those reads real memory on the original side and harness memory on
//    the lifted side. No case of this has been isolated, and the arbitration
//    in gap 5 does not cover it.

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

// The arena is split in two halves, and the split is what makes a two-step
// dereference survivable.
//
// A vbtable read is `mov eax,[this-N]; mov edx,[eax+K]`, and it needs the FIRST
// word to be a pointer and the SECOND to be a small displacement. A uniformly
// seeded arena cannot deliver both: whatever density makes step one a pointer
// makes step two one too, and `edx + this` is then nine megabytes away and
// faults. That single pattern is the largest identified family among the 522
// INCONCLUSIVE-original-fault rows - 0x00401060 dies on it at instruction four.
//
// NODE words are a mixture of pointers and values; LEAF words are never
// pointers. A pointer that aims at the leaf half therefore terminates the
// chain in a value, so `p->q` reaches something a displacement can be made of.
constexpr uint32_t OracleArenaNodeBase = OracleArenaBase;
constexpr uint32_t OracleArenaNodeSize = 0x00010000U;
constexpr uint32_t OracleArenaLeafBase = OracleArenaBase + 0x00010000U;
constexpr uint32_t OracleArenaLeafSize = 0x00010000U;

// Cases 0..3 are the four the first whole-image sweep used, byte for byte, so
// that every failure it found still reproduces and the delta is purely
// additive. Cases 4 and up are DRAWN: see `make_profile` in the .cpp.
constexpr int OracleLegacyCases = 4;
constexpr int OracleCasesPerFunction = 16;

// --------------------------------------------------------------------------
// Verdicts
// --------------------------------------------------------------------------

enum OracleVerdict {
    OracleUnrun = 0,
    OraclePass,               // both ran, registers + flags + memory agree
    // Every seed that could be judged agreed - but the STATIC plan says this
    // function can reach a construct the lift cannot run (an import, a callee
    // with no lifted body, the SEH chain), and no seed took that path. That is
    // weaker evidence than OraclePass and it gets its own name rather than
    // being folded into the coverage headline, because "agrees on the paths
    // sixteen seeds took" and "agrees on every path" are different claims and
    // only one of them has been tested.
    OraclePassPathsTaken,
    OracleFail,               // both ran, something disagrees
    OracleFailLiftedFault,    // original completed, lifted faulted
    OracleSkipTrap,           // lifted hit opensmacx_trap: not lowered yet
    // The lifted side ACTUALLY REACHED one of the constructs the static plan
    // flags: a call to an address with no lifted body, an import shim, an fs:
    // displacement outside the modelled TIB. Distinct from OracleSkipTrap,
    // which is an instruction the lowerer has not translated yet: this one
    // will not be fixed by lowering another mnemonic, and it is the evidence
    // that the static flag was describing a path the program really takes.
    OracleSkipReachedBlocked,
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
    // The ORIGINAL's answer changed when the unmodellable top 64 KiB was
    // filled with a different byte, so it read memory the lift cannot model
    // and the two sides were never looking at the same program. Established by
    // re-running the original twice with different fills, only on cases that
    // would otherwise be reported FAIL.
    OracleInconclusiveTopPage,
};

const char *oracle_verdict_name(OracleVerdict verdict);

// --------------------------------------------------------------------------
// One case
// --------------------------------------------------------------------------

struct OracleRegisters {
    uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi, eflags;
};

// The x87 state, reduced to the part that is defined on both sides.
//
// `depth` and the ST values are compared, the eight physical slots are NOT: a
// slot marked empty holds whatever the last value to occupy it left behind on
// real silicon, and nothing in the lift's model promises to reproduce that.
// Comparing all eight would report a divergence for every function that pops.
struct OracleX87 {
    uint32_t depth;            // live registers, ST(0)..ST(depth-1)
    uint16_t control;
    uint16_t condition_codes;  // C3 C2 C1 C0, masked out of the status word
    uint8_t st[8][10];         // ST(0) first, 80-bit encodings
};

struct OracleResult {
    OracleVerdict verdict;
    uint32_t address;
    int case_index;
    OracleRegisters before;
    OracleRegisters original;
    OracleRegisters lifted;
    OracleX87 original_x87;
    OracleX87 lifted_x87;
    uint32_t register_mask;    // bit i set: register i disagrees (index order)
    uint32_t flag_mask;        // OracleFlag* bits that disagree
    // bit 0 depth, bit 1 control word, bit 2 condition codes, bits 8+i ST(i).
    uint32_t x87_mask;
    uint32_t memory_diffs;     // number of differing 4-byte words
    uint32_t first_diff;       // guest address of the first differing word
    uint32_t first_diff_original;
    uint32_t first_diff_lifted;
    uint32_t first_diff_pristine;
    uint32_t original_steps;   // only counted when the budget is armed
    uint32_t fault_code;
    uint32_t fault_address;
    uint32_t fault_data;      // the address the faulting access named
    uint32_t fault_has_data;  // whether the record carried one at all: a null
                              // dereference names address 0, and without this
                              // that is indistinguishable from "no address"
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
// Overlay real .data/.bss from a hybrid dump. NEVER touches .text - see the
// definition for why that restriction is load-bearing.
const char *oracle_overlay_state(const char *path);

// Arms the vectored handler and the budget. `budget` of 0 disables the
// trap-flag instruction budget (much faster, no runaway protection beyond the
// watchdog); anything else is a hard cap on instructions executed by the
// original.
void oracle_arm(uint32_t budget, uint32_t watchdog_ms);

// Runs one case. Deterministic in (address, case_index).
OracleResult oracle_run_case(uint32_t address, int case_index);

// Prints the entry registers and a window of the seeded arena for one case, so
// that a failure can be reasoned about from the actual input rather than from
// the generator's description of it. Reporting an input class is not the same
// as reporting the input.
void oracle_dump_seed(uint32_t address, int case_index);

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
// 16 one byte of guest memory, 17 the x87 stack depth, 18 the value in ST(0),
// 19 the x87 control word, 20 the x87 condition codes.
//
// The four x87 perturbations are applied on a function that does not use the
// FPU on purpose. Both sides start from a defined, EMPTY x87 state, so damage
// to it is observable there too - and a comparison that only wakes up on the
// 58 x87-using functions could not be proved live on the address the selfcheck
// runs.
// OracleFlagBit set that the ARCHITECTURE leaves undefined on every path
// reaching the current function's RET, so it is excluded from the flag
// comparison. Set from the plan's `undef=<hex>` token before each function;
// zero means "compare every flag", which is what a run with no plan gets.
//
// This exists because the original side is not silicon: it is the host's x86
// layer, and it answers three different ways for one IDIV. See the block
// comment in lifted_oracle_plan.py for the measurements.
extern uint32_t oracle_undefined_exit_flags;

extern int oracle_perturb;
constexpr int OraclePerturbNone = 0;
constexpr int OraclePerturbRegisterFirst = 1;   // .. 8
constexpr int OraclePerturbFlagFirst = 9;       // .. 15
constexpr int OraclePerturbMemory = 16;
constexpr int OraclePerturbX87Depth = 17;
constexpr int OraclePerturbX87Value = 18;
constexpr int OraclePerturbX87Control = 19;
constexpr int OraclePerturbX87Codes = 20;
constexpr int OraclePerturbLast = 20;
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

// The top 64 KiB is the one range neither the wall nor the seal can cover:
// VirtualQuery refuses the address, and making it fault with VirtualProtect
// kills the process. It is where every small negative pointer lands. See the
// block comment in the .cpp; `oracle_arm` calls the probe.
void oracle_probe_top_page();
bool oracle_top_page_writable();

// Where the lifted image sits in the HOST's address space. Printed with every
// run because it is the only number that says whether a FAIL could be a
// host-layout artifact: see ORACLE_LAYOUT_SHIM in lifted_oracle.cpp.
const void *oracle_lifted_image_host_address();

// Does the top-page fill reach the END of the page? The first version of the
// arbitration used memset, which stops early there because the end pointer is
// 0x100000000 - and a detector that fills only the start silently reports "the
// original did not read the top page" for a case whose blamed instruction
// reads 0xFFFFFF88. Restoring memset leaves --selfcheck at 20/20 and drops
// top-page detections from 20 to 4, so the defect had no test at all. This is
// that test: --selftest fills the page and reads the last word back.
bool oracle_top_page_fill_reaches_end();

// The description depends on the ADDRESS as well as the case, because every
// case from OracleLegacyCases up draws its shape from the pair. Returns a
// pointer to a static buffer, valid until the next call.
const char *oracle_seed_description(uint32_t address, int case_index);
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
