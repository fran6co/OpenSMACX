"""Re-entrant Menu slot semantics checked against the pinned native bodies.

The C++ fixture is a mock ABI. It extracts the complete current product
definitions; it is not evidence of Win32 behavior or compiler equivalence.
"""
import os
import hashlib
from pathlib import Path
import shutil
import subprocess

import pytest

from recovery.tests.test_menu_input_actual import extract_definition, METHODS

ROOT = Path(__file__).resolve().parents[2]
IMAGE_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
MENU_METHODS = METHODS


def actual_fixture(tmp_path, *, damaged=None):
    base = (ROOT / "recovery/tests/menu-owner-harness.cpp").read_text()
    def replace_once(text, old, new):
        assert text.count(old) == 1, old
        return text.replace(old, new, 1)
    hook_mark = "class Font;\n"
    base = replace_once(base, hook_mark, "class Font;\nclass PullDown;\nstatic PullDown *observed_shown=0, *observed_selected=0;\nstatic void (*AfterRedraw)()=0; static void (*AfterFlush)()=0;\nstatic void (*AfterGetSelected)(PullDown*)=0; static void (*AfterShow)(PullDown*)=0;\n")
    base = replace_once(base, "virtual void redraw() { ++redraws; }", "virtual void redraw() { ++redraws; if (AfterRedraw) AfterRedraw(); }")
    base = replace_once(base, "void flush_input() { ++flushes; }", "void flush_input() { ++flushes; if (AfterFlush) AfterFlush(); }")
    base = replace_once(base, "void do_all_draws() { ++draws; }", "static void (*AfterDraws)()=0;\nvoid do_all_draws() { ++draws; if (AfterDraws) AfterDraws(); }")
    base = replace_once(base, "void show(int, int, int) { ++shows; }", "void show(int, int, int) { ++shows; observed_shown=this; if (AfterShow) AfterShow(this); }")
    base = replace_once(base, "int get_selected() { return selected_; }", "int get_selected() { observed_selected=this; int r=selected_; if (AfterGetSelected) AfterGetSelected(this); return r; }")
    task_stub = "static int last_task_count = 0, draws = 0, last_callback_id = -999;\nvoid do_all_tasks(Win *) { ++last_task_count; }"
    base = replace_once(base, task_stub, "static int last_task_count = 0, draws = 0, last_callback_id = -999;\nstatic void (*AfterTask)(Win*)=0; static Win *observed_tasks[32]; static int observed_task_count=0;\nvoid do_all_tasks(Win *w) { ++last_task_count; observed_tasks[observed_task_count++]=w; if (AfterTask) AfterTask(w); }")
    bodies = []
    for relative, signature in MENU_METHODS:
        body = extract_definition((ROOT / relative).read_text(), signature)
        if damaged == "cached":
            body = body.replace("PullDown **closing_slot = &entries_[selected_menu].pull_down;", "PullDown *closing_value = entries_[selected_menu].pull_down;")
            body = body.replace("PullDown **closing_slot = &entries_[field_A24_].pull_down;", "PullDown *closing_value = entries_[field_A24_].pull_down;")
            body = body.replace("*closing_slot", "closing_value")
            body = body.replace("PullDown **old_menu_slot = &entries_[current].pull_down;", "PullDown *old_menu_value = entries_[current].pull_down;").replace("*old_menu_slot", "old_menu_value")
        elif damaged == "recapture":
            body = body.replace("PullDown **closing_slot = &entries_[selected_menu].pull_down;", "")
            body = body.replace("PullDown **closing_slot = &entries_[field_A24_].pull_down;", "")
            body = body.replace("PullDown **old_menu_slot = &entries_[current].pull_down;", "")
            body = body.replace("*closing_slot", "entries_[field_A24_].pull_down")
            body = body.replace("*old_menu_slot", "entries_[field_A24_].pull_down")
        elif damaged == "mnemonic_index":
            body = body.replace("entries_[field_A24_].pull_down->show(point.x, point.y, 0)", "entries_[index].pull_down->show(point.x, point.y, 0)")
        elif damaged == "before_draws" and signature == "int Menu::on_key_click(int, int key)":
            body = replace_once(body,
                "do_all_draws();\n\n            const int item_index = (*closing_slot)->get_selected();",
                "PullDown *selected_before_draws = *closing_slot;\n            do_all_draws();\n\n            const int item_index = selected_before_draws->get_selected();")
        bodies.append(body)
    marker = '#include "GENERATED_MENU_SOURCE"'
    assert base.count(marker) == 1
    scenarios = r'''
static Menu *hook_menu; static PullDown *hook_values[8]; static int hook_cursor;
static void replace_on_flush() { hook_menu->entries_[0].pull_down=hook_values[hook_cursor++]; hook_menu->field_A24_=1; }
static void replace_on_task(Win *w) { if (w != hook_menu) hook_menu->entries_[0].pull_down=hook_values[hook_cursor++]; }
static void replace_on_draw() { hook_menu->entries_[0].pull_down=hook_values[hook_cursor++]; }
static void replace_on_select(PullDown *) { hook_menu->entries_[0].pull_down=hook_values[hook_cursor++]; }
static void select_live_slot() { hook_menu->field_A24_=0; }
static void scenario_enter_reload() {
    Menu m; PullDown p[6]; Win parent; hook_menu=&m; hook_cursor=0;
    for(int i=0;i<6;i++) hook_values[i]=&p[i];
    m.count_=2; m.proc_=callback; m.set_parent(&parent); m.field_A24_=0; m.field_A28_=4;
    p[0].selected_=0; p[0].items_[0].id=10; p[3].selected_=0; p[4].items_[0].id=91;
    m.entries_[0].pull_down=&p[0]; m.entries_[1].pull_down=&p[2];
    AfterFlush=replace_on_flush; AfterTask=replace_on_task; AfterRedraw=replace_on_draw; AfterGetSelected=replace_on_select;
    AfterDraws=replace_on_draw;
    observed_task_count=0; observed_selected=0; last_callback_id=-999;
    assert(m.on_key_click(0,0x0d)==1);
    assert(observed_tasks[0]==&p[1] && observed_tasks[1]==&m);
    assert(observed_selected==&p[3] && last_callback_id==91);
    AfterFlush=0; AfterTask=0; AfterRedraw=0; AfterGetSelected=0; AfterDraws=0;
}
static void scenario_close_key(int key) {
    Menu m; PullDown p[3]; m.field_A24_=0; m.field_A28_=4; m.entries_[0].pull_down=&p[0]; m.entries_[1].pull_down=&p[2];
    hook_menu=&m; hook_cursor=0; hook_values[0]=&p[1]; AfterFlush=replace_on_flush; AfterTask=replace_on_task;
    observed_task_count=0; assert(m.on_key_click(0,key)==1);
    assert(observed_tasks[0]==&p[1] && observed_tasks[1]==&m);
    AfterFlush=0; AfterTask=0;
}
static void scenario_mnemonic_live_show() {
    Menu m; PullDown p[2]; hook_menu=&m; m.count_=2; m.field_A24_=-1;
    char a[]="a", b[]="b"; m.entries_[0].mnemonic=a; m.entries_[1].mnemonic=b;
    m.entries_[0].pull_down=&p[0]; m.entries_[1].pull_down=&p[1];
    m.spot_.used=2; m.spot_.items[1].rect.left=20; m.spot_.items[1].rect.top=0;
    m.spot_.items[1].rect.right=40; m.spot_.items[1].rect.bottom=20;
    AfterRedraw=select_live_slot; observed_shown=0;
    assert(m.on_key_click(0,'b')==1); assert(observed_shown==&p[0]);
    AfterRedraw=0;
}
static void scenario_left_down_slot() {
    Menu m; PullDown p[4]; m.count_=2; m.field_A24_=0; m.field_A28_=4;
    m.entries_[0].pull_down=&p[0]; m.entries_[1].pull_down=&p[2];
    m.spot_.used=2; m.spot_.items[0].pos=0; m.spot_.items[0].rect.left=0; m.spot_.items[0].rect.top=0; m.spot_.items[0].rect.right=20; m.spot_.items[0].rect.bottom=20;
    m.spot_.items[1].pos=1; m.spot_.items[1].rect.left=20; m.spot_.items[1].rect.top=0; m.spot_.items[1].rect.right=40; m.spot_.items[1].rect.bottom=20;
    hook_menu=&m; hook_cursor=0; hook_values[0]=&p[1]; AfterFlush=replace_on_flush;
    AfterTask=replace_on_task; observed_task_count=0; m.on_left_down(1,1);
    assert(observed_tasks[0]==&p[1] && observed_tasks[1]==&m);
    AfterFlush=0; AfterTask=0;
}
static void scenario_left_down_switch_close() {
    Menu m; PullDown p[4]; m.count_=2; m.field_A24_=0; m.field_A28_=4;
    m.entries_[0].pull_down=&p[0]; m.entries_[1].pull_down=&p[2];
    m.spot_.used=2; m.spot_.items[0].pos=0; m.spot_.items[0].rect.left=0; m.spot_.items[0].rect.top=0; m.spot_.items[0].rect.right=20; m.spot_.items[0].rect.bottom=20;
    m.spot_.items[1].pos=1; m.spot_.items[1].rect.left=20; m.spot_.items[1].rect.top=0; m.spot_.items[1].rect.right=40; m.spot_.items[1].rect.bottom=20;
    hook_menu=&m; hook_cursor=0; hook_values[0]=&p[1]; AfterFlush=replace_on_flush;
    AfterTask=replace_on_task; observed_task_count=0; m.on_left_down(21,1);
    assert(observed_tasks[0]==&p[1] && observed_tasks[1]==&m && p[2].shows==1);
    AfterFlush=0; AfterTask=0;
}
'''
    base = base.replace(marker, "\n\n".join(bodies) + "\n\n" + scenarios, 1)
    base = base.replace("Font default_font(8); FontDefault=&default_font;", "Font default_font(8); FontDefault=&default_font;\n    scenario_enter_reload(); scenario_close_key(0x1b); scenario_close_key(0x40012); scenario_mnemonic_live_show(); scenario_left_down_slot(); scenario_left_down_switch_close();", 1)
    source = tmp_path / "menu-live-slot.cpp"
    source.write_text(base)
    return source


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("optimization", ("-O0", "-O2"))
def test_actual_menu_live_slot_reloads(compiler, optimization, tmp_path):
    executable = shutil.which(compiler)
    if not executable:
        pytest.skip(compiler + " unavailable")
    source = actual_fixture(tmp_path)
    binary = tmp_path / "menu-live-slot"
    subprocess.run([executable, "-std=c++03", optimization, "-Wall", "-Wextra", "-Werror", "-pedantic",
                    "-fsanitize=undefined", str(source), "-o", str(binary)], check=True)
    env = dict(os.environ)
    result = subprocess.run([str(binary)], check=True, capture_output=True, text=True, env=env)
    assert result.stderr == "", result.stderr


