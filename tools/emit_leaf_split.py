#!/usr/bin/env python3
"""Split tests/recovery_leaf_tests.cpp into tests/leaf/ family files.

One-shot: the pre-split file is gone from the tree, so re-running this needs
`git show <pre-split-rev>:tests/recovery_leaf_tests.cpp > tests/recovery_leaf_tests.cpp`
first.  It is kept so the carve is auditable rather than a claim - anyone can
regenerate tests/leaf/ and diff it against what is committed.

The split is mechanical: every top-level item of the original file is copied
verbatim into exactly one output file, in its original order.  The only edits
are (a) symbols referenced across a family boundary get an `extern`
declaration in leaf_shared.h and, if they lived in an unnamed namespace, are
lifted out of it, and (b) main() is replaced by the registry walk.
"""
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from split_leaf_tests import parse  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, 'tests', 'leaf')

# First baseline test index of each family, and the family's file stem.
FAMILIES = [
    (0,   'core_construction'),
    (18,  'leaf_recoveries'),
    (23,  'widget_accessors'),
    (53,  'text_and_resources'),
    (68,  'dialog_and_strings'),
    (77,  'win_and_graphic'),
    (92,  'destructors'),
    (104, 'buffer_and_font'),
    (118, 'constant_stubs'),
    (123, 'wave_and_sound'),
    (145, 'delegates'),
    (156, 'forwarders'),
    (166, 'stubs_and_locks'),
    (178, 'locks'),
    (188, 'device_forwarders'),
    (198, 'buffer_writers'),
    (208, 'pops_and_console'),
    (221, 'thunks'),
]

INCLUDE_LINES = (1, 110)        # the shared #include preamble
STUB_LINES = (112, 395)         # global-namespace stand-ins for src symbols
IN_BOX_DECL_LINE = 397
MAIN_START = 30703


def build():
    lines, clean, items = parse()
    for i, x in enumerate(items):
        x['i'] = i
    its = [x for x in items if x['kind'] == 'item']
    tests = [x for x in its if x['k'] == 'test']
    assert len(tests) == 229, len(tests)

    # Make the items tile the file: every line between two items - the comment
    # blocks that carry the recovery reasoning, above all - is folded into the
    # item that follows it, so the split copies each line exactly once.
    cursor = 1
    for x in items:
        x['emit_start'] = cursor
        cursor = x['end'] + 1

    bounds = []
    for f, _ in FAMILIES:
        bounds.append(tests[f]['i'] if f == 0 else tests[f - 1]['i'] + 1)
    mainitem = [x for x in items if x['kind'] == 'item'
                and x['start'] == MAIN_START][0]
    bounds.append(mainitem['i'])

    slices = [('preamble', 0, bounds[0])]
    for k, (_, nm) in enumerate(FAMILIES):
        slices.append((nm, bounds[k], bounds[k + 1]))
    slices.append(('tail', bounds[-1], len(items)))
    sid = {}
    for si, (nm, a, b) in enumerate(slices):
        for x in items[a:b]:
            sid[x['i']] = si
    tail_si = len(slices) - 1
    return lines, items, its, tests, slices, sid, tail_si


def shared_names(items, its, sid, tail_si):
    name2slice = {}
    for x in its:
        for n in x['defs']:
            name2slice.setdefault(n, set()).add(sid[x['i']])
    users = {}
    for x in items:
        if x['kind'] != 'item':
            continue
        s = sid[x['i']]
        if s == tail_si:
            continue
        for n in x['ids']:
            if n in name2slice and n not in x['defs']:
                for ds in name2slice[n]:
                    if ds != s:
                        users.setdefault(n, set()).add(s)
    return set(users)


def is_pure_declaration(c):
    return '{' not in c and '=' not in c and c.rstrip().endswith(';')


def is_template(c):
    return c.lstrip().startswith('template')


def header_definition(x):
    """Items whose whole text must live in leaf_shared.h."""
    c = x['clean']
    if x['k'] in ('type', 'alias'):
        return True
    if is_template(c):
        return True
    if x['k'] == 'func' and is_pure_declaration(c):
        return True
    return False


FUNCPTR = re.compile(r'\(\s*[A-Za-z_][\w\s]*\*\s*([A-Za-z_]\w*)\s*\)\s*\(')


