import json
from dataclasses import replace

import pytest

from match_recovery.source_form import check_source_form, explicit_assembly
from match_recovery.component_source import render_component
from test_components import vendor


@pytest.mark.parametrize("token", ["asm", "_asm", "__asm", "__asm__", "_emit", "__emit"])
def test_explicit_instruction_forms_fail(token):
    report = explicit_assembly("void body() { " + token + "(0x90); }")
    assert report["result"] == "fail"
    assert report["findings"] == [{"line": 1, "token": token}]


@pytest.mark.parametrize("source", [
    '// __asm\n/* _emit */ int x;',
    'const char* x = "__asm \\" _emit";',
    "char c = '\\''; int my_asm;",
    'const char* x = u8R"delim(__asm " _emit\n)delim";',
    "int n = 1'000; double x = 1.e+3;",
    '#include <asm/header.h>\nint x;',
    '// continued \\\n__asm\nint x;',
])
def test_prose_literals_header_names_and_numbers_are_not_code(source):
    assert explicit_assembly(source) == {"result": "pass", "findings": [], "unsupported": []}


def test_spliced_tokens_report_physical_lines_and_scan_inactive_branches():
    report = explicit_assembly("// header\n#if 0\n__a\\\nsm nop\n#endif\n_emit 0x90;")
    assert report["result"] == "fail"
    assert report["findings"] == [{"line": 3, "token": "__asm"}, {"line": 6, "token": "_emit"}]


@pytest.mark.parametrize("source", ['/* never closed', '"never closed', 'R"x(never closed',
                                   'R"bad delimiter(__asm)bad delimiter"', "??/\n__asm", r"\u0061sm"])
def test_unsupported_lexing_never_passes(source):
    report = explicit_assembly(source)
    assert report["result"] != "pass" and report["unsupported"]


def project(root):
    (root / "recovery").mkdir()
    (root / "code/seeds").mkdir(parents=True)
    (root / "recovery/project.json").write_text(json.dumps({"schema_version": 1, "source": {
        "image_pin": "code/PIN", "address_space": "test:va", "roots": ["code"], "seed_roots": ["code/seeds"]}}))
    return root


def test_seeds_are_hints_not_completed_cpp_and_unmapped_product_is_checked(tmp_path):
    root = project(tmp_path)
    (root / "code/seeds/old.cpp").write_text("__asm nop")
    (root / "code/product.cpp").write_text("int body() { return 0; }")
    assert check_source_form(root)["result"] == "pass"
    (root / "code/unmapped.h").write_text("inline void helper() { __asm nop }")
    report = check_source_form(root)
    assert report["result"] == "fail"
    assert report["findings"] == [{"path": "code/unmapped.h", "line": 1, "token": "__asm"}]
    assert report["excluded"] == [{"path": "code/seeds/old.cpp", "kind": "seed", "component": None}]


def test_missing_product_coverage_is_unknown(tmp_path):
    assert check_source_form(project(tmp_path))["result"] == "unknown"


def test_dependency_ownership_excludes_exact_files_not_arbitrary_neighbors(vendor):
    root, manifest, _ = vendor
    project(root)
    directory = root / "code/codec"
    directory.mkdir()
    manifest = replace(manifest, source_root="code/codec")
    (root / "code/codec.recovery.h").write_text(render_component(manifest))
    (directory / "codec.c").write_text("__asm nop")
    (directory / "codec.h").write_text("// upstream license")
    report = check_source_form(root)
    assert report["result"] == "pass"
    assert {row["path"] for row in report["excluded"]} == {"code/codec/codec.c", "code/codec/codec.h"}
    (directory / "application.cpp").write_text("__asm nop")
    assert check_source_form(root)["findings"][0]["path"] == "code/codec/application.cpp"
