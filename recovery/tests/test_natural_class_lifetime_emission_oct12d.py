"""Source-owned controls for four naturally emitted inline destructors.

These checks pin the live C++ lifetime shape and historical source identities.
They do not claim compiler ABI, linked retention, byte equality or runtime
acceptance; fresh physical-emission evidence is coordinator-owned.
"""

from __future__ import annotations

import re
import subprocess
from pathlib import Path

from match_recovery.source import read_file, read_text

ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())
BASELINE = "b19695d0c9d9c55cbb5a5e42ec52670780d8a2f2"
PRESERVED_RECORD_FIELDS = (
    "address", "mode", "state", "name", "image_spans", "symbol", "kind",
    "exclusion", "byte_exact", "semantic", "recipe", "levers", "ruled_out",
    "unrecoverable", "deferred", "source_role", "source_role_reason",
)


def _body(rel: str, signature: str) -> str:
    text = (ROOT / rel).read_text(encoding="utf-8")
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        if text[end] == "{":
            depth += 1
        elif text[end] == "}":
            depth -= 1
            if depth == 0:
                return text[start:end + 1]
    raise AssertionError(f"unterminated source body: {signature}")


def _inline_body(declaration: str, signature: str) -> str:
    match = re.search(r"(?m)^[ \t]*" + re.escape(signature) + r"[ \t]*", declaration)
    assert match, signature
    start = match.start()
    opening = declaration.index("{", match.end())
    depth = 0
    for end in range(opening, len(declaration)):
        if declaration[end] == "{":
            depth += 1
        elif declaration[end] == "}":
            depth -= 1
            if depth == 0:
                return declaration[start:end + 1].strip()
    raise AssertionError(f"unterminated inline destructor: {signature}")


