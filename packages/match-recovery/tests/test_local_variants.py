"""Synthetic controls for the bounded one-TU body-variant screen."""

import hashlib
import json

import pytest

from match_recovery.local_variants import measure_local_variants
from match_recovery.model import Origin
from match_recovery.tasks import Task, TaskStore
from match_recovery import local_variants
from match_recovery.source.reader import read_text
from test_binary import coff_fixture, pe_fixture


@pytest.fixture
def project(tmp_path):
    root = tmp_path
    (root / "code").mkdir()
    (root / "recovery").mkdir()
    image = pe_fixture()
    pin = hashlib.sha256(image).hexdigest()
    (root / "image.exe").write_bytes(image)
    (root / "code/PIN").write_text("sha256 " + pin + "\n")
    source = ("// ORIGINAL: 0x00401000 _f 0x00401000-0x00401001\n"
              "int _f() { return 0; }\n")
    (root / "code/unit.cc").write_text(source)
    (root / "recovery/project.json").write_text(json.dumps({
        "schema_version": 1,
        "source": {"image_pin": "code/PIN", "address_space": "pe:va",
                   "roots": ["code"], "include_paths": ["code"]},
        "measurement": {"backend": "pe-coff-i386", "compiler_profile": "synthetic",
                         "flag_sets": [["-O2", "-DNDEBUG"], ["-Od", "/DTEST"]], "layout_flags": [],
                         "unit_extensions": [".cc"], "inputs": [], "input_roots": []},
    }))
    task = Task("local", "synthetic", "coordinator", "screen one body", "a" * 40,
                (Origin(pin, "pe:va", 0x401000),), ("code/unit.cc",),
                ("synthetic task-bound body screen",))
    TaskStore(root).create(task)
    row = {"source_sha256": hashlib.sha256((root / "code/unit.cc").read_bytes()).hexdigest(),
           "before": "return 0;", "control": "for (;;) {}", "entity_id": task.subjects[0].entity_id}
    deck = {"schema_version": 2, "source": "code/unit.cc",
            "entity_id": task.subjects[0].entity_id,
            "files": {"code/unit.cc": row},
            "candidates": {"same": {"code/unit.cc": "return 0;"},
                           "different": {"code/unit.cc": "return 1;"}}}
    return root, task, deck


class SyntheticCompiler:
    def __init__(self, root, *, ineffective_damage=False, mutate_source=False, wrong_symbol=False):
        self.root = root
        self.ineffective_damage = ineffective_damage
        self.mutate_source = mutate_source
        self.wrong_symbol = wrong_symbol
        self.calls = []

    def snapshot(self):
        return "synthetic compiler snapshot"

    def _object(self, contents):
        code = b"\xc3"
        if not self.ineffective_damage and b"for (;;) {}" in contents:
            code = b"\xeb\xfe"
        elif b"return 1;" in contents:
            code = b"\x90\xc3"
        raw = coff_fixture(code)
        return raw.replace(b"_f\0\0\0\0\0\0", b"_x\0\0\0\0\0\0", 1) if self.wrong_symbol else raw

    def __call__(self, source, arguments, flags):
        self.calls.append(("direct", tuple(flags)))
        return self._object(source.read_bytes())

    def overlay(self, source, replacements, arguments, flags):
        self.calls.append(("overlay", tuple(flags)))
        contents = replacements["code/unit.cc"]
        if self.mutate_source and b"return 1;" in contents:
            source.write_bytes(source.read_bytes() + b"// changed during lease\n")
        return self._object(contents)


def run(project, compiler=None, deck=None):
    root, task, original_deck = project
    return measure_local_variants(root, task.task_id, root / "image.exe",
                                  original_deck if deck is None else deck,
                                  compiler=compiler or SyntheticCompiler(root))


