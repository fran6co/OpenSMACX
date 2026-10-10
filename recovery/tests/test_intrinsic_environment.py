"""Project-local intrinsic compiler controls, not application recovery proof."""

import json
import os
from pathlib import Path
import shutil
import subprocess
import sys

import pytest

from match_recovery.compilation_database import entries
from match_recovery.pe_coff import Coff
from recovery.environment.intrinsics import (OPTION, compiler_environment, explicit_arguments,
                                             parse_preprocessed, registration, toolchain_inputs)
from recovery.environment.recipe import invocation

ROOT = Path(__file__).resolve().parents[2]


def test_intrinsic_line_metadata_retains_actual_input_closure(tmp_path):
    header = tmp_path / "header.h"
    header.write_text("int value;\n")
    source = tmp_path / "probe.cpp"
    source.write_text("#include \"header.h\"\n")
    raw = (b'# 1 "<built-in>" 1\n# 1 "<command line>" 1\n'
           b'# 1 "probe.cpp" 2\n# 1 "header.h" 1\nint value;\n'
           b'# 2 "probe.cpp" 2\nint result;\n')
    text, paths = parse_preprocessed(raw, tmp_path)
    assert text == b"int value;\nint result;\n"
    assert paths == {source, header}
    with pytest.raises(ValueError, match="unobserved"):
        parse_preprocessed(b'# 1 "missing.h"\n', tmp_path)
    with pytest.raises(ValueError, match="unsupported"):
        parse_preprocessed(b'# 1 "probe.cpp" 9\n', tmp_path)
    with pytest.raises(ValueError, match="no real"):
        parse_preprocessed(b'# 1 "<built-in>"\n', tmp_path)


def test_application_recipe_retains_explicit_frontend(tmp_path):
    row, = entries(json.dumps([{"file": "probe.cpp", "directory": str(tmp_path),
        "arguments": ["./vc6-cl", OPTION, "/MT", "/O2", "/Z7", "probe.cpp"]}]).encode(), tmp_path)
    assert invocation(row) == [str(tmp_path / "vc6-cl"), "/nologo", OPTION, "/MT"]


@pytest.fixture
def intrinsic_compile(tmp_path, monkeypatch):
    if shutil.which("clang-cl") is None:
        pytest.skip("clang-cl is required for intrinsic compiler control")
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / "BIN/CL.EXE").is_file():
        pytest.skip("existing project wrapper requires configured VC6 tools")
    for name in ("CL", "_CL_", "INCLUDE", "CPATH", "C_INCLUDE_PATH", "CPLUS_INCLUDE_PATH"):
        monkeypatch.delenv(name, raising=False)
    source = tmp_path / "probe.cpp"
    source.write_text('extern "C" void __cpuid(int *, int);\n#pragma intrinsic(__cpuid)\n'
                      'extern "C" unsigned char probe_rank;\n'
                      'extern "C" unsigned probe(int *values) {\n'
                      ' unsigned saved = __builtin_ia32_readeflags_u32();\n'
                      ' __builtin_ia32_writeeflags_u32(saved);\n'
                      ' __cpuid(values, 0); probe_rank = (unsigned char)values[0]; return saved;\n}\n')
    output, receipt = tmp_path / "probe.obj", tmp_path / "probe.compile.json"
    def run(*extra):
        command = [sys.executable, "-I", "-B", str(ROOT / "recovery/environment/wine.py"),
                   "--tool=cl", OPTION, "--repro-root=" + str(tmp_path),
                   "--compile-diagnostics=" + str(receipt), "/nologo", "/c", "/O2", "/Gy",
                   "/Z7", "/Fo" + str(output), *extra, str(source)]
        return subprocess.run(command, cwd=tmp_path, capture_output=True, timeout=60)
    return run, output, receipt


def test_real_intrinsic_coff_and_actual_compiler_receipt(intrinsic_compile):
    run, output, receipt = intrinsic_compile
    result = run()
    assert result.returncode == 0, result.stderr.decode(errors="replace")
    obj = Coff(output.read_bytes())
    body, relocations, _ = obj.subject(obj.function("_probe"))
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    instructions = [i.mnemonic for i in Cs(CS_ARCH_X86, CS_MODE_32).disasm(body, 0)]
    assert "pushfd" in instructions and "popfd" in instructions and "cpuid" in instructions
    assert relocations
    assert {obj.symbols[int(row.target)].name for row in relocations} == {"_probe_rank"}
    assert not any(s.name.startswith(".debug") or s.name.startswith("/") for s in obj.sections)
    report = json.loads(receipt.read_text())
    assert report["exit_code"] == 0
    assert report["schema_version"] == 2
    assert Path(report["tool"]) == Path(shutil.which("clang-cl")).resolve()
    assert "--target=i686-pc-windows-msvc" in report["arguments"]
    assert "--no-default-config" in report["arguments"]
    assert "/clang:-march=i386" in report["arguments"]
    assert "/clang:-mgeneral-regs-only" in report["arguments"]
    assert "/Z7" not in report["arguments"] and "/GS-" in report["arguments"]
    assert report["output"]["path"] == str(output)
    assert any(Path(row["path"]).name == "llvm-objcopy" for row in report["inputs"])
    assert any(Path(row["path"]).name == "intrinsics.py" for row in report["inputs"])
    assert all(row["before"] == row["after"] for row in report["inputs"])
    from match_recovery.compile_file_evidence import verify_compile_files
    proof = verify_compile_files(receipt.read_bytes(), object_path=output)
    assert proof["result"] == "pass"
    assert proof["output_provenance"] == "postprocessed"
    assert proof["pipeline"]["transformation_semantics"] == "unknown"
    stage, = report["postprocess"]
    assert stage["argv"] == [stage["tool"], "--strip-debug", str(output)]
    assert stage["input_before"] == report["compiler_output"]["after"]
    assert stage["output_after"] == report["output"]["after"]
    assert stage["input_before"] != stage["output_after"]
    raw_output = output.with_name("raw.obj")
    raw_arguments = ["/Fo" + str(raw_output) if word.startswith("/Fo") else word
                     for word in report["arguments"]]
    raw = subprocess.run([report["tool"], *raw_arguments], cwd=report["cwd"],
                         capture_output=True, timeout=60)
    assert raw.returncode == 0, raw.stderr.decode(errors="replace")
    unstripped = Coff(raw_output.read_bytes())
    assert any(s.name.startswith(".debug") for s in unstripped.sections)
    def contents(coff):
        return [(s.name, s.flags, s.data,
                 [(offset, coff.symbols[index].name, kind) for offset, index, kind in s.relocations])
                for s in coff.sections if not s.name.startswith(".debug")]
    assert contents(obj) == contents(unstripped)
    repeated = run()
    assert repeated.returncode == 3 and b"new canonical" in repeated.stderr


