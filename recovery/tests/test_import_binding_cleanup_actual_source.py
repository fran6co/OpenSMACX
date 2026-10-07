"""Live-source controls for direct Windows import API spellings.

These are lexical API/order guards, not executable behavior or ABI proof.
There is no candidate-file fallback: tests read current checkout source.
"""

from pathlib import Path

from match_recovery.source import read_file


def repository_root():
    for path in Path(__file__).resolve().parents:
        if (path / "src/listbox.cpp").is_file():
            return path
    raise AssertionError("could not locate OpenSMACX source root")


ROOT = repository_root()


def read(relative):
    return (ROOT / relative).read_text(encoding="utf-8")


def require_import_contract(sources):
    listbox = sources["src/listbox.cpp"]
    popup_h = sources["src/popup.h"]
    popup = sources["src/popup.cpp"]
    netwin = sources["src/netwin.cpp"]
    mapwin = sources["src/mapwin.cpp"]
    console_h = sources["src/console.h"]
    console = sources["src/console.cpp"]
    veh = sources["src/veh.cpp"]
    stringstruct = sources["src/stringstruct.cpp"]
    win = sources["src/win.cpp"]

    assert "::ReleaseCapture();" in listbox
    assert "release_capture_fn" not in listbox
    assert "0x006692E0" not in listbox

    assert "#include <shellapi.h>" in popup
    assert "::ShellExecuteA(0, PopupLinkOpenVerb, PopupLinkTarget, 0, 0, 1);" in popup
    assert "PopupShellExecuteSlot" not in popup_h + popup
    assert "0x00669268" not in popup_h + popup

    assert "::UnionRect(name_rect, row_rect, &field);" in netwin
    assert "NetWinRectSlot" not in netwin
    assert "rect_slot_fn" not in netwin

    assert "static int *const IAT_GetKeyState" not in mapwin
    assert "::GetKeyState(0x91) & 1" in mapwin
    assert "IAT_GetAsyncKeyState" not in mapwin
    assert "KeyStateFn get_key_state = &::GetAsyncKeyState;" in mapwin
    assert mapwin.index("KeyStateFn get_key_state =") < mapwin.index("get_key_state(0x10)")
    assert "ConsoleEditKeyStateSlot" not in console_h + console + veh
    assert "::GetKeyState(VK_SCROLL) & 1" in console
    assert "::GetKeyState(VK_SHIFT) & 0xff00" in veh

    # StringStruct samples the async state once at its existing call point;
    # MapWin's reused pointer remains a typed local snapshot above.
    assert "IAT_GetAsyncKeyState" not in stringstruct
    assert "::GetAsyncKeyState(0x10) >> 8" in stringstruct

    # The Win message pump deliberately reads the three IAT slots in this
    # declaration order before entering the loop's do_video/check_net calls.
    pump_start = win.index("void __cdecl pump_mouse_messages() {")
    pump_end = win.index("// ===== homed from src/unrecovered/005f4d90.cpp =====", pump_start)
    pump = win[pump_start:pump_end]
    names = [
        "DispatchMessageProc const volatile dispatch = &::DispatchMessageA;",
        "PeekMessageProc const volatile peek = &::PeekMessageA;",
        "TranslateMessageProc const volatile translate = &::TranslateMessage;",
    ]
    assert all(name in pump for name in names)
    positions = [pump.index(name) for name in names]
    assert positions == sorted(positions)
    assert positions[-1] < pump.index("do_video();") < pump.index("check_net();")
    assert "translate(&msg);" in pump and "dispatch(&msg);" in pump
    return True


def test_live_import_api_migration_and_source_identity():
    source_file = ROOT / "src/listbox.cpp"
    records = read_file(source_file)
    assert any(record.address == 0x0060C9B0 for record in records)
    assert any(record.address == 0x004A0810 for record in read_file(ROOT / "src/stringstruct.cpp"))
    sources = {
        relative: read(relative)
        for relative in (
            "src/listbox.cpp", "src/popup.h", "src/popup.cpp", "src/netwin.cpp",
            "src/mapwin.cpp", "src/console.h", "src/console.cpp", "src/veh.cpp",
            "src/stringstruct.cpp", "src/win.cpp",
        )
    }
    assert require_import_contract(sources)


def test_import_migration_mutants_are_detected():
    sources = {
        relative: read(relative)
        for relative in (
            "src/listbox.cpp", "src/popup.h", "src/popup.cpp", "src/netwin.cpp",
            "src/mapwin.cpp", "src/console.h", "src/console.cpp", "src/veh.cpp",
            "src/stringstruct.cpp", "src/win.cpp",
        )
    }
    mutants = (
        ("src/listbox.cpp", "::ReleaseCapture();", "(*reinterpret_cast<int (__stdcall *)()>(0x006692E0))();"),
        ("src/netwin.cpp", "::UnionRect(name_rect, row_rect, &field);", "::UnionRect(row_rect, name_rect, &field);"),
        ("src/mapwin.cpp", "::GetKeyState(0x91) & 1", "::GetAsyncKeyState(0x91) & 1"),
        ("src/stringstruct.cpp", "::GetAsyncKeyState(0x10) >> 8", "::GetKeyState(0x10) >> 8"),
        ("src/win.cpp", "PeekMessageProc const volatile peek = &::PeekMessageA;", "PeekMessageProc const volatile peek = &::PeekMessageW;"),
    )
    for relative, original, replacement in mutants:
        changed = dict(sources)
        assert original in changed[relative]
        changed[relative] = changed[relative].replace(original, replacement, 1)
        try:
            require_import_contract(changed)
        except AssertionError:
            continue
        raise AssertionError("import-binding mutation survived: " + relative)
