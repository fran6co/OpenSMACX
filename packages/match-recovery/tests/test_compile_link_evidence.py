"""Exact file correspondence controls; synthetic compiler/link observations."""

from copy import deepcopy
import json
from pathlib import Path

import pytest

from match_recovery.compile_link_evidence import verify_compile_link_inputs
from test_compile_file_evidence import evidence as compilation_evidence, stamp
from test_link_file_evidence import evidence as link_evidence
from test_link_log import LOG


@pytest.fixture
def build(tmp_path):
    compile = compilation_evidence(tmp_path)
    link = link_evidence(tmp_path)
    output = compile['output']['path']
    link['explicit_inputs'].append(output)
    link['arguments'].append(output)
    link['files'].append({'path': output, 'before': compile['output']['after'],
                          'after': compile['output']['after']})
    return [compile], link, [Path(compile['source'])]


def verify(build, log=LOG):
    compiles, link, sources = build
    return verify_compile_link_inputs((json.dumps(row).encode() for row in compiles),
                                     json.dumps(link).encode(), log.encode(), sources=sources)


def _archive_record(field, body):
    header = (field.ljust(16, b" ") + b"0           " + b"0     " + b"0     " +
              b"100644  " + str(len(body)).encode("ascii").ljust(10, b" ") + b"`\n")
    assert len(header) == 60
    return header + body + (b"\n" if len(body) % 2 else b"")


def _archive(name, contents):
    return _archive_members([(name, contents)])


def _archive_members(entries):
    longnames = bytearray()
    fields = []
    for name, _ in entries:
        name.encode("ascii")
        if len(name) <= 15:
            fields.append((name + "/").encode("ascii"))
        else:
            offset = len(longnames)
            longnames.extend(name.encode("ascii") + b"\0")
            fields.append(("/" + str(offset)).encode("ascii"))
    archive = b"!<arch>\n"
    if longnames:
        archive += _archive_record(b"//", bytes(longnames))
    for (name, contents), field in zip(entries, fields, strict=True):
        archive += _archive_record(field, contents)
    return archive


@pytest.fixture
def archived(tmp_path):
    compile_row = compilation_evidence(tmp_path)
    output = compile_row["output"]["path"]
    member = "CMakeFiles/jackal.dir/src/jackal.cpp.obj"
    member_label = member.rsplit("/", 1)[-1]
    archive_path = tmp_path / "jackal.lib"
    spelling = str(archive_path)
    log = ("Start Pass1\nSearching Libraries\n  Searching " + spelling + ":\n"
           "End Pass1\nStart Pass2\n  jackal.lib(" + member_label + ")\nEnd Pass2\n")
    link = link_evidence(tmp_path, log)
    if output in link["explicit_inputs"]:
        link["explicit_inputs"].remove(output)
    if output in link["arguments"]:
        link["arguments"].remove(output)
    data = _archive(member, Path(output).read_bytes())
    archive_path.write_bytes(data)
    link["library_searches"] = [{"spelling": spelling, "path": spelling}]
    link["files"].append({"path": spelling, "before": stamp(data), "after": stamp(data)})
    binding = {"archive": spelling, "object": output, "member": member,
               "source": compile_row["source"]}
    build = ([compile_row], link, [Path(compile_row["source"])])
    return build, log, binding


def verify_archived(archived, *, bindings=None):
    build, log, binding = archived
    compiles, link, sources = build
    return verify_compile_link_inputs((json.dumps(row).encode() for row in compiles),
        json.dumps(link).encode(), log.encode(), sources=sources,
        archive_members=[binding] if bindings is None else bindings)


def test_exact_expected_compilations_bind_to_actual_link_input_files(build):
    report = verify(build)
    assert report['result'] == report['source_membership'] == 'pass'
    assert report['missing_sources'] == report['unexpected_sources'] == []
    row, = report['compilations']
    assert row['result'] == row['compilation']['result'] == row['link_correspondence'] == 'pass'
    assert report['other_explicit_inputs'] == build[1]['explicit_inputs'][:1]
    assert report['input_coverage'] == report['linked_selection'] == report['build_graph_verification'] == 'unknown'
    assert not report['persisted'] and not report['ready_for_collection']


