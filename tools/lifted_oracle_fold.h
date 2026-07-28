#ifndef OPENSMACX_LIFTED_ORACLE_FOLD_H
#define OPENSMACX_LIFTED_ORACLE_FOLD_H

// The per-function verdict state machine: how the sixteen per-case verdicts of
// one function become the ONE row the report prints and the ONE number the
// headline counts.
//
// ---------------------------------------------------------------------------
// WHY THIS IS A SEPARATE, PURE FUNCTION
// ---------------------------------------------------------------------------
//
// These rules used to live inline in the driver's case loop, tangled with
// printf, a report file and `oracle_run_case`, so the only way to exercise them
// was to run the whole harness under Wine against the user's own executable.
// That is not a test, and the consequence was measured: the arbitration was
// documented as one-sided - "it can turn a false FAIL into an INCONCLUSIVE and
// cannot turn a false PASS into anything" - which is true per CASE and false
// per FUNCTION, because the upgrade below turns the arbitrated case's function
// into a PASS row. The whole-image report printed "INCONCLUSIVE top-page 0"
// while nineteen functions had had a case thrown out, every one of them counted
// in the coverage headline.
//
// Nothing here touches memory, Wine, the image or a global. It takes verdicts
// and returns a verdict, so a table of sequences can prove each rule, and each
// rule can be damaged to prove the table is live. The driver CALLS it - there
// is deliberately no second copy of the rules in main(), because a test over a
// copy proves nothing about what the sweep runs.
//
// ---------------------------------------------------------------------------
// THE RULES, AND WHY EACH ONE IS THE WAY IT IS
// ---------------------------------------------------------------------------
//
//  * OracleFail and OracleFailLiftedFault STOP the loop and win. A divergence
//    is a finding; running eleven more seeds cannot retract it, and the seed
//    that produced it is the one worth reporting.
//
//  * OracleSkipTrap and OracleSkipReachedBlocked stop and win. The lifted body
//    reached an instruction that was never lowered, or a construct the lift
//    does not model, so no seed can be judged - and this is a SKIP, not a pass
//    and not a failure. They stop even when earlier seeds passed, and that is
//    deliberate: the question a runtime blocked-construct check asks is "does
//    any seed take that path", and once one has, later seeds cannot unmake it.
//
//  * A later OraclePass upgrades the verdicts that say the ORIGINAL side could
//    not be run on that seed: OracleUnrun, OracleInconclusiveFault,
//    OracleInconclusiveBudget, OracleInconclusiveTopPage. One good seed is
//    enough to compare the two sides, and a wild pointer that killed seed 3
//    says nothing about the lowering.
//
//  * That upgrade must NOT reach OracleInconclusiveLiftedBudget or
//    OracleInconclusiveOutOfSpan. Those describe the LIFTED side misbehaving
//    while the original was healthy - a finding, which one good seed does not
//    retract.
//
//  * `arbitrated` survives the upgrade. It is the count of cases the top-page
//    arbitration threw out, and it is the whole reason the caveat can be
//    printed on a PASS row at all. This is the exact field the bug above
//    deleted.
//
//  * Two timeouts stop the loop. The seventh watchdog kill costs four seconds
//    and says nothing the second did not, and a function that legitimately
//    takes a minute gets killed by the supervising sweep and recorded as a
//    HANG. The counter is NOT reset by an intervening PASS: two timeouts in a
//    function is two timeouts.
//
// ORDER MATTERS AND THE ASYMMETRY IS REAL. `[PASS, out-of-span]` folds to PASS
// while `[out-of-span, PASS]` folds to out-of-span, because the merge only ever
// promotes out of OracleUnrun. That is the behaviour the sweep has, so it is
// pinned by a test rather than left to be discovered from a headline.

#include <cstdio>
#include <cstring>

#include "lifted_oracle.h"

