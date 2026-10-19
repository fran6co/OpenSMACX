"""Reviewed source-object starts and SDK identities; no runtime acceptance."""

from dataclasses import replace
from functools import lru_cache
import hashlib
import io
import os
from pathlib import Path
import subprocess
import tarfile
from types import SimpleNamespace

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86_const import X86_OP_IMM, X86_OP_MEM

from match_recovery.component_source import parse_component
from match_recovery.import_source import import_targets, parse_imports, read_imports
from match_recovery.model import Span
from match_recovery.object_source import parse_objects, read_objects
from match_recovery.pe_coff import PeImage
from match_recovery.source_config import SourceConfiguration


ROOT = Path(__file__).resolve().parents[2]
BASE = "40c2f03d6e38ec63f7fd6ee6797667dd01b92e19"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
COMPANION = "src/vendor/vc6-runtime.recovery.h"
# Existing source labels; witnesses establish starts/slots, never allocation ends.
# unit, symbol, original storage start, independent original operand sites
OBJECTS = (
    ("src/win.cpp", "?WinMouseDirect@@3HA", 0x009B7AA4, (0x005F2266, 0x005F2309, 0x005F2352,)),
    ("src/game.cpp", "?IsMultiplayerPBEM@@3HA", 0x0093A95C, (0x00442721, 0x004713B3, 0x00539178,)),
    ("src/faction.cpp", "?PopupDialogFactionID@@3HA", 0x0093F7CC, (0x0053CB09, 0x0053E4CD, 0x005BFA11,)),
    ("src/time.cpp", "?TimerHoldFlag@@3HA", 0x00915620, (0x0050EE4C, 0x0050EE89, 0x0050EF19,)),
    ("src/win.cpp", "?WinScreenHeight@@3HA", 0x009B7B20, (0x0050E925, 0x005D559B, 0x005EFD14,)),
    ("src/basebutton.cpp", "?BaseButtonA94Default@@3IA", 0x009B8E2C, (0x00606FC9, 0x006070FE,)),
    ("src/basebutton.cpp", "?BaseButtonA98Default@@3IA", 0x009B8E30, (0x00607003, 0x00607138,)),
    ("src/basepop.cpp", "?BasePopFalloutFlag@@3HA", 0x009B8CFC, (0x00604A80, 0x00604A99,)),
    ("src/buffer.cpp", "?BufferField520Default@@3IA", 0x00696BF0, (0x005D72B7, 0x005D75C9,)),
    ("src/jackal/jackal.cpp", "?JackalInitFlags@@3HA", 0x009BC4B0, (0x0062D3C1, 0x0062D555,)),
    ("src/listbox.cpp", "?ListBoxClickGuard@@3HA", 0x009B8EEC, (0x0060AA20, 0x0060C6D0,)),
    ("src/faction.cpp", "?PopupWaitActive@@3HA", 0x0093FAC4, (0x005398F5, 0x00539942,)),
    ("src/scenario_undo.cpp", "?ScenEditorUndoPosition@@3IA", 0x00690D7C, (0x005ABE4E, 0x005ABF37,)),
    ("src/scroll.cpp", "?ScrollNonClientInit@@3HA", 0x009B8E24, (0x00605914, 0x00605964,)),
    ("src/win.cpp", "?WinModalDepth@@3HA", 0x009B7AE4, (0x0050E8A0, 0x005EE280,)),
    ("src/win.cpp", "?WinSizingFlag@@3HA", 0x009B7AD4, (0x00607832, 0x0060867A,)),
    ("src/time.cpp", "?TimeModal@Time@@2PAV1@A", 0x009B90D0, (0x006167E9, 0x00616829, 0x00616860,)),
    ("src/time.cpp", "?TimeInitCount@Time@@2HA", 0x009B90CC, (0x00616880, 0x00616890,)),
    ("src/win.cpp", "?WinTrackingWindow@@3PAVWin@@A", 0x009B7AAC, (0x005F2290, 0x005F2502, 0x005F257A,)),
    ("src/win.cpp", "?WinFocusWindow@@3PAVWin@@A", 0x009B7AE0, (0x005EE314, 0x0063C390, 0x0063C4E8,)),
    ("src/win.cpp", "?WinMessageHook@@3P6AXXZA", 0x009B7A88, (0x00445BC1, 0x005F255B, 0x005F2613,)),
    ("src/win.cpp", "?WinLeftDownHook@@3P6AXHH@ZA", 0x009B7A90, (0x005EE79A, 0x005F2543,)),
    ("src/net_class.cpp", "?ServiceEnumTarget@@3PAUServiceList@@A", 0x009BE5F0, (0x0062E279, 0x00630C09,)),
    ("src/stringstruct.cpp", "?StringAllocationHeap@@3PAVHeap@@A", 0x009B3374, (0x00401020, 0x00402319,)),
    ("src/pulldown.cpp", "?PullDownFieldF38Default@@3P6AHPAVGraphicWin@@HPAUtagRECT@@@ZA", 0x009B7B58, (0x005F8878, 0x005F89AF,)),
    ("src/pulldown.cpp", "?PullDownFieldF3CDefault@@3P6AHPAVGraphicWin@@@ZA", 0x009B7B5C, (0x005F8883, 0x005F89BA,)),
    ("src/general.cpp", "?SinTable@@3PAHA", 0x009BE6DC, (0x0063B950, 0x0063B9E0,)),
    ("src/veh.cpp", "?VehBattleModCount@@3PAIA", 0x00915614, (0x00501D32, 0x00501D5D,)),
    ("src/alpha.cpp", "?SocialCategories@@3PAURulesSocialCategory@@A", 0x0094B000, (0x00590A21,)),
    ("src/random.cpp", "?Rand@@3VRandom@@A", 0x009BB568, (0x00625705, 0x00625720, 0x006257E0,)),
    ("src/path.cpp", "?Paths@@3VPath@@A", 0x00945B00, (0x004E12D8, 0x0059A1C0, 0x005C5A5E,)),
    ("src/veh.cpp", "?Reactor@@3PAURulesReactor@@A", 0x009527F8, (0x0042A910,)),
    ("src/console.cpp", "?GoTimer@@3VTime@@A", 0x00939E60, (0x0050E9CE, 0x0050E9FA,)),
    ("src/console.cpp", "?TurnTimer@@3VTime@@A", 0x00915628, (0x0050E9D8, 0x0050E9F0, 0x0050F426,)),
)
# owner, emitted SDK pointer, DLL/export, original IAT slot, independent operand
IMPORTS = (
    ("src/jackal/font.cpp", "__imp__AddFontResourceA@4", "GDI32.dll", "AddFontResourceA", 0x00669090, 0x006191BB),
    ("src/jackal/palette.cpp", "__imp__AnimatePalette@16", "GDI32.dll", "AnimatePalette", 0x006690A8, 0x005FE4D5),
    ("src/jackal/font.cpp", "__imp__CreateCompatibleDC@4", "GDI32.dll", "CreateCompatibleDC", 0x00669070, 0x006195D2),
    ("src/jackal/font.cpp", "__imp__CreateScalableFontResourceA@16", "GDI32.dll", "CreateScalableFontResourceA", 0x0066908C, 0x006191B1),
    ("src/win.cpp", "__imp__DestroyWindow@4", "USER32.dll", "DestroyWindow", 0x0066930C, 0x005F04EA),
    ("src/jackal/font.cpp", "__imp__GetCurrentDirectoryA@8", "KERNEL32.dll", "GetCurrentDirectoryA", 0x006690E4, 0x006191A0),
    ("src/console_commands.cpp", "__imp__GetKeyState@4", "USER32.dll", "GetKeyState", 0x0066932C, 0x004E1F51),
    ("src/jackal/palette.cpp", "__imp__GetSystemPaletteEntries@16", "GDI32.dll", "GetSystemPaletteEntries", 0x006690AC, 0x005FE357),
    ("src/win.cpp", "__imp__LineTo@12", "GDI32.dll", "LineTo", 0x006690C4, 0x005EF2AE),
    ("src/general.cpp", "__imp__MessageBoxA@16", "USER32.dll", "MessageBoxA", 0x00669318, 0x005D452E),
    ("src/win.cpp", "__imp__MoveToEx@16", "GDI32.dll", "MoveToEx", 0x00669048, 0x005EF297),
    ("src/win.cpp", "__imp__RealizePalette@4", "GDI32.dll", "RealizePalette", 0x0066909C, 0x005F10EB),
    ("src/game.cpp", "__imp__RegCloseKey@4", "ADVAPI32.dll", "RegCloseKey", 0x00669010, 0x0051E07C),
    ("src/game.cpp", "__imp__RegCreateKeyExA@36", "ADVAPI32.dll", "RegCreateKeyExA", 0x00669004, 0x0051E026),
    ("src/game.cpp", "__imp__RegOpenKeyExA@20", "ADVAPI32.dll", "RegOpenKeyExA", 0x0066900C, 0x0052A9C9),
    ("src/game.cpp", "__imp__RegQueryValueExA@24", "ADVAPI32.dll", "RegQueryValueExA", 0x00669000, 0x0051E034),
    ("src/game.cpp", "__imp__RegSetValueExA@24", "ADVAPI32.dll", "RegSetValueExA", 0x00669008, 0x0051E0C2),
    ("src/win.cpp", "__imp__SelectPalette@12", "GDI32.dll", "SelectPalette", 0x006690B8, 0x005F10DE),
    ("src/console.cpp", "__imp__SetCursorPos@8", "USER32.dll", "SetCursorPos", 0x00669310, 0x0050E939),
    ("src/win.cpp", "__imp__UnregisterClassA@8", "USER32.dll", "UnregisterClassA", 0x006692A4, 0x005F050F),
    ("src/vendor/vc6-runtime.recovery.h", "__imp__TlsSetValue@8", "KERNEL32.dll", "TlsSetValue", 0x006691C4, 0x00647004),
)


