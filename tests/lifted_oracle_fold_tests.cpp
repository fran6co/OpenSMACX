// Unit tests for the differential oracle's per-function verdict state machine.
//
// WHY THESE PARTICULAR PROPERTIES
//
// This fold decides every number the oracle reports. Sixteen per-case verdicts
// go in and one row comes out, and that row is what the coverage headline
// counts. Until now it was tangled with printf and a report file inside the
// driver's case loop, so the only way to exercise it was to run the whole
// harness under Wine against the user's own terranx.exe - which means it had no
// test at all, and the consequences were measured rather than imagined:
//
//   * The top-page arbitration was documented as one-sided: "it can turn a
//     false FAIL into an INCONCLUSIVE and cannot turn a false PASS into
//     anything." True per CASE, false per FUNCTION - a later passing seed
//     upgrades the whole function to PASS. The report printed "INCONCLUSIVE
//     top-page 0" while nineteen functions had had a case thrown out, all of
//     them counted in the headline. The property that was missing is not "a
//     PASS may upgrade" - it is "the arbitrated COUNT survives the upgrade",
//     because that count is the only thing left that can put the caveat on the
//     PASS row.
//
//   * The distinction between the two kinds of INCONCLUSIVE is the whole
//     integrity of the harness. "The ORIGINAL could not run on this seed" is
//     not a finding and a later PASS retires it. "The LIFTED side misbehaved
//     while the original was healthy" IS a finding and one good seed does not
//     retract it. Both are spelled INCONCLUSIVE-something, they sit two enum
//     values apart, and the difference between them exists only in the membership
//     of one `if`. Nothing asserted that membership.
//
// So the tests below are not about "does the fold return a verdict". They are
// about the four ways this fold can lie in the direction that flatters the
// project: manufacture a PASS, erase a finding, lose the count that carries a
// caveat, or keep running after a divergence and let a later seed overwrite it.
//
// Two shapes of test, deliberately:
//
//   1. A table of hand-derived sequences. Each row's expectations were worked
//      out from the rules, not captured from a run - a table filled in from the
//      program's own output asserts only that the program is deterministic.
//
//   2. Exhaustive invariants over every sequence of length 1..3 drawn from all
//      ten verdicts, 1,110 sequences. A table row can be made to pass by an
//      accident that happens to line up; an invariant over the whole space of
//      short sequences cannot. This is where "the fold cannot manufacture a
//      PASS" and "arbitrated equals the number of top-page cases examined" are
//      proved, and it is what fails first when a rule is damaged.
//
// Nothing here loads an image, opens a window, or needs Wine to mean anything -
// the function under test is pure by construction, which is the point of having
// extracted it.

#include "../tools/lifted_oracle_fold.h"

#include <cstdio>
#include <cstring>

