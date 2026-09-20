"""BaseButton hide body/slot controls, not complete UI or linked-table proof.

Host execution uses the actual body and selected member declarations with an
explicit Win parent-call double. Native checks use ordinary product TUs; the
four unbounded table contracts below establish slot 2 only, not whole tables.
Run native cases inside an existing match_recovery resource_environment lease.
"""

from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest

from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.compiler import Compiler
from match_recovery.generated_binary import check_generated
from match_recovery.generated_source import GeneratedBinding
from match_recovery.measurement import Measurement
from match_recovery.model import Origin, Span
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.source import read_file
from match_recovery.source.asm import compare_subject, subject_asm
from match_recovery.source_inventory import image_pin


ROOT = Path(__file__).resolve().parents[2]
HIDE = "?hide@BaseButton@@UAEXXZ"
PARENT = "?hide@Win@@UAEXXZ"
TABLES = {
    "BaseButton": ("src/basebutton.cpp", 0x670290, 0x606F30),
    "FlatButton": ("src/flatbutton.cpp", 0x669754, 0x607CF0),
    "ImageButton": ("src/imagebutton.cpp", 0x670A94, 0x6252E0),
    "PushButton": ("src/gamma.cpp", 0x670C60, 0x62C010),
}


def hide_body():
    source = (ROOT / "src/basebutton.cpp").read_text()
    start = source.index("void BaseButton::hide() {")
    brace = source.index("{", start)
    depth = 0
    for end in range(brace, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated hide body")


def run_host(tmp_path, body):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host g++ required")
    header = (ROOT / "src/basebutton.h").read_text()
    declaration = re.search(r"virtual void hide\(\);", header).group()
    members = [re.search(r"\b(?:uint32_t|ButtonGroup \*)\s*" + name + r";", header).group()
               for name in ("field_A9C_", "group_")]
    source = """
        #include <cstdint>
        #include <cstdlib>
        struct ButtonGroup {};
        struct Win { virtual void hide(); };
        struct GraphicWin : Win {};
        struct BaseButton : GraphicWin {
            DECLARATION
            MEMBERS
        };
        int calls, wrong_dispatch;
        BaseButton *expected_receiver;
        uint32_t expected_field;
        void Win::hide() {
            ++calls;
            BaseButton *button = static_cast<BaseButton *>(this);
            if (button != expected_receiver || button->field_A9C_ != expected_field)
                std::abort();
        }
        BODY
        struct Derived : BaseButton { void hide() { ++wrong_dispatch; } };
        int main() {
            ButtonGroup group;
            for (int grouped=0; grouped!=2; ++grouped) {
                BaseButton button;
                button.group_ = grouped ? &group : nullptr;
                button.field_A9C_ = 0x12345678;
                expected_field = grouped ? 0x12345678 : 0;
                expected_receiver = &button;
                calls = 0;
                static_cast<Win *>(&button)->hide();
                if (calls != 1 || button.field_A9C_ != expected_field
                    || button.group_ != (grouped ? &group : nullptr)) return 1;
                // Even with a further override, the actual body must call its
                // qualified parent, after applying its own state transition.
                Derived derived;
                derived.group_ = button.group_;
                derived.field_A9C_ = 0x12345678;
                expected_receiver = &derived;
                calls = wrong_dispatch = 0;
                derived.BaseButton::hide();
                if (calls != 1 || wrong_dispatch != 0) return 2;
            }
        }
    """.replace("DECLARATION", declaration).replace("MEMBERS", "\n".join(members)).replace("BODY", body)
    path = tmp_path / "hide.cpp"
    executable = tmp_path / "hide"
    path.write_text(source)
    result = subprocess.run([compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
                             "-fno-sanitize-recover=undefined", str(path), "-o", str(executable)],
                            capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)


def test_actual_hide_state_and_qualified_parent_call(tmp_path):
    result = run_host(tmp_path, hide_body())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("before,after", [
    ("group_ == 0", "true"),
    ("group_ == 0", "group_ != 0"),
    ("field_A9C_ = 0;", "field_A9C_ = 1;"),
    ("Win::hide();", ""),
    ("Win::hide();", "Win::hide(); Win::hide();"),
])
def test_damaged_hide_is_rejected(tmp_path, before, after):
    body = hide_body()
    assert body.count(before) == 1
    assert run_host(tmp_path, body.replace(before, after)).returncode != 0


@pytest.fixture(scope="module")
def native():
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not os.environ.get("MATCH_RECOVERY_RESOURCE_LEASE"):
        pytest.skip("native checks require an existing compiler resource lease")
    if not path.is_file():
        pytest.skip("pinned original image required")
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    trials = [{unit: compiler(ROOT / unit, (), flags + settings.layout_flags)
               for unit, _, _ in TABLES.values()} for flags in settings.flag_sets]
    return compiler, settings, trials, PeImage(path.read_bytes(), image_pin(ROOT)), path


def binding(image):
    row, = (r for r in read_file(ROOT / "src/basebutton.cpp") if r.address == 0x406860)
    assert row.symbol == HIDE and row.image_spans == ((0x406860, 0x406879),)
    return FunctionBinding(Origin(image_pin(ROOT), "pe:va", row.address),
                           (Span(0x406860, 0x406879),), HIDE, "src/basebutton.cpp")


def table_rows(objects, image):
    contracts = [GeneratedBinding(Origin(image_pin(ROOT), "pe:va", address), None,
                    f"??_7{name}@@6BWin@@@", Origin(image_pin(ROOT), "pe:va", anchor), unit, 1)
                 for name, (unit, address, anchor) in TABLES.items()]
    _, rows = check_generated(contracts, objects, image, {HIDE: 0x406860})
    assert {r["symbol"] for r in rows} == {b.symbol for b in contracts}
    for row in rows:
        entry = row["diagnostic"]["entries"][2]
        assert entry["original_absolute_relocation"]
        assert entry["observation"] == "target_word_agrees", entry
        assert [r["symbol"] for r in entry["relocations"]] == [HIDE]
        assert row["content"] == "unknown"  # No complete-table claim.
    return rows


def test_full_original_body_and_parent_reference(native):
    _, _, trials, image, _ = native
    # Original complete five-instruction body independently identifies the
    # qualified parent target; the retained tree's lost Win fact is not used.
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    code = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(image.read(Span(0x406860, 0x406879)), 0x406860))
    assert [(i.mnemonic, i.op_str) for i in code] == [
        ("mov", "eax, dword ptr [ecx + 0xaa8]"), ("test", "eax, eax"),
        ("jne", "0x406874"), ("mov", "dword ptr [ecx + 0xa9c], 0"), ("jmp", "0x5edcd0")]
    reports = [check_objects([binding(image)], {u: Coff(b) for u, b in trial.items()}, image,
                              external_targets={PARENT: 0x5EDCD0})[0] for trial in trials]
    assert any(r["boundary"] == r["bytes"] == r["references"] == "pass" for r in reports), reports
    bad = [check_objects([binding(image)], {u: Coff(b) for u, b in trial.items()}, image,
                        external_targets={PARENT: 0x5ED9D0})[0] for trial in trials]
    assert all(r["references"] != "pass" for r in bad)