def test_absent_compilation_observations_stay_unknown(build):
    build[0].clear()
    report = verify(build)
    assert report['result'] == report['source_membership'] == 'unknown'
    assert report['missing_sources'] == [str(build[2][0])]
    assert report['other_explicit_inputs'] == build[1]['explicit_inputs']


def test_missing_expected_source_is_not_erased_by_supplied_subset(build, tmp_path):
    build[2].append(tmp_path / 'missing.cpp')
    report = verify(build)
    assert report['result'] == 'unknown'
    assert report['missing_sources'] == [str(tmp_path / 'missing.cpp')]


def test_unexpected_source_cannot_be_silently_dropped(build, tmp_path):
    build[2][:] = [tmp_path / 'different.cpp']
    report = verify(build)
    assert report['result'] == report['source_membership'] == 'fail'
    assert report['unexpected_sources'] == [build[0][0]['source']]


@pytest.mark.parametrize('damage', ['source', 'output', 'expected'])
def test_ambiguous_identities_refuse(build, tmp_path, damage):
    if damage == 'expected':
        build[2].append(build[2][0])
    else:
        duplicate = deepcopy(build[0][0])
        if damage == 'source':
            duplicate['output']['path'] = str(tmp_path / 'other.obj')
        else:
            original = duplicate['source']
            duplicate['source'] = str(tmp_path / 'different.cpp')
            next(row for row in duplicate['inputs'] if row['path'] == original)['path'] = duplicate['source']
        build[0].append(duplicate)
    with pytest.raises(ValueError, match='duplicate|ambiguous'):
        verify(build)


@pytest.mark.parametrize('damage', ['not_supplied', 'twice', 'same_name_same_bytes_elsewhere'])
def test_compiler_output_requires_one_exact_link_input_identity(build, tmp_path, damage):
    output = build[0][0]['output']['path']
    if damage == 'twice':
        build[1]['explicit_inputs'].append(output)
    else:
        build[1]['explicit_inputs'].remove(output)
        if damage != 'not_supplied':
            (tmp_path / 'other').mkdir()
            substitute = tmp_path / 'other' / Path(output).name
            substitute.write_bytes(Path(output).read_bytes())
            build[1]['explicit_inputs'].append(str(substitute))
            build[1]['files'].append({'path': str(substitute), 'before': stamp(substitute.read_bytes()),
                                      'after': stamp(substitute.read_bytes())})
    report = verify(build)
    assert report['result'] == report['compilations'][0]['link_correspondence'] == 'fail'


def test_different_compile_and_link_bytes_fail_even_when_current_object_matches_compile(build):
    output = build[0][0]['output']['path']
    item = next(row for row in build[1]['files'] if row['path'] == output)
    item['before'] = item['after'] = stamp(b'other linked contents')
    report = verify(build)
    assert report['compilations'][0]['compilation']['result'] == 'pass'
    assert report['result'] == report['compilations'][0]['link_correspondence'] == 'fail'


@pytest.mark.parametrize('damage', ['compile_failure', 'source_changed', 'tool_changed',
                                   'object_changed', 'library_changed', 'missing_observation'])
def test_aggregate_never_hides_failed_or_incomplete_file_evidence(build, damage):
    compile = build[0][0]
    if damage == 'compile_failure':
        compile['exit_code'] = 2
    elif damage == 'missing_observation':
        compile['output']['after'] = None
    else:
        path = (compile['source'] if damage == 'source_changed' else
                compile['tool'] if damage == 'tool_changed' else
                compile['output']['path'] if damage == 'object_changed' else
                build[1]['library_searches'][0]['path'])
        Path(path).write_bytes(b'changed observed input')
    assert verify(build)['result'] == ('unknown' if damage == 'missing_observation' else 'fail')


def test_link_diagnostics_must_belong_to_the_same_link(build):
    with pytest.raises(ValueError, match='different diagnostics'):
        verify(build, LOG + '\n')


@pytest.mark.parametrize('sources', [[], [Path('relative.cpp')], ['/absolute/../alias.cpp']])
def test_expected_source_identifiers_are_explicit_normalized_absolute_paths(build, sources):
    build[2][:] = sources
    with pytest.raises(ValueError):
        verify(build)


