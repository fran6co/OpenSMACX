"""Fresh source/object/link-file correspondence, not linked-section selection.

The caller supplies the expected source set from its current build context and
holds that context unchanged for the entire observation. Neither a compilation
database nor this report establishes that the expected set is complete/correct.
"""

from collections import Counter
import hashlib
import os
from pathlib import Path

from .compile_file_evidence import parse_compile_files, verify_compile_files
from .coff_archive import CoffArchive
from .components import archive_member_identity
from .link_file_evidence import _path, parse_link_files, verify_link_files
from .model import Result
from .msvc_link_log import parse_link_log


def _all(results):
    values = tuple(results)
    return (Result.FAIL if Result.FAIL in values else
            Result.UNKNOWN if not values or Result.UNKNOWN in values else Result.PASS)


def _library_basename(value):
    if not isinstance(value, str) or not value or not value.isascii():
        return None
    normalized = value.replace("\\", "/")
    if normalized.endswith("/"):
        return None
    return normalized.rsplit("/", 1)[-1].casefold()


def verify_compile_link_inputs(compilations, link_data: bytes, log_data: bytes, *, sources,
                               archive_members=()) -> dict:
    """Bind each expected output to one explicit input or declared archive member.

    Arguments and path spelling are never interpreted or normalized here. The
    configured compiler owns the observations. Same-named or same-content files
    elsewhere cannot substitute for the exact observed linker input. Additional
    link inputs (libraries, resources, unobserved compiler languages) stay visible
    and require separate coverage; they are not silently called recovered code.
    Archive bindings are explicit declarations. Final MSVC labels omit member
    directories, so every label must project to one exact physical archive name.
    """
    expected = [_path(os.fspath(source)) for source in sources]
    if len(set(expected)) != len(expected):
        raise ValueError("duplicate expected compilation source")
    if not expected:
        raise ValueError("compile/link correspondence requires expected sources")
    observed = [(raw, parse_compile_files(raw)) for raw in compilations]
    bindings = []
    bound_objects, bound_members = set(), set()
    for binding in archive_members:
        if not isinstance(binding, dict) or set(binding) != {"archive", "object", "member", "source"}:
            raise ValueError("archive correspondence requires validated archive, object, member and source bindings")
        archive_path = _path(binding["archive"])
        object_path = _path(binding["object"])
        source_path = _path(binding["source"]) if binding["source"] is not None else None
        member = archive_member_identity(binding["member"])
        if object_path in bound_objects or (archive_path, member) in bound_members:
            raise ValueError("duplicate archive member or object binding")
        bound_objects.add(object_path)
        bound_members.add((archive_path, member))
        bindings.append({"archive": archive_path, "object": object_path,
                         "member": member, "source": source_path})
    by_source, by_output = {}, {}
    for raw, value in observed:
        source, output = value["source"], value["output"]["path"]
        if source in by_source or output in by_output:
            raise ValueError("ambiguous compilation source or output identity")
        by_source[source], by_output[output] = (raw, value), source
    link = parse_link_files(link_data)
    link_proof = verify_link_files(link_data, log_data)
    log = parse_link_log(log_data)
    files = {row["path"]: row for row in link["files"]}
    verified_files = {row["path"]: row for row in link_proof["files"]}
    explicit = Counter(link["explicit_inputs"])
    rows = []
    bindings_by_object = {row["object"]: row for row in bindings}
    for source in sorted(by_source):
        raw, value = by_source[source]
        output = value["output"]["path"]
        proof = verify_compile_files(raw, object_path=Path(output))
        supplied = files.get(output)
        result, reason = Result.UNKNOWN, "missing compile/link content observation"
        archive_binding = bindings_by_object.get(output)
        if archive_binding is not None:
            if explicit[output] != 0:
                result, reason = Result.FAIL, "archive-bound compiler output is also supplied directly"
            else:
                result, reason = Result.UNKNOWN, "archive member correspondence is unavailable"
        elif explicit[output] != 1:
            result, reason = Result.FAIL, "compiler output must occur exactly once among explicit linker inputs"
        elif supplied is not None and all(item is not None for item in
                (value["output"]["after"], supplied["before"], supplied["after"])):
            if value["output"]["after"] == supplied["before"] == supplied["after"]:
                label = "pipeline output" if value.get("postprocess") else "compiled output"
                result, reason = Result.PASS, "exact " + label + " supplied unchanged to linker"
            else:
                result, reason = Result.FAIL, "object changed between compilation and linking"
        row = {"source": source, "output": output,
               "result": _all((proof["result"], result)), "compilation": proof,
               "link_correspondence": result, "reason": reason}
        if archive_binding is not None:
            archive_result, archive_reason = _archive_correspondence(
                archive_binding, source, output, value, verified_files,
                link_proof["library_searches"], log, explicit, by_output)
            row["archive_correspondence"] = {"archive": archive_binding["archive"],
                "member": archive_binding["member"], "object": output,
                "result": archive_result, "reason": archive_reason}
            if archive_result != Result.UNKNOWN:
                row["link_correspondence"] = archive_result
                row["reason"] = archive_reason
            row["result"] = _all((proof["result"], row["link_correspondence"]))
        rows.append(row)
    missing, unexpected = sorted(set(expected) - by_source.keys()), sorted(by_source.keys() - set(expected))
    membership = Result.FAIL if unexpected else Result.UNKNOWN if missing else Result.PASS
    archive_rows = [row["archive_correspondence"] for row in rows if "archive_correspondence" in row]
    unmatched_bindings = [binding for binding in bindings if binding["object"] not in by_output]
    archive_rows.extend({"archive": binding["archive"], "member": binding["member"],
                         "object": binding["object"], "result": Result.UNKNOWN,
                         "reason": "declared archive object has no compile observation"}
                        for binding in unmatched_bindings)
    archive_binding_result = _all(row["result"] for row in archive_rows) if bindings else Result.UNKNOWN
    overall = [membership, link_proof["result"], *(row["result"] for row in rows)]
    if bindings:
        overall.append(archive_binding_result)
    return {"check": "compiled_link_input_correspondence",
            "result": _all(overall),
            "sources": sorted(expected), "missing_sources": missing, "unexpected_sources": unexpected,
            "source_membership": membership, "compilations": rows, "link_files": link_proof,
            "archive_members": archive_rows, "archive_correspondence": archive_binding_result,
            "other_explicit_inputs": [path for path in link["explicit_inputs"] if path not in by_output],
            "input_coverage": Result.UNKNOWN, "linked_selection": Result.UNKNOWN,
            "build_graph_verification": Result.UNKNOWN,
            "persisted": False, "ready_for_collection": False,
            "limitations": ["expected sources and trusted compiler capture require an unchanged enclosing build context",
                            "no argument interpretation, complete include/dependency inventory or build-graph comparison",
                            "supplied object files or matching archive members do not prove retained sections, initializers, common allocation or behavior"]}


