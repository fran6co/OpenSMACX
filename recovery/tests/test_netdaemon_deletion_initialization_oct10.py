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
    from test_netdaemon_shared_deletion_storage_oct10 import restore_shared_storage_migration
    source = restore_shared_storage_migration(source)
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
    from test_netdaemon_shared_deletion_storage_oct10 import restore_shared_header_migration
    assert restore_shared_header_migration((ROOT / "src/netdaemon.h").read_text()) == old


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
    from test_netdaemon_shared_deletion_storage_oct10 import (
        restore_shared_storage_migration, restore_shared_header_migration,
    )
    source = restore_shared_storage_migration((ROOT / "src/netdaemon.cpp").read_text())
    assert source.count(AFTER) == 1
    header = restore_shared_header_migration((ROOT / "src/netdaemon.h").read_text())
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


# The host checks actual named-field effects and declaration traits. Native
# VC6/COFF evidence separately qualifies the x86 offsets; host pointers can be8B.
def _jc_actual_lock_declarations_and_state_tail():
    from test_netdaemon_transfer_modal import extract
    headers = "\n".join(
        extract((ROOT / ("src/" + name.lower() + ".h")).read_text(),
                "class " + name + " {") + ";"
        for name in ("SquareLock", "PlayerLock", "Lock")
    )
    owner_header = (ROOT / "src/netdaemon.h").read_text()
    states = "\n".join(extract(owner_header, "struct " + name + " {") + ";"
                       for name in ("PrimaryLockStateTail", "LockStateEntry"))
    ctor = extract((ROOT / "src/netdaemon.cpp").read_text(),
                   "NetDaemon::NetDaemon()")
    start = ctor.index("    primary_state_.leading_words_[0] = 0;")
    end = ctor.rindex("    locks_.clear();")
    return headers, states, ctor[start:end]


@pytest.mark.parametrize("damage", ["none", "old_mask_start", "omit_last_word",
                                    "clobber_unwritten", "wrong_channel_word",
                                    "explicit_empty_lifetime"])
def test_actual_constructor_state_domain_and_trivial_lock_lifetimes(tmp_path, damage):
    headers, states, tail = _jc_actual_lock_declarations_and_state_tail()
    edits = {
        "old_mask_start": ("trailing_words_[i + 1]", "trailing_words_[i]"),
        "omit_last_word": ("    channels_[1].trailing_words_[8] = 0;", ""),
        "clobber_unwritten": ("    primary_state_.leading_words_[0] = 0;",
                               "    channels_[0].leading_words_[3] = 0;\n    primary_state_.leading_words_[0] = 0;"),
        "wrong_channel_word": ("channels_[1].trailing_words_[5] = 0;",
                               "channels_[1].trailing_words_[3] = 0;"),
    }
    if damage in edits:
        old, new = edits[damage]
        assert tail.count(old) == 1
        tail = tail.replace(old, new, 1)
    elif damage == "explicit_empty_lifetime":
        old = "class SquareLock {\n public:"
        assert headers.count(old) == 1
        headers = headers.replace(old, old + "\n SquareLock() {} ~SquareLock() {}", 1)
    program = r'''
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <cstdio>
#define CHECK(x) do { if (!(x)) { std::fprintf(stderr,"constructor: %s\n",#x); return 7; } } while(0)
HEADERS
STATES
struct StateFixture {
    PrimaryLockStateTail primary_state_;
    LockStateEntry channels_[2];
    void initialize() { TAIL }
};
int main() {
    CHECK(std::is_trivially_default_constructible<SquareLock>::value);
    CHECK(std::is_trivially_destructible<SquareLock>::value);
    CHECK(std::is_trivially_default_constructible<PlayerLock>::value);
    CHECK(std::is_trivially_destructible<PlayerLock>::value);
    CHECK(std::is_trivially_default_constructible<Lock>::value);
    CHECK(std::is_trivially_destructible<Lock>::value);
    CHECK(sizeof(SquareLock)==12 && sizeof(PlayerLock)==28 && sizeof(Lock)==236);
    StateFixture f;
    std::memset(&f,0xA5,sizeof(f));
    f.initialize();
    const uint32_t poison=0xA5A5A5A5u;
    for(int i=0;i<3;++i) CHECK(f.primary_state_.leading_words_[i]==0);
    CHECK(f.primary_state_.flags_==0 && f.primary_state_.session_phase_==0);
    for(int i=0;i<8;++i) CHECK(f.primary_state_.trailing_words_[i]==0);
    CHECK(f.primary_state_.trailing_words_[8]==poison);
    CHECK(f.channels_[0].leading_words_[0]==0 && f.channels_[0].leading_words_[1]==0);
    CHECK(f.channels_[0].leading_words_[2]==poison && f.channels_[0].leading_words_[3]==poison);
    CHECK(f.channels_[0].payload_.vehicle_selection_follow==0);
    for(int i=0;i<10;++i) CHECK(f.channels_[0].trailing_words_[i]==0);
    for(int i=0;i<4;++i) CHECK(f.channels_[1].leading_words_[i]==0);
    CHECK(f.channels_[1].payload_.flags==0);
    CHECK(f.channels_[1].trailing_words_[0]==poison && f.channels_[1].trailing_words_[1]==poison);
    for(int i=2;i<6;++i) CHECK(f.channels_[1].trailing_words_[i]==0);
    CHECK(f.channels_[1].trailing_words_[6]==255);
    CHECK(f.channels_[1].trailing_words_[7]==0 && f.channels_[1].trailing_words_[8]==0);
    CHECK(f.channels_[1].trailing_words_[9]==poison);
}
'''.replace("HEADERS",headers).replace("STATES",states).replace("TAIL",tail)
    compiler = shutil.which("g++") or shutil.which("clang++")
    assert compiler, "host C++ compiler is required for the constructor control"
    cpp, exe = tmp_path / "constructor.cpp", tmp_path / "constructor"
    cpp.write_text(program)
    build = subprocess.run([compiler,"-std=c++11","-O2","-DNDEBUG",str(cpp),"-o",str(exe)],
                           capture_output=True,text=True,timeout=60)
    assert build.returncode==0, build.stdout+build.stderr
    run = subprocess.run([str(exe)],capture_output=True,text=True,timeout=10)
    assert run.returncode==(0 if damage=="none" else 7), run.stdout+run.stderr
    if damage!="none": assert "constructor:" in run.stderr