def test_original_and_all_emitted_derived_hide_slots(native):
    _, _, trials, image, _ = native
    for _, address, _ in TABLES.values():
        assert int.from_bytes(image.read(Span(address + 8, address + 12)), "little") == 0x406860
    for trial in trials:
        table_rows({u: Coff(b) for u, b in trial.items()}, image)


def test_removing_override_breaks_native_table_contract(native):
    compiler, settings, trials, image, _ = native
    header = (ROOT / "src/basebutton.h").read_bytes()
    source = (ROOT / "src/basebutton.cpp").read_bytes()
    assert header.count(b"  virtual void hide();") == 1
    overrides = {"src/basebutton.h": header.replace(b"  virtual void hide();", b""),
                 "src/basebutton.cpp": source.replace(hide_body().encode(), b"")}
    for flags, pristine in zip(settings.flag_sets, trials):
        objects = {u: Coff(b) for u, b in pristine.items()}
        table_rows(objects, image)
        objects["src/basebutton.cpp"] = Coff(compiler.overlay(ROOT / "src/basebutton.cpp",
                                          overrides, (), flags + settings.layout_flags))
        with pytest.raises(AssertionError):
            table_rows(objects, image)


def test_protected_family_claims_keep_legacy_byte_matches(native):
    _, _, trials, _, path = native
    # Historical masked-byte obligations, deliberately distinct from the
    # strict reference-aware check of hide above.
    for unit, _, _ in TABLES.values():
        for record in read_file(ROOT / unit):
            if not record.byte_exact:
                continue
            results = [compare_subject(record, path, subject_asm(t[unit], record)) for t in trials]
            assert any(r.verdict == "BYTE_EXACT" for r in results), (unit, record.name, results)