def _objects(text, unit):
    return parse_objects(text, unit, PIN, address_space="pe:va")


def _require_owner_objects(text, unit):
    rows = _objects(text, unit)
    for owner, symbol, address, _ in OBJECTS:
        if owner != unit:
            continue
        row, = [item for item in rows if item.symbol == symbol]
        assert row.origin.address == address
        assert row.origin.image_sha256 == PIN and row.origin.address_space == "pe:va"
        assert row.parent_symbol is None and row.target_symbol is None
        assert row.span is None and row.evidence is None and row.member_path == ()


def _require_owner_imports(text, unit):
    rows = parse_imports(text, unit)
    assert len({item.symbol for item in rows}) == len(rows)
    for owner, symbol, module, name, _, _ in IMPORTS:
        if owner != unit:
            continue
        row, = [item for item in rows if item.symbol == symbol]
        assert (row.module, row.name, row.ordinal) == (module, name, None)


@lru_cache(maxsize=1)
def _baseline_sources():
    # Read committed source in memory; no checkout, temporary Git state or writes.
    archive = subprocess.check_output(["git", "archive", BASE, "src"], cwd=ROOT)
    config = SourceConfiguration.read(ROOT)
    with tarfile.open(fileobj=io.BytesIO(archive)) as bundle:
        return {item.name: bundle.extractfile(item).read().decode("utf-8")
                for item in bundle.getmembers() if item.isfile()
                and Path(item.name).suffix in config.extensions
                and not config.is_seed(item.name)}


