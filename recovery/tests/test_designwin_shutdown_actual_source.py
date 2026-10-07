"""Shutdown call/order controls; not native layout or full-game proof."""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_listbox_dimensions_actual_source import extract_definition

ROOT = Path(__file__).resolve().parents[2]
SIGNATURE = "void DesignWin::shut_that_badboy_down()"
SUPPORT = r'''
#include <cassert>
#include <string>
#include <vector>
std::vector<std::string> calls;
int DesignTutorState;
class DesignWin;
DesignWin *DesignWindowSelf;
struct Time {
    void stop() { assert(DesignWindowSelf == 0); calls.push_back("stop"); }
    void release_modal() { calls.push_back("release"); }
};
void do_all_draws() { calls.push_back("draw"); }
class DesignWin {
public:
    Time time_;
    virtual ~DesignWin() {}
    virtual void hide() { calls.push_back("base-hide"); }
    void shut_that_badboy_down();
};
class Derived : public DesignWin {
public:
    virtual void hide() {
        calls.push_back("derived-hide");
        // Callback changes the condition: it must not be re-tested halfway.
        DesignTutorState = 1;
    }
};
/* PRODUCT */
int main() {
    Derived owner;
    const int states[] = {0, 1, 2, -1};
    for (int i = 0; i != 4; ++i) {
        calls.clear();
        DesignTutorState = states[i];
        DesignWindowSelf = &owner;
        owner.shut_that_badboy_down();
        assert(DesignTutorState == 0);
        if (states[i] == 1) {
            assert(calls.empty());
            assert(DesignWindowSelf == &owner);
        } else {
            assert(DesignWindowSelf == 0);
            assert(calls.size() == 4);
            assert(calls[0] == "derived-hide");
            assert(calls[1] == "draw");
            assert(calls[2] == "stop");
            assert(calls[3] == "release");
        }
    }
}
'''


def run_body(directory, body, compiler):
    directory.mkdir()
    source = directory / "control.cpp"
    binary = directory / "control"
    source.write_text(SUPPORT.replace("/* PRODUCT */", body))
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(binary)], check=True,
                   capture_output=True, text=True, timeout=30)
    return subprocess.run([str(binary)], capture_output=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_shutdown_virtual_dispatch_and_order(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable")
    body = extract_definition((ROOT / "src/designwin.cpp").read_text(), SIGNATURE)
    assert "vtable_slot" not in body and "ORIGINAL(" not in body
    assert run_body(tmp_path / "product", body, compiler).returncode == 0
    damages = (
        ("hide();", "DesignWin::hide();"),
        ("DesignTutorState != 1", "DesignTutorState == 1"),
        ("DesignWindowSelf = 0;", "DesignWindowSelf = this;"),
        ("modal->stop();\n        modal->release_modal();",
         "modal->release_modal();\n        modal->stop();"),
        ("    DesignTutorState = 0;", "    DesignTutorState = 1;"),
    )
    for index, (before, after) in enumerate(damages):
        assert body.count(before) == 1
        assert run_body(tmp_path / ("damage-" + str(index)),
                        body.replace(before, after), compiler).returncode != 0