def test_local_screen_reuses_each_full_tu_object_and_never_accepts(project):
    compiler = SyntheticCompiler(project[0])
    report = run(project, compiler)
    assert report["status"] == "scored"
    assert report["compile_attempts"] == 10  # 2 recipes * (direct + replica + damage + 2 candidates)
    assert report["elapsed_seconds"] >= 0
    assert len(report["flag_trials"]) == 2
    for trial in report["flag_trials"]:
        assert trial["controls"]["unchanged_replica"]["result"] == "pass"
        assert trial["controls"]["damage"]["result"] == "pass"
        assert trial["baseline"][0]["result"] == "pass"
        assert trial["candidates"]["same"]["measurements"][0]["result"] == "pass"
        assert trial["candidates"]["different"]["measurements"][0]["result"] == "fail"
        assert all(row["reused_compiled_object"] for row in trial["candidates"].values())
    assert report["strict_references"] == "unknown"
    assert report["strict_reference_result"] == "unknown"
    assert not report["scope_complete"] and not report["ready_for_collection"]
    assert not report["persisted"] and report["acceptance"] == "not_evaluated"
    assert (project[0] / "code/unit.cc").read_bytes() == (
        b"// ORIGINAL: 0x00401000 _f 0x00401000-0x00401001\nint _f() { return 0; }\n")


def test_local_screen_keeps_root_context_for_associated_owner(tmp_path):
    from test_associated_application import project as associated_project

    root, task, raw = associated_project(tmp_path)
    source = root / "code/unit.cc"
    original = source.read_text()
    assert "RECOVERY_ASSOCIATED:" in original
    assert "void f()" in original
    original = original.replace("void f()", "void _f()")
    source.write_text(original)
    anchor = "/* baseline */"
    deck = {
        "schema_version": 2,
        "source": "code/unit.cc",
        "entity_id": task.subjects[0].entity_id,
        "files": {"code/unit.cc": {
            "source_sha256": hashlib.sha256(source.read_bytes()).hexdigest(),
            "before": anchor,
            "control": "for (;;) {}",
            "entity_id": task.subjects[0].entity_id,
        }},
        "candidates": {
            "same": {"code/unit.cc": anchor},
            "different": {"code/unit.cc": "return 42;"},
        },
    }

    class AssociatedCompiler:
        def snapshot(self):
            return "associated synthetic compiler snapshot"

        def __call__(self, *_args):
            return raw

        def overlay(self, _source, replacements, _arguments, _flags):
            contents = replacements["code/unit.cc"]
            if b"for (;;) {}" in contents or b"return 42;" in contents:
                changed = raw.replace(b"\xc3", b"\x90", 1)
                assert changed != raw
                return changed
            return raw

    report = measure_local_variants(root, task.task_id, root / "image.exe", deck,
                                    compiler=AssociatedCompiler())

    assert report["status"] == "scored", report
    assert len(report["flag_trials"]) == 2
    for trial in report["flag_trials"]:
        assert trial["controls"]["unchanged_replica"]["result"] == "pass"
        assert trial["controls"]["damage"]["result"] == "pass"
        assert trial["baseline"][0]["result"] == "pass"
        assert trial["candidates"]["same"]["measurements"][0]["result"] == "pass"
        assert trial["candidates"]["different"]["measurements"][0]["result"] == "fail"
    assert report["strict_references"] == "unknown"
    assert report["strict_reference_result"] == "unknown"
    assert not report["scope_complete"] and not report["ready_for_collection"]
    assert not report["persisted"] and report["acceptance"] == "not_evaluated"


@pytest.mark.parametrize("newline", ["\n", "\r\n", "\r"])
def test_local_screen_allows_unchanged_trailing_spliced_macros(project, newline):
    root, _task, deck = project
    source = root / "code/unit.cc"
    original = source.read_bytes()
    prefix = "// UTF-8 prefix: café\n".encode("utf-8")
    trailing = newline.join((
        "#define NATIVE_COPY(dst, src, size) \\",
        "  do { \\",
        "    (dst) = (src); \\",
        "  } while (0)",
        "",
    )).encode("utf-8")
    contents = prefix + original + trailing
    source.write_bytes(contents)
    deck["files"]["code/unit.cc"]["source_sha256"] = hashlib.sha256(contents).hexdigest()

    compiler = SyntheticCompiler(root)
    report = run(project, compiler, deck)
    assert report["status"] == "scored"
    assert report["compile_attempts"] == 10
    assert [row["controls"]["unchanged_replica"]["result"]
            for row in report["flag_trials"]] == ["pass", "pass"]
    assert [row["controls"]["damage"]["result"]
            for row in report["flag_trials"]] == ["pass", "pass"]
    assert source.read_bytes() == contents


