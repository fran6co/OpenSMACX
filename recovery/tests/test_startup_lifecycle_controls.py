"""Actual-source host controls for static lifetimes.

The C++ probes extract the real Path/constructor/destructor method bodies and
owner declaration extents from tracked source. Probe-only fields/events are
not native layout claims. They instrument language-level initialization and
reverse destruction without asserting VC6 sizes, offsets, or vtable values.
"""

from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path

import pytest
from match_recovery.source import read_file
from match_recovery.object_source import read_objects
from match_recovery.source_inventory import image_pin


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/path.h").is_file() and (parent / "src/time.cpp").is_file())
HOST_COMPILERS = [path for path in (shutil.which("g++"), shutil.which("clang++")) if path]


@pytest.fixture(params=HOST_COMPILERS or [None], ids=[Path(p).name for p in HOST_COMPILERS] or ["no-host-compiler"])
def host_compiler(request):
    if request.param is None:
        pytest.skip("no GCC or Clang host compiler available")
    return request.param


def _mapped_record(source: str, address: int, name: str, span: tuple[int, int], symbol: str, byte_exact: bool):
    path = ROOT / source
    record, = (row for row in read_file(path) if row.address == address)
    assert record.name == name
    assert record.symbol == symbol
    assert record.image_spans == (span,)
    assert record.byte_exact is byte_exact
    return record


def _balanced_block(text: str, marker: str, opening: str = "{") -> str:
    start = text.index(marker)
    brace = text.index(opening, start)
    depth = 0
    for pos in range(brace, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[start : pos + 1]
    raise AssertionError(f"unclosed source block: {marker}")


def _compile_and_run(tmp_path: Path, source: str, compiler: str | None) -> subprocess.CompletedProcess[str]:
    if compiler is None:
        pytest.skip("no host C++ compiler available")
    tmp_path.mkdir(parents=True, exist_ok=True)
    cpp = tmp_path / "lifecycle_probe.cpp"
    exe = tmp_path / "lifecycle_probe"
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        text=True,
        capture_output=True,
        check=False,
        timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], text=True, capture_output=True, check=False, timeout=30)


def _path_projection(*, restore_explicit_ctor: bool = False, mutate_free_order: bool = False) -> str:
    header = (ROOT / "src/path.h").read_text(encoding="utf-8")
    block = _balanced_block(header, "class Path {")
    assert not re.search(r"^\s*Path\s*\(\s*\)", block, re.MULTILINE), "authoritative Path owner must use implicit default construction"
    # Host C++98 has no nullptr token; replacing only the null pointer literal
    # preserves this actual source method's behavior in the legacy-compatible probe.
    block = block.replace("nullptr", "0")
    block = block.replace("class Path {", "class Path {\n  friend struct PathTestAccess;", 1)
    if restore_explicit_ctor:
        # Deliberate regression mutant for the original unsupported startup
        # effect; static zero-initialized members otherwise remain untouched.
        block = block.replace(" public:\n", " public:\n  Path() { unk_ = 5; }\n", 1)
    if mutate_free_order:
        block = block.replace("free(map_table_);", "free(x_table_);", 1)
    return block


def _path_harness(block: str) -> str:
    return r"""
#include <stdint.h>
#include <stddef.h>
#define MEASURED
#define BOOL int
static void* freed[8];
static int free_count;
static void tracked_free(void* value) { freed[free_count++] = value; }
#define free tracked_free
""" + block + ";\n" + r"""
#undef free
struct PathTestAccess {
  static bool zero(const Path& p) {
    return p.map_table_ == 0 && p.x_table_ == 0 && p.y_table_ == 0 &&
      p.index1_ == 0 && p.index2_ == 0 && p.faction_id_ == 0 &&
      p.x_dst_ == 0 && p.y_dst_ == 0 && p.unk_ == 0 &&
      p.faction_id_2_ == 0 && p.proto_id_ == 0;
  }
  static void set_tables(Path& p, int* map, int16_t* x, int16_t* y) {
    p.map_table_ = map; p.x_table_ = x; p.y_table_ = y;
  }
};
Path Paths;
int main() {
  static int map_token;
  static int16_t x_token, y_token;
  if (!PathTestAccess::zero(Paths)) return 11;
  PathTestAccess::set_tables(Paths, &map_token, &x_token, &y_token);
  Paths.shutdown();
  if (free_count != 3 || freed[0] != &map_token ||
      freed[1] != &x_token || freed[2] != &y_token) return 12;
  if (!PathTestAccess::zero(Paths)) return 13;
  Paths.shutdown();
  if (free_count != 3) return 14;
  return 0;
}
"""


