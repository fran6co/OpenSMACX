from pathlib import Path
import hashlib
import shutil
import subprocess

import pytest

from match_recovery.inspection import inspect_source


ROOT = Path(__file__).resolve().parents[2]
ORIGINAL_IMAGE_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def _pinned_image() -> Path:
    for candidate_root in (ROOT, *ROOT.parents):
        candidate = candidate_root / ".opensmacx/game/terranx_original.exe"
        if candidate.is_file():
            return candidate
    raise AssertionError("pinned original image is unavailable")


def _definition(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for position in range(opening, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[start : position + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def _harness(source: str) -> str:
    helper = _definition(source, "void erase_string_list_current(")
    method = _definition(source, "void StringBox::UNK2(")
    return r"""
#include <stdint.h>
struct StringStructPayload {};
struct StringStructEntry {
    StringStructEntry *previous, *next;
    int id;
    void *payload;
    StringStructPayload *string_payload() {
        return static_cast<StringStructPayload *>(payload);
    }
};
struct StringList {
    StringStructEntry *head_, *current_;
    int entry_count_, current_position_;
    void on_remove(StringStructPayload *) {}
};
struct StringBox {
    StringList stringList_;
    int fixups;
    StringBox() : fixups(0) {}
    void add_fixup() { ++fixups; }
    void UNK2(int first_bound, int second_bound);
};
namespace {
""" + helper + r"""
}
""" + method + r"""

int main() {
    const int ids[] = {-1, -2, 0, 3, 4, 2};
    StringStructEntry *nodes[6];
    for (int i = 0; i != 6; ++i) {
        nodes[i] = new StringStructEntry;
        nodes[i]->previous = 0;
        nodes[i]->next = 0;
        nodes[i]->id = ids[i];
        nodes[i]->payload = 0;
    }
    for (int i = 0; i != 6; ++i) {
        nodes[i]->previous = nodes[(i + 5) % 6];
        nodes[i]->next = nodes[(i + 1) % 6];
    }
    StringBox box;
    box.stringList_.head_ = nodes[0];
    box.stringList_.current_ = nodes[0];
    box.stringList_.entry_count_ = 6;
    box.stringList_.current_position_ = 0;
    box.UNK2(-1, 3);

    // The pinned CMP/JAE and CMP/JBE sequence is unsigned: with first_bound
    // equal to UINT32_MAX, -2 remains inside the interval but -1 does not.
    // The original changing-count traversal leaves the final out-of-range 2
    // unvisited; caching the initial count would visit and remove that node.
    StringStructEntry *entry = box.stringList_.head_;
    const int expected[] = {-2, 4, 2};
    if (box.stringList_.entry_count_ != 3 || box.stringList_.current_position_ != 2
        || box.fixups != 1) return 1;
    for (int i = 0; i != 3; ++i) {
        if (entry == 0 || entry->id != expected[i]) return 2 + i;
        entry = entry->next;
    }
    return entry == box.stringList_.head_ ? 0 : 6;
}
"""


def _compile_and_run(tmp_path: Path, compiler: str, source: str, name: str) -> int:
    cpp = tmp_path / f"{name}.cpp"
    executable = tmp_path / name
    cpp.write_text(source)
    compiled = subprocess.run(
        [compiler, "-std=c++98", str(cpp), "-o", str(executable)],
        capture_output=True,
        text=True,
        timeout=20,
    )
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=5).returncode


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_stringbox_unk2_keeps_pinned_unsigned_interval_and_loop(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"host C++ compiler unavailable: {compiler_name}")
    source = (ROOT / "src/stringbox.cpp").read_text()
    harness = _harness(source)
    assert _compile_and_run(tmp_path, compiler, harness, "faithful") == 0

    signed_method = _definition(source, "void StringBox::UNK2(")
    signed_method = signed_method.replace(
        "const uint32_t id = static_cast<uint32_t>(list.current_->id);",
        "const int id = list.current_->id;",
    ).replace("static_cast<uint32_t>(first_bound)", "first_bound").replace(
        "static_cast<uint32_t>(second_bound)", "second_bound"
    )
    assert "const int id = list.current_->id;" in signed_method
    assert "static_cast<uint32_t>(first_bound)" not in signed_method
    assert "static_cast<uint32_t>(second_bound)" not in signed_method
    signed_source = source.replace(_definition(source, "void StringBox::UNK2("), signed_method)
    assert _compile_and_run(tmp_path, compiler, _harness(signed_source), "signed_mutant") != 0

    changing_count_method = _definition(source, "void StringBox::UNK2(")
    cached_count_method = changing_count_method.replace(
        "int visited = 0;",
        "int visited = 0;\n        const int original_count = list.entry_count_;",
    ).replace("while (visited < list.entry_count_);", "while (visited < original_count);")
    assert cached_count_method != changing_count_method
    assert "const int original_count = list.entry_count_;" in cached_count_method
    assert "while (visited < original_count);" in cached_count_method
    assert "while (visited < list.entry_count_);" not in cached_count_method
    cached_count_source = source.replace(changing_count_method, cached_count_method)
    assert _compile_and_run(tmp_path, compiler, _harness(cached_count_source), "cached_count_mutant") != 0


def test_stringbox_unk2_predicate_matches_pinned_unsigned_branches():
    image = _pinned_image()
    assert hashlib.sha256(image.read_bytes()).hexdigest() == ORIGINAL_IMAGE_SHA256
    report = inspect_source(ROOT, ["0x00629980"], image=image)
    assert report["result"] == "pass" and report["image_verified"]
    subject, = report["subjects"]
    assert subject["source"]["path"] == "src/stringbox.cpp"
    span, = subject["original"]["spans"]
    assert span["result"] == "pass"
    assert span["span"] == [0x00629980, 0x00629A63]
    instructions = {line.split()[0]: line for line in span["disassembly"]}
    assert "cmp ecx, ebp" in instructions["0x006299BC"]
    assert "jae 0x6299ce" in instructions["0x006299BE"]
    assert "cmp ecx, dword ptr [esp + 0x20]" in instructions["0x006299C0"]
    assert "jbe 0x6299ce" in instructions["0x006299C4"]
    assert "mov eax, dword ptr [eax + 0xc]" in instructions["0x006299C6"]