def test_local_screen_explicitly_refuses_an_emitting_context(project):
    root, _task, deck = project
    source = root / "code/unit.cc"
    text = source.read_text().replace(
        "// ORIGINAL: 0x00401000 _f 0x00401000-0x00401001",
        "// ORIGINAL: 0x00401000 _f 0x00401000-0x00401001\n// compile_context code/emitter.cc")
    source.write_text(text)
    deck["files"]["code/unit.cc"]["source_sha256"] = hashlib.sha256(source.read_bytes()).hexdigest()
    compiler = SyntheticCompiler(root)
    with pytest.raises(ValueError, match="compile_context"):
        run(project, compiler, deck)
    assert not compiler.calls


def test_comparison_uses_one_trial_per_complete_flag_tuple(project, monkeypatch):
    trials = []
    real_compare = local_variants.compare_unit

    def capture(records, exe, source, command, flag_sets, shared, **kwargs):
        trials.append(flag_sets)
        compile_object = kwargs["compiler"]

        def capture_trial(path, arguments, flags):
            trials.append(tuple(flags))
            return compile_object(path, arguments, flags)

        kwargs["compiler"] = capture_trial
        return real_compare(records, exe, source, command, flag_sets, shared, **kwargs)

    monkeypatch.setattr(local_variants, "compare_unit", capture)
    report = run(project)
    assert report["status"] == "scored"
    complete_sets = (("-O2", "-DNDEBUG"), ("-Od", "/DTEST"))
    assert trials == [item for flags in complete_sets for _ in range(3)
                      for item in [(flags,), flags]]


def test_damage_control_must_change_exact_native_target_function(project):
    report = run(project, SyntheticCompiler(project[0], ineffective_damage=True))
    assert report["status"] == "incomplete"
    for trial in report["flag_trials"]:
        assert trial["controls"]["damage"]["result"] == "fail"
        assert all(candidate["result"] == "unknown" for candidate in trial["candidates"].values())


def test_refuses_missing_target_and_wrong_native_coff_symbol(project):
    root, task, deck = project
    missing = json.loads(json.dumps(deck))
    missing["entity_id"] = Origin(task.subjects[0].image_sha256, "pe:va", 0x401010).entity_id
    missing["files"]["code/unit.cc"]["entity_id"] = missing["entity_id"]
    with pytest.raises(ValueError, match="task subject"):
        run(project, deck=missing)
    wrong = run(project, SyntheticCompiler(root, wrong_symbol=True))
    assert wrong["status"] == "incomplete"
    for trial in wrong["flag_trials"]:
        assert trial["controls"]["unchanged_replica"]["result"] == "fail"
        assert all(candidate["result"] == "unknown" for candidate in trial["candidates"].values())


def test_unavailable_task_neighbor_does_not_block_selected_target_scoring(project):
    root, task, deck = project
    source_path = root / "code/unit.cc"
    source = source_path.read_text() + (
        "// ORIGINAL: 0x00401001 _missing 0x00401001-0x00401002\n"
        "int _missing() { return 42; }\n")
    source_path.write_text(source)
    neighbor = Origin(task.subjects[0].image_sha256, "pe:va", 0x401001)
    expanded = Task(task.task_id, task.target, task.author, task.intent,
                    task.base_revision, (*task.subjects, neighbor),
                    task.editable_files, task.evidence)
    (root / "recovery/tasks/local.json").write_text(json.dumps(expanded.to_dict()))
    deck["files"]["code/unit.cc"]["source_sha256"] = hashlib.sha256(
        source_path.read_bytes()).hexdigest()

    report = run(project, SyntheticCompiler(root), deck)
    for trial in report["flag_trials"]:
        assert trial["controls"]["unchanged_replica"]["result"] == "pass"
        assert trial["controls"]["damage"]["result"] == "pass"
        for measurements, target_result in (
                (trial["baseline"], "pass"),
                (trial["candidates"]["same"]["measurements"], "pass"),
                (trial["candidates"]["different"]["measurements"], "fail")):
            by_address = {row["address"]: row for row in measurements}
            assert by_address["0x00401000"]["result"] == target_result
            assert by_address["0x00401001"]["result"] == "unknown"