def test_path_zero_bss_shutdown_order_and_mutation_controls(tmp_path: Path, host_compiler: str) -> None:
    path_cpp = (ROOT / "src/path.cpp").read_text(encoding="utf-8")
    assert len(re.findall(r"^Path\s+Paths\s*;", path_cpp, re.MULTILINE)) == 1
    _mapped_record("src/path.cpp", 0x0059A1B0, "??__EPaths@@YAXXZ", (0x0059A1B0, 0x0059A1BC), "_$E20", True)
    _mapped_record("src/path.cpp", 0x0059A1C0, "??__FPaths@@YAXXZ", (0x0059A1C0, 0x0059A215), "_$E19", True)
    assert "free(map_table_)" in (ROOT / "src/path.h").read_text(encoding="utf-8")
    assert "free(x_table_)" in (ROOT / "src/path.h").read_text(encoding="utf-8")
    assert "free(y_table_)" in (ROOT / "src/path.h").read_text(encoding="utf-8")

    baseline = _compile_and_run(tmp_path / "path-good", _path_harness(_path_projection()), host_compiler)
    assert baseline.returncode == 0, baseline.stdout + baseline.stderr

    ctor_mutant = _compile_and_run(
        tmp_path / "path-ctor-mutant",
        _path_harness(_path_projection(restore_explicit_ctor=True)), host_compiler,
    )
    assert ctor_mutant.returncode != 0, "restoring unsupported unk_=5 ctor must fail zero-BSS control"

    free_mutant = _compile_and_run(
        tmp_path / "path-free-mutant",
        _path_harness(_path_projection(mutate_free_order=True)), host_compiler,
    )
    assert free_mutant.returncode != 0, "mutating native map/x/y release order must fail control"


def _transcript_fixture(rows: int, cols: int, ctor_body: str, dtor_body: str, omit_head_init: bool = False) -> str:
    if omit_head_init:
        before, separator, after = ctor_body.partition("head_ = 0;")
        assert separator
        ctor_body = before + "/* mutation omitted head initialization */" + after
    return f"""
#include <cstddef>
static int constructed[128], construct_count;
static int removed[128], remove_count;
static int base_destroyed[128], base_destroy_count;
static int poison_storage;
struct StringStruct {{
  void* head_; void* current_; int entry_count_; int current_position_; void* allocator_;
  int test_id_;
  StringStruct() {{
    head_ = &poison_storage; current_ = &poison_storage;
    entry_count_ = -17; current_position_ = -19; allocator_ = &poison_storage;
    {ctor_body[1:-1]}
  }}
  virtual ~StringStruct() {{ base_destroyed[base_destroy_count++] = test_id_; }}
}};
struct StringList : StringStruct {{
  StringList() {{ test_id_ = construct_count; constructed[construct_count++] = test_id_; }}
  ~StringList() {dtor_body}
  void remove_all() {{ removed[remove_count++] = test_id_; }}
}};
int main() {{
  if ({rows} != 8 || {cols} != 8 || {rows} * {cols} != 64) return 21;
  {{
    StringList probe[{rows}][{cols}];
    if (construct_count != 64) return 22;
    for (int i = 0; i < 64; ++i) {{
      if (constructed[i] != i) return 23;
      if (probe[i / {cols}][i % {cols}].head_ != 0 ||
          probe[i / {cols}][i % {cols}].current_ != 0 ||
          probe[i / {cols}][i % {cols}].entry_count_ != 0 ||
          probe[i / {cols}][i % {cols}].current_position_ != 0 ||
          probe[i / {cols}][i % {cols}].allocator_ != 0) return 24;
    }}
  }}
  if (remove_count != 64 || base_destroy_count != 64) return 25;
  for (int i = 0; i < 64; ++i) {{
    const int reverse = 63 - i;
    if (removed[i] != reverse || base_destroyed[i] != reverse) return 26;
  }}
  return 0;
}}
"""