@pytest.mark.parametrize("extra", [(OPTION,), ("--frontend=unknown",),
    ("/clang:--target=x86_64-pc-windows-msvc",), ("/Zi",), ("-m64",), ("/arch:SSE2",),
    ("--config=hidden.cfg",), ("-xc",), ("-g",), ("/zi",), ("/Zd",),
    ("/D_M_X64=1",), ("/U_WIN32",), ("/D__cplusplus=1",), ("/D", "_MSC_VER=1200")])
def test_intrinsic_selection_and_debug_damage_refuse(intrinsic_compile, extra):
    run, output, receipt = intrinsic_compile
    result = run(*extra)
    assert result.returncode == 3
    assert not output.exists() and not receipt.exists()


@pytest.mark.skipif(os.environ.get("OPENSMACX_INTRINSIC_LINK_CONTROL") != "1",
                    reason="explicit live compiler/linker control required")
def test_intrinsic_pipeline_debug_links_with_vc6(intrinsic_compile):
    run, output, receipt = intrinsic_compile
    result = run()
    assert result.returncode == 0, result.stderr.decode(errors="replace")
    provider = output.with_name("provider.cpp")
    provider.write_text('extern "C" unsigned char probe_rank = 0;\n')
    provider_object = provider.with_suffix(".obj")
    compiled = subprocess.run([str(ROOT / "recovery/environment/vc6-cl"), "/nologo",
        "--repro-root=" + str(output.parent), "/c", "/O2", "/Z7", "/Zl",
        "/Fo" + str(provider_object), str(provider)], cwd=output.parent,
        capture_output=True, timeout=90)
    assert compiled.returncode == 0, (compiled.stdout + compiled.stderr).decode(errors="replace")
    dll, pdb = output.with_suffix(".dll"), output.with_suffix(".pdb")
    linked = subprocess.run([str(ROOT / "recovery/environment/vc6-link"), "/nologo",
        "/DLL", "/NOENTRY", "/NODEFAULTLIB", "/MACHINE:IX86", "/DEBUG",
        "/INCREMENTAL:NO", "/EXPORT:probe", "/OUT:" + str(dll), "/PDB:" + str(pdb), str(output), str(provider_object)],
        cwd=output.parent, capture_output=True, timeout=90)
    assert linked.returncode == 0, (linked.stdout + linked.stderr).decode(errors="replace")
    assert dll.read_bytes().startswith(b"MZ") and pdb.is_file()


@pytest.mark.parametrize("macro", ["__DATE__", "__TIME__", "__TIMESTAMP__", "__FILE__"])
def test_intrinsic_context_dependent_macro_refuses(intrinsic_compile, macro):
    run, output, receipt = intrinsic_compile
    (output.parent / "probe.cpp").write_text('const char *stamp = ' + macro + ';\n')
    result = run()
    assert result.returncode == 3 and b"changes preprocessing" in result.stderr
    assert not output.exists() and not receipt.exists()


@pytest.mark.parametrize("name", ["CCC_OVERRIDE_OPTIONS", "CLANG_CONFIG_FILE_USER_DIR",
    "CLANG_CONFIG_FILE_SYSTEM_DIR", "CPATH", "COMPILER_PATH", "CC_PRINT_OPTIONS", "ccc_override_options"])
def test_implicit_backend_and_config_environment_refuses(monkeypatch, name):
    monkeypatch.setenv(name, "hidden configuration")
    with pytest.raises(ValueError, match="implicit"):
        compiler_environment()


def test_runtime_dash_alias_and_debug_omission_are_explicit():
    assert explicit_arguments(["-MT", "/DWIN32", "/Z7", "/Fdignored.pdb", "/O2"]) == ["/MT", "/DWIN32", "/O2"]
    assert explicit_arguments(["/Fd", "ignored.pdb", "/D", "VALUE=1", "/TP"]) == ["/DVALUE=1", "/TP"]


def test_disabled_registration_needs_no_intrinsic_tools(tmp_path, monkeypatch):
    path = tmp_path / "frontends.json"
    path.write_text('{"schema_version":1,"intrinsics":{"enabled":false,"compiler":"missing-tool"}}')
    value = registration(path)
    monkeypatch.setattr("recovery.environment.intrinsics.registration", lambda: value)
    monkeypatch.setattr(shutil, "which", lambda _: pytest.fail("disabled registry looked for an intrinsic tool"))
    assert toolchain_inputs() == {"files": {}, "directories": {}}
