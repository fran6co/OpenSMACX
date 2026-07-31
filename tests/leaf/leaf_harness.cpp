// Failure bookkeeping, the case registry, and main() for the split leaf suite.
//
// main() is fixed: it walks whatever the family files registered.  Adding a
// test never edits this file.
#include "leaf_shared.h"

#include <cstdlib>

int failures = 0;

void expect_at(bool condition, const char *file, int line) {
    if (!condition) {
        // The original single-file harness printed only the line, which was
        // unambiguous when there was one file.  After the split every family
        // file has a line 453, so the file name has to travel with it.
        std::fprintf(stderr, "leaf expect failed at %s line %d\n", file, line);
        ++failures;
    }
}

void expect_tracked_free_calls(int expected) {
#if defined(__MINGW32__)
    expect(tracked_free_calls == expected);
#else
    static_cast<void>(expected);
#endif
}

namespace leaf {

Entry g_cases[kMaxCases];
int g_case_count = 0;
int g_overflow = 0;

Registrar::Registrar(int order, const char *name, CaseFn fn,
                     const char *family) {
    if (g_case_count >= kMaxCases) {
        ++g_overflow;
        return;
    }
    Entry &entry = g_cases[g_case_count++];
    entry.order = order;
    entry.name = name;
    entry.fn = fn;
    entry.family = family;
}

}  // namespace leaf

namespace {

// The manifest, materialised twice: once as the list of families that are
// supposed to exist, once as the count each is supposed to register.  It is
// generated at build time by tools/generate_leaf_manifest.py from a glob of
// tests/leaf/*_tests.cpp - deliberately not from the CMake source list it
// polices, so that a family dropping out of the build stays in the manifest
// and this check still fires.
#define LEAF_FAMILY_COUNT(family, count) {#family, (count), 0},
struct FamilyRow {
    const char *name;
    int expected;
    int seen;
};
FamilyRow g_families[] = {
#include "leaf_generated_manifest.h"
    {nullptr, 0, 0}
};
#undef LEAF_FAMILY_COUNT

int family_rows() {
    int n = 0;
    while (g_families[n].name != nullptr) {
        ++n;
    }
    return n;
}

// Sort by baseline position, then by name.  Ties on position cannot happen
// among the original 229 cases; for cases added later (LEAF_APPEND) the name
// is what makes the order a property of the sources rather than of the link
// line.
bool precedes(const leaf::Entry &a, const leaf::Entry &b) {
    if (a.order != b.order) {
        return a.order < b.order;
    }
    return std::strcmp(a.name, b.name) < 0;
}

void sort_cases() {
    for (int i = 1; i < leaf::g_case_count; ++i) {
        leaf::Entry key = leaf::g_cases[i];
        int j = i - 1;
        while (j >= 0 && precedes(key, leaf::g_cases[j])) {
            leaf::g_cases[j + 1] = leaf::g_cases[j];
            --j;
        }
        leaf::g_cases[j + 1] = key;
    }
}

// Every way the registry can be wrong has to be loud.  A test that silently
// stops running is the failure mode this whole structure exists to prevent,
// so the checks run before a single case does and the process refuses to
// report success without them.
bool verify_registry() {
    bool ok = true;

    if (leaf::g_overflow != 0) {
        std::fprintf(stderr,
                     "leaf registry: overflow, %d case(s) did not fit in %d "
                     "slots; raise leaf::kMaxCases\n",
                     leaf::g_overflow, static_cast<int>(leaf::kMaxCases));
        ok = false;
    }

    // Duplicate names would let one case shadow another: always an error,
    // at every position.
    //
    // Duplicate BASELINE positions - below LEAF_APPEND - are also an error.
    // Those 229 numbers are the pre-split main() call list; two cases claiming
    // one of them means the carve went wrong, and the order would no longer be
    // recoverable from the sources.
    //
    // At or above LEAF_APPEND a duplicate is not an error and must not be one.
    // precedes() already breaks position ties by name, so a shared position
    // there still yields one total order fixed by the sources alone.  Refusing
    // duplicates there rejected exactly the case the tie-break exists for, and
    // turned LEAF_APPEND into a global number namespace with no coordination
    // point: two authors in two families both writing the natural
    // `LEAF_APPEND + 1` merged clean, built clean, and then the suite refused
    // to run.  Appending is now `LEAF_CASE(LEAF_APPEND, test_x)` with no
    // number to pick and nothing to coordinate.
    for (int i = 0; i < leaf::g_case_count; ++i) {
        for (int j = i + 1; j < leaf::g_case_count; ++j) {
            if (std::strcmp(leaf::g_cases[i].name, leaf::g_cases[j].name) == 0) {
                std::fprintf(stderr,
                             "leaf registry: duplicate case name %s (%s and %s)\n",
                             leaf::g_cases[i].name, leaf::g_cases[i].family,
                             leaf::g_cases[j].family);
                ok = false;
            }
            if (leaf::g_cases[i].order == leaf::g_cases[j].order &&
                leaf::g_cases[i].order < LEAF_APPEND) {
                std::fprintf(stderr,
                             "leaf registry: duplicate baseline position %d "
                             "(%s in %s, %s in %s); positions below %d are the "
                             "pre-split main() call list and are unique\n",
                             leaf::g_cases[i].order, leaf::g_cases[i].name,
                             leaf::g_cases[i].family, leaf::g_cases[j].name,
                             leaf::g_cases[j].family,
                             static_cast<int>(LEAF_APPEND));
                ok = false;
            }
        }
    }

    const int rows = family_rows();
    for (int i = 0; i < leaf::g_case_count; ++i) {
        int row = -1;
        for (int r = 0; r < rows; ++r) {
            if (std::strcmp(g_families[r].name, leaf::g_cases[i].family) == 0) {
                row = r;
                break;
            }
        }
        if (row < 0) {
            std::fprintf(stderr,
                         "leaf registry: case %s came from family %s, which is "
                         "not in the derived manifest; LEAF_FAMILY must match "
                         "the <family>_tests.cpp file name\n",
                         leaf::g_cases[i].name, leaf::g_cases[i].family);
            ok = false;
            continue;
        }
        ++g_families[row].seen;
    }

    int expected_total = 0;
    for (int r = 0; r < rows; ++r) {
        expected_total += g_families[r].expected;
        if (g_families[r].seen != g_families[r].expected) {
            std::fprintf(stderr,
                         "leaf registry: family %s registered %d case(s), "
                         "manifest says %d (short by %d)\n",
                         g_families[r].name, g_families[r].seen,
                         g_families[r].expected,
                         g_families[r].expected - g_families[r].seen);
            ok = false;
        }
    }

    if (leaf::g_case_count != expected_total) {
        std::fprintf(stderr,
                     "leaf registry: %d case(s) registered, manifest expects "
                     "%d\n",
                     leaf::g_case_count, expected_total);
        ok = false;
    }

    if (!ok) {
        std::fprintf(stderr,
                     "leaf registry: refusing to run a suite that does not "
                     "match its manifest\n");
    }
    return ok;
}

}  // namespace