def _archive_correspondence(binding, source, output, compilation, files, searches, log, explicit, by_output):
    archive, member = binding["archive"], binding["member"]
    if binding["source"] is None:
        return Result.UNKNOWN, "declared archive object has no exact captured compile receipt"
    if source != binding["source"] or output != binding["object"]:
        return Result.FAIL, "archive binding does not identify this exact compiled source/output pair"
    if compilation["output"]["after"] is None:
        return Result.UNKNOWN, "archive binding lacks a fresh compile output receipt"
    if explicit[output]:
        return Result.FAIL, "compiler output is supplied both directly and through an archive"
    observed = files.get(archive)
    if observed is None:
        return Result.UNKNOWN, "configured archive is absent from captured linker files"
    matching_searches = [row for row in searches if row["path"] == archive]
    if not matching_searches:
        return Result.UNKNOWN, "configured archive is not an exact captured library search/path"
    if any(row.get("result") != Result.PASS for row in matching_searches) or observed.get("result") != Result.PASS:
        return Result.UNKNOWN, "configured archive path lacks unchanged pre/post linker evidence"
    bound_search_basenames = {_library_basename(row["spelling"]) for row in matching_searches}
    if None in bound_search_basenames:
        return Result.UNKNOWN, "configured archive has an unsupported captured search spelling"
    library_inputs = [row for row in log["inputs"]
                      if _library_basename(row["library"]) in bound_search_basenames]
    if not library_inputs:
        return Result.UNKNOWN, "configured archive basename is absent from the final link input list"
    labels = {row["library"] for row in library_inputs}
    if len(labels) != 1 or any("/" in label or "\\" in label for label in labels):
        return Result.FAIL, "final link archive labels must use one canonical basename"
    label = next(iter(labels))
    label_basename = _library_basename(label)
    candidate_searches = [row for row in searches
                          if _library_basename(row["spelling"]) == label_basename]
    if not candidate_searches:
        return Result.UNKNOWN, "final archive label has no captured library search candidates"
    if any(row["path"] is None for row in candidate_searches):
        return Result.UNKNOWN, "a matching archive search candidate has no resolved physical path"
    physical_candidates = {row["path"] for row in candidate_searches}
    if physical_candidates != {archive}:
        return Result.FAIL, "final archive basename does not resolve uniquely to the configured physical archive"
    if any(row.get("result") != Result.PASS for row in candidate_searches):
        return Result.UNKNOWN, "a matching archive search candidate lacks unchanged file evidence"
    log_searches = [row for row in log["searched_libraries"]
                    if _library_basename(row["path"]) == label_basename]
    if len(log_searches) < len(candidate_searches):
        return Result.UNKNOWN, "final archive label is not covered by the captured linker search sequence"
    member_label = member.rsplit("/", 1)[-1]
    member_inputs = [row for row in library_inputs if row["member"] == member_label]
    if len(member_inputs) != 1:
        if len(member_inputs) > 1 or library_inputs:
            return Result.FAIL, "final linker input labels conflict with the unique configured archive member"
        return Result.UNKNOWN, "configured archive member is absent from the final link input list"
    try:
        archive_bytes = Path(archive).read_bytes()
        compiled_bytes = Path(output).read_bytes()
    except OSError as error:
        return Result.UNKNOWN, str(error)
    try:
        archive_file = CoffArchive(archive_bytes)
        candidates = archive_file.members.get(member, ())
        if len(candidates) != 1:
            return Result.FAIL, f"expected one COFF archive member named {member!r}; found {len(candidates)}"
        if any(len(records) != 1 for records in archive_file.members.values()):
            return Result.FAIL, "archive contains duplicate physical member records"
        projected = [name for name in archive_file.members
                     if name.rsplit("/", 1)[-1] == member_label]
        if len(projected) != 1:
            return Result.FAIL, "final linker member basename projects to multiple archive member names"
        if projected[0] != member:
            return Result.FAIL, "final linker member basename projects to a different configured archive member"
        member_bytes = archive_file.member(member)
    except (OSError, ValueError) as error:
        return Result.FAIL, str(error)
    if ({"sha256": hashlib.sha256(archive_bytes).hexdigest(), "size": len(archive_bytes)}
            != observed.get("after")):
        return Result.FAIL, "configured archive bytes differ from captured unchanged linker input"
    if ({"sha256": hashlib.sha256(compiled_bytes).hexdigest(), "size": len(compiled_bytes)}
            != compilation["output"]["after"]):
        return Result.FAIL, "compiled output bytes differ from its fresh compiler receipt"
    if member_bytes != compiled_bytes:
        return Result.FAIL, "archive member bytes differ from the fresh compiled output"
    for other_output, other_source in by_output.items():
        if other_output == output or other_source == source:
            continue
        try:
            if Path(other_output).read_bytes() == compiled_bytes:
                return Result.FAIL, "same object bytes are emitted for a different compiled source"
        except OSError:
            return Result.UNKNOWN, "another compiler output needed to establish unique source correspondence is unavailable"
    return Result.PASS, "uniquely projected physical archive member matches the fresh compiled output and final linker label"