def test_actual_global_readers_keep_each_reviewed_fact_unique_and_source_owned():
    objects = read_objects(ROOT, PIN)
    imports = read_imports(ROOT)
    for owner, symbol, address, _ in OBJECTS:
        row, = [item for item in objects if item.symbol == symbol]
        assert (row.unit, row.origin.address) == (owner, address)
        _require_owner_objects((ROOT / owner).read_text(), owner)
    for owner, symbol, module, name, _, _ in IMPORTS:
        row, = [item for item in imports if item.symbol == symbol]
        assert (row.path, row.module, row.name, row.ordinal) == (owner, module, name, None)
        _require_owner_imports((ROOT / owner).read_text(), owner)

    timers = (ROOT / "src/console.cpp").read_text()
    shared = timers.index("Time BlinkTimer,")
    for symbol in ("?GoTimer@@3VTime@@A", "?TurnTimer@@3VTime@@A"):
        row, = [item for item in _objects(timers, "src/console.cpp") if item.symbol == symbol]
        assert timers.index(f"// RECOVERY_OBJECT: v1 0x{row.origin.address:08X} {symbol}") < shared
    companion = (ROOT / COMPANION).read_text()
    assert companion.index("// RECOVERY_IMPORT: v1 __imp__TlsSetValue@8 ") > companion.index(
        "// RECOVERY_COMPONENT_END")