// ---------------------------------------------------------------------------
// WHICH TRAPS MEAN "THE PROGRAM WENT SOMEWHERE THE LIFT CANNOT FOLLOW"
// ---------------------------------------------------------------------------
//
// Every trap arrives as one string, and two very different things produce one:
//
//   * a mnemonic the lowerer refused - "bt", "pushal", "loop". A phase-4 work
//     item: lower it and the function becomes testable.
//   * a construct the lift does not model at all - a callee it has no body
//     for, an import, an fs: displacement past the modelled TIB. Lowering
//     another mnemonic will never fix these, and reaching one is the RUNTIME
//     evidence that the static plan's flag described a path the program really
//     takes rather than a path that merely exists.
//
// The two must not share a verdict, because the second is the whole reason the
// harness attempts statically flagged functions at all. The strings are
// matched, not the addresses: a trap carries a guest address whose meaning
// differs between the two cases (a callee for one, an instruction for the
// other) and nothing in the address tells them apart.
//
// EACH PATTERN NAMES ITS SOURCE. If one of these strings is edited there, this
// list is wrong and the verdict silently degrades to SKIP-trap - so
// tools/test_lifted_oracle_fold.cpp pins every one of them against the real
// text, and damaging either side fails the test.
inline bool oracle_trap_is_blocked_construct(const char *reason) {
    if (!reason) return false;
    // tools/lift_whole_image.py, write_dispatch(): a direct call or jmp to an
    // address that is not a catalogued entry, and an indirect call that lands
    // on one, both arrive here.
    if (std::strcmp(reason, "call to an address with no lifted body") == 0)
        return true;
    // tools/lifted_tls.h, opensmacx_tib_at(): a real TEB field past the block
    // the lift models.
    if (std::strcmp(reason, "fs: displacement outside the modelled TIB") == 0)
        return true;
    // tools/generate_imports.py, opensmacx_import_dispatch(): a synthetic
    // import address the guest computed rather than loaded.
    if (std::strcmp(reason, "synthetic import address with no import") == 0)
        return true;
    // tools/generate_imports.py's shims for the imports with no body, and the
    // marshalling refusals in tools/lifted_imports.h. Both begin with the same
    // two words; every other trap reason in the tree is a bare mnemonic or a
    // sentence that does not. The trailing SPACE in each prefix is what stops
    // "imports" or "importance" from matching.
    if (std::strncmp(reason, "unimplemented import ", 21) == 0) return true;
    if (std::strncmp(reason, "import ", 7) == 0) return true;
    return false;
}

// A PASS on a function the static plan says CAN reach one of those constructs
// is not the same evidence as a PASS on a function that cannot: sixteen seeds
// agreed on the paths they took, and a path they did not take was never
// compared. It gets its own verdict rather than a footnote, so that no total
// anywhere can quietly absorb it.
//
// Only PASS is qualified. A FAIL is a divergence on a path that WAS taken and
// the static flag adds nothing to it; a SKIP or an INCONCLUSIVE was not a
// claim about agreement in the first place.
inline OracleVerdict oracle_qualify_verdict(OracleVerdict verdict,
                                            bool statically_blocked) {
    if (verdict == OraclePass && statically_blocked) return OraclePassPathsTaken;
    return verdict;
}

// The running state of the fold. The driver keeps one of these per function and
// feeds it one verdict at a time, so that OracleFail can still stop the loop
// before the remaining seeds are run - the early exit is a cost saving worth
// several hours across the image, and folding a pre-built array would lose it.
struct OracleFoldState {
    OracleVerdict verdict;  // the function-level verdict so far
    int arbitrated;         // cases the top-page arbitration threw out
    int compared;           // cases where BOTH sides ran, i.e. PASS or FAIL
    int ran;                // cases actually examined
    int timeouts;           // budget/watchdog kills seen, on either side
    // The case whose OracleResult the report's detail column describes, or -1
    // when no case has claimed it. The driver mirrors its `worst` result off
    // this changing, which keeps "which case do we report" a rule of the state
    // machine rather than a second, untested copy in the driver.
    int winning_case;
    int stopped_at;         // index of the last case examined, -1 when none
    bool done;              // the loop has been told to stop
};

// The two verdicts that say "the lifted side could not be run to the end on
// this seed, and no further seed changes that". Named once so the stepper's
// two arms cannot drift apart.
inline bool oracle_verdict_is_skip(OracleVerdict verdict) {
    return verdict == OracleSkipTrap || verdict == OracleSkipReachedBlocked;
}

inline void oracle_fold_reset(OracleFoldState *state) {
    state->verdict = OracleUnrun;
    state->arbitrated = 0;
    state->compared = 0;
    state->ran = 0;
    state->timeouts = 0;
    state->winning_case = -1;
    state->stopped_at = -1;
    state->done = false;
}

