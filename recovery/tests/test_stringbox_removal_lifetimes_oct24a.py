from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
CONTROL = Path(__file__).with_name("stringbox_removal_lifetimes_control_oct24a.cpp")


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


def _fixture(source: str) -> str:
    methods = "\n\n".join(
        _definition(source, signature)
        for signature in (
            "void StringBox::add_fixup(",
            "void StringBox::UNK1(",
            "void StringBox::UNK2(",
            "void StringBox::clip_ids(",
        )
    )
    return CONTROL.read_text().replace("/*SOURCE_METHODS*/", methods)


def _run(tmp_path: Path, compiler: str, source: str, name: str) -> int:
    cpp = tmp_path / f"{name}.cpp"
    executable = tmp_path / name
    cpp.write_text(source)
    compiled = subprocess.run(
        [compiler, "-std=c++98", str(cpp), "-o", str(executable)],
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run(
        [str(executable)], capture_output=True, text=True, timeout=5
    ).returncode


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_stringbox_removal_order_and_fixup_services_from_actual_bodies(
    tmp_path, compiler_name
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"host C++ compiler unavailable: {compiler_name}")
    source = (ROOT / "src/stringbox.cpp").read_text()
    fixture = _fixture(source)
    assert _run(tmp_path, compiler, fixture, "actual_source") == 0

    unk1 = _definition(source, "void StringBox::UNK1(")
    removal_sequence = (
        "                    list.current_->next->previous = list.current_->previous;\n"
        "                    list.current_->previous->next = list.current_->next;\n"
        "                    StringStructEntry *const entry = list.current_;\n"
        "                    if (entry == list.head_) {\n"
        "                        list.head_ = entry->next;\n"
        "                    }\n"
        "                    list.current_ = entry->next;\n"
        "                    StringStructPayload *const payload = entry->string_payload();\n"
        "                    list.on_remove(payload);"
    )
    callback_before_unlink_sequence = (
        "                    StringStructEntry *const entry = list.current_;\n"
        "                    StringStructPayload *const payload = entry->string_payload();\n"
        "                    list.on_remove(payload);\n"
        "                    list.current_->next->previous = list.current_->previous;\n"
        "                    list.current_->previous->next = list.current_->next;\n"
        "                    if (entry == list.head_) {\n"
        "                        list.head_ = entry->next;\n"
        "                    }\n"
        "                    list.current_ = entry->next;"
    )
    before_unlink = unk1.replace(removal_sequence, callback_before_unlink_sequence, 1)
    assert before_unlink != unk1
    mutant = source.replace(unk1, before_unlink)
    assert _run(tmp_path, compiler, _fixture(mutant), "callback_before_unlink") != 0

    clear_before_delete = unk1.replace(
        "                    delete payload;\n                    entry->payload = 0;",
        "                    entry->payload = 0;\n                    delete payload;",
        1,
    )
    assert clear_before_delete != unk1
    mutant = source.replace(unk1, clear_before_delete)
    assert _run(tmp_path, compiler, _fixture(mutant), "payload_clear_before_delete") != 0

    removal_and_count = (
        "                    list.on_remove(payload);\n"
        "                    delete payload;\n"
        "                    entry->payload = 0;\n"
        "                    delete entry;\n"
        "                    --list.entry_count_;"
    )
    count_before_callback = (
        "                    --list.entry_count_;\n"
        "                    list.on_remove(payload);\n"
        "                    delete payload;\n"
        "                    entry->payload = 0;\n"
        "                    delete entry;"
    )
    decrement_before_callback = unk1.replace(removal_and_count, count_before_callback, 1)
    assert decrement_before_callback != unk1
    mutant = source.replace(unk1, decrement_before_callback)
    assert _run(tmp_path, compiler, _fixture(mutant), "count_decrement_before_callback") != 0

    add_fixup = _definition(source, "void StringBox::add_fixup(")
    cached_height = add_fixup.replace(
        "void StringBox::add_fixup() {",
        "void StringBox::add_fixup() {\n    const int cached_height = dib_.bmiHeader.biHeight;",
        1,
    ).replace(
        "scroll_.paging_ = -1 - (dib_.bmiHeader.biHeight / line_height);",
        "scroll_.paging_ = -1 - (cached_height / line_height);",
        1,
    )
    assert cached_height != add_fixup
    mutant = source.replace(add_fixup, cached_height)
    assert _run(tmp_path, compiler, _fixture(mutant), "cached_height_after_callback") != 0
