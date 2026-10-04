"""Compile genuine source definitions and mutation controls with host compilers."""
from pathlib import Path
import shutil
import subprocess

import pytest

from recovery.tests.test_pact_ascent import ROOT, definition


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_queue_callbacks_and_mutations(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    header = (ROOT / "src/datalink.h").read_text()
    declarations = header[header.index("struct DatalinkSelectionEntry {"):]
    source = (ROOT / "src/datalink.cpp").read_text()
    patterns = [
        r"void DatalinkSelectionEntry::append_text\([^)]*\) const",
        r"void DatalinkSelectionEntry::show_help\(\) const",
        r"void DatalinkSelectionQueue::clear\(\)",
        r"void DatalinkSelectionQueue::append\([^)]*\)",
        r"void DatalinkSelectionQueue::dispatch_matching\([^)]*\)",
        r"DatalinkEntryCallbackTable::DatalinkEntryCallbackTable\(\)",
    ]
    bodies = "\n".join(definition(source, pattern) for pattern in patterns)
    fixture = Path(__file__).with_name("datalink_selection_queue_control.cpp").read_text()
    actual = fixture.replace("// @@DECLARATIONS@@", declarations).replace("// @@BODIES@@", bodies)
    mutations = {
        "wrong_record_value": ("callbacks.help(value);", "callbacks.help(category);"),
        "wrong_extra": ("callbacks.append_text(output, value, extra);", "callbacks.append_text(output, value, 0);"),
        "short_capacity": ("count_ < 0x80", "count_ < 0x7F"),
        "uncleared_count": ("count_ = 0;", "count_ = 1;"),
        "wrong_matching_id": ("entry.entry_id == entry_id", "entry.entry_id != entry_id"),
        "wrong_category": ("entry.category = 10;", "entry.category = 9;"),
        "wrong_threshold": ("entry.value >= 0x46", "entry.value > 0x46"),
        "wrong_remap": ("entry.value + 0xBA", "entry.value + 0xBB"),
        "missing_scratch_clear": ("StringTemp[0] = 0;", "StringTemp[0] = 'z';"),
        "wrong_detail": ("entry.append_text(StringTemp, 1);", "entry.append_text(StringTemp, 0);"),
        "wrong_selection": ("datalink_entry_selection(first_match_index);", "datalink_entry_selection(0);"),
        "wrong_modal_flag": ("DatalinkMain.set_modal(0, pop_wait, 0);", "DatalinkMain.set_modal(1, pop_wait, 0);"),
        "wrong_table_mapping": ("entries[6].help = help_social;", "entries[6].help = help_weapon;"),
        "cached_count": ("for (int index = 0; index < count_; ++index)", "int captured_count = count_; for (int index = 0; index < captured_count; ++index)"),
    }
    variants = {"actual": actual}
    for name, (before, after) in mutations.items():
        assert bodies.count(before) == 1, name
        damaged = bodies.replace(before, after, 1)
        variants[name] = fixture.replace("// @@DECLARATIONS@@", declarations).replace("// @@BODIES@@", damaged)
    for name, text in variants.items():
        cpp, binary = tmp_path / (name + ".cpp"), tmp_path / name
        cpp.write_text(text)
        build = subprocess.run([compiler, "-std=c++98", "-fsanitize=undefined",
            "-fno-sanitize-recover=all", str(cpp), "-o", str(binary)], capture_output=True, text=True, timeout=45)
        assert build.returncode == 0, name + build.stdout + build.stderr
        run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=15)
        assert (run.returncode == 0) == (name == "actual"), name + run.stdout + run.stderr