def _source_body(source: str, signature: str) -> str:
    block = _balanced_block(source, signature)
    return block[block.index("{") :]


def test_transcript_array_ctor_dtor_vector_order_and_mutation_controls(tmp_path: Path, host_compiler: str) -> None:
    header = (ROOT / "src/stringstruct.h").read_text(encoding="utf-8")
    impl = (ROOT / "src/stringstruct.cpp").read_text(encoding="utf-8")
    list_impl = (ROOT / "src/stringlist.cpp").read_text(encoding="utf-8")
    owner = (ROOT / "src/diplowin.cpp").read_text(encoding="utf-8")
    declaration = re.search(
        r"\bStringList\s+DiplomacyTranscriptLists\s*\[\s*(\d+)\s*\]\s*\[\s*(\d+)\s*\]\s*;",
        owner,
    )
    assert declaration, "real typed transcript array declaration must remain source-owned"
    rows, cols = map(int, declaration.groups())
    assert (rows, cols) == (8, 8), "native helper evidence is for exactly 64 elements"
    _mapped_record("src/diplowin.cpp", 0x00440F30, "sub_440f30", (0x00440F30, 0x00440F54), "_$E26", True)
    _mapped_record("src/diplowin.cpp", 0x00440F60, "sub_440f60", (0x00440F60, 0x00440F74), "_$E24", True)
    _mapped_record("src/diplowin.cpp", 0x004450B0, "sub_4450b0", (0x004450B0, 0x0044511C), "??0StringList@@QAE@XZ", False)
    objects = [row for row in read_objects(ROOT, image_pin(ROOT)) if row.origin.address == 0x00737CD8]
    assert len(objects) == 1
    assert objects[0].symbol == "?DiplomacyTranscriptLists@@3PAY07VStringList@@A"
    assert objects[0].unit == "src/diplowin.cpp"
    assert "StringList : public StringStruct" in header
    string_list = _balanced_block(header, "class StringList : public StringStruct {")
    assert not re.search(r"^\s*StringList\s*\(\s*\)\s*;", string_list, re.MULTILINE), "source must keep implicit derived construction"
    assert "~StringList();" in header
    assert "StringList::StringList(" not in list_impl + impl
    assert "remove_all();" in _source_body(list_impl, "StringList::~StringList()")
    assert "current_position_ = 0;" in _source_body(list_impl, "StringList::~StringList()")

    ctor = _source_body(impl, "StringStruct::StringStruct()")
    dtor = _source_body(list_impl, "StringList::~StringList()")
    baseline_source = _transcript_fixture(rows, cols, ctor, dtor)
    baseline = _compile_and_run(tmp_path / "transcript-good", baseline_source, host_compiler)
    assert baseline.returncode == 0, baseline.stdout + baseline.stderr

    init_mutant = _compile_and_run(
        tmp_path / "transcript-init-mutant",
        _transcript_fixture(rows, cols, ctor, dtor, omit_head_init=True), host_compiler,
    )
    assert init_mutant.returncode != 0, "omitting real StringStruct field initialization must fail"

    missing_cleanup = dtor.replace("remove_all();", "/* omitted list cleanup */", 1)
    assert missing_cleanup != dtor
    cleanup_mutant = _compile_and_run(
        tmp_path / "transcript-cleanup-mutant",
        _transcript_fixture(rows, cols, ctor, missing_cleanup), host_compiler,
    )
    assert cleanup_mutant.returncode != 0, "omitting real StringList cleanup must fail"

    resized_mutant = _compile_and_run(
        tmp_path / "transcript-size-mutant",
        _transcript_fixture(rows, cols - 1, ctor, dtor), host_compiler,
    )
    assert resized_mutant.returncode != 0, "changing source array extent from 64 elements must fail"