def test_prior_object_and_import_facts_survive_semantically_without_total_count_freeze():
    current = {item.symbol: item for item in read_objects(ROOT, PIN)}
    imports = {item.symbol: item for item in read_imports(ROOT)}
    for unit, text in _baseline_sources().items():
        for old in _objects(text, unit):
            now = current[old.symbol]
            assert now.origin == old.origin
            assert (now.parent_symbol, now.member_path, now.target_symbol) == (
                old.parent_symbol, old.member_path, old.target_symbol)
            if old.span is not None:
                assert (now.span, now.evidence) == (old.span, old.evidence)
        for old in parse_imports(text, unit):
            now = imports[old.symbol]
            assert (now.module.casefold(), now.name, now.ordinal) == (
                old.module.casefold(), old.name, old.ordinal)


def _core(binding):
    # Prose and future extra entries do not replace the original physical facts.
    changes = {"evidence": "independent evidence"}
    if hasattr(binding, "entries"):
        changes["entries"] = ()
    return replace(binding, **changes)


def test_runtime_manifest_keeps_all_previous_physical_bindings():
    old = parse_component(_baseline_sources()[COMPANION])
    now = parse_component((ROOT / COMPANION).read_text())
    assert (now.component.component_id, now.component.ownership, now.component.delivery,
            now.archive_sha256, now.license_evidence) == (
        old.component.component_id, old.component.ownership, old.component.delivery,
        old.archive_sha256, old.license_evidence)
    for field in ("bindings", "absolute_symbols", "data_sections", "code_sections"):
        current = getattr(now, field)
        for previous in getattr(old, field):
            matches = [item for item in current if _core(item) == _core(previous)]
            item, = matches
            if hasattr(previous, "entries"):
                for entry in previous.entries:
                    assert _core(entry) in {_core(value) for value in item.entries}


@pytest.mark.parametrize("kind", ["object", "import"])
@pytest.mark.parametrize("damage", ["missing", "wrong", "duplicate"])
def test_missing_wrong_and_duplicate_reviewed_facts_refuse(kind, damage):
    cases = OBJECTS if kind == "object" else IMPORTS
    for case in cases:
        unit, symbol = case[:2]
        text = (ROOT / unit).read_text()
        prefix = "// RECOVERY_OBJECT:" if kind == "object" else "// RECOVERY_IMPORT:"
        marker, = [line for line in text.splitlines()
                   if line.startswith(prefix) and symbol in line.split()]
        if damage == "missing":
            changed = text.replace(marker + "\n", "", 1)
        elif damage == "duplicate":
            changed = text.replace(marker, marker + "\n" + marker, 1)
        else:
            wrong = (marker.replace(f"0x{case[2]:08X}", f"0x{case[2] + 4:08X}")
                     if kind == "object" else marker.replace("name:", "name:wrong_"))
            changed = text.replace(marker, wrong, 1)
        check = _require_owner_objects if kind == "object" else _require_owner_imports
        with pytest.raises((AssertionError, ValueError)):
            check(changed, unit)