int main(int argc, char **argv) {
    // Sprite's constructor charges a fixed-address accounting global that is
    // only mapped inside the hybrid process. Objects embedding Sprite by value
    // are constructed throughout these tests, so bind the counter to process
    // storage for the whole run; individual tests rebind it as needed.
    static int sprite_memory_sink = 0;
    SpriteMemoryUsed = &sprite_memory_sink;

    // Argument parsing is strict on purpose.  An earlier standalone sweep of
    // this suite was vacuous because an unrecognised selector simply fell
    // through to the run loop and then to `return failures == 0`, so every
    // misspelled case name reported a pass without running anything.  An
    // unknown argument is now a hard error, and the run loop below refuses to
    // report success unless it can say how many cases it actually called.
    bool list_only = false;
    bool reverse = false;
    const char *only = nullptr;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--list") == 0) {
            list_only = true;
        } else if (std::strcmp(argv[i], "--reverse") == 0) {
            reverse = true;
        } else if (std::strcmp(argv[i], "--only") == 0) {
            if (i + 1 >= argc) {
                std::fprintf(stderr, "leaf: --only needs a case name\n");
                return 2;
            }
            only = argv[++i];
        } else {
            std::fprintf(stderr,
                         "leaf: unknown argument %s (want --list, --reverse "
                         "or --only <case>)\n",
                         argv[i]);
            return 2;
        }
    }

    if (!verify_registry()) {
        return 2;
    }
    sort_cases();

    // --list prints the names that would run, in run order.  The neutrality
    // proof diffs this against the baseline extracted from the pre-split
    // main().
    if (list_only) {
        for (int i = 0; i < leaf::g_case_count; ++i) {
            std::printf("%s\n", leaf::g_cases[i].name);
        }
        return 0;
    }

    // --reverse runs the same cases back to front.  It is the direct evidence
    // that no case depends on an earlier one having run: a suite that passes
    // in both directions has no ordering channel left to hide in.
    int ran = 0;
    for (int k = 0; k < leaf::g_case_count; ++k) {
        const int i = reverse ? (leaf::g_case_count - 1 - k) : k;
        if (only != nullptr && std::strcmp(leaf::g_cases[i].name, only) != 0) {
            continue;
        }
        if (only != nullptr) {
            std::fprintf(stderr, "leaf: RUN %s\n", leaf::g_cases[i].name);
            std::fflush(stderr);
        }
        leaf::g_cases[i].fn();
        ++ran;
        if (only != nullptr) {
            std::fprintf(stderr, "leaf: DONE %s (failures=%d)\n",
                         leaf::g_cases[i].name, failures);
            std::fflush(stderr);
        }
    }

    // The run marker.  It is printed unconditionally so that "the suite was
    // green" and "the suite ran something" are never the same claim.
    std::fprintf(stderr, "leaf: ran %d of %d case(s), %d failure(s)\n", ran,
                 leaf::g_case_count, failures);
    if (ran == 0) {
        std::fprintf(stderr,
                     "leaf: no case ran%s; refusing to report success\n",
                     only != nullptr
                         ? " - --only matched no registered case name"
                         : "");
        return 2;
    }
    if (only != nullptr && ran != 1) {
        std::fprintf(stderr,
                     "leaf: --only %s selected %d cases; names are unique so "
                     "this cannot happen\n",
                     only, ran);
        return 2;
    }
    return failures == 0 ? 0 : 1;
}
