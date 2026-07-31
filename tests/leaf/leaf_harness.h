// Shared preamble and test harness for the split recovery leaf suite.
//
// This file, tests/leaf/leaf_shared.h and tests/leaf/leaf_harness.cpp were
// carved out of the single 30,950-line tests/recovery_leaf_tests.cpp.  Test
// bodies live in tests/leaf/<family>_tests.cpp and register themselves; the
// harness never has to be edited to add one.
//
// ADDING A TEST
//   1. Append `void test_<name>()` to the family file it belongs to.
//   2. Append `LEAF_CASE(LEAF_APPEND + n, test_<name>);` at the bottom of that
//      same file.
//   3. Bump that family's number in tests/leaf/leaf_case_manifest.h.
// Step 3 is the only edit outside the family file, and it is checked at run
// time: get it wrong and the binary refuses to run rather than quietly losing
// or double-counting a case.  Nothing else - not main(), not the CMake list
// unless the family itself is new - ever has to change.
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

#include "../../src/stdafx.h"
#include <limits>
#include "../../src/alphanet.h"
#include "../../src/autosound.h"
#include "../../src/basepop.h"
#include "../../src/field_accessors.h"
#include "../../src/leaf_recoveries.h"
#include "../../src/basebutton.h"
#include "../../src/buttongroup.h"
#include "../../src/console.h"
#include "../../src/planwin.h"
#include "../../src/dialog.h"
#include "../../src/ambience.h"
#include "../../src/filemap.h"
#include "../../src/flatbutton.h"
#include "../../src/buffer.h"
#include "../../src/flic.h"
#include "../../src/font.h"
#include "../../src/graphicwin.h"
#include "../../src/font.h"
#include "../../src/log.h"
#include "../../src/maininterface.h"
#include "../../src/map.h"
#include "../../src/mapwin.h"
#include "../../src/alphamovie.h"
#include "../../src/basewin.h"
#include "../../src/datalink.h"
#include "../../src/designwin.h"
#include "../../src/diplowin.h"
#include "../../src/netwin.h"
#include "../../src/socialwin.h"
#include "../../src/worldwin.h"
#include "../../src/checkbutton.h"
#include "../../src/cursor.h"
#include "../../src/infowin.h"
#include "../../src/statuswin.h"
#include "../../src/filewin.h"
#include "../../src/wave.h"
#include "../../src/atexit_thunks.h"
#include "../../src/init_thunks.h"
#include "../../src/adjustor_thunks.h"
#include "../../src/deleting_thunks.h"
#include "../../src/fx.h"
#include "../../src/battlewin.h"
#include "../../src/councwin.h"
#include "../../src/credits.h"
#include "../../src/diplopop.h"
#include "../../src/gamma.h"
#include "../../src/messagewin.h"
#include "../../src/newtechwin.h"
#include "../../src/pickwin.h"
#include "../../src/prodpicker.h"
#include "../../src/texture.h"
#include "../../src/reportwin.h"
#include "../../src/setupwin.h"
#include "../../src/multidebug.h"
#include "../../src/tutwin.h"
#include "../../src/popmenu.h"
#include "../../src/popup.h"
#include "../../src/netmsg.h"
#include "../../src/radiobutton.h"
#include "../../src/checkbox.h"
#include "../../src/editgroup.h"
#include "../../src/dialogs.h"
#include "../../src/xpops.h"
#include "../../src/playerlock.h"
#include "../../src/netdaemon.h"
#include "../../src/replaywin.h"
#include "../../src/reportif.h"
#include "../../src/dipedit.h"
#include "../../src/sound.h"
#include "../../src/wave_device.h"
#include "../../src/spritebox.h"
#include "../../src/listbox.h"
#include "../../src/net_class.h"
#include "../../src/squarelock.h"
#include "../../src/deletionlist.h"
#include "../../src/lock.h"
#include "../../src/map.h"
#include "../../src/game.h"
#include "../../src/menu.h"
#include "../../src/palette.h"
#include "../../src/pulldown.h"
#include "../../src/random.h"
#include "../../src/scroll.h"
#include "../../src/sounddevice.h"
#include "../../src/spot.h"
#include "../../src/stringstruct.h"
#include "../../src/strings.h"
#include "../../src/text_recovery.h"
#include "../../src/textindex.h"
#include "../../src/tutwin.h"
#include "../../src/temp.h"
#include "../../src/time.h"
#include "../../src/vector.h"
#include "../../src/alphamenu.h"
#include "../../src/global_arith.h"
#include "../../src/guarded_teardowns.h"
#include "../../src/nullsub_thunks.h"
#include "../../src/delegation_thunks.h"
#include "../../src/uv2player.h"
#include "../../src/win.h"

#include <array>
#include <climits>
#include <cstring>
#include <float.h>
#include <new>
#include <type_traits>
#include <vector>

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
#define LEAF_CASE(order, fn)                                                  \
    const ::leaf::Registrar leaf_registrar_##fn((order), #fn, &(fn), LEAF_FAMILY)

// Baseline positions run 0..228.  A case added after the split uses
// LEAF_APPEND + n so it sorts after every original case; ties break by name,
// so the order is a property of the sources and not of the link line.
#define LEAF_APPEND 100000

#endif  // OPENSMACX_TESTS_LEAF_LEAF_HARNESS_H
