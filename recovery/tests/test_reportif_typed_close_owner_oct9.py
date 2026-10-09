from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def source_bodies():
    source = (ROOT / "src/reportif.cpp").read_text()
    close_start = source.index("void ReportIf::close_intel()")
    close_end = source.index("\n}\n", close_start) + 3
    done_start = source.index("void ReportIf::done()")
    done_end = source.index("\n}\n", done_start) + 3
    close_body = source[close_start:close_end]
    done_body = source[done_start:done_end]
    assert "listBox1_.close();" in close_body
    assert "listBox2_.close();" in close_body
    assert "reinterpret_cast" not in close_body
    assert "reinterpret_cast<SubInterface" not in done_body
    assert done_body.index("release_modal") < done_body.index("release_iface_mode")
    header = (ROOT / "src/reportif.h").read_text()
    assert "ListBox listBox1_;" in header and "ListBox listBox2_;" in header
    assert "class ReportIf : public SubInterface" in header
    return close_body, done_body


def host_source(close_body, done_body):
    return r'''#include <cassert>
#include <vector>
struct Event { int kind; const void *receiver; };
static std::vector<Event> events;
struct ListBox {
    void close() { events.push_back((Event){1, this}); }
};
struct SubInterface {
    void release_iface_mode() { events.push_back((Event){3, this}); }
};
struct Win {
    void release_modal() { events.push_back((Event){2, this}); }
};
struct ReportWin : Win {};
ReportWin ReportWinGlobal;
struct ReportIf : SubInterface {
    ListBox listBox1_;
    ListBox listBox2_;
    void close_intel();
    void done();
};
''' + close_body + "\n" + done_body + r'''
int main() {
    ReportIf report;
    report.close_intel();
    assert(events.size()==2);
    assert(events[0].kind==1 && events[0].receiver==&report.listBox1_);
    assert(events[1].kind==1 && events[1].receiver==&report.listBox2_);

    events.clear();
    report.done();
    assert(events.size()==2);
    assert(events[0].kind==2 && events[0].receiver==static_cast<Win *>(&ReportWinGlobal));
    assert(events[1].kind==3 && events[1].receiver==static_cast<SubInterface *>(&report));
    assert(static_cast<const void *>(&report)==
           static_cast<const void *>(static_cast<SubInterface *>(&report)));
}
'''


def compile_and_run(compiler, path, close_body, done_body):
    source = path.with_suffix(".cpp")
    source.write_text(host_source(close_body, done_body))
    executable = path
    compiled = subprocess.run(
        [compiler, "-std=c++98", str(source), "-o", str(executable)],
        capture_output=True, text=True,
    )
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_reportif_members_and_release_order_with_executed_mutants(
    tmp_path, compiler_name
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("requested host C++ compiler unavailable")
    close_body, done_body = source_bodies()
    actual = compile_and_run(compiler, tmp_path / "reportif_actual",
                             close_body, done_body)
    assert actual.returncode == 0, actual.stderr

    wrong_member = close_body.replace("listBox2_.close();", "listBox1_.close();")
    assert wrong_member != close_body
    mutant_member = compile_and_run(compiler, tmp_path / "reportif_wrong_member",
                                    wrong_member, done_body)
    assert mutant_member.returncode != 0

    swapped_release = done_body.replace(
        "ReportWinGlobal.Win::release_modal();\n    SubInterface::release_iface_mode();",
        "SubInterface::release_iface_mode();\n    ReportWinGlobal.Win::release_modal();",
    )
    assert swapped_release != done_body
    mutant_order = compile_and_run(compiler, tmp_path / "reportif_swapped_release",
                                   close_body, swapped_release)
    assert mutant_order.returncode != 0
