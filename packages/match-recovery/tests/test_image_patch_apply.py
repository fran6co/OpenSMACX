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


def test_nop_jmp_destination_may_be_a_multi_byte_instruction(tmp_path):
    # The destination is `mov eax, 1` (5 bytes), not a one-byte `ret`: reading
    # the built bytes only up to the destination's first byte truncated it and
    # refused a sound patch (Caviar::init_class 0x006185A0).
    code = PREFIX_CODE + CALL + bytes.fromhex("B801000000" "5E" "C3")
    target = ORIGINAL + 14
    options = {"code": code, "span_end": ORIGINAL + len(code)}
    report, _, out = run(tmp_path, f"0x{START:08X} 0x{END:08X} nop-jmp 0x{START + 2:08X} 0x{target:08X}",
                         **options)
    assert report["result"] == "pass" and report["changed_bytes"] == len(CALL)
    written = b"\x90\x90" + b"\xE9" + (target - (START + 7)).to_bytes(4, "little") + b"\x90" * 4
    assert out.read_bytes()[FILE_AT:FILE_AT + len(CALL)] == written
    # Its interior still refuses, and so does the last instruction's interior.
    for bad in (target + 1, target + 4):
        report, _, out = run(tmp_path, f"0x{START:08X} 0x{END:08X} nop-jmp 0x{START + 2:08X} 0x{bad:08X}",
                             **options)
        assert report["result"] == "fail" and not out.exists()
        assert "not a built instruction start" in report["refused"][0]["reason"]


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
def test_sole_refused_patch_writes_nothing_and_removes_a_stale_output(tmp_path, change, reason):
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


# A second function `g`, a copy of `f` placed 0x20 further on in both images.
G_ORIGINAL, G_BUILT = ORIGINAL + 0x20, BUILT + 0x20
G_NOP = f"0x{G_ORIGINAL + 3:08X} 0x{G_ORIGINAL + 3 + len(CALL):08X} nop"
G_FILE_AT = FILE_AT + 0x20
BROKEN_F = PREFIX_CODE + bytes.fromhex("6A00" "B800000000" "B800000000") + SUFFIX_CODE   # straddles +14


def two_functions(tmp_path, *declarations, f_code=FUNCTION, g_code=FUNCTION):
    exe, mapping, out = project(
        tmp_path, code=f_code + b"\xcc" * (0x20 - len(f_code)) + g_code,
        mapping=MAP.replace(" entry point", " 0001:00000030 ?g@C@@QAEXXZ 00401030 f a.obj\n entry point"))
    (tmp_path / "code/f.cc").write_text(
        "".join(f"// RECOVERY_IMAGE_PATCH: v1 {text}\n" for text in declarations)
        + f"// ORIGINAL: 0x{ORIGINAL:08X} ?f@C@@QAEXXZ 0x{ORIGINAL:08X}-0x{ORIGINAL + len(FUNCTION):08X}\n"
        "void C::f() {\n}\n"
        + f"// ORIGINAL: 0x{G_ORIGINAL:08X} ?g@C@@QAEXXZ 0x{G_ORIGINAL:08X}-0x{G_ORIGINAL + len(FUNCTION):08X}\n"
        "void C::g() {\n}\n")
    return exe, mapping, out


def test_patches_are_independent_a_refused_one_does_not_withhold_the_others(tmp_path):
    exe, mapping, out = two_functions(tmp_path, NOP, G_NOP)
    before = exe.read_bytes()
    report = apply_image_patches(tmp_path, exe, mapping, out)
    assert (report["result"], report["complete"], report["declared"]) == ("pass", True, 2)
    assert [row["offset"] for row in report["patches"]] == [FILE_AT, G_FILE_AT]
    complete = out.read_bytes()

    # `f` is being reworked: its patch refuses, `g`'s is still written.
    exe, mapping, out = two_functions(tmp_path, NOP, G_NOP, f_code=BROKEN_F)
    before = exe.read_bytes()
    report = apply_image_patches(tmp_path, exe, mapping, out)
    assert (report["result"], report["written"], report["complete"], report["declared"]) == (
        "partial", True, False, 2)
    refused, = report["refused"]
    assert refused["function"] == f"0x{ORIGINAL:08X}" and refused["source"] == "f.cc:1"
    assert "no run of complete built instructions" in refused["reason"]
    assert [row["function"] for row in report["patches"]] == [f"0x{G_ORIGINAL:08X}"]
    after = out.read_bytes()
    assert after == before[:G_FILE_AT] + b"\x90" * len(CALL) + before[G_FILE_AT + len(CALL):]
    assert after != complete and report["changed_bytes"] == len(CALL)
    assert exe.read_bytes() == before

    # Declaration order does not matter, and neither does which one refuses.
    exe, mapping, out = two_functions(tmp_path, G_NOP, NOP, g_code=BROKEN_F)
    report = apply_image_patches(tmp_path, exe, mapping, out)
    assert report["result"] == "partial"
    assert [row["function"] for row in report["patches"]] == [f"0x{ORIGINAL:08X}"]
    assert [row["function"] for row in report["refused"]] == [f"0x{G_ORIGINAL:08X}"]

    # Every patch refused: nothing is written and the earlier partial output goes.
    exe, mapping, out = two_functions(tmp_path, NOP, G_NOP, f_code=BROKEN_F, g_code=BROKEN_F)
    assert out.exists()
    report = apply_image_patches(tmp_path, exe, mapping, out)
    assert (report["result"], report["written"], report["patches"]) == ("fail", False, [])
    assert len(report["refused"]) == 2 and not out.exists()