@pytest.mark.parametrize("damage", ("cached", "recapture", "mnemonic_index", "before_draws"))
def test_live_slot_mutants_are_reached(tmp_path, damage):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("no host C++ compiler")
    source = actual_fixture(tmp_path, damaged=damage)
    binary = tmp_path / ("mutant-" + damage)
    built = subprocess.run([compiler, "-std=c++03", "-O0", "-Wall", "-Wextra", "-Werror", "-pedantic",
                            str(source), "-o", str(binary)], capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True)
    assert result.returncode != 0, damage + " mutant survived runtime controls"
    expected_assertion = {
        "mnemonic_index": "observed_shown==&p[0]",
        "before_draws": "observed_selected==&p[3]",
        "cached": "observed_tasks[0]==&p[1]",
        "recapture": "observed_tasks[0]==&p[1]",
    }[damage]
    assert expected_assertion in result.stderr, (damage, result.returncode, result.stderr)


def test_native_slot_reload_and_order_anchors():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin
    image_path = Path(os.environ.get("OPENSMACX_IMAGE", ""))
    assert image_path.is_absolute() and image_path.is_file(), "OPENSMACX_IMAGE must name the pinned executable explicitly"
    image_bytes = image_path.read_bytes()
    assert hashlib.sha256(image_bytes).hexdigest() == IMAGE_SHA256
    image = PeImage(image_bytes, image_pin(ROOT))
    dis = Cs(CS_ARCH_X86, CS_MODE_32)
    def instructions(span):
        rows = [(i.address, i.mnemonic, i.op_str, i.size) for i in dis.disasm(image.read(span), span.start)]
        cursor = span.start
        for address, _, _, size in rows:
            assert address == cursor
            cursor += size
        assert cursor == span.end
        return rows
    left = instructions(Span(0x5FBC20,0x5FBDAB))
    key = instructions(Span(0x5FBF00,0x5FC21F))
    assert (0x5FBC76,"lea","edi, [esi + eax*4 + 0xa48]") in [r[:3] for r in left]
    assert (0x5FBC9D,"mov","ecx, dword ptr [edi]") in [r[:3] for r in left]
    assert (0x5FBCFF,"mov","ecx, dword ptr [edi]") in [r[:3] for r in left]
    expected = (
        (0x5FBF2A,"mov","ecx, dword ptr [esi + eax*4 + 0xa48]"),
        (0x5FBF31,"lea","edi, [esi + eax*4 + 0xa48]"),
        (0x5FBFA5,"lea","edi, [esi + eax*4 + 0xa48]"),
        (0x5FBF58,"mov","ecx, dword ptr [edi]"),
        (0x5FBFCC,"mov","ecx, dword ptr [edi]"),
        (0x5FBFDD,"call","0x5fcbb0"),
        (0x5FBFE2,"mov","ecx, dword ptr [edi]"),
        (0x5FBFE4,"call","0x5f9f40"),
        (0x5FBFF9,"mov","eax, dword ptr [edi]"),
        (0x5FC14B,"mov","ecx, dword ptr [esi + eax*4 + 0xa48]"),
        (0x5FC152,"lea","edi, [esi + eax*4 + 0xa48]"),
        (0x5FC179,"mov","ecx, dword ptr [edi]"),
        (0x5FC1F7,"mov","eax, dword ptr [esi + 0xa24]"),
        (0x5FC204,"mov","ecx, dword ptr [esi + eax*4 + 0xa48]"),
        (0x5FC20B,"call","0x5f9170"),
    )
    key_ops = [r[:3] for r in key]
    for anchor in expected:
        assert anchor in key_ops, anchor