def test_refuses_non_body_and_preprocessor_edits(project):
    root, _task, deck = project
    outside = json.loads(json.dumps(deck))
    outside["files"]["code/unit.cc"]["before"] = "00401000 _f"
    outside["files"]["code/unit.cc"]["control"] = "00401000 _f"
    outside["candidates"] = {"outside": {"code/unit.cc": "00401001 _f"}}
    with pytest.raises(ValueError, match="only the selected function body"):
        run(project, deck=outside)
    directive = json.loads(json.dumps(deck))
    directive["candidates"] = {"directive": {"code/unit.cc": "#define BAD 1\nreturn 0;"}}
    with pytest.raises(ValueError, match="preprocessor directives"):
        run(project, deck=directive)
    header = json.loads(json.dumps(deck))
    (root / "code/unit.h").write_text("int _f();\n")
    header["source"] = "code/unit.h"
    header["files"] = {"code/unit.h": {**header["files"]["code/unit.cc"],
                                        "source_sha256": hashlib.sha256((root / "code/unit.h").read_bytes()).hexdigest()}}
    header["candidates"] = {"header": {"code/unit.h": "int _f();\n"}}
    with pytest.raises(ValueError, match="authorized product translation unit"):
        run(project, deck=header)


def test_full_function_anchor_is_allowed_only_when_signature_is_preserved(project):
    root, _task, deck = project
    whole = json.loads(json.dumps(deck))
    whole["files"]["code/unit.cc"]["before"] = "int _f() { return 0; }"
    whole["files"]["code/unit.cc"]["control"] = "int _f() { for (;;) {} }"
    whole["candidates"] = {
        "same_signature": {"code/unit.cc": "int _f() { return 0; }"},
        "changed_body": {"code/unit.cc": "int _f() { return 1; }"},
    }
    report = run(project, SyntheticCompiler(root), whole)
    assert report["status"] == "scored"
    changed = json.loads(json.dumps(whole))
    changed["candidates"] = {
        "changed_signature": {"code/unit.cc": "long _f() { return 1; }"},
    }
    with pytest.raises(ValueError, match="only the selected function body"):
        run(project, deck=changed)


def test_changed_source_during_lease_refuses_result(project):
    compiler = SyntheticCompiler(project[0], mutate_source=True)
    with pytest.raises(ValueError, match="inputs changed during the compiler lease"):
        run(project, compiler)
    assert len(compiler.calls) == 5  # Abort immediately after the changed candidate overlay.


def test_mapped_overload_body_is_selected_by_exact_marker_interval(tmp_path):
    source = tmp_path / "unit.cc"
    text = ("// ORIGINAL: 0x00401000 _first 0x00401000-0x00401001\n"
            "int Owner::fill(int x) { return x; }\n"
            "/* a comment opened before the target marker\n"
            "// ORIGINAL: 0x00401001 _target 0x00401001-0x00401002\n"
            "*/\n"
            "void Owner::fill(int color) {\n"
            "  const char *sample = R\"tag(})tag\"; // } is not the close\n"
            "  return;\n"
            "}\n"
            "// ORIGINAL: 0x00401002 _third 0x00401002-0x00401003\n"
            "int Owner::fill(double x) { return 0; }\n")
    records = read_text(text, source)
    target = next(row for row in records if row.address == 0x401001)
    markers = tuple(local_variants._marker_identity(row) for row in records)
    body = local_variants._bound_function_body_bytes(text, "Owner::fill", source,
                                                     target, markers)
    assert text.encode()[body[0]:body[1]].startswith(b"{\n  const char")
    assert text.encode()[body[0]:body[1]].endswith(b"  return;\n}")

    good = text.replace("  return;", "  int changed = color;\n  return;").encode()
    local_variants._check_body_only(text, body, "Owner::fill", [good],
                                    source=source, target_record=target,
                                    expected_markers=markers)

    for changed in (
        text.replace("return x;", "return x + 1;"),
        text.replace("void Owner::fill(int color)", "int Owner::fill(int color)"),
        text.replace("void Owner::fill(int color)",
                     "void unrelated() {} void Owner::fill(int color)"),
        text.replace("_target 0x00401001", "_other 0x00401001"),
        text.replace("  return;", "  // ORIGINAL: 0x00401004 _fake 0x00401004-0x00401005\n  return;"),
        text.replace("// ORIGINAL: 0x00401002", "void Owner::fill(char x) {}\n"
                     "// ORIGINAL: 0x00401002"),
        text.replace("void Owner::fill(int color) {\n"
                     "  const char *sample = R\"tag(})tag\"; // } is not the close\n"
                     "  return;\n}\n", "") +
        "void Owner::fill(int color) { return; }\n",
    ):
        with pytest.raises(ValueError, match="selected function definition|only the selected function body"):
            local_variants._check_body_only(text, body, "Owner::fill", [changed.encode()],
                                            source=source, target_record=target,
                                            expected_markers=markers)