namespace {

int failures = 0;
const char *current_case = "";

void expect_at(bool condition, const char *what, int line) {
    if (!condition) {
        std::fprintf(stderr, "fold expect failed at line %d: %s [%s]\n", line,
                     what, current_case);
        ++failures;
    }
}

#define expect(condition) expect_at((condition), #condition, __LINE__)

void expect_int_at(long actual, long wanted, const char *what, int line) {
    if (actual != wanted) {
        std::fprintf(stderr,
                     "fold expect failed at line %d: %s was %ld, wanted %ld "
                     "[%s]\n",
                     line, what, actual, wanted, current_case);
        ++failures;
    }
}

#define expect_int(actual, wanted) \
    expect_int_at((actual), (wanted), #actual, __LINE__)

// Short aliases so a sequence reads as a sequence. The comment beside each is
// the half of the enum's meaning that decides how the fold treats it.
const OracleVerdict U = OracleUnrun;
const OracleVerdict P = OraclePass;
const OracleVerdict F = OracleFail;                       // divergence, stops
const OracleVerdict LF = OracleFailLiftedFault;           // lifted invented an access
const OracleVerdict ST = OracleSkipTrap;                  // never lowered
const OracleVerdict SRB = OracleSkipReachedBlocked;       // reached an import etc.
const OracleVerdict IF = OracleInconclusiveFault;         // ORIGINAL faulted
const OracleVerdict IB = OracleInconclusiveBudget;        // ORIGINAL ran away
const OracleVerdict ILB = OracleInconclusiveLiftedBudget; // LIFTED ran away
const OracleVerdict IOS = OracleInconclusiveOutOfSpan;    // LIFTED left the span
const OracleVerdict ITP = OracleInconclusiveTopPage;      // ORIGINAL read 0xffffxxxx
// NOT a per-case verdict and deliberately absent from all_verdicts below: no
// run produces it. It is what oracle_qualify_verdict turns a whole function's
// PASS into when the static plan says that function has a path to something
// the lift cannot run. Tested in test_qualify_verdict, not through the fold.
const OracleVerdict PPT = OraclePassPathsTaken;

const OracleVerdict all_verdicts[] = {U, P, F, LF, ST, SRB, IF, IB, ILB, IOS, ITP};
const int all_verdicts_count = int(sizeof all_verdicts / sizeof all_verdicts[0]);

// For failure messages only. Deliberately NOT oracle_verdict_name, which lives
// in lifted_oracle.cpp behind windows.h and the whole loaded image; a label
// table used only to print a diagnostic is not a rule and asserts nothing.
const char *label(OracleVerdict v) {
    switch (v) {
        case OracleUnrun: return "UNRUN";
        case OraclePass: return "PASS";
        case OraclePassPathsTaken: return "PASS-paths-taken";
        case OracleSkipReachedBlocked: return "SKIP-reached-blocked";
        case OracleFail: return "FAIL";
        case OracleFailLiftedFault: return "FAIL-lifted-fault";
        case OracleSkipTrap: return "SKIP-trap";
        case OracleInconclusiveFault: return "INC-original-fault";
        case OracleInconclusiveBudget: return "INC-original-timeout";
        case OracleInconclusiveLiftedBudget: return "INC-lifted-budget";
        case OracleInconclusiveOutOfSpan: return "INC-lifted-out-of-span";
        case OracleInconclusiveTopPage: return "INC-original-top-page";
    }
    return "?";
}

// --------------------------------------------------------------------------
// 1. The table
// --------------------------------------------------------------------------

struct FoldRow {
    const char *why;                // the property this row exists to prove
    OracleVerdict verdicts[16];
    int count;
    OracleVerdict expected;
    int arbitrated;
    int compared;
    int ran;
    int stopped_at;
    int winning_case;               // which case the report's detail describes
};

const FoldRow rows[] = {
    // ---- degenerate ------------------------------------------------------
    {"zero cases is UNRUN and nothing else: a function the driver was told to "
     "run with --cases 0 must not become a PASS",
     {}, 0, U, 0, 0, 0, -1, -1},
    {"a single case verdict of UNRUN stays UNRUN - the fold has no default "
     "that resolves to something reportable",
     {U}, 1, U, 0, 0, 1, 0, 0},
    {"one clean PASS is a PASS, compared once, and claims no detail case",
     {P}, 1, P, 0, 1, 1, 0, -1},
    {"sixteen clean PASSes compare sixteen times - `compared` is the column "
     "that says how many seeds actually got a verdict",
     {P, P, P, P, P, P, P, P, P, P, P, P, P, P, P, P}, 16, P, 0, 16, 16, 15, -1},

    // ---- a FAIL stops the loop immediately and wins -----------------------
    {"a FAIL on the first seed stops the loop: the eleven seeds that would "
     "have passed afterwards cannot retract it",
     {F, P, P}, 3, F, 0, 1, 1, 0, 0},
    {"a FAIL after a PASS still wins, and the detail describes the FAILING "
     "case, not the passing one",
     {P, F, P}, 3, F, 0, 2, 2, 1, 1},
    {"a FAIL beats a later SKIP-trap by stopping before it is ever seen",
     {F, ST}, 2, F, 0, 1, 1, 0, 0},
    {"lifted-faulted is the other stopping failure: the original completed "
     "and the lift invented an access, and no later seed retracts that",
     {LF, P, P, P}, 4, LF, 0, 0, 1, 0, 0},

    // ---- SKIP-trap wins and stops ----------------------------------------
    {"a trap stops the loop: the body was never lowered, so no seed can be "
     "judged and the function is SKIPPED, not passed",
     {ST, F}, 2, ST, 0, 0, 1, 0, 0},
    {"a trap on a later seed overrides an earlier PASS - a body that traps at "
     "all is not lowered, whatever one seed managed to reach",
     {P, ST, P}, 3, ST, 0, 1, 2, 1, 1},
    {"the all-trap sequence stops on the first one and reports SKIP-trap once",
     {ST, ST, ST, ST, ST, ST, ST, ST, ST, ST, ST, ST, ST, ST, ST, ST}, 16,
     ST, 0, 0, 1, 0, 0},

    // ---- SKIP-reached-blocked behaves exactly like SKIP-trap -------------
    // Same stopping rule, different meaning, and the two must not be merged:
    // one is a mnemonic waiting to be lowered, the other is the RUNTIME proof
    // that a static blocking flag described a path this program takes. The
    // measured cohort had 79 of these against 0 SKIP-trap, so a fold that
    // collapsed them would have reported 79 phase-4 work items that do not
    // exist.
    {"reaching a blocked construct stops the loop and reports itself, not "
     "SKIP-trap",
     {SRB, F}, 2, SRB, 0, 0, 1, 0, 0},
    {"a seed that reaches a blocked construct overrides earlier PASSes: the "
     "question is whether ANY seed takes that path, and one has",
     {P, P, SRB, P}, 4, SRB, 0, 2, 3, 2, 2},
    {"...and this is the exact shape measured at 0x004015b0, where six seeds "
     "were compared before case 9 reached 0x00644f3a",
     {P, P, P, P, P, P, SRB}, 7, SRB, 0, 6, 7, 6, 6},
    {"a FAIL still beats it by stopping first - a compared divergence is the "
     "stronger statement",
     {F, SRB}, 2, F, 0, 1, 1, 0, 0},
    {"the two skips do not merge: whichever is seen first names the row",
     {SRB, ST}, 2, SRB, 0, 0, 1, 0, 0},
    {"and in the other order",
     {ST, SRB}, 2, ST, 0, 0, 1, 0, 0},

    // ---- a later PASS upgrades the ORIGINAL-side inconclusives ------------
    {"a later PASS upgrades UNRUN",
     {U, P}, 2, P, 0, 1, 2, 1, 0},
    {"a later PASS upgrades INCONCLUSIVE-original-fault: a wild seeded pointer "
     "that killed seed 0 says nothing about the lowering",
     {IF, P}, 2, P, 0, 1, 2, 1, 0},
    {"a later PASS upgrades INCONCLUSIVE-original-timeout",
     {IB, P}, 2, P, 0, 1, 2, 1, 0},
    {"a later PASS upgrades INCONCLUSIVE-original-top-page AND THE ARBITRATED "
     "COUNT SURVIVES - this is the exact bug: the function is counted in "
     "PASSED, so the count is the only thing that can still say a seed was "
     "thrown out",
     {ITP, P}, 2, P, 1, 1, 2, 1, 0},
    {"two thrown-out seeds are counted as two, not collapsed to a flag",
     {ITP, ITP, P}, 3, P, 2, 1, 3, 2, 0},
    {"a seed thrown out AFTER the function already passed is still counted - "
     "the caveat does not depend on which seed came first",
     {P, ITP}, 2, P, 1, 1, 2, 1, -1},
    {"the arbitrated count survives a FAIL too, so a failing function's row "
     "can still say a seed never got a verdict",
     {ITP, F}, 2, F, 1, 1, 2, 1, 1},
    {"an arbitrated seed between a timeout and a PASS is counted, and the "
     "PASS still upgrades the timeout",
     {IB, ITP, P}, 3, P, 1, 1, 3, 2, 0},
    {"all sixteen seeds thrown out: the function is INCONCLUSIVE-top-page and "
     "the count is sixteen, not one",
     {ITP, ITP, ITP, ITP, ITP, ITP, ITP, ITP, ITP, ITP, ITP, ITP, ITP, ITP,
      ITP, ITP}, 16, ITP, 16, 0, 16, 15, 0},
    {"an original that faults on every seed is INCONCLUSIVE, never PASS, and "
     "runs every seed looking for one that works",
     {IF, IF, IF, IF, IF, IF, IF, IF, IF, IF, IF, IF, IF, IF, IF, IF}, 16,
     IF, 0, 0, 16, 15, 0},
    {"the upgrade survives further original-side faults after it",
     {IF, IF, P, IF}, 4, P, 0, 1, 4, 3, 0},

    // ---- a later PASS must NOT upgrade a LIFTED-side finding --------------
    {"a later PASS must NOT erase INCONCLUSIVE-lifted-budget: the original was "
     "healthy and the LIFT ran away, which is a finding",
     {ILB, P}, 2, ILB, 0, 1, 2, 1, 0},
    {"a later PASS must NOT erase INCONCLUSIVE-lifted-out-of-span: the lift "
     "computed an address its own model does not cover",
     {IOS, P}, 2, IOS, 0, 1, 2, 1, 0},
    {"three later PASSes do not erase it either - it is not a vote",
     {IOS, P, P, P}, 4, IOS, 0, 3, 4, 3, 0},
    {"a FAIL still overrides a lifted-side inconclusive, because a compared "
     "divergence is the stronger statement",
     {ILB, F}, 2, F, 0, 1, 2, 1, 1},

    // ---- two timeouts stop the loop --------------------------------------
    {"two original-side timeouts stop the loop: the seventh watchdog kill "
     "costs four seconds and tells you nothing the second did not",
     {IB, IB, P}, 3, IB, 0, 0, 2, 1, 0},
    {"the two timeouts need not be the same kind - one on each side still "
     "stops, and the first one seen names the verdict",
     {IB, ILB, P}, 3, IB, 0, 0, 2, 1, 0},
    {"and in the other order the lifted-side one names it",
     {ILB, IB, P}, 3, ILB, 0, 0, 2, 1, 0},
    {"an intervening PASS does not reset the timeout counter: two timeouts in "
     "one function is two timeouts, and the loop stops even though the "
     "function has already been upgraded to PASS",
     {IB, P, IB, P}, 4, P, 0, 1, 3, 2, 0},

    // ---- pinned asymmetries, NOT endorsements ----------------------------
    // These two rows record what the sweep does today. Both are the same
    // shape of hole as the top-page bug - a finding that arrives second is
    // dropped - and both are pinned here so that a change to them is a
    // deliberate act with a failing test attached, rather than a number that
    // moves in a headline.
    {"PINNED ASYMMETRY: out-of-span AFTER a PASS is dropped, while the same "
     "pair in the other order reports out-of-span. The merge only ever "
     "promotes out of UNRUN",
     {P, IOS}, 2, P, 0, 1, 2, 1, -1},
    {"PINNED ASYMMETRY: a lifted-side finding that arrives after an "
     "original-side fault is erased by a later PASS, because the fault took "
     "the slot first and only the slot's occupant is consulted",
     {IF, ILB, P}, 3, P, 0, 1, 3, 2, 0},
    {"an original-side fault after a PASS does not downgrade the PASS",
     {P, IF}, 2, P, 0, 1, 2, 1, -1},
};

void test_table() {
    for (const FoldRow &row : rows) {
        current_case = row.why;
        OracleFunctionVerdict got = oracle_fold_cases(row.verdicts, row.count);
        if (got.verdict != row.expected) {
            std::fprintf(stderr,
                         "fold verdict was %s, wanted %s [%s]\n",
                         label(got.verdict), label(row.expected), row.why);
            ++failures;
        }
        expect_int(got.arbitrated, row.arbitrated);
        expect_int(got.compared, row.compared);
        expect_int(got.ran, row.ran);
        expect_int(got.stopped_at, row.stopped_at);
        expect_int(got.winning_case, row.winning_case);
    }
    current_case = "";
}

// --------------------------------------------------------------------------
// 2. Exhaustive invariants over every sequence of length 1..3
//
// 10 + 100 + 1,000 = 1,110 sequences. Short, but long enough to contain every
// interaction the rules have: a stop, an upgrade, a refused upgrade, two
// timeouts, and every ordering of the three.
// --------------------------------------------------------------------------

int count_in(const OracleVerdict *v, int upto, OracleVerdict want) {
    int n = 0;
    for (int i = 0; i <= upto; ++i)
        if (v[i] == want) ++n;
    return n;
}

bool contains(const OracleVerdict *v, int upto, OracleVerdict want) {
    return count_in(v, upto, want) > 0;
}

char message[256];

void check_invariants(const OracleVerdict *v, int count) {
    OracleFunctionVerdict got = oracle_fold_cases(v, count);
    int n = std::snprintf(message, sizeof message, "sequence");
    for (int i = 0; i < count; ++i)
        n += std::snprintf(message + n, sizeof message - n, " %s", label(v[i]));
    std::snprintf(message + n, sizeof message - n, " -> %s", label(got.verdict));
    current_case = message;

    const int last = got.stopped_at;
    expect(got.ran == last + 1);
    expect(got.ran >= 1 && got.ran <= count);

    // The count that carries the caveat. If a PASS could silently erase a
    // top-page case this is the assertion that goes red, for every one of the
    // sequences in which that happens.
    expect_int(got.arbitrated, count_in(v, last, ITP));
    // `compared` must mean "both sides ran and were compared" and nothing else.
    expect_int(got.compared, count_in(v, last, P) + count_in(v, last, F));

    // A divergence stops the loop where it is found. If the break is dropped,
    // a FAIL at index 0 of a three-case sequence keeps running and this fires.
    for (int i = 0; i <= last; ++i) {
        if (v[i] == F || v[i] == LF || v[i] == ST || v[i] == SRB)
            expect(i == last);
    }

    // The fold cannot manufacture a verdict out of nothing.
    if (got.verdict == P) expect(contains(v, last, P));
    if (got.verdict == F) expect(contains(v, last, F));
    if (got.verdict == LF) expect(contains(v, last, LF));
    if (got.verdict == ST) expect(contains(v, last, ST));
    if (got.verdict == SRB) expect(contains(v, last, SRB));
    if (got.verdict == ITP) expect(contains(v, last, ITP));
    // And it never invents the qualified pass: that is not a case verdict, it
    // is what oracle_qualify_verdict makes of a whole function's PASS. A fold
    // that produced it would be deciding on evidence it does not have.
    expect(got.verdict != PPT);

    // A lifted-side finding on the first seed is never talked out of. The only
    // things that may replace it are the two failures and the trap, all of
    // which are at least as serious.
    if (v[0] == ILB || v[0] == IOS) {
        expect(got.verdict != P);
        expect(got.verdict == v[0] || got.verdict == F || got.verdict == LF ||
               got.verdict == ST || got.verdict == SRB);
    }

    // Two timeouts stop, wherever they are.
    int timeouts = 0, stop_expected = -1;
    for (int i = 0; i < count && stop_expected < 0; ++i) {
        // a stronger stop
        if (v[i] == F || v[i] == LF || v[i] == ST || v[i] == SRB) break;
        if (v[i] == IB || v[i] == ILB)
            if (++timeouts >= 2) stop_expected = i;
    }
    if (stop_expected >= 0) expect(last <= stop_expected);

    // `winning_case` is the index whose OracleResult the driver mirrors into
    // the report's detail column, so it decides WHICH case the row describes.
    // Nothing asserted it at all, and `winning_case = case_index` -> `= 0` in
    // the OracleUnrun arm changed 2,724 of 111,111 sequences with the whole
    // suite green. It must name a case that was actually examined, and it must
    // name the case that put the surviving verdict there.
    expect(got.winning_case >= -1 && got.winning_case <= last);
    // Every verdict except PASS is described by the case that produced it.
    if (got.verdict != P) {
        expect(got.winning_case >= 0);
        if (got.winning_case >= 0) expect(v[got.winning_case] == got.verdict);
    }
    // A stop is always described by the case that caused it.
    if (got.verdict == F || got.verdict == LF || got.verdict == ST ||
        got.verdict == SRB)
        expect_int(got.winning_case, last);
    // PASS is the exception, and the exception is worth stating rather than
    // stepping around: the upgrade to PASS does NOT move the slot. So either
    // no case ever claimed it (a PASS that passed from the start), or the slot
    // still names the case that was upgraded AWAY - which is necessarily one
    // of the four "the original could not be run" verdicts. It can never name
    // a lifted-side finding, because those are never upgraded.
    if (got.verdict == P && got.winning_case >= 0) {
        const OracleVerdict claimed = v[got.winning_case];
        expect(claimed == U || claimed == IF || claimed == IB ||
               claimed == ITP);
    }

    // A case that was never examined cannot have contributed anything.
    if (last + 1 < count) {
        OracleFunctionVerdict truncated = oracle_fold_cases(v, last + 1);
        expect(truncated.verdict == got.verdict);
        expect(truncated.arbitrated == got.arbitrated);
        expect(truncated.compared == got.compared);
    }

    current_case = "";
}

int sequences_checked = 0;

void test_exhaustive_short_sequences() {
    OracleVerdict v[3];
    for (int a = 0; a < all_verdicts_count; ++a) {
        v[0] = all_verdicts[a];
        check_invariants(v, 1);
        ++sequences_checked;
        for (int b = 0; b < all_verdicts_count; ++b) {
            v[1] = all_verdicts[b];
            check_invariants(v, 2);
            ++sequences_checked;
            for (int c = 0; c < all_verdicts_count; ++c) {
                v[2] = all_verdicts[c];
                check_invariants(v, 3);
                ++sequences_checked;
            }
        }
    }
}

// --------------------------------------------------------------------------
// 3. The stepper the driver actually calls
//
// oracle_fold_cases is a convenience for tests; lifted_oracle_main.cpp drives
// oracle_fold_step one case at a time so it can stop running seeds. If the two
// could disagree, every test above would be describing a program the sweep does
// not run - so drive the stepper by hand over the same table and require the
// same answer, including the early exit.
// --------------------------------------------------------------------------

void test_stepper_matches_the_whole_fold() {
    for (const FoldRow &row : rows) {
        current_case = row.why;
        OracleFoldState state;
        oracle_fold_reset(&state);
        int stepped = 0;
        // What the driver does with `worst`: mirror the OracleResult of
        // whichever case the state machine says owns the detail column.
        int mirrored_case = -1;
        int mirror_writes = 0;
        for (int c = 0; c < row.count; ++c) {
            const int previous = state.winning_case;
            const bool keep_going = oracle_fold_step(&state, row.verdicts[c], c);
            if (state.winning_case != previous) {
                mirrored_case = c;
                ++mirror_writes;
            }
            ++stepped;
            if (!keep_going) break;
        }
        OracleFunctionVerdict whole =
            oracle_fold_cases(row.verdicts, row.count);
        expect(state.verdict == whole.verdict);
        expect_int(state.arbitrated, whole.arbitrated);
        expect_int(state.compared, whole.compared);
        expect_int(state.ran, whole.ran);
        expect_int(state.stopped_at, whole.stopped_at);
        expect_int(state.winning_case, whole.winning_case);
        // The stepper must have stopped where the fold says it stopped, or the
        // driver is running seeds the state machine already ignored.
        expect_int(stepped, row.ran);
        if (stepped < row.count) expect(state.done);
        // The mirror must land on the case the fold names, and must be written
        // at most once per winner change - i.e. the driver's `worst` cannot be
        // left describing a case the row does not name.
        expect_int(mirrored_case, whole.winning_case);
        expect(mirror_writes <= 2);
    }
    current_case = "";
}

// A separate, blunt statement of the bug that reached committed numbers, kept
// out of the table so that it cannot be lost in a row edit.
void test_the_regression_that_reached_the_headline() {
    current_case = "the nineteen arbitrated functions counted as clean PASSes";
    const OracleVerdict thrown_out_then_passed[] = {ITP, ITP, P, P};
    OracleFunctionVerdict got = oracle_fold_cases(thrown_out_then_passed, 4);
    // The function IS counted in PASSED. That is the behaviour, and it is why
    // the count below is the only surviving evidence.
    expect(got.verdict == OraclePass);
    expect_int(got.arbitrated, 2);
    expect_int(got.compared, 2);
    // And the report's detail column is driven off the arbitrated count being
    // non-zero, so a zero here is a silent PASS row.
    expect(got.arbitrated != 0);

    current_case = "a PASS may not erase a lifted-side finding";
    const OracleVerdict lifted_finding_then_passes[] = {IOS, P, P, P};
    got = oracle_fold_cases(lifted_finding_then_passes, 4);
    expect(got.verdict == OracleInconclusiveOutOfSpan);
    const OracleVerdict lifted_ran_away_then_passes[] = {ILB, P, P, P};
    got = oracle_fold_cases(lifted_ran_away_then_passes, 4);
    expect(got.verdict == OracleInconclusiveLiftedBudget);
    current_case = "";
}

// ---------------------------------------------------------------------------
// The last mile: fold state -> the numbers the row and the tally print.
//
// These four values used to be bare reads off `fold` inside the driver, and
// nothing in this repo compiles the driver. Both of the project's reported
// bugs are one character of them, and both were reinstated by an earlier
// verification pass with the whole suite green:
//
//     const int arbitrated = fold.arbitrated;  ->  = 0;
//         "PASSED with a case arbitrated 1" becomes 0, and the caveat
//         disappears from the PASS row. The exact committed bug.
//
//     const int compared = fold.compared;      ->  = fold.ran;
//         every row claims all sixteen seeds compared, and the summary then
//         prints "0 PASSes rest on a single seed" where the truth was 141.
// ---------------------------------------------------------------------------
void test_row_counts() {
    current_case = "cases attempted is not cases compared";
    // Fifteen seeds the original could not be run on, one that could. The row
    // must say 16 attempted and 1 compared - the weakest kind of PASS there
    // is, and the one the seed histogram exists to surface.
    OracleVerdict v[16];
    for (int i = 0; i < 15; ++i) v[i] = IF;
    v[15] = P;
    OracleFoldState state;
    oracle_fold_reset(&state);
    for (int i = 0; i < 16; ++i) oracle_fold_step(&state, v[i], i);
    OracleRowCounts counts = oracle_row_counts(state);
    expect(state.verdict == OraclePass);
    expect_int(counts.cases, 16);
    expect_int(counts.compared, 1);
    // The two columns must not be the same number, or the histogram collapses.
    expect(counts.cases != counts.compared);

    current_case = "a fully compared function reports both columns equal";
    oracle_fold_reset(&state);
    for (int i = 0; i < 4; ++i) oracle_fold_step(&state, P, i);
    counts = oracle_row_counts(state);
    expect_int(counts.cases, 4);
    expect_int(counts.compared, 4);

    current_case = "a FAIL stops, and both columns say so";
    oracle_fold_reset(&state);
    oracle_fold_step(&state, P, 0);
    oracle_fold_step(&state, F, 1);
    counts = oracle_row_counts(state);
    expect_int(counts.cases, 2);
    expect_int(counts.compared, 2);

    current_case = "an all-skip-trap function compared nothing";
    oracle_fold_reset(&state);
    oracle_fold_step(&state, ST, 0);
    counts = oracle_row_counts(state);
    expect_int(counts.cases, 1);
    expect_int(counts.compared, 0);

    current_case = "counts agree with the whole-sequence fold";
    OracleFunctionVerdict whole = oracle_fold_cases(v, 16);
    oracle_fold_reset(&state);
    for (int i = 0; i < 16; ++i) oracle_fold_step(&state, v[i], i);
    counts = oracle_row_counts(state);
    expect_int(counts.cases, whole.ran);
    expect_int(counts.compared, whole.compared);
    current_case = "";
}

bool caveat_mentions(const char *text, int n) {
    char wanted[32];
    std::snprintf(wanted, sizeof wanted, "%d case(s) thrown out", n);
    return std::strstr(text, wanted) != nullptr;
}

void test_pass_caveat() {
    char detail[512];
    OracleFoldState state;

    current_case = "an arbitrated PASS carries the caveat and the count";
    // The reported bug, end to end: two cases thrown out by the top-page
    // arbitration, a later seed passes, the function is counted in PASSED -
    // and the ONLY surviving evidence is this caveat.
    oracle_fold_reset(&state);
    oracle_fold_step(&state, ITP, 0);
    oracle_fold_step(&state, ITP, 1);
    oracle_fold_step(&state, P, 2);
    expect(state.verdict == OraclePass);
    expect(oracle_pass_caveat(state, detail, sizeof detail));
    expect(caveat_mentions(detail, 2));
    expect(std::strstr(detail, "top 64 KiB") != nullptr);
    // ...and the count in the text is the real count, not a hard-coded 1.
    expect(!caveat_mentions(detail, 1));

    current_case = "one arbitrated case reads as one";
    oracle_fold_reset(&state);
    oracle_fold_step(&state, ITP, 0);
    oracle_fold_step(&state, P, 1);
    expect(oracle_pass_caveat(state, detail, sizeof detail));
    expect(caveat_mentions(detail, 1));

    current_case = "a clean PASS carries no caveat at all";
    // An unconditional caveat would put "0 case(s) thrown out" on all 1,465
    // PASS rows and make the report unreadable.
    oracle_fold_reset(&state);
    oracle_fold_step(&state, P, 0);
    expect(!oracle_pass_caveat(state, detail, sizeof detail));
    expect_int(int(std::strlen(detail)), 0);

    current_case = "a non-PASS verdict never claims an arbitrated pass";
    // The tally field is `passed_with_arbitration`; a FAIL that happened to
    // examine a top-page case earlier must not increment it.
    oracle_fold_reset(&state);
    oracle_fold_step(&state, ITP, 0);
    oracle_fold_step(&state, F, 1);
    expect(state.verdict == OracleFail);
    expect_int(state.arbitrated, 1);
    expect(!oracle_pass_caveat(state, detail, sizeof detail));
    expect_int(int(std::strlen(detail)), 0);

    current_case = "the caveat and the tally are one decision";
    // Every sequence of length <= 3: the caveat is written exactly when the
    // function is a PASS with a surviving arbitrated count, which is exactly
    // the condition the driver increments the tally on. Two conditions here
    // would be two chances to drift.
    OracleVerdict v[3];
    for (int a = 0; a < all_verdicts_count; ++a) {
        v[0] = all_verdicts[a];
        for (int b = 0; b < all_verdicts_count; ++b) {
            v[1] = all_verdicts[b];
            for (int c = 0; c < all_verdicts_count; ++c) {
                v[2] = all_verdicts[c];
                oracle_fold_reset(&state);
                for (int i = 0; i < 3; ++i)
                    if (!oracle_fold_step(&state, v[i], i)) break;
                const bool wrote = oracle_pass_caveat(state, detail,
                                                      sizeof detail);
                expect(wrote == (state.verdict == OraclePass &&
                                 state.arbitrated > 0));
                expect(wrote == (detail[0] != '\0'));
                if (wrote) expect(caveat_mentions(detail, state.arbitrated));
            }
        }
    }
    current_case = "";
}

// ---------------------------------------------------------------------------
// Telling a phase-4 work item from a wall the lift will never climb.
//
// Every trap arrives as one string. `bt` means "lower this mnemonic and the
// function becomes testable"; "call to an address with no lifted body" means
// the run walked into one of the constructs the static plan flags, and that is
// the entire evidence for whether a flagged path is a path the program takes.
// Measured on the 470 extcall-only functions: 79 SKIP-reached-blocked and 0
// SKIP-trap, so a classifier stuck at `false` would have reported 79 phase-4
// items that do not exist, and one stuck at `true` would have reported the
// image's 92 un-lowered instructions as unreachable walls.
//
// The strings are matched, so the risk is that one is edited at its source and
// this list silently stops recognising it. tools/test_lifted_oracle_trap_
// reasons.py pins each pattern against the file that emits it; this test pins
// the BEHAVIOUR, including the near misses that a sloppier match would accept.
// ---------------------------------------------------------------------------
void test_trap_classification() {
    current_case = "the four blocked constructs are recognised";
    // tools/lift_whole_image.py, write_dispatch(). This is the one that fired
    // 79 times, at 0x00644f3a among others.
    expect(oracle_trap_is_blocked_construct(
        "call to an address with no lifted body"));
    // tools/lifted_tls.h, opensmacx_tib_at().
    expect(oracle_trap_is_blocked_construct(
        "fs: displacement outside the modelled TIB"));
    // tools/generate_imports.py's shim for an import with no body.
    expect(oracle_trap_is_blocked_construct(
        "unimplemented import ADVAPI32.dll!RegQueryValueExA"));
    expect(oracle_trap_is_blocked_construct(
        "unimplemented import KERNEL32.dll!ExitProcess"));
    // tools/lifted_imports.h's marshalling refusals, all four of them.
    expect(oracle_trap_is_blocked_construct(
        "import argument is not an object of that size inside the image"));
    expect(oracle_trap_is_blocked_construct(
        "import array argument is larger than the whole image"));
    expect(oracle_trap_is_blocked_construct(
        "import string argument has no terminator before the end of the "
        "image"));
    expect(oracle_trap_is_blocked_construct(
        "import returned host memory the guest cannot address; the image "
        "needs its canonical base (see opensmacx_guest)"));
    // tools/generate_imports.py, opensmacx_import_dispatch().
    expect(oracle_trap_is_blocked_construct(
        "synthetic import address with no import"));

    current_case = "a mnemonic the lowerer refused is NOT a blocked construct";
    // Every reason the shards emit is the bare mnemonic. These eight are the
    // ones the committed report actually contains, plus the three whole-body
    // reasons lift_whole_image.py writes.
    static const char *const lowering_refusals[] = {
        "bt", "loop", "pushal", "pushfd", "popf", "int3", "cpuid", "out",
        "self-modifying section", "fell off the end of the body",
        "not lowered",
    };
    for (const char *reason : lowering_refusals)
        expect(!oracle_trap_is_blocked_construct(reason));

    current_case = "near misses are rejected, so the match cannot drift wide";
    // A prefix of the exact-match string.
    expect(!oracle_trap_is_blocked_construct("call to an address with no"));
    // The exact-match string with something appended.
    expect(!oracle_trap_is_blocked_construct(
        "call to an address with no lifted bodies"));
    // The old wording, which is what a half-finished rename would leave
    // behind. It must NOT be recognised, or this test would pass against a
    // generator that no longer emits what the list claims.
    expect(!oracle_trap_is_blocked_construct(
        "indirect call to an address with no function"));
    // "import" as a word inside something longer, with no following space:
    // the prefix test requires the space precisely so this is not a match.
    expect(!oracle_trap_is_blocked_construct("importance sampling"));
    expect(!oracle_trap_is_blocked_construct("imports"));
    // And a bare "fs" is a mnemonic-shaped string, not the TIB message.
    expect(!oracle_trap_is_blocked_construct("fs"));

    current_case = "no reason at all is not a blocked construct";
    // g_trap_reason is null until a trap sets it, and the driver prints "?"
    // for it. Classifying null as blocked would turn every trap with a lost
    // reason into a wall.
    expect(!oracle_trap_is_blocked_construct(nullptr));
    expect(!oracle_trap_is_blocked_construct(""));
    current_case = "";
}

// ---------------------------------------------------------------------------
// A PASS on a statically flagged function is not a PASS.
//
// This is the one rule that stops the whole change from being a way to inflate
// the coverage headline. 55 of the 470 extcall-only functions agreed with the
// original on every seed that could be judged - real evidence, and NOT the
// same evidence as a clean PASS, because the path to the callee the lift has
// no body for was never taken by any of the sixteen seeds. If this function
// returned its argument unchanged, those 55 would be added to PASSED and the
// project's headline would move by 19,876 bytes on weaker evidence than the
// number claims.
// ---------------------------------------------------------------------------
void test_qualify_verdict() {
    current_case = "a PASS on a flagged function is downgraded, and named";
    expect(oracle_qualify_verdict(P, true) == PPT);
    expect(oracle_qualify_verdict(P, true) != P);

    current_case = "a PASS on a clean function is untouched";
    expect(oracle_qualify_verdict(P, false) == P);

    current_case = "nothing but a PASS is qualified";
    // A FAIL is a divergence on a path that WAS taken and the static flag adds
    // nothing to it; softening one would be the same mistake in the other
    // direction. A SKIP or an INCONCLUSIVE was never a claim about agreement.
    for (int i = 0; i < all_verdicts_count; ++i) {
        const OracleVerdict v = all_verdicts[i];
        if (v == P) continue;
        expect(oracle_qualify_verdict(v, true) == v);
        expect(oracle_qualify_verdict(v, false) == v);
    }

    current_case = "the qualified pass is reachable ONLY through a flag";
    // If any verdict/flag pair other than (PASS, blocked) produced it, the
    // report could print PASS-paths-taken for a function whose paths were
    // never in question.
    for (int i = 0; i < all_verdicts_count; ++i) {
        for (int b = 0; b < 2; ++b) {
            const bool blocked = b != 0;
            const bool qualified =
                oracle_qualify_verdict(all_verdicts[i], blocked) == PPT;
            expect(qualified == (all_verdicts[i] == P && blocked));
        }
    }
    current_case = "";
}

}  // namespace

int main() {
    test_table();
    test_exhaustive_short_sequences();
    test_stepper_matches_the_whole_fold();
    test_the_regression_that_reached_the_headline();
    test_row_counts();
    test_pass_caveat();
    test_trap_classification();
    test_qualify_verdict();
    std::printf("lifted-oracle-fold: %d table rows, %d sequences, %d failure(s)\n",
                int(sizeof rows / sizeof rows[0]), sequences_checked, failures);
    return failures == 0 ? 0 : 1;
}
