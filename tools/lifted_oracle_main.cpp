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

#include "lifted_oracle.h"

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
    bool run_anyway = false, verbose = false, selftest = false, blame = true;
    bool selfcheck = false;
    int dump_seed = -1;
    uint32_t selfcheck_address = 0x00401000U;
    bool append = false;
    uint32_t resume_after = 0;

    for (int i = 1; i < argc; ++i) {
        const char *a = argv[i];
        auto next = [&]() { return (i + 1 < argc) ? argv[++i] : ""; };
        if (!std::strcmp(a, "--exe")) exe_path = next();
        else if (!std::strcmp(a, "--list")) list_path = next();
        else if (!std::strcmp(a, "--report")) report_path = next();
        else if (!std::strcmp(a, "--only")) only = uint32_t(std::strtoul(next(), nullptr, 0));
        else if (!std::strcmp(a, "--limit")) limit = std::strtol(next(), nullptr, 0);
        else if (!std::strcmp(a, "--cases")) cases = int(std::strtol(next(), nullptr, 0));
        else if (!std::strcmp(a, "--budget")) budget = uint32_t(std::strtoul(next(), nullptr, 0));
        else if (!std::strcmp(a, "--watchdog")) watchdog = uint32_t(std::strtoul(next(), nullptr, 0));
        else if (!std::strcmp(a, "--run-anyway")) run_anyway = true;
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
        // The synthetic entry says "ok" so that --only keeps overriding every
        // skip, as it always has. It must still carry this function's `undef=`
        // token, or the one-function reproduction of a whole-plan FAIL would
        // compare a flag the plan run does not, and the two would disagree for
        // a reason that is in the harness.
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
                    if (uint32_t mask = undefined_exit_flags(tab1 + 1)) {
                        char token[32];
                        std::snprintf(token, sizeof token, ",undef=%x", unsigned(mask));
                        flags += token;
                    }
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
        "lifted body traps (instruction never lowered)", "", ""};
    long x87_tested = 0, indirect_tested = 0;
    long failures_printed = 0;
    double started = double(GetTickCount());

    for (size_t index = 0; index < plan.size(); ++index) {
        const PlanEntry &entry = plan[index];
        oracle_undefined_exit_flags = undefined_exit_flags(entry.flags);
        const char *reason = run_anyway ? nullptr : skip_reason(entry.flags);
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

        OracleVerdict best = OracleUnrun;
        OracleResult worst;
        std::memset(&worst, 0, sizeof worst);
        int ran = 0;
        // Cases ATTEMPTED is not cases COMPARED. A function whose original
        // side faults on three of four seeds is reported PASS on the strength
        // of one, and the column said "4" either way - so 141 of the 1,366
        // passes rested on a single seed with nothing in the report to say so.
        int compared = 0;
        // A function whose ORIGINAL loops forever costs one watchdog period per
        // case, and the case count went from four to sixteen. At the 4 s
        // default that is 64 seconds for one function, which is both wasted -
        // the seventh timeout tells you nothing the second did not - and
        // actively harmful: the supervising sweep decides a run has hung by
        // watching the report stop growing, so a function that legitimately
        // takes a minute is killed and recorded as a HANG. Three of those
        // appeared in the first sixteen-case sweep. Two timeouts is the answer;
        // the verdict is the same either way.
        int timeouts = 0;
        // Cases the top-page arbitration threw out. Survives the function-level
        // upgrade to PASS so the report can say the seed never got a verdict.
        int arbitrated = 0;
        for (int c = 0; c < cases; ++c) {
            if (verbose) std::printf("stage: case %#010x/%d\n", unsigned(entry.address), c);
            OracleResult r = oracle_run_case(entry.address, c);
            if (verbose) std::printf("stage:   -> %s\n", oracle_verdict_name(r.verdict));
            ran++;
            if (r.verdict == OraclePass || r.verdict == OracleFail) compared++;
            if (r.verdict == OracleInconclusiveTopPage) arbitrated++;
            if (r.verdict == OracleFail || r.verdict == OracleFailLiftedFault) {
                best = r.verdict;
                worst = r;
                break;
            }
            if (r.verdict == OraclePass && best != OracleSkipTrap) {
                // A later PASS upgrades away the verdicts that describe the
                // ORIGINAL side being unrunnable on that seed. It must NOT
                // upgrade away the two that describe the LIFTED side
                // misbehaving while the original was healthy - those are
                // findings, and one good seed does not retract them.
                if (best == OracleUnrun || best == OracleInconclusiveFault ||
                    best == OracleInconclusiveBudget ||
                    best == OracleInconclusiveTopPage) {
                    best = OraclePass;
                }
            } else if (r.verdict == OracleSkipTrap) {
                best = OracleSkipTrap;
                worst = r;
                break;
            } else if (best == OracleUnrun) {
                best = r.verdict;
                worst = r;
            }
            if (r.verdict == OracleInconclusiveBudget ||
                r.verdict == OracleInconclusiveLiftedBudget) {
                if (++timeouts >= 2) break;
            }
        }

        if (has_flag(entry.flags, "x87") && best == OraclePass) x87_tested++;
        if (has_flag(entry.flags, "indirect") && best == OraclePass) indirect_tested++;

        char detail[512] = "";
        switch (best) {
            case OraclePass:
                tally.passed++;
                if (arbitrated) {
                    tally.passed_with_arbitration++;
                    std::snprintf(detail, sizeof detail,
                                  "passed on the seeds that could be judged; "
                                  "%d case(s) thrown out - the original read "
                                  "the unmodellable top 64 KiB",
                                  arbitrated);
                }
                break;
            case OracleSkipTrap:
                tally.trap++; tally.skipped++; skip_counts[5]++;
                std::snprintf(detail, sizeof detail, "trap at %#010x: %s",
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
                std::snprintf(detail, sizeof detail, "%s code %#lx at %#010x after %u steps",
                              oracle_verdict_name(best), (unsigned long)worst.fault_code,
                              unsigned(worst.fault_address), unsigned(worst.original_steps));
                break;
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
    std::printf("FAILED (compare)             %ld\n", tally.failed);
    std::printf("    of those, flags only     %ld\n", tally.failed_flags_only);
    std::printf("    of those, x87 only       %ld\n", tally.failed_x87_only);
    std::printf("FAILED (lifted faulted)      %ld\n", tally.lifted_fault);
    std::printf("SKIPPED                      %ld\n", tally.skipped);
    for (int k = 0; k < 6; ++k)
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