def test_grouped_timer_declaration_compiler_order_and_object_marker_mutation(tmp_path: Path, host_compiler: str) -> None:
    time_cpp = (ROOT / "src/console.cpp").read_text(encoding="utf-8")
    _mapped_record("src/console.cpp", 0x0050E9B0, "sub_50e9b0", (0x0050E9B0, 0x0050E9EE), "_$E21", True)
    _mapped_record("src/console.cpp", 0x0050E9F0, "sub_50e9f0", (0x0050E9F0, 0x0050EA22), "_$E19", True)
    expected_names = ["BlinkTimer", "Blink2Timer", "LineTimer", "GoTimer", "TurnTimer"]
    assert "Time ConsoleTimer;" in time_cpp
    grouped = re.search(r"Time\s+BlinkTimer,(.*?)TurnTimer\s*;", time_cpp, re.DOTALL)
    assert grouped
    declarators = re.sub(r"//[^\n]*", "", grouped.group(0))
    found = re.findall(r"\b(BlinkTimer|Blink2Timer|LineTimer|GoTimer|TurnTimer)\b", declarators)
    assert found == expected_names
    timer_objects = [row for row in read_objects(ROOT, image_pin(ROOT)) if row.unit == "src/console.cpp"]
    actual_object_bindings = {row.symbol: row.origin.address for row in timer_objects
                              if row.symbol in ("?LineTimer@@3VTime@@A", "?BlinkTimer@@3VTime@@A",
                                                "?Blink2Timer@@3VTime@@A")}
    assert actual_object_bindings == {
        "?BlinkTimer@@3VTime@@A": 0x00915688,
        "?Blink2Timer@@3VTime@@A": 0x00939EB0,
        "?LineTimer@@3VTime@@A": 0x00915658,
    }
    console_timer, = [row for row in timer_objects if row.symbol == "?ConsoleTimer@@3VTime@@A"]
    assert console_timer.origin.address == 0x00939E88
    assert console_timer.span is None and console_timer.evidence is None
    console_cpp = (ROOT / "src/console.cpp").read_text(encoding="utf-8")
    assert "Time g_CONSOLE_TIMER;" not in console_cpp
    assert console_cpp.index("Console ConsoleGlobal;") < console_cpp.index("Time ConsoleTimer;") < console_cpp.index("Time BlinkTimer,")

    def probe(names: list[str]) -> str:
        declaration = "Probe " + ", ".join(names) + ";"
        return """
#include <cstddef>
struct Probe;
static Probe* built[8]; static int built_n;
static Probe* destroyed[8]; static int destroyed_n;
struct Probe { Probe() { built[built_n++] = this; } ~Probe() { destroyed[destroyed_n++] = this; } };
int main() {
  Probe* expected[5];
  {
""" + "    " + declaration + "\n" + """
    expected[0] = &BlinkTimer; expected[1] = &Blink2Timer; expected[2] = &LineTimer;
    expected[3] = &GoTimer; expected[4] = &TurnTimer;
    if (built_n != 5) return 31;
    for (int i = 0; i < 5; ++i) if (built[i] != expected[i]) return 32;
  }
  if (destroyed_n != 5) return 33;
  for (int i = 0; i < 5; ++i) if (destroyed[i] != expected[4 - i]) return 34;
  return 0;
}
"""

    baseline = _compile_and_run(tmp_path / "timer-good", probe(expected_names), host_compiler)
    assert baseline.returncode == 0, baseline.stdout + baseline.stderr
    reordered = ["Blink2Timer", "BlinkTimer", "LineTimer", "GoTimer", "TurnTimer"]
    mutant = _compile_and_run(tmp_path / "timer-order-mutant", probe(reordered), host_compiler)
    assert mutant.returncode != 0, "swapping first two grouped timer declarations must fail order control"
