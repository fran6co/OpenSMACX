"""Actual-source controls for the report commands' virtual modal dispatch."""
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = next(p for p in Path(__file__).resolve().parents
            if (p / "src/reportif.cpp").is_file())
SOURCE = ROOT / "src/reportif.cpp"


def extract_function(source, signature):
    from match_recovery.includes import COMMENTS_OR_STRINGS

    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " "
                              for char in match[0]), source)
    start = masked.find(signature)
    assert start >= 0, f"missing source definition: {signature}"
    opening = masked.find("{", start)
    assert opening >= 0, f"missing function body: {signature}"
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError(f"unterminated function: {signature}")


def test_report_commands_keep_modal_virtual_dispatch_and_direct_report_behavior():
    source = SOURCE.read_text()
    names = ("report_labs", "report_energy", "report_ops", "report_projects",
             "report_sat", "report_security", "report_score")
    bodies = [extract_function(source, f"void __cdecl {name}(") for name in names]
    assert all("Win &modal_window = ReportWinGlobal;" in body for body in bodies)
    assert all("modal_window.set_modal(0, pop_wait, &MainInterfaceGlobal);" in body
               for body in bodies)
    assert "ReportIfGlobal.report(8, ReportIfGlobal.report_faction_id(), 0);" in bodies[-1]

    harness = r'''
#include <cassert>
#define __cdecl
struct Win {
    virtual int set_modal(int flag, int (*poll)(), Win *owner) {
        (void)flag; (void)poll; (void)owner;
        return 0;
    }
};
struct ReportWin : Win {
    int modal_calls;
    int modal_flag;
    int (*modal_poll)();
    Win *modal_owner;
    ReportWin() : modal_calls(0), modal_flag(-1), modal_poll(0), modal_owner(0) {}
    int set_modal(int flag, int (*poll)(), Win *owner) override {
        ++modal_calls;
        modal_flag = flag;
        modal_poll = poll;
        modal_owner = owner;
        return 23;
    }
};
struct ReportIf {
    int field_8_;
    int report_calls;
    int category;
    int faction;
    int selection;
    ReportIf() : field_8_(-1), report_calls(0), category(-1), faction(-1), selection(-1) {}
    int report_faction_id() const { return 91; }
    void report(int c, int f, int s) {
        ++report_calls; category = c; faction = f; selection = s;
    }
};
Win MainInterfaceGlobal;
ReportWin ReportWinGlobal;
ReportIf ReportIfGlobal;
int ActiveIfaceIdStorage;
int IfaceIdToSlotStorage[30];
int pop_wait() { return 17; }
'''
    harness += "\n".join(bodies)
    harness += r'''
int main() {
    void (__cdecl *commands[6])() = {
        report_labs, report_energy, report_ops, report_projects,
        report_sat, report_security
    };
    const int categories[7] = {2, 3, 4, 5, 6, 7, 8};
    for (int i = 0; i < 7; ++i) {
        ActiveIfaceIdStorage = i;
        IfaceIdToSlotStorage[i] = 4;
        ReportWinGlobal.modal_calls = 0;
        ReportIfGlobal.report_calls = 0;
        if (i < 6) commands[i]();
        else report_score(0x12345678);
        assert(ReportIfGlobal.field_8_ == categories[i]);
        assert(ReportWinGlobal.modal_calls == 1);
        assert(ReportWinGlobal.modal_flag == 0);
        assert(ReportWinGlobal.modal_poll == &pop_wait);
        assert(ReportWinGlobal.modal_owner == &MainInterfaceGlobal);
        assert(ReportIfGlobal.report_calls == 0);

        IfaceIdToSlotStorage[i] = 5;
        ReportWinGlobal.modal_calls = 0;
        ReportIfGlobal.report_calls = 0;
        if (i < 6) commands[i]();
        else report_score(-0x12345678);
        assert(ReportIfGlobal.field_8_ == categories[i]);
        assert(ReportWinGlobal.modal_calls == 0);
        assert(ReportIfGlobal.report_calls == 1);
        assert(ReportIfGlobal.category == categories[i]);
        assert(ReportIfGlobal.faction == 91);
        assert(ReportIfGlobal.selection == 0);
    }
}
'''

    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers, "at least one host C++ compiler is required"
    for compiler in compilers:
        with tempfile.TemporaryDirectory() as directory:
            cpp = Path(directory) / "report_modal_dispatch.cpp"
            executable = Path(directory) / "report_modal_dispatch"
            cpp.write_text(harness)
            built = subprocess.run(
                [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                 str(cpp), "-o", str(executable)],
                capture_output=True, text=True, timeout=30)
            assert built.returncode == 0, built.stderr
            ran = subprocess.run([str(executable)], capture_output=True,
                                 text=True, timeout=10)
            assert ran.returncode == 0, ran.stderr