def declaration_for(x, lines):
    """The `extern` declaration that lets another family see this item."""
    raw = '\n'.join(lines[x['start'] - 1:x['end']])
    c = x['clean']
    proto = c.split('{')[0].split('=')[0]
    if any(re.search(r'\b\w+\s*::\s*' + re.escape(n) + r'\b', proto)
           for n in x['defs']):
        return None  # out-of-class member definition; src header declares it
    if x['k'] == 'func' and not FUNCPTR.search(c.split('{')[0]):
        head = raw[:raw.index('{')] if '{' in raw else raw
        return head.rstrip().rstrip(';').rstrip() + ';'
    # variable (including function-pointer variables)
    stop = len(raw)
    for ch in ('=', ';'):
        p = raw.find(ch)
        if p >= 0:
            stop = min(stop, p)
    head = raw[:stop].rstrip()
    assert not head.lstrip().startswith(('const', 'constexpr')), head
    return 'extern ' + head + ';'


def main():
    lines, items, its, tests, slices, sid, tail_si = build()
    sh = shared_names(items, its, sid, tail_si)
    sh -= set(t['name'] for t in tests)

    # Items that contribute to leaf_shared.h.
    promoted = {}
    for x in its:
        if x['k'] in ('pp', 'assert'):
            continue
        if any(n in sh for n in x['defs']):
            promoted[x['i']] = x

    # Transitive closure: text placed in the header may name other test-file
    # symbols, which then need their own declaration earlier in the header.
    name2item = {}
    for x in its:
        for n in x['defs']:
            name2item.setdefault(n, x)
    changed = True
    while changed:
        changed = False
        for x in list(promoted.values()):
            text = x['clean'] if header_definition(x) else (
                declaration_for(x, lines) or '')
            for n in set(re.findall(r'[A-Za-z_]\w*', text)):
                y = name2item.get(n)
                if y is None or y['i'] in promoted or y['k'] == 'test':
                    continue
                if y['k'] in ('pp', 'assert'):
                    continue
                if y['start'] > x['start']:
                    raise SystemExit(
                        'forward reference into header: %s (line %d) needed by '
                        'line %d' % (n, y['start'], x['start']))
                promoted[y['i']] = y
                changed = True

    # Preprocessor nesting: a promoted item must not sit inside #if/#endif,
    # because lifting it out of an unnamed namespace would unbalance braces
    # when the branch is not taken.
    ppstack = {}
    stack = []
    for x in items:
        ppstack[x['i']] = list(stack)
        if x['kind'] == 'item' and x['k'] == 'pp':
            h = lines[x['start'] - 1].strip()
            if re.match(r'^#\s*(if|ifdef|ifndef)\b', h):
                stack.append(h)
            elif re.match(r'^#\s*endif\b', h):
                stack.pop()
    for i in promoted:
        # Lifting an item out of an unnamed namespace inserts a brace, which
        # would unbalance the file when the branch is not taken.
        if ppstack[i] and items[i].get('inns'):
            raise SystemExit('promoted unnamed-namespace item inside #if at '
                             'line %d' % items[i]['start'])

    emit(lines, items, its, tests, slices, sid, tail_si, promoted, ppstack)


