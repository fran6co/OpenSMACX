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
    unk1 = _definition(source, "void StringBox::UNK1(")
    clip_ids = _definition(source, "void StringBox::clip_ids(")
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
    int entry_count_, current_position_, remove_calls;
    void on_remove(StringStructPayload *) { ++remove_calls; }
};
struct StringBox {
    StringList stringList_;
    int fixups;
    StringBox() : fixups(0) {}
    void add_fixup() { ++fixups; }
    void UNK1(int id);
    void clip_ids(unsigned long count);
};
namespace {
""" + helper + r"""
}
""" + unk1 + "\n" + clip_ids + r"""

static void make_ring(StringBox &box, StringStructEntry **nodes,
                      const int *ids, int count, int cursor) {
    box.stringList_.remove_calls = 0;
    for (int i = 0; i != count; ++i) {
        nodes[i] = new StringStructEntry;
        nodes[i]->id = ids[i];
        nodes[i]->payload = 0;
    }
    for (int i = 0; i != count; ++i) {
        nodes[i]->previous = nodes[(i + count - 1) % count];
        nodes[i]->next = nodes[(i + 1) % count];
    }
    box.stringList_.head_ = count ? nodes[0] : 0;
    box.stringList_.current_ = count ? nodes[cursor] : 0;
    box.stringList_.entry_count_ = count;
    box.stringList_.current_position_ = cursor;
}

static int ids_are(StringBox &box, const int *expected, int count) {
    if (box.stringList_.entry_count_ != count) return 0;
    if (count == 0) return box.stringList_.head_ == 0;
    StringStructEntry *entry = box.stringList_.head_;
    for (int i = 0; i != count; ++i) {
        if (entry == 0 || entry->id != expected[i]) return 0;
        if (entry->next->previous != entry || entry->previous->next != entry) return 0;
        entry = entry->next;
    }
    return entry == box.stringList_.head_;
}

int main() {
    {
        const int input[] = {5, 7, 5, 9};
        StringStructEntry *nodes[4];
        StringBox box;
        make_ring(box, nodes, input, 4, 1);
        box.UNK1(5);
        const int expected[] = {5, 7, 9};
        if (!ids_are(box, expected, 3) || box.stringList_.current_ != nodes[3] ||
            box.stringList_.current_position_ != 2 || box.stringList_.remove_calls != 1 ||
            box.fixups != 1) return 1;
    }
    {
        const int input[] = {5, 7, 9};
        StringStructEntry *nodes[3];
        StringBox box;
        make_ring(box, nodes, input, 3, 1);
        box.UNK1(22);
        if (!ids_are(box, input, 3) || box.stringList_.current_ != nodes[1] ||
            box.stringList_.current_position_ != 2 || box.stringList_.remove_calls != 0 ||
            box.fixups != 1) return 2;
    }
    {
        const int input[] = {4, 1, 8, 2, 9};
        StringStructEntry *nodes[5];
        StringBox box;
        make_ring(box, nodes, input, 5, 0);
        box.clip_ids(2);
        const int expected[] = {2, 6, 9};
        // The loop rereads the shrinking count: the fifth ID remains unvisited.
        if (!ids_are(box, expected, 3) || box.stringList_.current_ != nodes[4] ||
            box.stringList_.current_position_ != 2 || box.stringList_.remove_calls != 2 ||
            box.fixups != 1) return 3;
    }
    {
        const int input[] = {4, 8};
        StringStructEntry *nodes[2];
        StringBox box;
        make_ring(box, nodes, input, 2, 0);
        box.clip_ids(2);
        const int expected[] = {2, 6};
        // Surviving IDs are rebased even though this does not trigger a fixup.
        if (!ids_are(box, expected, 2) || box.stringList_.current_ != nodes[0] ||
            box.stringList_.current_position_ != 1 || box.stringList_.remove_calls != 0 ||
            box.fixups != 0) return 4;
    }
    {
        const int input[] = {-1, 3};
        StringStructEntry *nodes[2];
        StringBox box;
        make_ring(box, nodes, input, 2, 0);
        box.clip_ids(0);
        if (!ids_are(box, input, 2) || box.stringList_.current_ != nodes[0] ||
            box.stringList_.current_position_ != 1 || box.stringList_.remove_calls != 0 ||
            box.fixups != 0) return 5;
    }
    return 0;
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
def test_stringbox_actual_unk1_and_clip_ids_bodies(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"host C++ compiler unavailable: {compiler_name}")
    source = (ROOT / "src/stringbox.cpp").read_text()
    assert _compile_and_run(tmp_path, compiler, _harness(source), "faithful") == 0

    unk1 = _definition(source, "void StringBox::UNK1(")
    remove_all = unk1.replace(
        "if (list.current_->id == id) {\n                    found = true;\n                    break;\n                }",
        "if (list.current_->id == id) {\n                    found = true;\n                    erase_string_list_current(list);\n                }",
    )
    assert remove_all != unk1
    assert "found = true;\n                    erase_string_list_current(list);" in remove_all
    remove_all_source = source.replace(unk1, remove_all)
    assert _compile_and_run(tmp_path, compiler, _harness(remove_all_source), "remove_all_mutant") != 0

    clip = _definition(source, "void StringBox::clip_ids(")
    no_rebase = clip.replace(
        "list.current_->id = static_cast<int>(id - count);",
        "list.current_->id = static_cast<int>(id);",
    )
    assert no_rebase != clip
    assert "list.current_->id = static_cast<int>(id);" in no_rebase
    no_rebase_source = source.replace(clip, no_rebase)
    assert _compile_and_run(tmp_path, compiler, _harness(no_rebase_source), "no_rebase_mutant") != 0

    signed_clip = clip.replace(
        "if (id <= count) {",
        "if (list.current_->id <= static_cast<int>(count)) {",
    )
    assert signed_clip != clip
    assert "if (list.current_->id <= static_cast<int>(count)) {" in signed_clip
    signed_clip_source = source.replace(clip, signed_clip)
    assert _compile_and_run(tmp_path, compiler, _harness(signed_clip_source), "signed_clip_mutant") != 0


def test_stringbox_loop_and_clip_branches_match_pinned_instruction_spans():
    image = _pinned_image()
    assert hashlib.sha256(image.read_bytes()).hexdigest() == ORIGINAL_IMAGE_SHA256
    report = inspect_source(ROOT, ["0x006298B0", "0x00629A70"], image=image)
    assert report["result"] == "pass" and report["image_verified"]
    for subject, start, end in zip(report["subjects"], (0x006298B0, 0x00629A70), (0x0062997B, 0x00629B69)):
        span, = subject["original"]["spans"]
        assert span["result"] == "pass" and span["span"] == [start, end]
        lines = {line.split()[0]: line for line in span["disassembly"]}
        if start == 0x006298B0:
            assert "cmp ecx, edx" in lines["0x006298EB"]
            assert "jl 0x6298df" in lines["0x006298F0"]
            assert "dec eax" in lines["0x0062995B"]
            assert "call 0x629490" in lines["0x00629970"]
        else:
            assert "cmp ecx, ebp" in lines["0x00629AAE"]
            assert "jbe 0x629ac2" in lines["0x00629AB0"]
            assert "sub ecx, ebp" in lines["0x00629AB2"]
            assert "mov dword ptr [eax + 4], ecx" in lines["0x00629AB4"]
            assert "cmp eax, ecx" in lines["0x00629B32"]
            assert "jl 0x629aa8" in lines["0x00629B38"]
            assert "call 0x629490" in lines["0x00629B5D"]
