"""Actual-source full base_build controls, not native/original-game proof.

The installed body and actual project owner are compiled, without ignored packet
includes or a second recovered implementation. Services and other storage are
explicit doubles; original invalid-index/uninitialized paths are not certified.
"""
from pathlib import Path
import os
import re
import resource
import shutil
import subprocess

import pytest
from match_recovery.source import read_file

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent

def actual_project_owner():
    source = (ROOT / "src/base.h").read_text()
    match = re.search(r"^struct BaseSecretProject \{.*?^\};", source, re.M | re.S)
    assert match, "actual project owner missing"
    owner = match.group()
    assert "int project_base[64];" in owner
    assert "operator[](int project)" in owner
    return owner

def actual_build_body():
    row, = (r for r in read_file(ROOT / "src/base.cpp") if r.address == 0x4f81a0)
    assert row.name == "?base_build@@YAHHPAH00@Z"
    assert row.image_spans == ((0x4f81a0, 0x4fffb8),)
    assert not row.extract_error and "BODY GOES HERE." not in row.region
    start = re.search(r"^int __cdecl base_build\(", row.region, re.M)
    assert start, "complete canonical base_build missing"
    return row.region[start.start():]

def no_core():
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))

def compile_run(tmp_path, compiler, source, expect_pass=True):
    command = shutil.which(compiler)
    if command is None:
        pytest.skip("host compiler unavailable: " + compiler)
    cpp, executable = tmp_path / "actual.cpp", tmp_path / "actual"
    cpp.write_text(source)
    result = subprocess.run([command, "-std=c++98", "-O2", "-w", "-fwrapv",
                             str(cpp), "-o", str(executable)],
                            capture_output=True, text=True, timeout=60)
    assert result.returncode == 0, result.stdout + result.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True,
                            timeout=20, preexec_fn=no_core)
    assert (result.returncode == 0) == expect_pass, result.stdout + result.stderr
    return result

def actual_harness(body):
    fixture = (HERE / "base_build_host_fixture.cpp").read_text()
    assert fixture.count("/*ACTUAL_PROJECT_OWNER*/") == fixture.count("/*ACTUAL_BASE_BUILD*/") == 1
    return fixture.replace("/*ACTUAL_PROJECT_OWNER*/", actual_project_owner()).replace("/*ACTUAL_BASE_BUILD*/", body)

@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_full_build_selection_and_output_controls(tmp_path, compiler):
    result = compile_run(tmp_path, compiler, actual_harness(actual_build_body()))
    assert "result=-69 secondary=99999 tertiary=99999 score=999999" in result.stdout
    assert "negative-mode result=-69 score=999999" in result.stdout
    assert "prototype=0 result=0 score=36864" in result.stdout
    assert result.stdout.count("facility=") == 10

def test_actual_lifecycle_damage_is_detected(tmp_path):
    body = actual_build_body()
    assert body.count("set_base(base_id);") == 1
    damage = body.replace("set_base(base_id);", "set_base(base_id+1);", 1)
    compile_run(tmp_path, "g++", actual_harness(damage), expect_pass=False)

def test_actual_project_storage_all_named_accessor_aliases(tmp_path):
    fixture = (HERE / "base_secret_project_accessor_fixture.cpp").read_text()
    assert fixture.count("/*ACTUAL_PROJECT_OWNER*/") == 1
    compile_run(tmp_path, "g++", fixture.replace("/*ACTUAL_PROJECT_OWNER*/", actual_project_owner()))

def test_actual_wrong_project_accessor_index_is_detected(tmp_path):
    owner = actual_project_owner()
    anchor = "int &network_backbone() { return project_base[21]; }"
    assert owner.count(anchor) == 1
    damage = owner.replace(anchor, "int &network_backbone() { return project_base[20]; }", 1)
    fixture = (HERE / "base_secret_project_accessor_fixture.cpp").read_text()
    compile_run(tmp_path, "g++", fixture.replace("/*ACTUAL_PROJECT_OWNER*/", damage), expect_pass=False)

def test_current_product_consumers_use_index_or_reference_accessors():
    from match_recovery.source_config import SourceConfiguration
    config = SourceConfiguration.read(ROOT)
    names = re.findall(r"int &(\w+)\(\) \{ return project_base\[\d+\]; \}", actual_project_owner())
    assert len(names) == 64
    old_access = re.compile(r"\bSecretProject\.(?:" + "|".join(names) + r")\b(?!\s*\()")
    protected = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'', re.S)
    for path in config.files(ROOT):
        relative = path.relative_to(ROOT).as_posix()
        if config.is_seed(relative) or relative.startswith("src/vendor/"):
            continue
        code = protected.sub("", path.read_text())
        assert not old_access.search(code), relative
        assert "*(&SecretProject.human_genome_project" not in code, relative

def test_historical_project_member_obligations_are_not_discarded():
    from match_recovery.layout_source import parse_members
    from match_recovery.source_inventory import image_pin
    rows = parse_members((ROOT / "src/base.h").read_text(), "src/base.h", image_pin(ROOT), address_space="pe:va")
    expected = {"empath_guild": 0x10, "network_backbone": 0x54, "cloning_vats": 0x60,
                "self_aware_colony": 0x64, "space_elevator": 0x6c, "cloudbase_academy": 0x8c}
    assert {r.member: r.offset for r in rows if r.class_name == "BaseSecretProject"} == expected
    # These are original interpretations, not observed accessor/array-element
    # offsets. Current native layout binding does not support that mapping.