def _file_mode_source(symbol="?create@AlphaNet@@QAEHXZ", *, extra=""):
    return ("// ORIGINAL: 0x00401000 " + symbol +
            " 0x00401000-0x00401020 FILE\n"
            "// symbol    " + symbol + "\n" + extra +
            "// ORIGINAL: 0x00401009 _other 0x00401030-0x00401040 FILE\n"
            "// implementation follows later in the translation unit\n"
            "int AlphaNet::create() {\n"
            "  const char *raw = R\"tag(})tag\"; // brace in data/comment\n"
            "  return 0;\n"
            "}\n"
            "void unrelated() { }\n")


def _file_mode_binding(text, source, name="AlphaNet::create"):
    markers = read_text(text, source)
    target = next(row for row in markers if row.address == 0x401000)
    identities = tuple(local_variants._marker_identity(row) for row in markers)
    body = local_variants._bound_function_body_bytes(text, name, source, target, identities)
    return target, identities, body


def test_file_mode_binding_uses_explicit_symbol_for_unique_global_body(tmp_path):
    source = tmp_path / "unit.cpp"
    text = _file_mode_source()
    target, identities, body = _file_mode_binding(text, source)
    assert text.encode()[body[0]:body[1]].startswith(b"{\n  const char *raw")
    candidate = text.replace("return 0;", "return 1;")
    local_variants._check_body_only(text, body, "AlphaNet::create", [candidate.encode()],
                                    source=source, target_record=target,
                                    expected_markers=identities)
    sibling = candidate.replace("void unrelated() { }", "void unrelated() { return; }")
    with pytest.raises(ValueError, match="only the selected function body"):
        local_variants._check_body_only(text, body, "AlphaNet::create", [sibling.encode()],
                                        source=source, target_record=target,
                                        expected_markers=identities)


@pytest.mark.parametrize("symbol,name", [
    ("?create@Other@@QAEHXZ", "AlphaNet::create"),
    ("not-a-recognized-symbol!", "AlphaNet::create"),
    ("?create@AlphaNet@@QAEHXZ", "AlphaNet::other"),
])
def test_file_mode_binding_rejects_wrong_or_unrecognized_current_symbol(tmp_path, symbol, name):
    source = tmp_path / "unit.cpp"
    text = _file_mode_source(symbol)
    markers = read_text(text, source)
    target = markers[0]
    with pytest.raises(ValueError, match="FILE target lacks an explicit matching emitted symbol"):
        local_variants._bound_function_body_bytes(
            text, name, source, target,
            tuple(local_variants._marker_identity(row) for row in markers))


def test_file_mode_binding_rejects_competing_mapped_qualified_name(tmp_path):
    source = tmp_path / "unit.cpp"
    text = _file_mode_source(extra=(
        "// ORIGINAL: 0x00401001 ?create@AlphaNet@@QAEHXZ "
        "0x00401020-0x00401030 FILE\n"
        "// symbol    ?create@AlphaNet@@QAEHXZ\n"))
    markers = read_text(text, source)
    with pytest.raises(ValueError, match="competing source mapping ownership"):
        local_variants._bound_function_body_bytes(
            text, "AlphaNet::create", source, markers[0],
            tuple(local_variants._marker_identity(row) for row in markers))


def test_file_mode_fallback_rejects_competing_catalogued_name_without_symbol(tmp_path):
    source = tmp_path / "unit.cpp"
    text = _file_mode_source(extra=(
        "// ORIGINAL: 0x00401001 ?create@AlphaNet@@QAEHXZ "
        "0x00401020-0x00401030 FILE\n"))
    markers = read_text(text, source)
    with pytest.raises(ValueError, match="competing source mapping ownership"):
        local_variants._bound_function_body_bytes(
            text, "AlphaNet::create", source, markers[0],
            tuple(local_variants._marker_identity(row) for row in markers))