def test_valid_syntax_unproved_v2_promotion_cannot_replace_a_reviewed_start_only_fact():
    unit, symbol, address, _ = OBJECTS[0]
    text = (ROOT / unit).read_text()
    marker = f"// RECOVERY_OBJECT: v1 0x{address:08X} {symbol}"
    promoted = (f"// RECOVERY_OBJECT: v2 0x{address:08X} 0x{address + 4:08X} "
                f"{symbol} 0x005F2290")
    changed = text.replace(marker, promoted, 1)
    row, = [item for item in _objects(changed, unit) if item.symbol == symbol]
    assert row.origin.address == address
    assert row.span == Span(address, address + 4) and row.evidence is not None
    with pytest.raises(AssertionError):
        _require_owner_objects(changed, unit)


@pytest.mark.parametrize("reader, marker, unit", [
    (read_objects, "// RECOVERY_OBJECT: v1 0x009B7AA4 ?WinMouseDirect@@3HA", "src/win.cpp"),
    (read_imports, "// RECOVERY_IMPORT: v1 __imp__TlsSetValue@8 KERNEL32.dll name:TlsSetValue",
     COMPANION),
])
def test_native_global_readers_reject_duplicate_facts_across_actual_source(
        monkeypatch, reader, marker, unit):
    original = Path.read_text
    def damaged(path, *args, **kwargs):
        text = original(path, *args, **kwargs)
        if path.resolve() == (ROOT / unit).resolve():
            return text + "\n" + marker + "\n"
        return text
    monkeypatch.setattr(Path, "read_text", damaged)
    with pytest.raises(ValueError, match="duplicate"):
        reader(ROOT, PIN) if reader is read_objects else reader(ROOT)


def _repository_root():
    common = subprocess.check_output(["git", "rev-parse", "--git-common-dir"],
                                     cwd=ROOT, text=True).strip()
    return (ROOT / common).resolve().parent


def _image_pin():
    fields = [line.split() for line in (ROOT / "src/IMAGE_PIN").read_text().splitlines()
              if line.strip().startswith("sha256 ")]
    if fields != [["sha256", PIN]]:
        raise ValueError("src/IMAGE_PIN disagrees with the independent task pin")
    return PIN


def _image_path():
    explicit = os.environ.get("OPENSMACX_IMAGE")
    if explicit is not None:
        path = Path(explicit)
        if not path.is_absolute() or not path.is_file():
            raise ValueError("explicit OPENSMACX_IMAGE is missing or not absolute")
        return path
    for root in (_repository_root(), ROOT):
        path = root / ".opensmacx/game/terranx_original.exe"
        if path.is_file():
            return path
    raise ValueError("pinned original image is unavailable")


def _load_image(path):
    if not path.is_file():
        raise ValueError("pinned original image is missing")
    data = path.read_bytes()
    if hashlib.sha256(data).hexdigest() != _image_pin():
        raise ValueError("pinned original image SHA-256 mismatch")
    return PeImage(data, PIN)


@pytest.fixture(scope="module")
def image():
    return _load_image(_image_path())


