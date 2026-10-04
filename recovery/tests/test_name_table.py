"""Compile the real name-table family and reject semantic damage controls."""
from pathlib import Path
import shutil
import subprocess
import pytest
from recovery.tests.test_pact_ascent import ROOT, definition


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_name_table_and_damage(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    header = (ROOT / "src/stringstruct.h").read_text()
    declarations = header[header.index("struct NameTableEntry {"):header.index("static_assert(sizeof(NameTableEntry)")]
    source = (ROOT / "src/stringstruct.cpp").read_text()
    patterns = [r"NameTableEntry \*NameTableEntry::initialize\(\)",
        r"NameTable \*NameTable::initialize\(\)", r"int NameTable::clear\(\)",
        r"unsigned int NameTable::remove_range\([^)]*\)",
        r"void NameTable::install\([^)]*\)", r"void NameTable::set_name\([^)]*\)",
        r"void NameTable::rewrite_directory\([^)]*\)"]
    bodies = "\n".join(definition(source, pattern) for pattern in patterns)
    fixture = Path(__file__).with_name("name_table_control.cpp").read_text()
    mutations = {
        "wrong_entry_status": ("status_ = 1;", "status_ = 2;"),
        "wrong_table_flags": ("flags_ = (flags_ & ~4u) | 2u;", "flags_ = 4u;"),
        "wrong_limit": ("slot_index < 0x80", "slot_index < 0x7F"),
        "wrong_entry_extent": ("memset(name_, 0, 0x104)", "memset(name_, 0, 0x103)"),
        "wrong_clone_status": ("clone->status_ = entry->status_;", "clone->status_ = 1;"),
        "wrong_install_start": ("NameTableEntry **slot = slots_ + low;\n        unsigned int count = high - low + 1;", "NameTableEntry **slot = slots_ + low + 1;\n        unsigned int count = high - low + 1;"),
        "wrong_install_end": ("unsigned int count = high - low + 1;", "unsigned int count = high - low;"),
        "wrong_basename": ("slash + 1 : name", "slash : name"),
        "wrong_directory_separator": ("dir_buf[length] = '\\\\';", "dir_buf[length] = '/';"),
        "wrong_reversed_low": ("clone->low_index_ = high;", "clone->low_index_ = low;"),
        "changed_original_skip": ("slot_index = record->high_index_ + 1;", "slot_index = record->high_index_;"),
        "omitted_owned_string_free": ("::operator delete(name_);\n        name_ = 0;", "name_ = 0;"),
    }
    for name in ("actual", *mutations):
        altered = bodies
        if name != "actual":
            before, after = mutations[name]
            assert bodies.count(before) == 1, name
            altered = bodies.replace(before, after, 1)
        text = fixture.replace("// @@DECLARATIONS@@", declarations).replace("// @@BODIES@@", altered)
        cpp, binary = tmp_path / (name + ".cpp"), tmp_path / name
        cpp.write_text(text)
        build = subprocess.run([compiler, "-std=c++98", "-fsanitize=undefined",
            "-fno-sanitize-recover=all", str(cpp), "-o", str(binary)],
            capture_output=True, text=True, timeout=45)
        assert build.returncode == 0, name + build.stderr
        run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
        assert (run.returncode == 0) == (name == "actual"), name + run.stderr