// Folds one case's verdict in. Returns true when the caller should run the next
// case, false when the state machine has decided and further seeds cannot
// change the answer.
inline bool oracle_fold_step(OracleFoldState *state, OracleVerdict verdict,
                             int case_index) {
    state->ran++;
    state->stopped_at = case_index;
    // Cases ATTEMPTED is not cases COMPARED. A function whose original side
    // faults on three of four seeds is reported PASS on the strength of one.
    if (verdict == OraclePass || verdict == OracleFail) state->compared++;
    if (verdict == OracleInconclusiveTopPage) state->arbitrated++;
    if (verdict == OracleFail || verdict == OracleFailLiftedFault) {
        state->verdict = verdict;
        state->winning_case = case_index;
        state->done = true;
        return false;
    }
    if (verdict == OraclePass && !oracle_verdict_is_skip(state->verdict)) {
        // A later PASS upgrades away the verdicts that describe the ORIGINAL
        // side being unrunnable on that seed. It must NOT upgrade away the two
        // that describe the LIFTED side misbehaving while the original was
        // healthy - those are findings, and one good seed does not retract
        // them.
        if (state->verdict == OracleUnrun ||
            state->verdict == OracleInconclusiveFault ||
            state->verdict == OracleInconclusiveBudget ||
            state->verdict == OracleInconclusiveTopPage) {
            state->verdict = OraclePass;
        }
    } else if (oracle_verdict_is_skip(verdict)) {
        state->verdict = verdict;
        state->winning_case = case_index;
        state->done = true;
        return false;
    } else if (state->verdict == OracleUnrun) {
        state->verdict = verdict;
        state->winning_case = case_index;
    }
    if (verdict == OracleInconclusiveBudget ||
        verdict == OracleInconclusiveLiftedBudget) {
        if (++state->timeouts >= 2) {
            state->done = true;
            return false;
        }
    }
    return true;
}

// What one function's row says.
struct OracleFunctionVerdict {
    OracleVerdict verdict;
    int arbitrated;
    int compared;
    int ran;
    int stopped_at;
    int winning_case;
};

// ---------------------------------------------------------------------------
// THE LAST MILE: fold state -> what the row and the tally actually say
// ---------------------------------------------------------------------------
//
// The two functions below existed as four bare assignments in the driver:
//
//     const int compared   = fold.compared;
//     const int arbitrated = fold.arbitrated;
//
// and nothing compiled the driver, so nothing tested them. Both of this
// project's headline bugs are one character of those two lines:
//
//   * `arbitrated` forced to 0 deletes the caveat from every PASS row and
//     takes "PASSED with a case arbitrated" to 0 - which is EXACTLY the
//     reported bug: nineteen functions had a case thrown out and the report
//     said none had.
//   * `compared` reading `fold.ran` instead makes every row claim all sixteen
//     seeds compared, and lifted_oracle_summary.py then prints "0 PASSes rest
//     on a single seed" where the truth was 141 of 1,366.
//
// Both were reinstated with the whole suite green. They live here now so that
// the same table-driven tests that cover the rules cover the numbers the rules
// produce.

// The two count columns of a report row: `cases` (attempted) and `compared`.
// They are DIFFERENT numbers and the whole seed histogram rests on that: a
// function whose original side faulted on fifteen of sixteen seeds is reported
// PASS on the strength of one, and only the second column says so.
struct OracleRowCounts {
    int cases;      // cases attempted, i.e. examined before the fold stopped
    int compared;   // cases where BOTH sides ran
};

inline OracleRowCounts oracle_row_counts(const OracleFoldState &state) {
    OracleRowCounts counts;
    counts.cases = state.ran;
    counts.compared = state.compared;
    return counts;
}

// The caveat a PASS row carries when the top-page arbitration threw cases out.
// Returns true when the function passed AND at least one case was arbitrated -
// which is both the condition for the text and the condition for the
// `passed_with_arbitration` tally, so the two cannot drift apart.
//
// `out` is left as an empty string when there is nothing to say, because an
// unconditional caveat would put "0 case(s) thrown out" on all 1,465 PASS rows.
inline bool oracle_pass_caveat(const OracleFoldState &state, char *out,
                               unsigned long size) {
    if (size) out[0] = '\0';
    if (state.verdict != OraclePass || state.arbitrated <= 0) return false;
    std::snprintf(out, size,
                  "passed on the seeds that could be judged; "
                  "%d case(s) thrown out - the original read "
                  "the unmodellable top 64 KiB",
                  state.arbitrated);
    return true;
}

// The whole fold over a sequence that is already known. This is what a test
// calls; the driver drives the stepper directly so that it can stop running
// seeds. Both go through the same rules.
inline OracleFunctionVerdict oracle_fold_cases(const OracleVerdict *verdicts,
                                               int count) {
    OracleFoldState state;
    oracle_fold_reset(&state);
    for (int c = 0; c < count; ++c)
        if (!oracle_fold_step(&state, verdicts[c], c)) break;
    OracleFunctionVerdict result;
    result.verdict = state.verdict;
    result.arbitrated = state.arbitrated;
    result.compared = state.compared;
    result.ran = state.ran;
    result.stopped_at = state.stopped_at;
    result.winning_case = state.winning_case;
    return result;
}

#endif
