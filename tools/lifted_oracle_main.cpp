// Driver for the lifted-vs-original differential oracle.
//
//   lifted_oracle.exe --exe <terranx.exe> --list <plan.tsv> [options]
//
// The plan is produced by tools/lifted_oracle_plan.py: one line per function,
//     <address>\t<flags>\t<name>
// where flags is a comma-separated set out of
//     ok        nothing found that makes the function untestable
//     iat       reaches an import, directly or through a callee
//     fs        touches fs:, i.e. the SEH chain, which is not in the image
//     extcall   calls an address the lift has no body for
//     selfmod   reaches _SELFMOD
//     x87       uses the FPU (tested, but the FPU is not compared)
//     indirect  has an indirect call (tested, counted separately)
//
// Every function the driver refuses to run is reported as SKIPPED with the
// reason, never as passed. That count is as important as the pass count.
//
// ---------------------------------------------------------------------------
// THE PLAN'S FLAGS ARE A SCHEDULING HINT. THEY DO NOT CERTIFY ANYTHING.
// ---------------------------------------------------------------------------
//
// They are computed STATICALLY and propagated along every call edge to a
// fixpoint with no reachability filter, so `iat` means "some path COULD reach
// an import", not "a run DID". That is the right way to compute a hint and the
// wrong way to decide a verdict, and the difference is 88% of the image by
// byte: 2,031 functions carry extcall+fs+iat, and a large share of the `fs`
// among them is the `jmp ___CxxFrameHandler` stub that only the kernel ever
// enters.
//
// So by default this driver ATTEMPTS a statically flagged function and lets
// the RUN decide. Three outcomes are then possible and all three are named:
//
//   SKIP-reached-blocked  a seed really did reach the construct. The static
//                         flag described a path this program takes.
//   PASS-paths-taken      every judged seed agreed and none reached it. Real
//                         evidence, WEAKER than PASS, and never counted as it.
//   FAIL                  a divergence on a path that was taken, which is the
//                         same finding it has always been.
//
// `--refuse-blocked` restores the old behaviour: refuse on the flag alone. It
// is much faster and is the right thing when the run is a scheduling pass; it
// cannot produce any of the three verdicts above, so it can only ever
// under-count coverage, never over-count it.
//
// WHAT THE FLIPPED DEFAULT COSTS, MEASURED. The default was validated on the
// `extcall`-only cohort (470 functions, 3.09% of bytes) and on `extcall+fs`
// (0.42%). It now also attempts the 2,983 `iat`-bearing functions - 84% of the
// scope by byte - that had never been executed at all, and on THIS host some
// of them do not merely fail:
//
//     rosetta error: unsupported privilege level: 0
//
// Rosetta 2 refuses the instruction and the process is gone; no guard runs, no
// report row is written. Over the first 40 `iat`-flagged functions, 5 die that
// way - 0x00421b20, 0x00421b40, 0x00422ed0, 0x00447360 and 0x0045c180 - each
// reproducible on its own with `--only`. The other 35 came back 30
// PASS-paths-taken and 5 INCONCLUSIVE-original-fault, so the yield is real;
// the cost is that every death burns a full watchdog period and a process
// restart in `lifted_oracle_sweep.sh`. That script now writes those rows as
// KILLED-host-refused rather than HANG, because the watchdog never observed
// anything. Pass `--refuse-blocked` for a scheduling pass, or when the sweep's
// wall-clock matters more than the extra evidence.

#include "lifted_oracle.h"
#include "lifted_oracle_fold.h"

// OpensmacxHeapSize, for reporting the heap the guest was given.
#include "lifted_loader.h"

#include <windows.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

struct PlanEntry {
    uint32_t address;
    std::string flags;
    std::string name;
};

static bool has_flag(const std::string &flags, const char *flag) {
    size_t n = std::strlen(flag);
    for (size_t i = 0; i <= flags.size();) {
        size_t j = flags.find(',', i);
        if (j == std::string::npos) j = flags.size();
        if (j - i == n && flags.compare(i, n, flag) == 0) return true;
        i = j + 1;
        if (j == flags.size()) break;
    }
    return false;
}

// `undef=<hex>` in the plan's flag set: the OracleFlagBit values that are
// architecturally undefined on every path to this function's RET. Absent means
// zero, which is "compare every flag" - so a plan generated before this token
// existed behaves exactly as it did.
static uint32_t undefined_exit_flags(const std::string &flags) {
    static const char *const key = "undef=";
    const size_t n = std::strlen(key);
    for (size_t i = 0; i <= flags.size();) {
        size_t j = flags.find(',', i);
        if (j == std::string::npos) j = flags.size();
        if (j - i > n && flags.compare(i, n, key) == 0)
            return uint32_t(std::strtoul(flags.substr(i + n, j - i - n).c_str(),
                                         nullptr, 16));
        i = j + 1;
        if (j == flags.size()) break;
    }
    return 0;
}

// Does the STATIC plan say this function can reach something the lift cannot
// run? Exactly the set skip_reason() below refuses on, named separately
// because the two uses are different: one decides whether to attempt the
// function, the other decides whether a PASS may be called a PASS.
static bool statically_blocked(const std::string &flags) {
    return has_flag(flags, "selfmod") || has_flag(flags, "iat")
        || has_flag(flags, "fs") || has_flag(flags, "extcall");
}

static const char *skip_reason(const std::string &flags) {
    if (has_flag(flags, "selfmod")) return "self-modifying section";
    if (has_flag(flags, "iat")) return "reaches an import";
    if (has_flag(flags, "fs")) return "uses fs: (SEH chain is not in the image)";
    if (has_flag(flags, "extcall")) return "calls a body the lift does not have";
    // NOTE: lifted_oracle_plan.py emits no "nodispatch" flag, so this arm is
    // dead. It is kept because the condition is real - a function absent from
    // the dispatch table cannot be run - and the planner is the thing that
    // should learn to say it; deleting the arm would hide that it never does.
    return nullptr;
}