def emit(lines, items, its, tests, slices, sid, tail_si, promoted, ppstack):
    os.makedirs(OUT, exist_ok=True)
    # The baseline run order is main()'s call list, NOT the order the bodies
    # happen to sit in the file.  The two differ, and the difference matters:
    # test_wave_device_construction faults unless test_wave_destructor has run
    # first (see the hazard note in leaf_harness.h), so the ordinal has to come
    # from main().
    order_of = {}
    for line in lines[MAIN_START - 1:]:
        m = re.match(r'^\s*(test_\w+)\(\);\s*$', line)
        if m:
            assert m.group(1) not in order_of, m.group(1)
            order_of[m.group(1)] = len(order_of)
        if line.strip().startswith('return failures'):
            break
    assert len(order_of) == len(tests), (len(order_of), len(tests))
    assert set(order_of) == set(t['name'] for t in tests)

    # ---- leaf_harness.h -------------------------------------------------
    inc = '\n'.join(lines[INCLUDE_LINES[0] - 1:INCLUDE_LINES[1]])
    # The originals were relative to tests/; these files sit one level deeper.
    inc = inc.replace('#include "../src/', '#include "../../src/')
    open(os.path.join(OUT, 'leaf_harness.h'), 'w').write(HARNESS_H % inc)

    # ---- leaf_shared.h --------------------------------------------------
    out = [SHARED_H_HEAD]
    for i in sorted(promoted):
        x = promoted[i]
        if header_definition(x):
            out.append('\n'.join(lines[x['emit_start'] - 1:x['end']]).strip('\n'))
        else:
            decl = declaration_for(x, lines)
            if decl:
                guard = ppstack[i]
                if guard:
                    decl = ('\n'.join(guard) + '\n' + decl + '\n'
                            + '#endif\n' * len(guard)).rstrip()
                out.append(decl)
    out.append(SHARED_H_TAIL)
    open(os.path.join(OUT, 'leaf_shared.h'), 'w').write('\n\n'.join(out) + '\n')

    # ---- leaf_stubs.cpp -------------------------------------------------
    stub = '\n'.join(lines[STUB_LINES[0] - 1:STUB_LINES[1]])
    open(os.path.join(OUT, 'leaf_stubs.cpp'), 'w').write(STUBS_CPP % stub)

    # ---- family files ---------------------------------------------------
    counts = []
    for si, (nm, a, b) in enumerate(slices):
        if si == 0 or si == tail_si:
            continue
        body = []
        inns = False          # unnamed namespace open in the output?
        names = []
        for x in items[a:b]:
            if x['kind'] == 'nsopen' or x['kind'] == 'nsclose':
                # The brace itself is re-derived below, but the lines before
                # it are content.
                gap = '\n'.join(lines[x['emit_start'] - 1:x['start'] - 1])
                if gap.strip():
                    body.append(gap.strip('\n'))
                continue
            if x['i'] in promoted and header_definition(x):
                continue      # text and leading comment now live in leaf_shared.h
            want = x.get('inns', False) and x['i'] not in promoted
            if want and not inns:
                body.append('namespace {')
                inns = True
            elif not want and inns:
                body.append('}  // namespace')
                inns = False
            body.append('\n'.join(lines[x['emit_start'] - 1:x['end']]).strip('\n'))
            if x['k'] == 'test':
                names.append(x['name'])
        if inns:
            body.append('}  // namespace')
        reg = ['', '// Self-registration.  Each case carries its baseline '
               'position in the original', '// main() call list, so the run '
               'order does not depend on link or static-init', '// order.']
        reg.append('namespace {')
        for n in names:
            reg.append('LEAF_CASE(%d, %s);' % (order_of[n], n))
        reg.append('}  // namespace')
        text = FAMILY_CPP % (nm, nm, '\n\n'.join(body) + '\n' + '\n'.join(reg))
        open(os.path.join(OUT, nm + '_tests.cpp'), 'w').write(text)
        counts.append((nm, len(names)))

    # No manifest is written here.  The per-family case counts are derived at
    # build time by tools/generate_leaf_manifest.py, from a glob of the family
    # files rather than from the CMake source list, so that nothing an author
    # edits and nothing this script emits has to be kept in sync by hand.
    print('families: %d  cases: %d' % (len(counts), sum(c for _, c in counts)))
    for n, c in counts:
        print('  %-22s %3d' % (n, c))