def test_inputs_and_output_rows_are_not_mutated_or_persisted(build):
    before = deepcopy(build)
    first, second = verify(build), verify(build)
    assert first == second and build == before


def test_exact_unique_archive_member_corresponds_to_fresh_source_object(archived):
    report = verify_archived(archived)
    assert report["result"] == report["archive_correspondence"] == "pass"
    row, = report["archive_members"]
    assert row["result"] == "pass"
    assert "uniquely projected physical archive member" in row["reason"]
    assert report["input_coverage"] == report["linked_selection"] == report["build_graph_verification"] == "unknown"


def test_archive_member_label_resolution_allows_repeated_same_path_searches(archived):
    build, log, binding = archived
    log = log.replace("End Pass1", "  Searching " + binding["archive"] + ":\nEnd Pass1")
    build[1]["library_searches"].append(dict(build[1]["library_searches"][0]))
    build[1]["link_log_sha256"] = stamp(log.encode())["sha256"]
    report = verify_compile_link_inputs((json.dumps(item).encode() for item in build[0]),
        json.dumps(build[1]).encode(), log.encode(), sources=build[2], archive_members=[binding])
    assert report["result"] == report["archive_correspondence"] == "pass"


def test_archive_label_projects_from_absolute_search_spelling(archived):
    build, _, binding = archived
    spelling = r"Z:\worker\jackal.lib"
    log = ("Start Pass1\nSearching Libraries\n  Searching " + spelling + ":\n"
           "End Pass1\nStart Pass2\n  JACKAL.LIB(jackal.cpp.obj)\nEnd Pass2\n")
    build[1]["library_searches"] = [{"spelling": spelling, "path": binding["archive"]}]
    build[1]["link_log_sha256"] = stamp(log.encode())["sha256"]
    report = verify_compile_link_inputs((json.dumps(item).encode() for item in build[0]),
        json.dumps(build[1]).encode(), log.encode(), sources=build[2], archive_members=[binding])
    assert report["result"] == report["archive_correspondence"] == "pass"


def test_same_archive_basename_at_multiple_physical_search_paths_refuses(archived, tmp_path):
    build, _, binding = archived
    other = tmp_path / "other" / "jackal.lib"
    other.parent.mkdir()
    other.write_bytes(Path(binding["archive"]).read_bytes())
    spelling = r"Y:\runtime\jackal.lib"
    build[1]["library_searches"].append({"spelling": spelling, "path": str(other)})
    other_stamp = stamp(other.read_bytes())
    build[1]["files"].append({"path": str(other), "before": other_stamp, "after": other_stamp})
    log = ("Start Pass1\nSearching Libraries\n  Searching " + binding["archive"] + ":\n"
           "  Searching " + spelling + ":\nEnd Pass1\nStart Pass2\n  jackal.lib(jackal.cpp.obj)\nEnd Pass2\n")
    build[1]["link_log_sha256"] = stamp(log.encode())["sha256"]
    report = verify_compile_link_inputs((json.dumps(item).encode() for item in build[0]),
        json.dumps(build[1]).encode(), log.encode(), sources=build[2], archive_members=[binding])
    assert report["result"] == report["archive_correspondence"] == "fail"


def test_archive_with_directory_ambiguous_final_label_refuses(archived):
    build, log, binding = archived
    archive_path = Path(binding["archive"])
    archive_bytes = _archive_members([(binding["member"], Path(binding["object"]).read_bytes()),
                                      ("other/CMakeFiles/jackal.dir/src/jackal.cpp.obj", b"other")])
    archive_path.write_bytes(archive_bytes)
    archive_stamp = stamp(archive_path.read_bytes())
    observed = next(row for row in build[1]["files"] if row["path"] == binding["archive"])
    observed["before"] = observed["after"] = archive_stamp
    report = verify_compile_link_inputs((json.dumps(item).encode() for item in build[0]),
        json.dumps(build[1]).encode(), log.encode(), sources=build[2], archive_members=[binding])
    assert report["result"] == report["archive_correspondence"] == "fail"


