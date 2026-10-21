"""Exercise Net::send_group's actual signed size/capacity guards."""
from pathlib import Path
import shutil
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[2]


def capacity_guard_block(source: str) -> str:
    start = source.index("int Net::send_group(")
    guard_start = source.index("if (size < 0) return 0;", start)
    end_marker = "\n\n    check_polling();"
    guard_end = source.index(end_marker, guard_start)
    block = source[guard_start:guard_end]
    assert block.count("if (") == 2
    assert "static_cast<int>(field_6E4_) < size" in block
    return block


def compile_and_run(block: str, compiler: str) -> subprocess.CompletedProcess:
    program = r'''#include <assert.h>
#include <limits.h>
#include <stdint.h>

static int source_capacity_guard(uint32_t field_6E4_, int size) {
BLOCK
    return 1;
}

int main() {
    assert(source_capacity_guard(0x2000U, -1) == 0);
    assert(source_capacity_guard(0x2000U, INT_MIN) == 0);
    assert(source_capacity_guard(0U, 0) == 1);
    assert(source_capacity_guard(0U, 1) == 0);
    assert(source_capacity_guard(0x2000U, 0x2000) == 1);
    assert(source_capacity_guard(0x2000U, 0x2001) == 0);
    assert(source_capacity_guard(0x7fffffffU, INT_MAX) == 1);
    assert(source_capacity_guard(0x80000000U, 0) == 0);
    assert(source_capacity_guard(0x80000000U, 1) == 0);
    assert(source_capacity_guard(0x80000000U, INT_MIN) == 0);
    assert(source_capacity_guard(0xffffffffU, INT_MAX) == 0);
    assert(source_capacity_guard(0xffffffffU, -1) == 0);
}
'''.replace("BLOCK", "\n".join("    " + line for line in block.splitlines()))

    with tempfile.TemporaryDirectory(prefix="net-group-capacity-") as temp:
        cpp = Path(temp) / "capacity.cpp"
        exe = Path(temp) / "capacity"
        cpp.write_text(program)
        built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
             str(cpp), "-o", str(exe)], capture_output=True, text=True,
            timeout=45)
        assert built.returncode == 0, built.stdout + built.stderr
        return subprocess.run([str(exe)], capture_output=True, text=True,
                              timeout=15)


def test_actual_send_group_size_guards_match_signed_pinned_comparison():
    source = (ROOT / "src/net_class.cpp").read_text()
    block = capacity_guard_block(source)
    compilers = [name for name in ("g++", "clang++") if shutil.which(name)]
    assert compilers
    for compiler in compilers:
        result = compile_and_run(block, compiler)
        assert result.returncode == 0, result.stdout + result.stderr


def test_unsigned_and_off_by_one_capacity_mutants_compile_but_fail_controls():
    source = (ROOT / "src/net_class.cpp").read_text()
    block = capacity_guard_block(source)
    mutants = (
        ("if (size < 0) return 0;\n", ""),
        ("static_cast<int>(field_6E4_) < size",
         "field_6E4_ < static_cast<uint32_t>(size)"),
        ("static_cast<int>(field_6E4_) < size",
         "static_cast<int>(field_6E4_) <= size"),
    )
    compilers = [name for name in ("g++", "clang++") if shutil.which(name)]
    assert compilers
    for old, new in mutants:
        assert block.count(old) == 1
        mutant = block.replace(old, new, 1)
        for compiler in compilers:
            result = compile_and_run(mutant, compiler)
            assert result.returncode != 0, f"mutant survived: {new} ({compiler})"
