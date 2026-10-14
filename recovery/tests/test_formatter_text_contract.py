"""Source-level contract controls for formatter output buffers.

These checks establish the C++ pointer type and callback propagation only;
they are not native ABI or game-acceptance evidence.
"""
from pathlib import Path

from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = Path(__file__).resolve().parents[2]


def code_only(text: str) -> str:
    return COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if c == "\n" else " " for c in match.group(0)),
        text,
    )


def function_body(source: str, signature: str) -> str:
    code = code_only(source)
    assert code.count(signature) == 1, signature
    start = code.index(signature)
    opening = code.index("{", start + len(signature))
    depth = 0
    for pos in range(opening, len(code)):
        if code[pos] == "{":
            depth += 1
        elif code[pos] == "}":
            depth -= 1
            if depth == 0:
                return code[start:pos + 1]
    raise AssertionError(f"unterminated function: {signature}")


def test_formatter_api_uses_character_destinations() -> None:
    strings_h = code_only((ROOT / "src/strings.h").read_text())
    map_cpp = code_only((ROOT / "src/map.cpp").read_text())
    declarations = (
        "void __cdecl say_reactor(char *output, int reactor_id, int with_detail);",
        "void __cdecl say_weapon(char *output, int weapon_id, int with_detail);",
        "void __cdecl say_armor(char *output, int armor_id, int with_detail);",
        "void __cdecl say_chassis(char *output, int chassis_id, int with_detail);",
        "void __cdecl say_techs(char *output, int tech_id, int with_detail);",
        "void __cdecl say_facility(char *output, int index, int detail);",
        "void __cdecl say_ability(char *output, int index, int detail);",
        "void __cdecl say_society(char *output, int index, int detail);",
        "void __cdecl say_vehicles(char *output, int index, int detail);",
        "void __cdecl say_project(char *output, int index, int detail);",
    )
    assert all(declaration in strings_h for declaration in declarations)
    assert "void __cdecl say_vehicles(char *text, int chassis_id, int with_height)" in map_cpp
    assert "void __cdecl say_project(char *output, int project_id, int with_cost)" in map_cpp
    assert "void *output" not in strings_h + map_cpp


def test_datalink_callback_keeps_typed_output_through_dispatch() -> None:
    header = code_only((ROOT / "src/datalink.h").read_text())
    source = (ROOT / "src/datalink.cpp").read_text()
    assert "typedef void (__cdecl *DatalinkTextCallback)(char *, int, int);" in header
    assert "void invoke_text(char *output, int value, int extra) const;" in header
    selection = function_body(source, "void DatalinkSelectionEntry::append_text(char *output, int extra) const")
    dispatch = function_body(source, "void DatalinkEntryCallbacks::invoke_text(char *output, int value, int extra) const")
    assert "callbacks.append_text(output, value, extra);" in selection
    assert "append_text(output, value, extra);" in dispatch
    queue_dispatch = function_body(source, "void DatalinkSelectionQueue::dispatch_matching(int entry_id)")
    assert "entry.append_text(StringTemp, 1);" in queue_dispatch

