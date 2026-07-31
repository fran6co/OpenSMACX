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
// supposed to exist, once as the count each is supposed to register.
#define LEAF_FAMILY_COUNT(family, count) {#family, (count), 0},
struct FamilyRow {
    const char *name;
    int expected;
    int seen;
};
FamilyRow g_families[] = {
#include "leaf_case_manifest.h"
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
// among the original 229 cases; for cases added later (LEAF_APPEND + n) the
// name is what makes the order a property of the sources rather than of the
// link line.
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

    // Duplicate names would let one case shadow another.  Duplicate baseline
    // positions would make the run order depend on something other than the
    // sources.  Both are errors, not warnings.
    for (int i = 0; i < leaf::g_case_count; ++i) {
        for (int j = i + 1; j < leaf::g_case_count; ++j) {
            if (std::strcmp(leaf::g_cases[i].name, leaf::g_cases[j].name) == 0) {
                std::fprintf(stderr,
                             "leaf registry: duplicate case name %s (%s and %s)\n",
                             leaf::g_cases[i].name, leaf::g_cases[i].family,
                             leaf::g_cases[j].family);
                ok = false;
            }
            if (leaf::g_cases[i].order == leaf::g_cases[j].order) {
                std::fprintf(stderr,
                             "leaf registry: duplicate position %d (%s in %s, "
                             "%s in %s)\n",
                             leaf::g_cases[i].order, leaf::g_cases[i].name,
                             leaf::g_cases[i].family, leaf::g_cases[j].name,
                             leaf::g_cases[j].family);
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
                         "not in tests/leaf/leaf_case_manifest.h\n",
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

    if (!verify_registry()) {
        return 2;
    }
    sort_cases();

    // --list prints the names that would run, in run order.  The neutrality
    // proof diffs this against the baseline extracted from the pre-split
    // main().
    if (argc > 1 && std::strcmp(argv[1], "--list") == 0) {
        for (int i = 0; i < leaf::g_case_count; ++i) {
            std::printf("%s\n", leaf::g_cases[i].name);
        }
        return 0;
    }

    for (int i = 0; i < leaf::g_case_count; ++i) {
        leaf::g_cases[i].fn();
    }
    return failures == 0 ? 0 : 1;
}