def test_two_distinct_projected_members_bind_two_fresh_source_outputs(archived, tmp_path):
    build, _, first = archived
    first_member = "CMakeFiles/one.dir/src/jackal.cpp.obj"
    second_member = "CMakeFiles/two.dir/src/second.cpp.obj"
    second_source = tmp_path / "second.cc"
    second_source.write_bytes(b"second source")
    second_output = tmp_path / "second.obj"
    second_output.write_bytes(b"second object bytes")
    second = deepcopy(build[0][0])
    original_source = second["source"]
    second["source"] = str(second_source)
    input_row = next(row for row in second["inputs"] if row["path"] == original_source)
    input_row.update(path=str(second_source), before=stamp(second_source.read_bytes()),
                     after=stamp(second_source.read_bytes()))
    second["output"].update(path=str(second_output), after=stamp(second_output.read_bytes()))
    build[0][0]["output"]["path"] = str(Path(first["object"]))
    first["member"] = first_member
    build[0].append(second)
    build[2].append(second_source)
    archive_path = Path(first["archive"])
    first_bytes = Path(first["object"]).read_bytes()
    archive_bytes = _archive_members([(first_member, first_bytes), (second_member, second_output.read_bytes())])
    archive_path.write_bytes(archive_bytes)
    archive_row = next(row for row in build[1]["files"] if row["path"] == first["archive"])
    archive_row["before"] = archive_row["after"] = stamp(archive_bytes)
    label1, label2 = first_member.rsplit("/", 1)[-1], second_member.rsplit("/", 1)[-1]
    library_label = Path(first["archive"]).name
    log = ("Start Pass1\nSearching Libraries\n  Searching " + first["archive"] + ":\n"
           "End Pass1\nStart Pass2\n  " + library_label + "(" + label1 + ")\n  "
           + library_label + "(" + label2 + ")\nEnd Pass2\n")
    build[1]["link_log_sha256"] = stamp(log.encode())["sha256"]
    second_binding = {"archive": first["archive"], "object": str(second_output),
                      "member": second_member, "source": str(second_source)}
    report = verify_compile_link_inputs((json.dumps(item).encode() for item in build[0]),
        json.dumps(build[1]).encode(), log.encode(), sources=build[2],
        archive_members=[first, second_binding])
    assert report["result"] == report["archive_correspondence"] == "pass"
    assert [row["result"] for row in report["archive_members"]] == ["pass", "pass"]


def test_same_full_member_identity_is_valid_in_two_distinct_archives(archived, tmp_path):
    build, _, first = archived
    first_member = second_member = "CMakeFiles/shared.dir/shared.obj"
    second_source = tmp_path / "second.cc"
    second_source.write_bytes(b"second source")
    second_output = tmp_path / "second.obj"
    second_output.write_bytes(b"different second object")
    second = deepcopy(build[0][0])
    original_source = second["source"]
    second["source"] = str(second_source)
    input_row = next(row for row in second["inputs"] if row["path"] == original_source)
    input_row.update(path=str(second_source), before=stamp(second_source.read_bytes()),
                     after=stamp(second_source.read_bytes()))
    second["output"].update(path=str(second_output), after=stamp(second_output.read_bytes()))
    build[0].append(second)
    build[2].append(second_source)
    first["member"] = first_member
    archive1 = Path(first["archive"])
    archive1.write_bytes(_archive(first_member, Path(first["object"]).read_bytes()))
    archive2 = tmp_path / "second.lib"
    archive2.write_bytes(_archive(second_member, second_output.read_bytes()))
    first_archive_stamp = stamp(archive1.read_bytes())
    first_file_row = next(row for row in build[1]["files"] if row["path"] == str(archive1))
    first_file_row["before"] = first_file_row["after"] = first_archive_stamp
    second_archive_stamp = stamp(archive2.read_bytes())
    build[1]["files"].append({"path": str(archive2), "before": second_archive_stamp,
                              "after": second_archive_stamp})
    spelling1, spelling2 = "jackal.lib", "second.lib"
    build[1]["library_searches"] = [
        {"spelling": spelling1, "path": str(archive1)},
        {"spelling": spelling2, "path": str(archive2)}]
    log = ("Start Pass1\nSearching Libraries\n  Searching " + spelling1 + ":\n  Searching "
           + spelling2 + ":\nEnd Pass1\nStart Pass2\n  jackal.lib(shared.obj)\n  second.lib(shared.obj)\nEnd Pass2\n")
    build[1]["link_log_sha256"] = stamp(log.encode())["sha256"]
    second_binding = {"archive": str(archive2), "object": str(second_output),
                      "member": second_member, "source": str(second_source)}
    report = verify_compile_link_inputs((json.dumps(item).encode() for item in build[0]),
        json.dumps(build[1]).encode(), log.encode(), sources=build[2],
        archive_members=[first, second_binding])
    assert report["result"] == report["archive_correspondence"] == "pass"
    assert [row["result"] for row in report["archive_members"]] == ["pass", "pass"]