def test_file_mode_adjacent_definition_keeps_interval_binding_without_symbol(tmp_path):
    source = tmp_path / "unit.cpp"
    text = ("// ORIGINAL: 0x00401000 _legacy 0x00401000-0x00401020 FILE\n"
            "int AlphaNet::create() { return 0; }\n"
            "// ORIGINAL: 0x00401001 _next 0x00401020-0x00401030 FILE\n")
    markers = read_text(text, source)
    body = local_variants._bound_function_body_bytes(
        text, "AlphaNet::create", source, markers[0],
        tuple(local_variants._marker_identity(row) for row in markers))
    assert text.encode()[body[0]:body[1]] == b"{ return 0; }"


@pytest.mark.parametrize("extra_definition", [
    "int AlphaNet::create(int) { return 2; }\n",
    "int AlphaNet::create() { return 2; }\n",
])
def test_file_mode_binding_rejects_overloaded_or_duplicate_global_definitions(
        tmp_path, extra_definition):
    source = tmp_path / "unit.cpp"
    text = _file_mode_source() + extra_definition
    markers = read_text(text, source)
    with pytest.raises(ValueError, match="2 definitions found"):
        local_variants._bound_function_body_bytes(
            text, "AlphaNet::create", source, markers[0],
            tuple(local_variants._marker_identity(row) for row in markers))


def test_body_mode_does_not_fall_back_to_translation_unit_lookup(tmp_path):
    source = tmp_path / "unit.cpp"
    text = ("// ORIGINAL: 0x00401000 _before 0x00401000-0x00401001 BODY\n"
            "// ORIGINAL: 0x00401001 ?create@AlphaNet@@QAEHXZ "
            "0x00401001-0x00401020 BODY\n"
            "// symbol    ?create@AlphaNet@@QAEHXZ\n"
            "// ORIGINAL: 0x00401002 _after 0x00401020-0x00401030 BODY\n"
            "int AlphaNet::create() { return 0; }\n")
    markers = read_text(text, source)
    target = markers[1]
    with pytest.raises(ValueError, match="0 definitions found"):
        local_variants._bound_function_body_bytes(
            text, "AlphaNet::create", source, target,
            tuple(local_variants._marker_identity(row) for row in markers))


def test_local_body_screen_refuses_default_lambda_braces_in_signature(tmp_path):
    source = tmp_path / "unit.cc"
    text = ("// ORIGINAL: 0x00401000 _fill 0x00401000-0x00401001\n"
            "void Owner::fill(int x = [] { return 1; }()) { return; }\n")
    record, = read_text(text, source)
    markers = (local_variants._marker_identity(record),)
    with pytest.raises(ValueError, match="brace inside its signature"):
        local_variants._bound_function_body_bytes(text, "Owner::fill", source,
                                                  record, markers)


@pytest.mark.parametrize("newline", ["\n", "\r\n", "\r"])
def test_local_body_screen_refuses_physical_line_splicing_and_sibling_escape(tmp_path, newline):
    source = tmp_path / "unit.cc"
    text = newline.join((
        "// ORIGINAL: 0x00401000 _target 0x00401000-0x00401001",
        "void Owner::fill(int color) {",
        "  // selected-body comment",
        "  return;",
        "}",
        "// ORIGINAL: 0x00401001 _sibling 0x00401001-0x00401002",
        "int Owner::fill(double color) { return 2; }",
        ""))
    records = read_text(text, source)
    target = records[0]
    markers = tuple(local_variants._marker_identity(row) for row in records)
    body = local_variants._bound_function_body_bytes(text, "Owner::fill", source,
                                                     target, markers)
    # The physical continuation changes the compiler's comment boundary;
    # a source-level brace walk must not treat a sibling as editable body.
    sibling_escape = (text.replace("  // selected-body comment" + newline,
                                   "  // selected-body comment \\" + newline)
                      .replace("return 2;", "return 99;"))
    with pytest.raises(ValueError, match="backslash-newline splicing"):
        local_variants._check_body_only(text, body, "Owner::fill",
                                        [sibling_escape.encode()], source=source,
                                        target_record=target, expected_markers=markers)
    with pytest.raises(ValueError, match="backslash-newline splicing"):
        local_variants._bound_function_body_bytes(sibling_escape, "Owner::fill",
                                                  source, target, markers)