def test_unowned_declaration_refuses_alone(tmp_path):
    stray = f"0x{ORIGINAL + 0x100:08X} 0x{ORIGINAL + 0x105:08X} nop"
    exe, mapping, out = two_functions(tmp_path, NOP, stray)
    report = apply_image_patches(tmp_path, exe, mapping, out)
    assert report["result"] == "partial" and len(report["patches"]) == 1 and report["declared"] == 2
    assert "expected exactly one" in report["refused"][0]["reason"]
    assert out.read_bytes()[FILE_AT:FILE_AT + len(CALL)] == b"\x90" * len(CALL)
    with pytest.raises(ValueError, match="expected exactly one"):
        declared_patches(tmp_path)


def test_unusable_inputs_fail_whole_and_remove_the_output(tmp_path):
    exe, mapping, out = two_functions(tmp_path, NOP, G_NOP)
    out.write_bytes(b"stale")
    mapping.write_text(mapping.read_text().replace("00401030 f", "00401038 f").replace("0001:00000030", "0001:00000038")
                       .replace("Preferred load address is 00400000", "Preferred load address is 00500000"))
    report = apply_image_patches(tmp_path, exe, mapping, out)
    assert report["result"] == "fail" and not out.exists() and report["patches"] == []
    out.write_bytes(b"stale")
    exe.unlink()
    report = apply_image_patches(tmp_path, exe, mapping, out)
    assert report["result"] == "fail" and not out.exists() and report["refused"]


def test_io_error_leaves_no_half_written_file(tmp_path, monkeypatch):
    from pathlib import Path

    exe, mapping, out = two_functions(tmp_path, NOP, G_NOP, f_code=BROKEN_F)
    out.write_bytes(b"stale")
    real = Path.replace

    def failing(self, target):
        if self.name.endswith(".tmp"):
            assert self.exists(), "the temporary file was written first"
            raise OSError("disk full")
        return real(self, target)

    monkeypatch.setattr(Path, "replace", failing)
    report = apply_image_patches(tmp_path, exe, mapping, out)
    assert report["result"] == "fail" and not report["written"] and report["patches"] == []
    assert "disk full" in report["refused"][-1]["reason"]
    assert not out.exists() and not out.with_name(out.name + ".tmp").exists()


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
    captured = capsys.readouterr()
    assert json.loads(captured.out.split("\n}\n")[-2] + "\n}")["result"] == "fail"
    assert "NOT written" in captured.err and "already hold the patch" in captured.err


def test_cli_partial_exit_code_and_refused_list(tmp_path, capsys):
    exe, mapping, out = two_functions(tmp_path, NOP, G_NOP, f_code=BROKEN_F)
    arguments = ["apply-image-patches", "--root", str(tmp_path), "--exe", str(exe),
                 "--map", str(mapping), "--out", str(out), "--summary"]
    assert main(arguments) == 2 and out.exists()          # fatal for a build that requires every patch
    out.unlink()
    capsys.readouterr()
    assert main([*arguments, "--warn-only"]) == 0 and out.exists()
    captured = capsys.readouterr()
    printed = json.loads(captured.out)
    assert printed["result"] == "partial" and not printed["complete"]
    assert "1 declared patch(es) refused, 1 applied; app.patched.exe written WITHOUT them" in captured.err
    assert f"refused 0x{ORIGINAL:08X} f.cc:1" in captured.err and "no run of complete" in captured.err
