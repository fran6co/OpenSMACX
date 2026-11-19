"""Post-link application of declared image patches; synthetic PE, MAP and source only."""

import json

import pytest

from match_recovery.cli import main
from match_recovery.image_patch_apply import apply_image_patches, declared_patches
from test_link_map import executable

ORIGINAL = 0x00600400            # where the pinned image has the function
BUILT = 0x00401010               # where this link placed it
# push esi; mov esi, ecx | push 0; push 0; mov ecx, esi; call rel32 | pop esi; ret
PREFIX_CODE = bytes.fromhex("56" "8BF1")
CALL = bytes.fromhex("6A00" "6A00" "8BCE" "E8D0FFFFFF")       # 11 bytes at +3
SUFFIX_CODE = bytes.fromhex("5E" "C3")
FUNCTION = PREFIX_CODE + CALL + SUFFIX_CODE
START, END = ORIGINAL + 3, ORIGINAL + 3 + len(CALL)
FILE_AT = 512 + 0x10 + 3
NOP = f"0x{START:08X} 0x{END:08X} nop"
MAP = ("demo\n Preferred load address is 00400000\n"
       " Address         Publics by Value              Rva+Base     Lib:Object\n\n"
       " 0001:00000000 _start 00401000 f a.obj\n"
       " 0001:00000010 ?f@C@@QAEXXZ 00401010 f a.obj\n"
       " entry point at 0001:00000000\n Static symbols\n")


def project(tmp_path, *declarations, code=FUNCTION, mapping=MAP, absolute_site=None,
            span_end=ORIGINAL + len(FUNCTION)):
    for name in ("code", "recovery", "build"):
        (tmp_path / name).mkdir(exist_ok=True)
    (tmp_path / "code/PIN").write_text("sha256 " + "0" * 64 + "\n")
    (tmp_path / "recovery/project.json").write_text(json.dumps({
        "schema_version": 1,
        "source": {"image_pin": "code/PIN", "address_space": "pe:va", "roots": ["code"],
                   "seed_roots": ["code/seeds"], "include_paths": ["code"]}}))
    (tmp_path / "code/f.cc").write_text(
        "".join(f"// RECOVERY_IMAGE_PATCH: v1 {text}\n" for text in declarations)
        + f"// ORIGINAL: 0x{ORIGINAL:08X} ?f@C@@QAEXXZ 0x{ORIGINAL:08X}-0x{span_end:08X}\n"
        "void C::f() {\n}\n")
    exe = tmp_path / "build/app.exe"
    exe.write_bytes(executable(b"\xc3" + b"\xcc" * 15 + code, absolute_site=absolute_site))
    (tmp_path / "build/app.map").write_text(mapping)
    return exe, tmp_path / "build/app.map", tmp_path / "build/app.patched.exe"


def run(tmp_path, *declarations, **options):
    exe, mapping, out = project(tmp_path, *declarations, **options)
    before = exe.read_bytes()
    report = apply_image_patches(tmp_path, exe, mapping, out)
    assert exe.read_bytes() == before, "the linked executable is never modified"
    return report, before, out


def test_nop_patch_is_written_at_the_built_address_and_nothing_else_changes(tmp_path):
    report, before, out = run(tmp_path, NOP)
    assert report["result"] == "pass" and report["written"] and report["changed_bytes"] == len(CALL)
    row, = report["patches"]
    assert (row["built_start"], row["built_end"]) == (f"0x{BUILT + 3:08X}", f"0x{BUILT + 14:08X}")
    assert row["offset"] == FILE_AT and row["replaced_bytes"] == CALL.hex()
    assert [line.split("  ")[-1].strip() for line in row["replaced_instructions"]] == [
        "push 0", "push 0", "mov ecx, esi", "call 0x400fee"]
    after = out.read_bytes()
    assert after == before[:FILE_AT] + b"\x90" * len(CALL) + before[FILE_AT + len(CALL):]
    assert [(record.address, patch.describe()) for record, patch in declared_patches(tmp_path)] == [
        (ORIGINAL, f"0x{START:08X}-0x{END:08X} nop")]


def test_nop_jmp_keeps_its_relative_displacement_and_checks_the_destination(tmp_path):
    target = ORIGINAL + 15                                  # the `ret` after `pop esi`
    report, before, out = run(tmp_path, f"0x{START:08X} 0x{END:08X} nop-jmp 0x{START + 2:08X} 0x{target:08X}")
    assert report["result"] == "pass"
    written = b"\x90\x90" + b"\xE9" + (target - (START + 7)).to_bytes(4, "little") + b"\x90" * 4
    assert out.read_bytes()[FILE_AT:FILE_AT + len(CALL)] == written

    # A destination inside an instruction, or outside the function, refuses.
    for bad in (ORIGINAL + 2, ORIGINAL + 0x40):
        report, _, out = run(tmp_path, f"0x{START:08X} 0x{END:08X} nop-jmp 0x{START + 2:08X} 0x{bad:08X}")
        assert report["result"] == "fail" and not out.exists()