struct Tally {
    long passed = 0, failed = 0, lifted_fault = 0, skipped = 0;
    long inconclusive = 0, trap = 0, out_of_span = 0, top_page = 0;
    // Deliberately NOT added to `passed` anywhere. Every seed agreed, on a
    // function the static plan says has a path to something the lift cannot
    // run; no seed took that path. Folding it into the headline would be the
    // exact move the project's standard forbids - turning an honest SKIP into
    // a flattering PASS by weakening the conditions and reusing the name.
    long passed_paths_taken = 0;
    // Cases where a seed really did reach the blocked construct. This is the
    // number that says which static flags describe live paths.
    long reached_blocked = 0;
    // Counted apart because it is a different KIND of finding: every one seen
    // so far is a flag the manuals leave undefined after imul or idiv, where
    // the lift leaves the flag alone and the silicon does not.
    long failed_flags_only = 0;
    // Counted apart for the opposite reason: the x87 file was not compared at
    // all until now, so a failure that ONLY the x87 comparison sees is a
    // finding this oracle could not previously make, and folding it into the
    // total would hide how much of the delta the new comparison accounts for.
    long failed_x87_only = 0;
    // Functions where at least one case was thrown out by the top-page
    // arbitration but a LATER case passed, so the function is counted in
    // `passed`. The arbitration was described as one-sided ("it can turn a
    // false FAIL into an INCONCLUSIVE and cannot turn a false PASS into
    // anything"), and per CASE that is true - but the function-level upgrade
    // below turns the same case into a PASS row, and the whole-image report
    // then printed "INCONCLUSIVE top-page 0" while nineteen functions had been
    // arbitrated. So the count is kept, and the report row says so.
    long passed_with_arbitration = 0;
};

// Used only by --selftest, to find out whether this host really enforces
// no-execute rather than to report back the protection value we just set.
static volatile LONG g_nx_faulted = 0;

static LONG CALLBACK nx_probe_handler(EXCEPTION_POINTERS *ep) {
    if (ep->ExceptionRecord->ExceptionCode != DWORD(EXCEPTION_ACCESS_VIOLATION))
        return EXCEPTION_CONTINUE_SEARCH;
    g_nx_faulted = 1;
    // Perform the return the faulting CALL had already set up.
    CONTEXT *c = ep->ContextRecord;
    DWORD return_address = 0;
    std::memcpy(&return_address,
                reinterpret_cast<const void *>(uintptr_t(c->Esp)), 4);
    c->Eip = return_address;
    c->Esp += 4;
    return EXCEPTION_CONTINUE_EXECUTION;
}