@pytest.mark.parametrize("newline", ["\n", "\r\n", "\r"])
@pytest.mark.parametrize("location", ["before", "body"])
def test_local_screen_rejects_spliced_original_before_compilation(project, newline, location):
    root, _task, deck = project
    path = root / "code/unit.cc"
    source = path.read_text()
    if location == "before":
        source = ("#define PRE_TARGET 1 \\" + newline + "\n" + source)
    else:
        source = source.replace(
            "return 0;", "// continued \\" + newline + "return 0;", 1)
    path.write_bytes(source.encode())
    deck["files"]["code/unit.cc"]["source_sha256"] = hashlib.sha256(path.read_bytes()).hexdigest()
    compiler = SyntheticCompiler(root)
    with pytest.raises(ValueError, match="backslash-newline splicing"):
        run(project, compiler, deck)
    assert compiler.calls == []


def test_local_screen_conservatively_refuses_spliced_macros():
    with pytest.raises(ValueError, match="backslash-newline splicing"):
        local_variants._reject_line_splicing("#define WIDTH \\  \n 42\n")


@pytest.mark.parametrize("newline", ["\n", "\r\n", "\r"])
def test_local_screen_refuses_reproduced_continued_comment_sibling_escape(tmp_path, newline):
    source = tmp_path / "unit.cc"
    text = newline.join((
        "// ORIGINAL: 0x00401000 _f 0x00401000-0x00401001",
        "void f() { //\\",
        "{",
        "}",
        "void g() { return 1; }",
        "//\\",
        "}",
        "// ORIGINAL: 0x00401001 _next 0x00401001-0x00401002",
        ""))
    candidate = text.replace("void g() { return 1; }", "void g() { return 2; }")
    markers = read_text(text, source)
    target = markers[0]
    identities = tuple(local_variants._marker_identity(row) for row in markers)
    # Without phase-two splicing the brace walk widens f's selected body
    # across g, so the sibling-only edit would be misclassified as local.
    widened = local_variants._function_body_bytes(text, "f", after_line=1, before_line=8)
    assert b"void g()" in text.encode()[widened[0]:widened[1]]
    with pytest.raises(ValueError, match="backslash-newline splicing"):
        local_variants._bound_function_body_bytes(text, "f", source, target, identities)
    with pytest.raises(ValueError, match="backslash-newline splicing"):
        local_variants._check_body_only(text, widened, "f", [candidate.encode()],
                                        source=source, target_record=target,
                                        expected_markers=identities)


def test_local_candidate_cannot_change_trailing_splice_bytes_or_hide_them(tmp_path):
    source = tmp_path / "unit.cc"
    original = (
        "// UTF-8 prefix: café\n"
        "// ORIGINAL: 0x00401000 _f 0x00401000-0x00401001\n"
        "int _f() { return 0; }\n"
        "#define NATIVE_COPY(dst, src) \\\n"
        "  ((dst) = (src))\n"
    )
    markers = read_text(original, source)
    target, = markers
    identities = tuple(local_variants._marker_identity(row) for row in markers)
    body = local_variants._bound_function_body_bytes(
        original, "_f", source, target, identities)
    prefix = original.encode("utf-8")[:body[0]]
    suffix = original.encode("utf-8")[body[1]:]
    assert suffix.startswith(b"\n#define NATIVE_COPY")
    candidate = original.replace("return 0;", "return 1;")

    with pytest.raises(ValueError, match="trailing backslash-newline splicing"):
        local_variants._check_body_only(
            original, body, "_f", [candidate.replace("\\\n", "\n").encode()],
            source=source, target_record=target, expected_markers=identities)

    with pytest.raises(ValueError, match="trailing backslash-newline splicing"):
        changed_suffix = candidate + "// added \\\n\n"
        local_variants._check_body_only(
            original, body, "_f", [changed_suffix.encode()],
            source=source, target_record=target, expected_markers=identities)


def test_local_screen_default_splice_rejection_remains_whole_text():
    with pytest.raises(ValueError, match="backslash-newline splicing"):
        local_variants._reject_line_splicing("int f() { return 0; }\n// tail \\\ncontinued\n")
