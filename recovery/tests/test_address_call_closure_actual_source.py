from pathlib import Path
import re


ROOT = next(
    parent for parent in Path(__file__).resolve().parents
    if (parent / "src").is_dir() and (parent / "recovery").is_dir()
)


def _function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:index]
    raise AssertionError(f"unterminated function: {signature}")


def test_designwin_uses_typed_source_owned_stats_formatter():
    source = (ROOT / "src/designwin.cpp").read_text()
    veh_header = (ROOT / "src/veh.h").read_text()
    veh_source = (ROOT / "src/veh.cpp").read_text()
    assert "void __cdecl say_stats_3(LPSTR stat, int proto_id);" in veh_header
    assert "void __cdecl say_stats_3(LPSTR stat, int proto_id) {" in veh_source

    draw_stack = _function_body(source, "void DesignWin::draw_stack()")
    draw_flash = _function_body(source, "void DesignWin::draw_flash()")
    assert re.search(
        r"say_stats_3\(StringTemp,\s*\*reinterpret_cast<const short \*>\(\s*"
        r"ProtoAbilityIdBase\s*\+\s*unit_id\s*\*\s*0x34\s*\)\);",
        draw_stack,
    )
    assert "say_stats_3(StringTemp, ability_id);" in draw_flash
    assert "0x0057D7D0" not in draw_stack
    assert "0x0057D7D0" not in draw_flash
    assert "say_stats_3_seam" not in draw_stack + draw_flash


def test_select_part_modal_passes_real_cdecl_wait_callback():
    source = (ROOT / "src/selectpartwin.cpp").read_text()
    faction_header = (ROOT / "src/faction.h").read_text()
    faction_source = (ROOT / "src/faction.cpp").read_text()
    win_header = (ROOT / "src/win.h").read_text()
    assert '#include "faction.h"' in source
    assert "int __cdecl pop_wait();" in faction_header
    assert "int __cdecl pop_wait() {" in faction_source
    assert re.search(r"virtual int set_modal\(int flag, int \(\*poll\)\(\), Win \*owner\);", win_header)
    body = _function_body(source, "int SelectPartWin::exec(")
    assert "set_modal(1, pop_wait, 0);" in body
    assert body.index("scroll_show(this, &scroll_);") < body.index(
        "set_modal(1, pop_wait, 0);") < body.index("scroll_hide(this, &scroll_);")
    assert "SelectPartModalPump" not in source
    assert "(int (*)())0x005398E0" not in source


def test_netwin_calls_owned_string_and_edit_box_members():
    source = (ROOT / "src/netwin.cpp").read_text()
    stringbox_header = (ROOT / "src/stringbox.h").read_text()
    stringbox_source = (ROOT / "src/stringbox.cpp").read_text()
    editbox_header = (ROOT / "src/editbox.h").read_text()
    editbox_source = (ROOT / "src/editbox.cpp").read_text()
    assert "void close();" in stringbox_header
    assert "void StringBox::close() {" in stringbox_source
    assert "void set_text(char *text);" in editbox_header
    assert "void EditBox::set_text(char *text) {" in editbox_source

    assert "static void string_box_close(" not in source
    assert "static void edit_box_set_text(" not in source
    assert "reinterpret_cast<pending>(static_cast<unsigned long>(0x00629210))" not in source
    assert "reinterpret_cast<pending>(static_cast<unsigned long>(0x006151E0))" not in source
    assert source.count("stringBox_.close();") == 4
    assert source.count("editBox_.set_text(0);") == 1
    close_body = _function_body(source, "void NetWin::close()")
    key_body = _function_body(source, "int NetWin::on_key_down(int key)")
    assert "stringBox_.close();" in close_body
    assert close_body.index("spot_.shutdown();") < close_body.index("stringBox_.close();") < close_body.index("editBox_.close();")
    assert "stringBox_.close();" in key_body
    assert "editBox_.set_text(0);" in key_body
    assert key_body.index("spot_.shutdown();") < key_body.index("stringBox_.close();")
    assert key_body.index("message_chat(0x1E00, 0,") < key_body.index("editBox_.set_text(0);")


def test_console_updates_the_real_virtual_graphicwin_view():
    console_source = (ROOT / "src/console.cpp").read_text()
    console_header = (ROOT / "src/console.h").read_text()
    mapwin_header = (ROOT / "src/mapwin.h").read_text()
    graphicwin_header = (ROOT / "src/graphicwin.h").read_text()
    graphicwin_source = (ROOT / "src/graphicwin.cpp").read_text()

    assert "class Console : public MapWin" in console_header
    assert "class MapWin : public virtual GraphicWin" in mapwin_header
    assert "void update(int x, int y, int width, int height, GraphicWin *source);" in graphicwin_header
    body = _function_body(console_source, "void Console::on_nc_mouse_move(")
    assert "GraphicWin::update(0, ConsoleDragLineY," in body
    assert "static_cast<int>(dib_.bmiHeader.biWidth), 1, nullptr);" in body
    assert "GraphicWin::update(ConsoleDragLineX, 0, 1," in body
    assert "-static_cast<int>(dib_.bmiHeader.biHeight), nullptr);" in body
    assert "UpdateRectSeam" not in body
    assert "static_cast<unsigned long>(0x005D55D0)" not in body
    first_update = body.index("GraphicWin::update(0, ConsoleDragLineY,")
    second_update = body.index("GraphicWin::update(ConsoleDragLineX, 0, 1,")
    draw_lines = body.index("window_line_raw(", second_update)
    assert first_update < second_update < draw_lines < body.index("ConsoleDragLineX = line_x;")

    update_body = _function_body(
        graphicwin_source,
        "void GraphicWin::update(int x, int y, int width, int height,",
    )
    calls = ["is_visible()", "update_window(&rect)",
             "client_to_screen(&rect)", "flip(&rect)"]
    positions = [update_body.index(call) for call in calls]
    assert positions == sorted(positions)