int main(int argc, char **argv) {
    // The report is read through a pipe, and the interesting runs are the ones
    // that hang: block buffering would hide every line written before the
    // hang, which is exactly the evidence needed.
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
    int exit_code = 0;
    int role = oracle_bootstrap(argc, argv, &exit_code);
    if (role == 1) return exit_code;
    if (role != 0) return role;

    const char *exe_path = nullptr;
    const char *list_path = nullptr;
    const char *report_path = nullptr;
    uint32_t only = 0;
    long limit = -1;
    int cases = OracleCasesPerFunction;
    uint32_t budget = 0;
    uint32_t watchdog = 4000;
    bool refuse_blocked = false, verbose = false, selftest = false, blame = true;
    bool selfcheck = false;
    int dump_seed = -1;
    const char *state_path = nullptr;
    const char *build_state_path = nullptr;
    uint32_t selfcheck_address = 0x00401000U;
    bool append = false;
    uint32_t resume_after = 0;

    for (int i = 1; i < argc; ++i) {
        const char *a = argv[i];
        auto next = [&]() { return (i + 1 < argc) ? argv[++i] : ""; };
        if (!std::strcmp(a, "--exe")) exe_path = next();
        else if (!std::strcmp(a, "--state")) state_path = next();
        else if (!std::strcmp(a, "--build-state")) build_state_path = next();
        else if (!std::strcmp(a, "--list")) list_path = next();
        else if (!std::strcmp(a, "--report")) report_path = next();
        else if (!std::strcmp(a, "--only")) only = uint32_t(std::strtoul(next(), nullptr, 0));
        else if (!std::strcmp(a, "--limit")) limit = std::strtol(next(), nullptr, 0);
        else if (!std::strcmp(a, "--cases")) cases = int(std::strtol(next(), nullptr, 0));
        else if (!std::strcmp(a, "--budget")) budget = uint32_t(std::strtoul(next(), nullptr, 0));
        else if (!std::strcmp(a, "--watchdog")) watchdog = uint32_t(std::strtoul(next(), nullptr, 0));
        else if (!std::strcmp(a, "--refuse-blocked")) refuse_blocked = true;
        else if (!std::strcmp(a, "--verbose")) verbose = true;
        else if (!std::strcmp(a, "--no-blame")) blame = false;
        else if (!std::strcmp(a, "--selftest")) selftest = true;
        else if (!std::strcmp(a, "--selfcheck")) selfcheck = true;
        else if (!std::strcmp(a, "--dump-seed"))
            dump_seed = int(std::strtol(next(), nullptr, 0));
        else if (!std::strcmp(a, "--selfcheck-at"))
            selfcheck_address = uint32_t(std::strtoul(next(), nullptr, 0));
        else if (!std::strcmp(a, "--trace")) oracle_trace = true;
        // A hung case used to cost the whole run. The supervisor script kills
        // the child, notes where the report stopped, and starts again after
        // that address, appending; nothing already measured is thrown away.
        else if (!std::strcmp(a, "--resume-after"))
            resume_after = uint32_t(std::strtoul(next(), nullptr, 0));
        else if (!std::strcmp(a, "--append")) append = true;
        else if (!std::strcmp(a, "--oracle-child")) {}
        else { std::fprintf(stderr, "oracle: unknown option %s\n", a); return 2; }
    }
    if (!exe_path) { std::fprintf(stderr, "oracle: --exe is required\n"); return 2; }

    std::printf("stage: loading %s\n", exe_path);
    if (const char *error = oracle_load_image(exe_path)) {
        std::fprintf(stderr, "oracle: %s (%s)\n", error, exe_path);
        return 2;
    }
    // AFTER the image, BEFORE anything runs: the overlay rewrites the pristine
    // master that every case is restored from.
    if (state_path) {
        if (const char *error = oracle_overlay_state(state_path)) {
            std::fprintf(stderr, "oracle: --state: %s\n", error);
            return 2;
        }
    }
    std::printf("stage: image loaded\n");
    oracle_arm(budget, watchdog);
    std::printf("stage: armed (budget %u watchdog %u)\n", unsigned(budget), unsigned(watchdog));
    if (budget) {
        // Measured on this host (Wine on Apple silicon, x86 under Rosetta):
        // arming TF for the whole original run produces a spurious access
        // violation after a dozen steps in functions that PASS cleanly without
        // it - 0x00401660 faults at 0x00401698 after 13 steps. So a budget run
        // turns PASSes into INCONCLUSIVEs that describe the INSTRUMENTATION,
        // not the code.
        //
        // It is not removed, because it is the only guard that can bound a
        // runaway ORIGINAL side by instruction count and it may work on a real
        // x86 host. But a guard whose failures look like findings has to
        // announce itself, and previously it did not: the headline sweep ran
        // with budget 0, so the "instruction budget catches the loop" claim was
        // never exercised at all - those INCONCLUSIVEs were watchdog kills.
        std::fprintf(stderr,
            "oracle: WARNING - --budget arms the trap flag for the whole\n"
            "  original run, and on this host single-stepping faults\n"
            "  spuriously after a few instructions. Verdicts from this run\n"
            "  describe the instrumentation, not the lowering. The watchdog\n"
            "  (--watchdog, default on) is the guard that works here.\n");
    }

    if (build_state_path) {
        OracleBuildStateReport report;
        const char *error = oracle_build_state(build_state_path, &report);
        // The import figures print even on failure: a run that faults with
        // zero imports overridden failed for a reason that has nothing to do
        // with __cinit, and the two must not be confused.
        std::printf("build-state: imports %u slots, %u bound to real Win32, "
                    "%u overridden by this harness, %u unresolved\n",
                    unsigned(report.imports.slots), unsigned(report.imports.bound),
                    unsigned(report.imports.overridden),
                    unsigned(report.imports.unresolved));
        if (report.imports.unresolved)
            std::printf("build-state:   first unresolved: %s\n",
                        report.imports.first_unresolved);
        if (report.imports.missing_modules)
            std::printf("build-state:   %u DLL(s) would not load\n",
                        unsigned(report.imports.missing_modules));
        std::printf("build-state: guest heap %u call(s), %u bytes served, "
                    "%u refused, %u of %u bytes used\n",
                    unsigned(report.alloc_calls), unsigned(report.alloc_bytes),
                    unsigned(report.alloc_failures), unsigned(report.heap_used),
                    unsigned(OpensmacxHeapSize));
        if (report.alloc_largest_refused)
            std::printf("build-state: the largest REFUSED request was %u bytes "
                        "- that is how much guest heap this needs\n",
                        unsigned(report.alloc_largest_refused));
        std::printf("build-state: %u word(s) changed in the dump window, "
                    "%u of them point into the guest heap\n",
                    unsigned(report.words_changed),
                    unsigned(report.points_into_heap));
        if (report.address_shaped)
            std::printf("build-state: of the CHANGED words, %u of %u "
                        "address-shaped are in-span (%.1f%%)\n",
                        unsigned(report.address_in_span),
                        unsigned(report.address_shaped),
                        100.0 * double(report.address_in_span) /
                            double(report.address_shaped));
        if (report.dialogs)
            std::printf("build-state: %u MessageBoxA call(s) answered IDOK "
                        "without showing anything\n", unsigned(report.dialogs));
        if (report.refused)
            std::printf("build-state: the guest called %s; refused rather than "
                        "taking the harness down with it\n", report.refused);
        if (error) {
            std::fprintf(stderr, "oracle: --build-state: %s\n", error);
            return 2;
        }
        std::printf("build-state: startup steps %u attempted, %u returned\n",
                    unsigned(report.steps_attempted),
                    unsigned(report.steps_returned));
        if (!report.returned) {
            // NOT a state file. Saying where it stopped is the whole value of a
            // failed run - it is the next work item - but writing a dump of a
            // half-run __cinit would produce a file that looks like state.
            std::printf("BUILD-STATE-STOPPED-IN %s at %#010x  code %#010lx "
                        "accessing %#010x\n",
                        report.stopped_in ? report.stopped_in : "?",
                        unsigned(report.fault_address),
                        (unsigned long)report.fault_code,
                        unsigned(report.fault_data));
            std::printf("build-state: __cinit did not return, so NO state file "
                        "was written\n");
            return 1;
        }
        std::printf("BUILD-STATE-COMPLETE eax=%#010x, wrote %u bytes to %s\n",
                    unsigned(report.eax), unsigned(report.wrote),
                    build_state_path);
        return 0;
    }

    if (selftest) {
        // Does this host enforce the no-execute half of the page protection?
        // Reported rather than assumed, because the whole "a wild jump faults
        // instead of interpreting the seed" argument rests on it.
        unsigned char *arena =
            reinterpret_cast<unsigned char *>(static_cast<uintptr_t>(OracleArenaBase));
        arena[0] = 0xC3;  // ret
        DWORD old = 0;
        VirtualProtect(arena, 0x1000, PAGE_READWRITE, &old);
        MEMORY_BASIC_INFORMATION mbi;
        VirtualQuery(arena, &mbi, sizeof mbi);
        std::printf("selftest: arena protection %#lx (PAGE_READWRITE is %#x)\n",
                    (unsigned long)mbi.Protect, PAGE_READWRITE);
        // Fault and budget are exercised by running a case against a known
        // address below; here just confirm the span is ours.
        VirtualQuery(reinterpret_cast<void *>(static_cast<uintptr_t>(OracleImageBase)),
                     &mbi, sizeof mbi);
        std::printf("selftest: guest span at %p size %#zx state %#lx protect %#lx\n",
                    mbi.BaseAddress, size_t(mbi.RegionSize),
                    (unsigned long)mbi.State, (unsigned long)mbi.Protect);

        // Actually JUMP into the arena. Printing back the protection value we
        // just asked for proves only that VirtualQuery works; the claim being
        // made is that the host ENFORCES no-execute, and the only way to learn
        // that is to try. The VEH is already installed, and g_side is idle, so
        // an access violation here is caught by __try rather than by the case
        // machinery.
        // mingw has no __try/__except, and longjmp out of a vectored handler
        // is not something to rely on. But the jump below is a CALL, so at the
        // moment of the fault [ESP] still holds a perfectly good return
        // address into this function - the handler can simply perform the
        // `ret` the arena page was never going to reach.
        g_nx_faulted = 0;
        PVOID cookie = AddVectoredExceptionHandler(1, nx_probe_handler);
        reinterpret_cast<void (*)()>(arena)();
        RemoveVectoredExceptionHandler(cookie);
        const bool nx_enforced = g_nx_faulted != 0;
        std::printf("selftest: no-execute is %s on this host\n",
                    nx_enforced ? "ENFORCED (a wild jump faults)"
                                : "NOT ENFORCED - a wild jump would interpret "
                                  "the seed as code");
        if (!nx_enforced) {
            std::fprintf(stderr, "oracle: SELFTEST FAILED - the wall argument "
                                 "does not hold on this host\n");
            return 3;
        }

        // Does FSAVE write the register file in STACK order (ST(0) first) or in
        // PHYSICAL order (R0 first)? The whole x87 comparison is wrong by one
        // rotation if this is assumed and the assumption is wrong, and the
        // symptom would be "every x87 function disagrees", which reads like a
        // finding about the lift. Three distinct values are pushed, and the
        // slot at offset 28 must hold the LAST of them.
        {
            alignas(16) unsigned char image[108];
            std::memset(image, 0xAA, sizeof image);
            asm volatile("fninit\n\tfldz\n\tfld1\n\tfldpi\n\tfnsave %0"
                         : "=m"(image) :: "memory");
            long double first = 0;
            std::memcpy(&first, image + 28, 10);
            const bool stack_order = first > 3.14L && first < 3.15L;
            std::printf("selftest: fnsave stores %s (slot 0 = %.10Lg, "
                        "expected pi in stack order)\n",
                        stack_order ? "ST(0) FIRST - stack order, as assumed"
                                    : "SOMETHING ELSE",
                        first);
            if (!stack_order) {
                std::fprintf(stderr, "oracle: SELFTEST FAILED - the x87 "
                                     "comparison reads the fnsave image in the "
                                     "wrong order on this host\n");
                return 3;
            }
        }

        // Does the top-page fill reach the END of the page? See
        // oracle_top_page_fill_reaches_end: with std::memset in its place the
        // arbitration is blind to exactly the addresses it exists for, and no
        // test noticed.
        oracle_probe_top_page();
        if (oracle_top_page_writable()) {
            const bool reaches = oracle_top_page_fill_reaches_end();
            std::printf("selftest: top-page fill reaches 0xffffff88 and the "
                        "last word: %s\n", reaches ? "yes" : "NO");
            if (!reaches) {
                std::fprintf(stderr, "oracle: SELFTEST FAILED - the top-page "
                                     "arbitration cannot see the end of the "
                                     "page it fills\n");
                return 3;
            }
        } else {
            // Said out loud, because it is the whole reason for moving to a
            // native x86-64 host and because "the arbitration did not run" and
            // "the arbitration ran and found nothing" are indistinguishable in
            // a report that does not print this line.
            std::printf("selftest: guest 0xffff0000 is NOT accessible on this "
                        "host - it faults, so the three-fill top-page "
                        "arbitration is inert and every case that would have "
                        "needed it now faults into INCONCLUSIVE instead\n");
        }
    }

    // Sealed BEFORE the selfcheck, not after it. The selfcheck has to run under
    // the same address space the sweep runs under, or it is checking a
    // different harness from the one that produces the report.
    std::printf("sealed %u MiB of free address space\n",
                unsigned(oracle_seal_address_space()));
    // Printed on every run, because a FAIL whose detail changes when this
    // number changes is the harness reading its own memory, not a lowering
    // divergence. Rebuild with -DORACLE_LAYOUT_SHIM=0x51000 to move it.
    std::printf("lifted image at host %p\n", oracle_lifted_image_host_address());

    if (dump_seed >= 0) {
        oracle_dump_seed(only, dump_seed);
        return 0;
    }

    if (selfcheck) {
        // Prove every comparison is live, on a function that PASSES.
        std::printf("selfcheck: %#010x must PASS clean, then FAIL under each "
                    "single-field perturbation\n", unsigned(selfcheck_address));
        oracle_perturb = OraclePerturbNone;
        // Any seed that PASSES will do as the baseline, and which seeds pass
        // is a property of the function, not something to assume: 141 of the
        // passing functions pass on exactly one of the four.
        int baseline_case = -1;
        for (int c = 0; c < OracleCasesPerFunction; ++c) {
            if (oracle_run_case(selfcheck_address, c).verdict == OraclePass) {
                baseline_case = c;
                break;
            }
        }
        int bad = 0;
        if (baseline_case < 0) {
            std::printf("  UNUSABLE  no seed PASSES here - pick another "
                        "address with --selfcheck-at\n");
            return 3;
        }
        std::printf("  ok        baseline PASS on case %d\n", baseline_case);
        for (int which = OraclePerturbRegisterFirst;
             which <= OraclePerturbLast; ++which) {
            oracle_perturb = which;
            OracleResult r = oracle_run_case(selfcheck_address, baseline_case);
            const bool caught = r.verdict == OracleFail;
            if (!caught) ++bad;
            std::printf("  %-8s  perturbing %-26s -> %s\n",
                        caught ? "ok" : "MISSED", oracle_perturb_name(which),
                        oracle_verdict_name(r.verdict));
        }
        oracle_perturb = OraclePerturbNone;
        std::printf("selfcheck: %d of %d comparisons live\n",
                    OraclePerturbLast - bad, OraclePerturbLast);
        if (bad) {
            std::fprintf(stderr, "oracle: SELFCHECK FAILED - %d comparison(s) "
                                 "would not notice a divergence\n", bad);
            return 3;
        }
        return 0;
    }

    std::vector<PlanEntry> plan;
    if (only) {
        // --only keeps overriding every refusal, as it always has, so the
        // refusal is switched off rather than the flags being faked. The whole
        // flag string is then taken from the plan verbatim, because BOTH the
        // `undef=` token and the blocking flags change what the run reports: a
        // faked "ok" would compare a flag the plan run masks (the two would
        // then disagree for a reason that is in the harness) and would print
        // PASS where the plan run prints PASS-paths-taken.
        refuse_blocked = false;
        std::string flags = "ok";
        if (list_path) {
            if (FILE *f = std::fopen(list_path, "r")) {
                char line[1024];
                while (std::fgets(line, sizeof line, f)) {
                    if (line[0] == '#' || line[0] == '\n') continue;
                    char *tab1 = std::strchr(line, '\t');
                    if (!tab1) continue;
                    *tab1 = 0;
                    if (uint32_t(std::strtoul(line, nullptr, 0)) != only) continue;
                    char *tab2 = std::strchr(tab1 + 1, '\t');
                    if (tab2) *tab2 = 0;
                    flags = tab1 + 1;
                    break;
                }
                std::fclose(f);
            }
        }
        plan.push_back({only, flags, "--only"});
    } else if (list_path) {
        FILE *f = std::fopen(list_path, "r");
        if (!f) { std::fprintf(stderr, "oracle: cannot open %s\n", list_path); return 2; }
        char line[1024];
        while (std::fgets(line, sizeof line, f)) {
            if (line[0] == '#' || line[0] == '\n') continue;
            char *tab1 = std::strchr(line, '\t');
            if (!tab1) continue;
            *tab1 = 0;
            char *tab2 = std::strchr(tab1 + 1, '\t');
            if (tab2) *tab2 = 0;
            std::string name = tab2 ? std::string(tab2 + 1) : std::string();
            while (!name.empty() && (name.back() == '\n' || name.back() == '\r')) name.pop_back();
            plan.push_back({uint32_t(std::strtoul(line, nullptr, 0)), tab1 + 1, name});
        }
        std::fclose(f);
    } else {
        std::fprintf(stderr, "oracle: one of --list or --only is required\n");
        return 2;
    }
    if (limit >= 0 && plan.size() > size_t(limit)) plan.resize(size_t(limit));

    if (resume_after) {
        size_t keep = 0;
        while (keep < plan.size() && plan[keep].address <= resume_after) ++keep;
        plan.erase(plan.begin(), plan.begin() + keep);
        std::printf("resuming after %#010x: %zu functions left\n",
                    unsigned(resume_after), plan.size());
    }

    FILE *report = report_path ? std::fopen(report_path, append ? "a" : "w") : nullptr;
    if (report && !append)
        std::fprintf(report,
                     "address\tverdict\tcases\tcompared\tdetail\tname\n");

    Tally tally;
    long skip_counts[8] = {};
    static const char *skip_names[8] = {
        "self-modifying section", "reaches an import",
        "uses fs: (SEH chain is not in the image)",
        "calls a body the lift does not have", "not in the dispatch table",
        "lifted body traps (instruction never lowered)",
        "a seed REACHED a blocked construct", ""};
    long x87_tested = 0, indirect_tested = 0;
    long failures_printed = 0;
    double started = double(GetTickCount());

    // THE PLAN IS CHECKED FOR CORRUPTION AFTER EVERY FUNCTION.
    //
    // A sweep once emitted 3,928 rows whose address was zero, contiguously from
    // one point to the end, and reported "finished, 0 hang(s) and 0 host
    // death(s)". The parser was ruled out - the longest plan line is 123 bytes
    // against a 1024-byte buffer, and a parse bug would scatter the damage
    // rather than run it to the end of the file. What is left is that the GUEST
    // wrote over the plan: lifted_oracle_run.sh already warns that a seeded
    // pointer can land on the host's stack and overwrite the registers the
    // runner saved there, and a std::vector's heap buffer is no better
    // protected than a saved register.
    //
    // That hypothesis was never confirmed, so this confirms or kills it, and
    // does it for every future run rather than one experiment. If the harness's
    // own memory can be rewritten by the code under test, then EVERY figure
    // this tool has ever published is suspect, which is worth one comparison
    // per function to rule out.
    std::vector<uint32_t> plan_addresses;
    plan_addresses.reserve(plan.size());
    for (const PlanEntry &e : plan) plan_addresses.push_back(e.address);

    for (size_t index = 0; index < plan.size(); ++index) {
        if (plan[index].address != plan_addresses[index]) {
            std::fprintf(stderr,
                         "oracle: PLAN CORRUPTED at entry %zu: address was "
                         "%#010x, now %#010x. The harness's own memory has been "
                         "written by something else - every figure from this "
                         "run is void.\n",
                         index, unsigned(plan_addresses[index]),
                         unsigned(plan[index].address));
            std::fflush(nullptr);
            return 3;
        }
        const PlanEntry &entry = plan[index];
        oracle_undefined_exit_flags = undefined_exit_flags(entry.flags);
        const bool blocked = statically_blocked(entry.flags);
        const char *reason = refuse_blocked ? skip_reason(entry.flags) : nullptr;
        if (reason) {
            tally.skipped++;
            for (int k = 0; k < 5; ++k)
                if (!std::strcmp(reason, skip_names[k])) skip_counts[k]++;
            if (report) {
                std::fprintf(report, "%#010x\tSKIP\t0\t0\t%s\t%s\n",
                             unsigned(entry.address), reason, entry.name.c_str());
                std::fflush(report);
            }
            continue;
        }

        // The rules live in oracle_fold_cases / oracle_fold_step, in
        // lifted_oracle_fold.h, and are unit-tested there. They are NOT
        // restated here: this loop's only job is to run cases, hand each
        // verdict to the state machine, and keep the OracleResult of whichever
        // case the state machine says the report should describe. A second
        // copy of the rules in this file would make the unit tests describe a
        // program that is not the one the sweep runs.
        OracleFoldState fold;
        oracle_fold_reset(&fold);
        OracleResult worst;
        std::memset(&worst, 0, sizeof worst);
        for (int c = 0; c < cases; ++c) {
            if (verbose) std::printf("stage: case %#010x/%d\n", unsigned(entry.address), c);
            OracleResult r = oracle_run_case(entry.address, c);
            if (verbose) {
                // The verdict alone cannot distinguish "the original read a
                // global nothing constructed" from any other fault, and that
                // distinction is the whole question behind the one-seed-short
                // cohort. fault_data is the address the access NAMED, so a
                // small value is a near-null deref off a zeroed pointer.
                if (r.fault_code)
                    std::printf("stage:   -> %s  code=%#010lx eip=%#010x "
                                "data=%#010x\n",
                                oracle_verdict_name(r.verdict),
                                (unsigned long)r.fault_code,
                                unsigned(r.fault_address),
                                unsigned(r.fault_data));
                else
                    std::printf("stage:   -> %s\n",
                                oracle_verdict_name(r.verdict));
            }
            const int previous_winner = fold.winning_case;
            const bool keep_going = oracle_fold_step(&fold, r.verdict, c);
            if (fold.winning_case != previous_winner) worst = r;
            if (!keep_going) break;
        }
        // A PASS on a statically flagged function is downgraded HERE, by the
        // same unit-tested header the rules live in, so that nothing between
        // the fold and the report can call it a PASS.
        const OracleVerdict best = oracle_qualify_verdict(fold.verdict, blocked);
        // The two count columns and the PASS caveat come out of the same
        // unit-tested header as the rules. They used to be bare reads off
        // `fold` here, which is how `compared` could quietly become `ran` and
        // `arbitrated` could quietly become 0 with every test green.
        const OracleRowCounts counts = oracle_row_counts(fold);
        const int ran = counts.cases;
        const int compared = counts.compared;

        const bool agreed = best == OraclePass || best == OraclePassPathsTaken;
        if (has_flag(entry.flags, "x87") && agreed) x87_tested++;
        if (has_flag(entry.flags, "indirect") && agreed) indirect_tested++;

        char detail[512] = "";
        switch (best) {
            case OraclePass:
            case OraclePassPathsTaken:
                if (best == OraclePass) tally.passed++;
                else tally.passed_paths_taken++;
                // One call decides both the caveat text and the tally, so the
                // printed row and the printed total cannot disagree.
                if (oracle_pass_caveat(fold, detail, sizeof detail))
                    tally.passed_with_arbitration++;
                break;
            case OracleSkipTrap:
                tally.trap++; tally.skipped++; skip_counts[5]++;
                std::snprintf(detail, sizeof detail, "trap at %#010x: %s",
                              unsigned(worst.fault_address),
                              worst.trap_reason ? worst.trap_reason : "?");
                break;
            case OracleSkipReachedBlocked:
                tally.reached_blocked++; tally.skipped++; skip_counts[6]++;
                std::snprintf(detail, sizeof detail,
                              "case %d reached %#010x: %s", worst.case_index,
                              unsigned(worst.fault_address),
                              worst.trap_reason ? worst.trap_reason : "?");
                break;
            case OracleFail:
            case OracleFailLiftedFault: {
                if (best == OracleFail) {
                    tally.failed++;
                    if (!worst.register_mask && !worst.memory_diffs &&
                        !worst.x87_mask)
                        tally.failed_flags_only++;
                    if (!worst.register_mask && !worst.memory_diffs &&
                        !worst.flag_mask && worst.x87_mask)
                        tally.failed_x87_only++;
                } else {
                    tally.lifted_fault++;
                }
                size_t n = 0;
                if (best == OracleFailLiftedFault) {
                    n += std::snprintf(detail + n, sizeof detail - n,
                                       "lifted faulted %#lx at %#010x reaching %#010x; ",
                                       (unsigned long)worst.fault_code,
                                       unsigned(worst.fault_address),
                                       unsigned(worst.fault_data));
                }
                for (int i = 0; i < 8; ++i) {
                    if (!(worst.register_mask & (1U << i))) continue;
                    n += std::snprintf(detail + n, sizeof detail - n,
                                       "%s original=%#010x lifted=%#010x; ",
                                       oracle_register_name(i),
                                       unsigned(reinterpret_cast<const uint32_t *>(&worst.original)[i]),
                                       unsigned(reinterpret_cast<const uint32_t *>(&worst.lifted)[i]));
                }
                if (worst.flag_mask) {
                    n += std::snprintf(detail + n, sizeof detail - n, "flags%s%s%s%s%s%s%s differ; ",
                                       (worst.flag_mask & OracleFlagCF) ? " CF" : "",
                                       (worst.flag_mask & OracleFlagPF) ? " PF" : "",
                                       (worst.flag_mask & OracleFlagAF) ? " AF" : "",
                                       (worst.flag_mask & OracleFlagZF) ? " ZF" : "",
                                       (worst.flag_mask & OracleFlagSF) ? " SF" : "",
                                       (worst.flag_mask & OracleFlagOF) ? " OF" : "",
                                       (worst.flag_mask & OracleFlagDF) ? " DF" : "");
                }
                if (worst.x87_mask) {
                    n += std::snprintf(detail + n, sizeof detail - n,
                                       "x87");
                    if (worst.x87_mask & 1U)
                        n += std::snprintf(detail + n, sizeof detail - n,
                                           " depth original=%u lifted=%u",
                                           unsigned(worst.original_x87.depth),
                                           unsigned(worst.lifted_x87.depth));
                    if (worst.x87_mask & 2U)
                        n += std::snprintf(detail + n, sizeof detail - n,
                                           " control original=%#06x lifted=%#06x",
                                           unsigned(worst.original_x87.control),
                                           unsigned(worst.lifted_x87.control));
                    if (worst.x87_mask & 4U)
                        n += std::snprintf(detail + n, sizeof detail - n,
                                           " codes original=%#06x lifted=%#06x",
                                           unsigned(worst.original_x87.condition_codes),
                                           unsigned(worst.lifted_x87.condition_codes));
                    for (int i = 0; i < 8; ++i) {
                        if (!(worst.x87_mask & (1U << (8 + i)))) continue;
                        long double a = 0, b = 0;
                        std::memcpy(&a, worst.original_x87.st[i], 10);
                        std::memcpy(&b, worst.lifted_x87.st[i], 10);
                        n += std::snprintf(detail + n, sizeof detail - n,
                                           " st%d original=%.19Lg lifted=%.19Lg",
                                           i, a, b);
                    }
                    n += std::snprintf(detail + n, sizeof detail - n, "; ");
                }
                if (worst.memory_diffs) {
                    n += std::snprintf(detail + n, sizeof detail - n,
                                       "%u words of memory differ, first at %#010x "
                                       "(was %#010x, original=%#010x lifted=%#010x); ",
                                       unsigned(worst.memory_diffs), unsigned(worst.first_diff),
                                       unsigned(worst.first_diff_pristine),
                                       unsigned(worst.first_diff_original),
                                       unsigned(worst.first_diff_lifted));
                }
                if (blame && best == OracleFail) {
                    // Flags-only failures get index 8, the flags themselves.
                    int watch_register = worst.flag_mask ? 8 : -1;
                    for (int i = 0; i < 8; ++i)
                        if (worst.register_mask & (1U << i)) { watch_register = i; break; }
                    uint32_t eip = oracle_blame(
                        entry.address, worst.case_index,
                        worst.memory_diffs ? worst.first_diff : 0, watch_register);
                    if (eip)
                        n += std::snprintf(detail + n, sizeof detail - n,
                                           "blamed instruction at %#010x", unsigned(eip));
                }
                if (failures_printed++ < 40) {
                    std::printf("FAIL %#010x case %d (%s)  %s\n       %s\n",
                                unsigned(entry.address), worst.case_index,
                                oracle_seed_description(entry.address,
                                                        worst.case_index),
                                entry.name.c_str(), detail);
                    std::fflush(stdout);
                }
                break;
            }
            case OracleInconclusiveTopPage:
                tally.top_page++;
                std::snprintf(detail, sizeof detail,
                              "%s: case %d made the original read 0xffff0000.."
                              "0xffffffff, which the lift cannot address",
                              oracle_verdict_name(best), worst.case_index);
                break;
            default:
                if (best == OracleInconclusiveOutOfSpan) tally.out_of_span++;
                else tally.inconclusive++;
                // The DATA address is reported beside the faulting EIP.
                // Without it, an access violation says only where execution
                // was, and "the original jumped somewhere wild" and "the
                // original dereferenced a seeded pointer" are the same line -
                // which are opposite problems with opposite fixes. Seeding
                // work needs to know WHICH pointer to make valid, and that is
                // the address named here.
                //
                // Gated on fault_has_data, NOT on fault_data != 0. A null
                // dereference names address 0, and 0x004031a0 - which faults
                // reading 0x5c, a field off a null object - would otherwise
                // print as though the record carried no address at all.
                if (worst.fault_has_data)
                    std::snprintf(detail, sizeof detail,
                                  "%s code %#lx at %#010x accessing 0x%08x "
                                  "after %u steps",
                                  oracle_verdict_name(best),
                                  (unsigned long)worst.fault_code,
                                  unsigned(worst.fault_address),
                                  unsigned(worst.fault_data),
                                  unsigned(worst.original_steps));
                else
                    std::snprintf(detail, sizeof detail,
                                  "%s code %#lx at %#010x after %u steps",
                                  oracle_verdict_name(best),
                                  (unsigned long)worst.fault_code,
                                  unsigned(worst.fault_address),
                                  unsigned(worst.original_steps));
                break;
        }
        // A PLAN ENTRY WITH NO ADDRESS MUST NOT BECOME A REPORT ROW. The image
        // base is 0x00400000, so address zero is never a function; an entry
        // holding one means the plan was built wrong or grown with defaults.
        //
        // Writing it anyway is not a cosmetic problem. `%#010x` emits "0x" only
        // for a NONZERO value, so a zero address renders "0000000000", which
        // lifted_oracle_summary.py cannot parse and therefore drops. A sweep
        // that emitted 3,928 such rows still reported "finished, 0 hang(s) and
        // 0 host death(s)", and the headline figure - computed over the 1,745
        // real rows that remained - printed INCONCLUSIVE-original-fault at
        // 15.85% of scope. That is the number this project is trying to push
        // down, produced by a run that covered 31% of the plan.
        //
        // The unparseable format is the only reason it was caught. That is
        // luck, not a design, so refuse at the source and say so.
        if (entry.address == 0) {
            std::fprintf(stderr, "oracle: plan entry %u has no address; "
                         "refusing to write a report row for it\n",
                         unsigned(&entry - plan.data()));
            continue;
        }
        if (report) {
            std::fprintf(report, "%#010x\t%s\t%d\t%d\t%s\t%s\n", unsigned(entry.address),
                         oracle_verdict_name(best), ran, compared, detail,
                         entry.name.c_str());
            // Flushed per function: the supervisor decides a run is hung by
            // watching this file stop growing, so a buffered report would make
            // every run look hung for a thousand functions at a time.
            std::fflush(report);
        }
        if (verbose)
            std::printf("%#010x %-28s %s\n", unsigned(entry.address),
                        oracle_verdict_name(best), entry.name.c_str());
        if ((index % 200) == 199) {
            std::printf("... %zu/%zu  pass %ld  fail %ld  skip %ld  inconclusive %ld\n",
                        index + 1, plan.size(), tally.passed,
                        tally.failed + tally.lifted_fault, tally.skipped, tally.inconclusive);
            std::fflush(stdout);
        }
    }
    double elapsed = (double(GetTickCount()) - started) / 1000.0;
    if (report) std::fclose(report);

    std::printf("\n=== differential oracle: lifted vs original machine code ===\n");
    std::printf("functions in plan            %zu\n", plan.size());
    std::printf("PASSED                       %ld\n", tally.passed);
    std::printf("PASS-paths-taken             %ld   (every judged seed agreed, "
                "on a function the static plan says has a path to something\n"
                "                                  the lift cannot run - and "
                "no seed took it. NOT counted in PASSED above)\n",
                tally.passed_paths_taken);
    std::printf("FAILED (compare)             %ld\n", tally.failed);
    std::printf("    of those, flags only     %ld\n", tally.failed_flags_only);
    std::printf("    of those, x87 only       %ld\n", tally.failed_x87_only);
    std::printf("FAILED (lifted faulted)      %ld\n", tally.lifted_fault);
    std::printf("SKIPPED                      %ld\n", tally.skipped);
    for (int k = 0; k < 7; ++k)
        if (skip_counts[k])
            std::printf("    %-24s %ld\n", skip_names[k], skip_counts[k]);
    std::printf("INCONCLUSIVE                 %ld\n", tally.inconclusive);
    std::printf("INCONCLUSIVE out-of-span     %ld   (lifted read outside the "
                "modelled 6.3 MB; may hide address bugs)\n", tally.out_of_span);
    std::printf("INCONCLUSIVE top-page        %ld   (the original's answer "
                "moved when 0xffff0000..0xffffffff was refilled: it read the "
                "one range\n                                  neither the wall "
                "nor the seal covers, so the two sides were not running the "
                "same program)\n", tally.top_page);
    std::printf("PASSED with a case arbitrated %ld   (counted in PASSED above: "
                "a later seed was judged, an earlier one could not be)\n",
                tally.passed_with_arbitration);
    std::printf("top 64 KiB is %s, so the arbitration above %s\n",
                oracle_top_page_writable() ? "writable" : "NOT writable",
                oracle_top_page_writable() ? "is armed"
                                           : "CANNOT RUN - failures may be "
                                             "unmodellable memory, not lowering bugs");
    std::printf("  of the passes, %ld use the x87 and %ld contain an indirect call\n",
                x87_tested, indirect_tested);
    std::printf("cases compared               %llu\n",
                (unsigned long long)oracle_cost.cases);
    // Every tally above resets when lifted_oracle_sweep.sh restarts the
    // process after a hang, so on a resumed sweep these figures describe the
    // last segment, not the plan. The report is the only whole-image record.
    std::printf("NOTE: these totals cover THIS process only. A resumed sweep\n"
                "  restarts them; for whole-image figures run\n"
                "  tools/lifted_oracle_summary.py over the report.\n");
    std::printf("seconds  total %.1f  reset %.1f  original %.1f  lifted %.1f  compare %.1f\n",
                elapsed, oracle_cost.reset_seconds, oracle_cost.original_seconds,
                oracle_cost.lifted_seconds, oracle_cost.compare_seconds);
    if (oracle_cost.cases)
        std::printf("per case ms: reset %.3f  compare %.3f  (6.3 MB each way)\n",
                    1000.0 * oracle_cost.reset_seconds / double(oracle_cost.cases),
                    1000.0 * oracle_cost.compare_seconds / double(oracle_cost.cases));
    return (tally.failed || tally.lifted_fault) ? 1 : 0;
}