def test_base_relocations_inside_the_span_are_left_and_reported(tmp_path):
    # push imm32 with a HIGHLOW entry on the immediate, wholly inside the span.
    code = PREFIX_CODE + bytes.fromhex("6800204000" "6A00" "6A00" "8BCE") + SUFFIX_CODE
    report, before, out = run(tmp_path, NOP, code=code, absolute_site=0x10 + 4)
    assert report["result"] == "pass"
    assert report["patches"][0]["base_relocations_left_in_span"] == [f"0x{BUILT + 4:08X}"]
    after = out.read_bytes()
    assert after[768:780] == before[768:780], "the relocation block is untouched"
    assert sum(a != b for a, b in zip(before, after)) == report["changed_bytes"]


@pytest.mark.parametrize("change, reason", [
    (dict(code=PREFIX_CODE + bytes.fromhex("B800000000" "B800000000" "9090") + SUFFIX_CODE,
          declarations=(f"0x{START + 1:08X} 0x{END:08X} nop",)), "no run of complete built instructions"),
    (dict(code=PREFIX_CODE + b"\x90" * len(CALL) + SUFFIX_CODE), "already hold the patch"),
    (dict(mapping=MAP.replace("?f@C@@QAEXXZ", "?g@C@@QAEXXZ")), "expected exactly one"),
    (dict(code=PREFIX_CODE + bytes.fromhex("6A00" "6A00" "8BCE" "A100204000") + SUFFIX_CODE,
          absolute_site=0x10 + 11, declarations=(f"0x{START:08X} 0x{END - 5:08X} nop",)), None),
    (dict(code=bytes.fromhex("56" "B800204000") + CALL[2:] + bytes.fromhex("90" "5E" "C3"),
          absolute_site=0x10 + 2, declarations=(f"0x{START + 3:08X} 0x{END + 1:08X} nop",)), None),
])
def test_refusals_write_nothing_and_remove_a_stale_output(tmp_path, change, reason):
    declarations = change.pop("declarations", (NOP,))
    exe, mapping, out = project(tmp_path, *declarations, **change)
    out.write_bytes(b"stale")
    report = apply_image_patches(tmp_path, exe, mapping, out)
    if reason is None:
        # Controls for the two relocation cases above: fields wholly inside
        # or wholly outside the span never refuse.
        assert report["result"] == "pass", report["refused"]
        return
    assert report["result"] == "fail" and not report["written"] and not out.exists()
    assert reason in report["refused"][0]["reason"]


def test_relocation_field_crossing_a_span_edge_refuses(tmp_path):
    # mov eax, [abs32] ends the span's last instruction; a declared span that
    # stopped inside it is already refused by instruction coverage, so place
    # the relocation site itself across the edge of an otherwise whole run.
    code = PREFIX_CODE + bytes.fromhex("6A00" "6A00" "8BCE" "9090909090") + SUFFIX_CODE
    exe, mapping, out = project(tmp_path, f"0x{START:08X} 0x{START + 6:08X} nop",
                                code=code, absolute_site=0x10 + 3 + 4)
    report = apply_image_patches(tmp_path, exe, mapping, out)
    assert report["result"] == "fail" and "crosses the span edge" in report["refused"][0]["reason"]


def test_output_cannot_be_the_linked_executable_and_no_declarations_copy(tmp_path):
    exe, mapping, out = project(tmp_path, NOP)
    with pytest.raises(ValueError, match="separate output"):
        apply_image_patches(tmp_path, exe, mapping, exe)
    exe, mapping, out = project(tmp_path)
    report = apply_image_patches(tmp_path, exe, mapping, out)
    assert report["result"] == "pass" and report["patches"] == [] and out.read_bytes() == exe.read_bytes()


def test_declaration_outside_every_mapped_function_refuses(tmp_path):
    exe, mapping, out = project(tmp_path, f"0x{ORIGINAL + 0x100:08X} 0x{ORIGINAL + 0x105:08X} nop")
    report = apply_image_patches(tmp_path, exe, mapping, out)
    assert report["result"] == "fail" and "expected exactly one" in report["refused"][0]["reason"]


def test_cli_exit_codes(tmp_path, capsys):
    exe, mapping, out = project(tmp_path, NOP)
    arguments = ["apply-image-patches", "--root", str(tmp_path), "--exe", str(exe),
                 "--map", str(mapping), "--out", str(out), "--summary"]
    assert main(arguments) == 0
    printed = json.loads(capsys.readouterr().out)
    assert printed["result"] == "pass" and "replaced_instructions" not in printed["patches"][0]
    exe.write_bytes(out.read_bytes())            # feeding the patched file back refuses
    assert main(arguments) == 1 and not out.exists()
    assert main([*arguments, "--warn-only"]) == 0 and not out.exists()
    assert json.loads(capsys.readouterr().out.split("\n}\n")[-2] + "\n}")["result"] == "fail"