def _class_text(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        if text[end] == "{":
            depth += 1
        elif text[end] == "}":
            depth -= 1
            if depth == 0:
                return text[start:end + 1]
    raise AssertionError(f"unterminated class: {signature}")


def _class(rel: str, signature: str) -> str:
    return _class_text((ROOT / rel).read_text(encoding="utf-8"), signature)


def _code_tokens(text: str) -> list[str]:
    tokens = re.findall(
        r"//[^\n]*|/\*[\s\S]*?\*/|\"(?:\\.|[^\"\\])*\"|"
        r"'(?:\\.|[^'\\])*'|[A-Za-z_]\w*|0[xX][0-9A-Fa-f]+|\d+|"
        r"::|->|&&|\|\||==|!=|<=|>=|\+\+|--|[^\s]",
        text,
    )
    return [token for token in tokens if not token.startswith(("//", "/*"))]


def _normalized_class_tokens(text: str, signature: str, class_name: str) -> list[str]:
    tokens = _code_tokens(_class_text(text, signature))
    marker = ["MEASURED", "~", class_name, "(", ")"]
    indexes = [index for index in range(len(tokens) - len(marker) + 1)
               if tokens[index:index + len(marker)] == marker]
    if indexes:
        assert len(indexes) == 1, (signature, indexes)
        del tokens[indexes[0]]
    return tokens


def _baseline_text(rel: str) -> str:
    completed = subprocess.run(
        ["git", "show", f"{BASELINE}:{rel}"], cwd=ROOT, text=True,
        capture_output=True, timeout=20,
    )
    assert completed.returncode == 0, (
        f"cannot read immutable task baseline {BASELINE}:{rel}: " + completed.stderr
    )
    return completed.stdout


def _baseline_records(rel: str):
    return read_text(_baseline_text(rel), f"{BASELINE}:{rel}")


def _record(rel: str, address: int):
    found = [row for row in read_file(ROOT / rel) if row.address == address]
    assert len(found) == 1, f"expected one original identity {address:#x} in {rel}"
    return found[0]


def _preserved_record(row):
    fields = tuple(getattr(row, name) for name in PRESERVED_RECORD_FIELDS)
    origin = tuple(
        (item.kind, item.key, item.prose, tuple(text for _, text in item.lines))
        for item in row.origin or ()
    )
    return fields + (origin,)


def test_natural_inline_destructor_bodies_and_member_order_remain_source_owned():
    cases = (
        ("src/flatbutton.h", "class FlatButton :", "~FlatButton()", "{ close(); }",
         "public BaseButton", "src/flatbutton.cpp", r"FlatButton::~FlatButton\s*\(",
         "FlatButton"),
        ("src/basepop.h", "class BasePop :", "~BasePop()", "{ close(); }",
         "public GraphicWin", "src/basepop.cpp", r"BasePop::~BasePop\s*\(",
         "BasePop"),
        ("src/mapwin.h", "class MapWin :", "~MapWin()", "{ clear(0); }",
         "public virtual GraphicWin", "src/mapwin.cpp", r"MapWin::~MapWin\s*\(",
         "MapWin"),
        ("src/console.h", "class Console :", "~Console()", "{ close(); }",
         "public MapWin", "src/console.cpp", r"Console::~Console\s*\(",
         "Console"),
    )
    for header, signature, dtor, body, base, owner, out_of_line, class_name in cases:
        declaration = _class(header, signature)
        assert base in declaration
        assert _inline_body(declaration, dtor) == dtor + " " + body
        assert not re.search(r"\bMEASURED\s+" + re.escape(dtor), declaration)
        assert not re.search(r"(?m)^[ \t]*" + out_of_line,
                             (ROOT / owner).read_text(encoding="utf-8"))
        baseline_declaration = _baseline_text(header)
        baseline_tokens = _normalized_class_tokens(baseline_declaration, signature, class_name)
        current_tokens = _normalized_class_tokens(
            (ROOT / header).read_text(encoding="utf-8"), signature, class_name
        )
        assert current_tokens == baseline_tokens, (
            f"{class_name} code, bases, member declarations or order changed"
        )

    # Structural sentinels in declaration order protect the destructor-owned
    # subobjects without copying ignored physical probe inventories into tests.
    ordered_members = (
        ("src/flatbutton.h", "class FlatButton :", (
            "uint32_t field_AB8_;", "uint32_t field_ABC_[3];",
            "BoxSpriteParams *border_styles_[3];", "Sprite *field_AD4_[3];",
            "Sprite *config[27];")),
        ("src/basepop.h", "class BasePop :", (
            "GraphicWin *external_window_;", "FlatButton flat_button1_;",
            "FlatButton flat_button2_;", "FlatButton *item_buttons_[8];",
            "Sprite sprite_;", "StringList item_ids_;", "StringList unit_ids_;",
            "Dialogs dialogs_;", "Spot spot_;")),
        ("src/mapwin.h", "class MapWin :", (
            "TextureStore textures_a_[4];", "TextureStore textures_b_[0xC4];",
            "TextureStore texture_store_;", "Buffer buffer_c_;", "Buffer buffer_b_;",
            "Buffer buffer_a_;", "Font font_a_;", "Font font_b_;", "Font font_c_;",
            "ImageButton image_buttons_[4];")),
        ("src/console.h", "class Console :", (
            "AlphaMenu alpha_menu_;", "Sprite sprite_b_;", "Sprite sprite_a_;")),
    )
    for rel, signature, members in ordered_members:
        declaration = _class(rel, signature)
        positions = [declaration.index(member) for member in members]
        assert positions == sorted(positions), (rel, members)


def test_original_destructor_and_owner_phase_identities_are_preserved():
    destructor_rows = (
        ("src/flatbutton.cpp", 0x00406880, ((0x00406880, 0x004068D8),
                                             (0x006509A0, 0x006509B2)),
         "??1FlatButton@@UAE@XZ", True),
        ("src/basepop.cpp", 0x004064D0, ((0x004064D0, 0x0040666F),
                                          (0x006508D0, 0x00650978)),
         "??1BasePop@@UAE@XZ", False),
        ("src/mapwin.cpp", 0x00420F90, ((0x00420F90, 0x004210CB),
                                         (0x00651190, 0x00651250)),
         "??1MapWin@@UAE@XZ", True),
        ("src/console.cpp", 0x0051D9F0, ((0x0051D9F0, 0x0051DC10),
                                          (0x0065D0E0, 0x0065D220)),
         "??1Console@@UAE@XZ", False),
    )
    for rel, address, spans, symbol, byte_exact in destructor_rows:
        row = _record(rel, address)
        assert row.image_spans == spans
        assert row.symbol == symbol
        assert row.byte_exact is byte_exact
        assert not row.semantic

    # Keep the historical scalar primaries and view-specific adjustors under
    # their exact original identities as well; natural source ownership does
    # not replace or waive any generated-entry restoration obligation.
    deleting_rows = (
        ("src/flatbutton.cpp", 0x004068E0, "??_GFlatButton@@UAEPAXI@Z",
         (0x004068E0, 0x00406901), "??_GFlatButton@@UAEPAXI@Z", True),
        ("src/flatbutton.cpp", 0x004070E0, "??_GFlatButton@@WEEE@AEPAXI@Z",
         (0x004070E0, 0x004070EB), "??_EFlatButton@@WEEE@AEPAXI@Z", True),
        ("src/basepop.cpp", 0x00406B40, "??_GBasePop@@UAEPAXI@Z",
         (0x00406B40, 0x00406B61), "??_GBasePop@@UAEPAXI@Z", True),
        ("src/basepop.cpp", 0x004070B0, "??_GBasePop@@WEEE@AEPAXI@Z",
         (0x004070B0, 0x004070BB), "??_EBasePop@@WEEE@AEPAXI@Z", True),
        ("src/mapwin.cpp", 0x00421830, "??_GMapWin@@UAEPAXI@Z",
         (0x00421830, 0x00421866), None, True),
        ("src/mapwin.cpp", 0x00421820, "??_Gthunk1_MapWin@@UAEPAXI@Z",
         (0x00421820, 0x00421828), "??_EMapWin@@$4PPPPPPPM@A@AEPAXI@Z", False),
        ("src/mapwin.cpp", 0x00421950, "??_Gthunk2_MapWin@@UAEPAXI@Z",
         (0x00421950, 0x00421961), "??_EMapWin@@$4PPPPPLLI@EEE@AEPAXI@Z", False),
        ("src/console.cpp", 0x0051DDF0, "??_GConsole@@UAEPAXI@Z",
         (0x0051DDF0, 0x0051DE26), None, True),
        ("src/console.cpp", 0x0051DDE0, "??_Gthunk1_Console@@UAEPAXI@Z",
         (0x0051DDE0, 0x0051DDE8), "??_EConsole@@$4PPPPPPPM@A@AEPAXI@Z", False),
        ("src/console.cpp", 0x0051DF50, "??_Gthunk2_Console@@UAEPAXI@Z",
         (0x0051DF50, 0x0051DF61), "??_EConsole@@$4PPPPPLLI@EEE@AEPAXI@Z", False),
    )
    for rel, address, name, span, symbol, byte_exact in deleting_rows:
        row = _record(rel, address)
        assert row.name == name
        assert row.image_spans == (span,)
        assert row.byte_exact is byte_exact
        if symbol is not None:
            assert row.symbol == symbol

    # Original constructor and by-name cleanup anchors remain in their actual
    # owners; this guards against replacing source lifetimes with stand-ins.
    owner_rows = (
        ("src/flatbutton.cpp", 0x00607CF0),
        ("src/flatbutton.cpp", 0x00607DA0),
        ("src/basepop.cpp", 0x00600860),
        ("src/basepop.cpp", 0x00600F00),
        ("src/basepop.cpp", 0x00604E40),
        ("src/mapwin.cpp", 0x004626E0),
        ("src/mapwin.cpp", 0x00462870),
        ("src/console.cpp", 0x0050F460),
        ("src/console.cpp", 0x0051D7D0),
    )
    for rel, address in owner_rows:
        _record(rel, address)

    basepop_allocator = _body("src/basepop.cpp", "BasePop *BasePop::basepop_alloc()")
    assert "new BasePop()" in basepop_allocator

    phase_rows = ([(rel, address) for rel, address, *_ in destructor_rows] +
                  [(rel, address) for rel, address, *_ in deleting_rows] +
                  list(owner_rows))
    addresses_by_source = {}
    for rel, address in phase_rows:
        addresses_by_source.setdefault(rel, set()).add(address)
    for rel, addresses in addresses_by_source.items():
        baseline_rows = {row.address: row for row in _baseline_records(rel)}
        current_rows = {row.address: row for row in read_file(ROOT / rel)}
        assert addresses <= baseline_rows.keys(), (BASELINE, rel, addresses)
        assert addresses <= current_rows.keys(), (rel, addresses)
        for address in addresses:
            historical, current = baseline_rows[address], current_rows[address]
            assert _preserved_record(current) == _preserved_record(historical), (
                f"{rel} 0x{address:08X}: historical identity, claims or lesson changed"
            )

