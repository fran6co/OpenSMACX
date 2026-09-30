"""Control the product redraw against original 496D00/table496DE4 evidence.

The drawers are observers, not a claim that the unfinished pages work in-game.
"""
from pathlib import Path
import shutil
import subprocess

import pytest


def product_body():
    root = Path(__file__).resolve().parents[2]
    source = (root / "src/reportwin.cpp").read_text()
    start = source.index("void ReportWin::on_redraw() {")
    return source[start:source.index("\n}\n", start) + 3]


def harness(body):
    return r'''
struct RECT {};
int events[10], count;
void event(int id) { events[count++] = id; }
struct Buffer {
    RECT working;
    void set_clip(RECT *area) { event(area == &working ? 11 : 10); }
};
struct ReportIfFixture { unsigned int field_8_; } ReportIfGlobal;
struct ReportWin : Buffer {
    int field_59AC_;
    RECT energy_panel_rect_;
    void reset() { event(9); }
    RECT *buffer_working_rect() { return &working; }
    void draw_intel() { event(0); }
    void draw_labs() { event(2); }
    void draw_energy() { event(3); }
    void draw_ops() { event(4); }
    void draw_projects() { event(5); }
    void draw_sat() { event(6); }
    void draw_security() { event(7); }
    void draw_score() { event(8); }
    void on_redraw();
};
''' + body + r'''
int main() {
    // Table entries1/default do not draw. Unsigned ja handles negative IDs.
    const unsigned int categories[] = {0,1,2,3,4,5,6,7,8,9,100,~0u};
    for (unsigned int i=0; i<sizeof(categories)/sizeof(categories[0]); ++i) {
        ReportWin window;
        window.field_59AC_=73;
        ReportIfGlobal.field_8_=categories[i];
        count=0;
        window.on_redraw();
        bool drawn=categories[i]<=8 && categories[i]!=1;
        if(window.field_59AC_!=-1 || count!=(drawn?4:3)) return 1;
        if(events[0]!=9 || events[1]!=10 || events[count-1]!=11) return 2;
        if(drawn && events[2]!=static_cast<int>(categories[i])) return 3;
    }
    return 0;
}
'''


@pytest.mark.parametrize("damage", [None, "dispatch", "reset", "selection", "clip"])
def test_product_reportwin_redraw(tmp_path, damage):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    body = product_body()
    mutations = {
        "dispatch": ("case 0: draw_intel();", "case 0: draw_labs();"),
        "reset": ("reset();", ""),
        "selection": ("field_59AC_ = -1;", "field_59AC_ = 0;"),
        "clip": ("canvas.set_clip(buffer_working_rect());", ""),
    }
    if damage is not None:
        before, after = mutations[damage]
        assert body.count(before) == 1
        body = body.replace(before, after)
    source = tmp_path / "redraw.cpp"
    source.write_text(harness(body))
    executable = tmp_path / "redraw"
    compiled = subprocess.run([compiler, "-std=c++98", str(source), "-o", str(executable)],
                              capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True)
    assert (result.returncode == 0) == (damage is None)
