"""Live close_opening ownership controls, not original ABI or game proof.

The host double retains Console -> MapWin -> virtual GraphicWin -> Win.
Its nonzero base adjustment is observed, not copied from the Win32 layout.
Only the pointer-to-word publication is widened explicitly for the host;
the product's uint32_t storage is appropriate to its Win32 target.
"""

from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
PUBLICATION = "BasePopDefaultField322C = reinterpret_cast<uint32_t>(console_window);"


def live_body():
    path = ROOT / "src/game.cpp"
    record, = (r for r in read_file(path) if r.address == 0x589B20)
    assert record.image_spans == ((0x589B20, 0x589B57),)
    source = path.read_text()
    signature = "void __cdecl close_opening()"
    assert source.count(signature) == 1
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            body = source[start:end + 1]
            assert body.count(PUBLICATION) == 1
            return body
    raise AssertionError("unterminated live close_opening body")


HARNESS = r"""
    #include <cstddef>
    #include <cstdint>
    #include <cstdlib>
    #include <new>
    #define __cdecl
    void check(bool condition) { if (!condition) std::exit(91); }

    struct Win;
    struct Buffer;
    struct GraphicWin;
    GraphicWin *slot = 0, *expected_opening = 0;
    Win *expected_win = 0;
    Buffer *expected_buffer = 0;
    void *expected_allocation = 0;
    std::uintptr_t BasePopDefaultField322C = 0, expected_parent = 0;
    bool tracking = false;
    int phase = 0, releases = 0, accesses = 0, derived_destructions = 0;
    int graphic_destructions = 0, buffer_destructions = 0, win_destructions = 0;

    void event(int next) {
        check(tracking && next == phase + 1);
        check(BasePopDefaultField322C == expected_parent);
        // The owned pointer is published until destruction AND deallocation
        // finish. Equality only: never dereference it after lifetime ends.
        check(slot == expected_opening);
        phase = next;
    }
    struct Win {
        virtual ~Win() {
            if (tracking && this == expected_win) { ++win_destructions; event(4); }
        }
        int win_field;
    };
    struct Buffer {
        virtual ~Buffer() {
            if (tracking && this == expected_buffer) { ++buffer_destructions; event(3); }
        }
        int buffer_field;
    };
    struct GraphicWin : Win, Buffer {
        virtual ~GraphicWin() {
            if (tracking && this == expected_opening) { ++graphic_destructions; event(2); }
        }
        virtual void on_dialog_focus(int) { check(false); }
    };
    struct MapWin : virtual GraphicWin {
        virtual void map_method() {}
        int map_field;
    };
    struct Console : MapWin { int console_field; } ConsoleGlobal;
    struct Opening : GraphicWin {
        virtual ~Opening() { ++derived_destructions; event(1); }
        static void operator delete(void *allocation) noexcept {
            check(allocation == expected_allocation);
            event(5);
            ++releases;
            ::operator delete(allocation);
        }
    };
    GraphicWin *&opening_window() {
        ++accesses;
        // The second accessor is the final clearing write, after deleting.
        if (accesses == 2) check(phase == 5 && releases == 1);
        return slot;
    }

    // LIVE_BODY

    int main() {
        Win *adjusted = &ConsoleGlobal;
        expected_parent = reinterpret_cast<std::uintptr_t>(adjusted);
        check(expected_parent != reinterpret_cast<std::uintptr_t>(&ConsoleGlobal));
        // A raw complete-object address must not accidentally satisfy this
        // fixture; virtual inheritance causes a real, nonzero adjustment.
        check(static_cast<GraphicWin *>(&ConsoleGlobal) != 0);

        BasePopDefaultField322C = 17;
        close_opening();
        check(BasePopDefaultField322C == expected_parent && slot == 0);
        check(accesses == 1 && releases == 0 && phase == 0);

        Opening *allocation = new Opening;
        slot = allocation;
        expected_opening = allocation;
        expected_win = allocation;
        expected_buffer = allocation;
        expected_allocation = allocation;
        accesses = 0;
        BasePopDefaultField322C = 19;
        tracking = true;
        close_opening();
        check(slot == 0 && phase == 5 && releases == 1 && accesses == 2);
        check(derived_destructions == 1 && graphic_destructions == 1);
        check(buffer_destructions == 1 && win_destructions == 1);
        check(BasePopDefaultField322C == expected_parent);
        tracking = false;

        // Repeated close is null-safe and republishes the parent, without
        // trying to destroy or free the allocation a second time.
        accesses = 0;
        BasePopDefaultField322C = 23;
        close_opening();
        check(slot == 0 && accesses == 1 && releases == 1 && phase == 5);
        check(derived_destructions == 1 && graphic_destructions == 1);
        check(buffer_destructions == 1 && win_destructions == 1);
        check(BasePopDefaultField322C == expected_parent);
        return 0;
    }
"""


def run_host(tmp_path, body):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host g++ required")
    # Explicit host-only width bridge: do not typedef uint32_t to a wider
    # type, truncate the pointer, or change product globals/source.
    old = "reinterpret_cast<uint32_t>(console_window)"
    assert body.count(old) == 1
    body = body.replace(old, "reinterpret_cast<std::uintptr_t>(console_window)")
    source = tmp_path / "close_opening.cpp"
    executable = tmp_path / "close_opening"
    source.write_text(HARNESS.replace("// LIVE_BODY", body))
    compiled = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
         "-fno-sanitize-recover=undefined", str(source), "-o", str(executable)],
        capture_output=True, text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)


def test_live_close_opening_adjusts_publishes_deletes_and_clears(tmp_path):
    result = run_host(tmp_path, live_body())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("damage", [
    "raw_parent", "publish_only_nonnull", "publish_after_delete", "clear_before_delete",
    "missing_clear", "missing_delete", "destruct_without_free", "focus_instead_of_delete",
])
def test_close_opening_controls_reject_behavior_not_compile_failure(tmp_path, damage):
    body = live_body()
    if damage == "raw_parent":
        old, new = "Win *console_window = &ConsoleGlobal;", \
            "Win *console_window = reinterpret_cast<Win *>(&ConsoleGlobal);"
    elif damage == "publish_only_nonnull":
        body = body.replace(PUBLICATION, "")
        old, new = "if (opening != 0) {", "if (opening != 0) {\n" + PUBLICATION
    elif damage == "publish_after_delete":
        old, new = PUBLICATION, ""
    elif damage == "clear_before_delete":
        old, new = "delete opening;", "opening_window() = 0;\ndelete opening;"
    elif damage == "missing_clear":
        old, new = "opening_window() = 0;", ""
    elif damage == "missing_delete":
        old, new = "delete opening;", ""
    elif damage == "destruct_without_free":
        old, new = "delete opening;", "opening->~GraphicWin();"
    else:
        old, new = "delete opening;", "opening->on_dialog_focus(1);"
    assert body.count(old) == 1
    body = body.replace(old, new)
    if damage == "publish_after_delete":
        # Preserve publication on the null path so the non-null destructor
        # specifically detects late publication, not its omission.
        end = body.rfind("}")
        body = body[:end] + PUBLICATION + "\n" + body[end:]
    result = run_host(tmp_path, body)
    assert result.returncode == 91, result.stderr
