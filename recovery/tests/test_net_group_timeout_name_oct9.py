"""The original timeout lookup miss passes null to strcat; never execute it."""
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def name_argument_block(source: str) -> str:
    start = source.index("int found_index = -1;", source.index("int Net::send_group("))
    end_marker = "strcat(StringTemp, name);"
    end = source.index(end_marker, start) + len(end_marker)
    block = source[start:end]
    assert block.count(end_marker) == 1
    block = block.replace(end_marker, "intercept_strcat(StringTemp, name);")
    # Bounded seam: a timeout's already-selected peer id may cease to match
    # the live table. This models only the lookup result, not concurrency.
    return block.replace("players_[search].field_0_ == players_[index].field_0_",
                         "players_[search].field_0_ == requested_id")


def compile_and_run(block: str, compiler: str) -> subprocess.CompletedProcess:
    program = r'''#include <cassert>
#include <cstdio>
struct NetPlayer { unsigned long field_0_; unsigned long field_8_; char name_and_tail_[64]; };
static char *observed;
static void intercept_strcat(char *, char *value) { observed = value; }
static void run(NetPlayer *players_, int index, unsigned long requested_id, char *StringTemp) {
    (void)StringTemp;
    if (players_[index].field_8_ != 0) {
    BLOCK
    }
}
int main() {
    (void)intercept_strcat;
    NetPlayer players[16] = {};
    char output[8] = {};
    for (int i=0; i<16; ++i) {
        players[i].field_0_ = (unsigned long)(100+i);
        players[i].field_8_ = 1;
        players[i].name_and_tail_[0] = (char)('A'+i);
        players[i].name_and_tail_[1] = 0;
    }
    for (int i=0; i<16; ++i) {
        observed = 0;
            run(players, i, players[i].field_0_, output);
        assert(observed == players[i].name_and_tail_);
    }
    observed = (char *)1;
    run(players, 7, 999, output);
    assert(observed == 0);
    puts("timeout strcat argument controls passed");
}
'''.replace("    BLOCK", "    " + block.replace("\n", "\n    "))
    with tempfile.TemporaryDirectory(prefix="net-timeout-name-") as tmp:
        cpp = Path(tmp) / "control.cpp"
        exe = Path(tmp) / "control"
        cpp.write_text(program)
        built = subprocess.run([compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)], capture_output=True, text=True)
        assert built.returncode == 0, built.stdout + built.stderr
        return subprocess.run([str(exe)], capture_output=True, text=True)


def test_timeout_lookup_passes_null_on_miss_and_found_slot_name_on_hits():
    source = (ROOT / "src/net_class.cpp").read_text()
    block = name_argument_block(source)
    assert "? 0" in block
    for compiler in ("g++", "clang++"):
        if shutil.which(compiler) is None:
            continue
        run = compile_and_run(block, compiler)
        assert run.returncode == 0, run.stdout + run.stderr
        assert "passed" in run.stdout


def test_timeout_lookup_rejects_slot_zero_fallback_and_skip_mutants():
    source = (ROOT / "src/net_class.cpp").read_text()
    block = name_argument_block(source)
    mutants = (
        ("? 0", "? players_[0].name_and_tail_"),
        ("intercept_strcat(StringTemp, name);", "(void)name;"),
    )
    compilers = [c for c in ("g++", "clang++") if shutil.which(c)]
    assert compilers
    for old, new in mutants:
        assert block.count(old) == 1
        damaged = block.replace(old, new, 1)
        for compiler in compilers:
            run = compile_and_run(damaged, compiler)
            assert run.returncode != 0, f"mutant survived: {old} ({compiler})"