def test_explicit_missing_wrong_corrupt_pins_refuse_and_fallback_is_real(tmp_path, monkeypatch):
    raw = _image_path().read_bytes()
    monkeypatch.setenv("OPENSMACX_IMAGE", str(tmp_path / "absent.exe"))
    with pytest.raises(ValueError, match="explicit"):
        _image_path()
    for name, data in (("wrong.exe", b"not the original image"),
                       ("corrupt.exe", bytes([raw[0] ^ 1]) + raw[1:])):
        path = tmp_path / name
        path.write_bytes(data)
        monkeypatch.setenv("OPENSMACX_IMAGE", str(path))
        with pytest.raises(ValueError, match="SHA-256"):
            _load_image(_image_path())
    monkeypatch.delenv("OPENSMACX_IMAGE")
    monkeypatch.setitem(_image_path.__globals__, "_repository_root",
                        lambda: tmp_path / "missing-outer")
    monkeypatch.setitem(_image_path.__globals__, "ROOT", tmp_path)
    local = tmp_path / ".opensmacx/game/terranx_original.exe"
    local.parent.mkdir(parents=True)
    local.write_bytes(raw)
    pin = tmp_path / "src/IMAGE_PIN"
    pin.parent.mkdir()
    pin.write_text("sha256 " + PIN + "\n")
    assert _image_path() == local
    _load_image(local)
    pin.write_text("sha256 " + "0" * 64 + "\n")
    with pytest.raises(ValueError, match="IMAGE_PIN"):
        _load_image(local)
    local.unlink()
    with pytest.raises(ValueError, match="unavailable"):
        _image_path()


def _operand(image, pc, target):
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    instruction = next(decoder.disasm(image.read(Span(pc, pc + 15)), pc))
    matches = []
    for operand in instruction.operands:
        if operand.type == X86_OP_MEM and operand.mem.disp == target:
            matches.append(instruction.disp_offset)
        elif operand.type == X86_OP_IMM and operand.imm == target:
            matches.append(instruction.imm_offset)
    field, = matches
    assert field > 0
    assert field in image.absolute_sites(Span(pc, pc + instruction.size))
    return instruction


@pytest.mark.parametrize("unit,symbol,address,sites", OBJECTS,
                         ids=[case[1] for case in OBJECTS])
def test_independently_pinned_object_start_operands_have_highlow(image, unit, symbol, address, sites):
    for pc in sites:
        _operand(image, pc, address)
    # A nearby value cannot be blessed by a declaration or current COFF name.
    with pytest.raises((AssertionError, ValueError)):
        _operand(image, sites[0], address + 4)


def _ordinary_import(symbol):
    # Resolver control only; fresh application COFF/ABI proof is coordinator-owned.
    return SimpleNamespace(symbols={1: SimpleNamespace(
        name=symbol, storage=2, section=0, value=0)},
        sections=[SimpleNamespace(relocations=[(0, 1, 6)])])


@pytest.mark.parametrize("owner,symbol,module,name,slot,pc", IMPORTS,
                         ids=[case[1] for case in IMPORTS])
def test_pinned_unique_sdk_slots_and_strict_identity_refusals(
        image, owner, symbol, module, name, slot, pc):
    binding, = [item for item in read_imports(ROOT) if item.symbol == symbol]
    original, = [item for item in image.imports()
                 if item.module.casefold() == module.casefold()
                 and item.name == name and item.ordinal is None]
    assert original.address == slot
    instruction = _operand(image, pc, slot)
    assert instruction.mnemonic in ("call", "mov", "jmp")
    objects = {owner: _ordinary_import(symbol)}
    targets, reports = import_targets((binding,), objects, image)
    assert targets == {symbol: slot}
    assert reports[0]["identity"] == "pass" and reports[0]["link_selection"] == "unknown"
    ambiguous = SimpleNamespace(imports=lambda: (*image.imports(), original))
    targets, reports = import_targets((binding,), objects, ambiguous)
    assert targets == {} and reports[0]["identity"] == "unknown"
    for wrong in (replace(binding, module="not-the-pinned-dll.dll"),
                  replace(binding, name="not_the_pinned_export")):
        targets, reports = import_targets((wrong,), objects, image)
        assert targets == {} and reports[0]["identity"] == "unknown"
    with pytest.raises(ValueError, match="duplicate"):
        import_targets((binding, binding), objects, image)
    for change in ({"storage": 105}, {"section": 1}, {"value": 4}):
        reference = _ordinary_import(symbol)
        for key, value in change.items():
            setattr(reference.symbols[1], key, value)
        targets, reports = import_targets((binding,), {owner: reference}, image)
        assert targets == {} and reports[0]["identity"] == "unknown"

