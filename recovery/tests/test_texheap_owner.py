"""Exercise actual typed TexHeap source; not native or whole-game acceptance."""
from pathlib import Path
import shutil
import subprocess
import pytest
from recovery.tests.test_pact_ascent import ROOT, definition


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_texheap_actual_source_and_damage(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    header = (ROOT / "src/sprite.h").read_text()
    texheap = header[header.index("struct TexHeap {"):header.index("static_assert(sizeof(TexHeap)")]
    heap_header = (ROOT / "src/heap.h").read_text()
    heap = heap_header[heap_header.index("class Heap {"):heap_header.index("#if defined(_M_IX86)")]
    source = (ROOT / "src/sprite.cpp").read_text()
    patterns = [r"TexHeap::TexHeap\(\)", r"TexHeap \*TexHeap::initialize\([^)]*\)",
                r"int TexHeap::configure\([^)]*\)", r"void TexHeap::release_allocations\(\)",
                r"void TexHeap::shutdown\(\)", r"void \*TexHeap::get_mem\([^)]*\)",
                r"void \*TexHeap::get_tex_mem\([^)]*\)"]
    bodies = [definition(source, pattern) for pattern in patterns]
    fixture = Path(__file__).with_name("texheap_owner_control.cpp").read_text()
    mutations = {
        "constructor_nonzero": (0, "count_ = 0;", "count_ = 1;"),
        "wrong_receiver": (1, "return this;", "return 0;"),
        "wrong_capacity": (2, "entries << 16", "entries << 15"),
        "wrong_table_count": (2, "entries * sizeof(*blocks_)", "count_ * sizeof(*blocks_)"),
        "wrong_first_boundary": (2, "+ 0x10000;", "+ 0x8000;"),
        "wrong_interior_size": (2, "(cursor), 0xffff)", "(cursor), 0x10000)"),
        "wrong_boundary_stride": (2, "cursor += 0x10000;", "cursor += 0x8000;"),
        "wrong_terminal_size": (2, "arena + capacity_ - cursor", "arena + capacity_ - cursor - 1"),
        "omit_release": (3, "free(block);", "(void)block;"),
        "omit_count_reset": (4, "count_ = 0;", "(void)count_;"),
        "wrong_ordinary_filter": (5, ">= size", "> size + 65536"),
        "wrong_aligned_filter": (6, "& 0xffff) == 0", "& 0xffff) != 0"),
    }
    for name in ("actual", *mutations):
        pieces = list(bodies)
        if name != "actual":
            index, before, after = mutations[name]
            assert pieces[index].count(before) == 1, name
            pieces[index] = pieces[index].replace(before, after, 1)
        text = fixture.replace("// @@DECLARATIONS@@", heap + "\n" + texheap)
        text = text.replace("// @@BODIES@@", "\n".join(pieces))
        cpp, binary = tmp_path / (name + ".cpp"), tmp_path / name
        cpp.write_text(text)
        result = subprocess.run([compiler, "-std=c++98", "-fsanitize=undefined",
                                 "-fno-sanitize-recover=all", str(cpp), "-o", str(binary)],
                                capture_output=True, text=True, timeout=45)
        assert result.returncode == 0, name + result.stderr
        run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
        assert (run.returncode == 0) == (name == "actual"), name + run.stderr