@pytest.mark.parametrize("damage", ["member_bytes", "missing_member", "duplicate_member",
                                    "wrong_member_input", "multimember_archive"])
def test_archive_member_correspondence_fails_or_stays_unknown_without_exact_evidence(archived, damage):
    build, log, binding = archived
    archive_path = Path(binding["archive"])
    if damage == "member_bytes":
        archive_path.write_bytes(_archive(binding["member"], b"different object bytes"))
    elif damage == "missing_member":
        archive_path.write_bytes(_archive("another.obj", Path(binding["object"]).read_bytes()))
    elif damage == "duplicate_member":
        body = Path(binding["object"]).read_bytes()
        archive_path.write_bytes(_archive(binding["member"], body) + _archive_record(b"/0", body))
    elif damage == "multimember_archive":
        archive_path.write_bytes(_archive_members([
            (binding["member"], Path(binding["object"]).read_bytes()),
            ("other/" + binding["member"].rsplit("/", 1)[-1], b"other")]))
    else:
        log = log.replace(binding["member"].rsplit("/", 1)[-1], "other.obj")
        from match_recovery.link_file_evidence import parse_link_files
        value = parse_link_files(json.dumps(build[1]).encode())
        value["link_log_sha256"] = stamp(log.encode())["sha256"]
        build[1]["link_log_sha256"] = value["link_log_sha256"]
    raw = archive_path.read_bytes()
    row = next(row for row in build[1]["files"] if row["path"] == binding["archive"])
    row["before"] = row["after"] = stamp(raw)
    report = verify_compile_link_inputs((json.dumps(item).encode() for item in build[0]),
        json.dumps(build[1]).encode(), log.encode(), sources=build[2], archive_members=[binding])
    assert report["result"] != "pass"


def test_archive_binding_refuses_direct_and_archive_duplicate(archived):
    build, log, binding = archived
    build[1]["explicit_inputs"].append(binding["object"])
    body_stamp = stamp(Path(binding["object"]).read_bytes())
    build[1]["files"].append({"path": binding["object"], "before": body_stamp, "after": body_stamp})
    report = verify_compile_link_inputs((json.dumps(item).encode() for item in build[0]),
        json.dumps(build[1]).encode(), log.encode(), sources=build[2], archive_members=[binding])
    assert report["result"] == report["archive_members"][0]["result"] == "fail"


def test_duplicate_archive_bindings_refuse(archived):
    _, _, binding = archived
    with pytest.raises(ValueError, match="duplicate"):
        verify_archived(archived, bindings=[binding, dict(binding)])


def test_same_body_for_different_source_outputs_refuses_archive_owner(archived, tmp_path):
    build, log, binding = archived
    second_source = tmp_path / "other.cc"
    second_source.write_bytes(Path(binding["source"]).read_bytes())
    second_output = tmp_path / "other.obj"
    second_output.write_bytes(Path(binding["object"]).read_bytes())
    second = deepcopy(build[0][0])
    second["source"] = str(second_source)
    second["output"]["path"] = str(second_output)
    second["inputs"][0]["path"] = str(second_source)
    second["inputs"][0]["before"] = second["inputs"][0]["after"] = stamp(second_source.read_bytes())
    second["output"]["after"] = stamp(second_output.read_bytes())
    report = verify_compile_link_inputs((json.dumps(item).encode() for item in [*build[0], second]),
        json.dumps(build[1]).encode(), log.encode(), sources=[*build[2], second_source], archive_members=[binding])
    assert report["result"] == report["archive_members"][0]["result"] == "fail"
