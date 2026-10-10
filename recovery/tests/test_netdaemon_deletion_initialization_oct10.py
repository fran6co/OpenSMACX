"""Native deletion-header initialization domain, not complete owner acceptance."""
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
BASE = "dbef453ae8912bb57bf67982b1e9fd8932fb9182"
BEFORE = """    for (int32_t m = 0; m < 0x18; ++m) {
        lock_table_[m].flag_ = 0xFF;
        lock_table_[m].word_8_ = 0;
        // the per-entry scratch dword lives past the entry, at +0x5A0
        lock_state_[m].leading_words_[0] = 0;
    }
"""
AFTER = """    // Native 0x538A53 clears the shared deletion owner once; the backedge
    // at 0x538A64 targets 0x538A59, after this store.
    lock_state_[0].leading_words_[0] = 0;
    for (int32_t m = 0; m < 0x18; ++m) {
        lock_table_[m].flag_ = 0xFF;
        lock_table_[m].word_8_ = 0;
    }
"""
HEADER_EDITS = (
    ("""// The constructor also clears leading word0 in entries4..23, partially
// initializing that overlapping view; do not add whole-snapshot initialization.
""", """// The constructor's 0x538A53 owner clear is outside the record loop:
// 0x538A64 branches to 0x538A59. It does not initialize entries3..23.
// Only channels0..2 have evidenced runtime state accesses; the broader array
// and overlapping save view below remain structural recovery obligations.
"""),
    ("""  // bytes later), which is why the ctor's merged loop writes [entry+0x5A0]
  // alongside [entry] and [entry+8] with one pointer. Entries e0..e2
  // carry the named dwords; e3 aliases the file-index pointer and save
  // view, while e4..e23 retain the constructor's partial alias clears.
""", """  // bytes later). The ctor clears its first word ONCE before the record
  // header loop; it does not sweep state/pointer/save words at this stride.
  // Entries e0..e2 carry the named dwords; the broader array aliases the
  // file-index pointer and save view but is not a proven original array.
"""),
)


def restore_deletion_initialization_migration(source):
    assert source.count(AFTER) == 1
    return source.replace(AFTER, BEFORE, 1)


def baseline(relative):
    return subprocess.check_output(["git", "show", BASE + ":" + relative], cwd=ROOT).decode()


def test_only_owner_clear_domain_and_contradicted_comments_change():
    source = (ROOT / "src/netdaemon.cpp").read_text()
    assert restore_deletion_initialization_migration(source) == baseline("src/netdaemon.cpp")
    old = baseline("src/netdaemon.h")
    for before, after in HEADER_EDITS:
        assert old.count(before) == 1
        old = old.replace(before, after, 1)
    assert (ROOT / "src/netdaemon.h").read_text() == old


def test_pinned_backedge_excludes_owner_store_and_keeps_record_widths():
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    from match_recovery.source_inventory import image_pin

    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.exists():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    expected = bytes.fromhex("8d868c150000b91800000089b8a0050000c600ff6689780883c03c4975f3")
    assert image.read(Span(0x538A48, 0x538A66)) == expected
    displacement = expected[-1] - 256
    loop_entry = 0x538A66 + displacement
    assert loop_entry == 0x538A59
    assert 0x538A53 < loop_entry
    assert 0x93CD90 + 0x158C == 0x93E31C
    assert 24 * 0x3C == 0x5A0


def test_actual_initialization_fragment_preserves_other_storage(tmp_path):
    """Compile the real fragment; poisoned bytes expose the erroneous sweep.

    This fixed-width host storage models only the observed fragment, not the
    application's inheritance/lifecycle, VC6 ABI, or full constructor effects.
    """
    source = (ROOT / "src/netdaemon.cpp").read_text()
    assert source.count(AFTER) == 1
    header = (ROOT / "src/netdaemon.h").read_text()
    record = header.split("struct LockTableEntry {", 1)[1].split("};", 1)[0]
    harness = """#include <stdint.h>
#include <string.h>
struct LockTableEntry { @@RECORD@@ };
struct State { uint32_t leading_words_[4]; uint32_t payload; uint32_t trailing[10]; };
struct Fixture {
    LockTableEntry lock_table_[24];
    State lock_state_[24];
    void initialize() { @@BODY@@ }
};
int main() {
    if (sizeof(LockTableEntry) != 0x3C || sizeof(State) != 0x3C) return 2;
    Fixture actual, expected;
    memset(&actual, 0xA5, sizeof actual);
    memcpy(&expected, &actual, sizeof actual);
    expected.lock_state_[0].leading_words_[0] = 0;
    for (int m = 0; m < 24; ++m) {
        expected.lock_table_[m].flag_ = 0xFF;
        expected.lock_table_[m].word_8_ = 0;
    }
    actual.initialize();
    return memcmp(&actual, &expected, sizeof actual) == 0 ? 0 : 1;
}
""".replace("@@RECORD@@", record)
    compilers = [x for name in ("g++", "clang++") if (x := shutil.which(name))]
    if not compilers:
        pytest.skip("no host C++ compiler available")
    for compiler in compilers:
        for name, body in (("real", AFTER), ("erroneous-sweep", BEFORE)):
            cpp = tmp_path / (Path(compiler).name + "-" + name + ".cpp")
            binary = cpp.with_suffix("")
            cpp.write_text(harness.replace("@@BODY@@", body))
            subprocess.run([compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror",
                            str(cpp), "-o", str(binary)], check=True, timeout=30)
            result = subprocess.run([str(binary)], timeout=30)
            assert result.returncode == (0 if name == "real" else 1)