HARNESS_H = '''// Shared preamble and test harness for the split recovery leaf suite.
//
// This file, tests/leaf/leaf_shared.h and tests/leaf/leaf_harness.cpp were
// carved out of the single 30,950-line tests/recovery_leaf_tests.cpp.  Test
// bodies live in tests/leaf/<family>_tests.cpp and register themselves; the
// harness never has to be edited to add one.
//
// ADDING A TEST - ONE FILE
//   1. Append `void test_<name>()` to the family file it belongs to.
//   2. Append `LEAF_CASE(LEAF_APPEND, test_<name>);` at the bottom of that
//      same file.
// That is the whole procedure.  There is no number to choose and no second
// file to touch, so two recoveries in two different families share no file at
// all and cannot conflict.  The per-family case counts are derived at build
// time by tools/generate_leaf_manifest.py and still checked at run time, so a
// case that stops registering still makes the binary refuse to run rather than
// quietly losing coverage.  Only a brand-new family file needs a CMake edit,
// and configure fails loudly if you forget it.
//
// WHY CASES CARRY A NUMBER
//   The order is load-bearing, not cosmetic.  Reversing the pre-split main()
//   call list faults under Wine: test_wave_device_construction reads through
//   g_wave_dtor_obj, which only test_wave_destructor (position 200 of 229)
//   ever assigns, so running the former first dereferences a null pointer.
//   Static-initialisation order across translation units is unspecified, so a
//   registry that ran cases in registration order would be at the mercy of the
//   link line.  Each case therefore carries its position in the original
//   main() call list and the harness sorts by it.  The order is a property of
//   the sources.
//
//   There is a second, independent hazard at the same seam.  The pre-split
//   main() carried this note, and it still applies:
//
//     test_button_group_add() is deliberately called last, not adjacent to
//     test_button_group_lifecycle() where it logically belongs. Placing any
//     extra call - even an empty one, even a second call to an
//     already-passing existing test - immediately before
//     test_base_pop_string_font() reproducibly crashes the whole suite
//     (confirmed by duplicating test_button_group_lifecycle() itself with no
//     other change). This is a pre-existing stack/Wine-environment fragility
//     around that call site, not a bug in add() or in this test; moving the
//     call site to the end of main() avoids it without masking it.
//
//   Both hazards are preserved rather than fixed: the split reproduces the
//   baseline order exactly, and neither is this refactor's to repair.
#ifndef OPENSMACX_TESTS_LEAF_LEAF_HARNESS_H
#define OPENSMACX_TESTS_LEAF_LEAF_HARNESS_H

%s

// ---------------------------------------------------------------------------
// Failure bookkeeping.
//
// `failures` is part of the test-facing surface: about ten cases snapshot it
// and compare afterwards to decide whether to print a diff, so it stays a
// plain global with the name the tests already use.
extern int failures;

void expect_at(bool condition, const char *file, int line);
void expect_tracked_free_calls(int expected);

#define expect(condition) expect_at((condition), __FILE__, __LINE__)

// ---------------------------------------------------------------------------
// Case registry.
//
// Storage is a plain POD array so it is zero-initialised before any dynamic
// initialiser runs; a std::vector here would depend on its own construction
// happening before the first registrar, which is not ordered.
namespace leaf {

typedef void (*CaseFn)();

struct Entry {
    int order;
    const char *name;
    CaseFn fn;
    const char *family;
};

enum { kMaxCases = 1024 };

extern Entry g_cases[kMaxCases];
extern int g_case_count;
extern int g_overflow;

struct Registrar {
    Registrar(int order, const char *name, CaseFn fn, const char *family);
};

}  // namespace leaf

// LEAF_FAMILY is defined at the top of each family file.
#define LEAF_CASE(order, fn)                                                  \\
    const ::leaf::Registrar leaf_registrar_##fn((order), #fn, &(fn), LEAF_FAMILY)

// Baseline positions run 0..228.  A case added after the split registers at
// LEAF_APPEND - bare, no offset - so it sorts after every original case.
// Ties at LEAF_APPEND break by name, which is why no offset is needed and why
// none should be invented: a number picked here would have to be unique across
// every family file at once, which is precisely the global coordination this
// split exists to remove.  The harness allows duplicate positions at or above
// LEAF_APPEND and forbids them below it.
#define LEAF_APPEND 100000

#endif  // OPENSMACX_TESTS_LEAF_LEAF_HARNESS_H
'''

SHARED_H_HEAD = '''// Declarations for the fixtures that more than one family file touches.
//
// GENERATED by tools/emit_leaf_split.py - the definitions still live in the
// family file that owned them before the split; only the declaration is here.
// Entries appear in original-file order, so a fixture is always declared
// after everything it names.
#ifndef OPENSMACX_TESTS_LEAF_LEAF_SHARED_H
#define OPENSMACX_TESTS_LEAF_LEAF_SHARED_H

#include "leaf_harness.h"

// Several of these fixtures are __thiscall free functions, which GCC warns
// about on every translation unit that reads this header.  The original file
// silenced the same warning around the same declarations.
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif'''

SHARED_H_TAIL = '''#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif  // OPENSMACX_TESTS_LEAF_LEAF_SHARED_H'''

STUBS_CPP = '''// Stand-ins for src symbols the leaf suite deliberately does not link.
//
// Copied verbatim out of tests/recovery_leaf_tests.cpp; these were always at
// global scope, so they keep the external linkage they had.  Every family
// file sees them through tests/leaf/leaf_shared.h.
#include "leaf_shared.h"

%s
'''

FAMILY_CPP = '''// Recovery leaf tests: %s.
//
// Carved verbatim out of tests/recovery_leaf_tests.cpp.  Add a case by
// appending a `void test_*()` here plus `LEAF_CASE(LEAF_APPEND, test_*);` at
// the bottom.  This file is the only file that edit touches: the per-family
// case counts are derived from these `LEAF_CASE(` lines at build time, and
// LEAF_APPEND needs no offset because ties there break by name.
#include "leaf_shared.h"

#define LEAF_FAMILY "%s"

%s
'''

if __name__ == '__main__':
    main()
