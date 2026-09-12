"""Fresh source/object/link-file correspondence, not linked-section selection.

The caller supplies the expected source set from its current build context and
holds that context unchanged for the entire observation. Neither a compilation
database nor this report establishes that the expected set is complete/correct.
"""

from collections import Counter
import os
from pathlib import Path

from .compile_file_evidence import parse_compile_files, verify_compile_files
from .link_file_evidence import _path, parse_link_files, verify_link_files
from .model import Result


def _all(results):
    values = tuple(results)
    return (Result.FAIL if Result.FAIL in values else
            Result.UNKNOWN if not values or Result.UNKNOWN in values else Result.PASS)


def verify_compile_link_inputs(compilations, link_data: bytes, log_data: bytes, *, sources) -> dict:
    """Bind exactly one observed output per expected source to explicit link inputs.

    Arguments and path spelling are never interpreted or normalized here. The
    configured compiler owns the observations. Same-named or same-content files
    elsewhere cannot substitute for the exact observed linker input. Additional
    link inputs (libraries, resources, unobserved compiler languages) stay visible
    and require separate coverage; they are not silently called recovered code.
    """
    expected = [_path(os.fspath(source)) for source in sources]
    if len(set(expected)) != len(expected):
        raise ValueError("duplicate expected compilation source")
    if not expected:
        raise ValueError("compile/link correspondence requires expected sources")
    observed = [(raw, parse_compile_files(raw)) for raw in compilations]
    by_source, by_output = {}, {}
    for raw, value in observed:
        source, output = value["source"], value["output"]["path"]
        if source in by_source or output in by_output:
            raise ValueError("ambiguous compilation source or output identity")
        by_source[source], by_output[output] = (raw, value), source
    link = parse_link_files(link_data)
    link_proof = verify_link_files(link_data, log_data)
    files = {row["path"]: row for row in link["files"]}
    explicit = Counter(link["explicit_inputs"])
    rows = []
    for source in sorted(by_source):
        raw, value = by_source[source]
        output = value["output"]["path"]
        proof = verify_compile_files(raw, object_path=Path(output))
        supplied = files.get(output)
        result, reason = Result.UNKNOWN, "missing compile/link content observation"
        if explicit[output] != 1:
            result, reason = Result.FAIL, "compiler output must occur exactly once among explicit linker inputs"
        elif supplied is not None and all(item is not None for item in
                (value["output"]["after"], supplied["before"], supplied["after"])):
            if value["output"]["after"] == supplied["before"] == supplied["after"]:
                result, reason = Result.PASS, "exact compiled output supplied unchanged to linker"
            else:
                result, reason = Result.FAIL, "object changed between compilation and linking"
        rows.append({"source": source, "output": output,
                     "result": _all((proof["result"], result)), "compilation": proof,
                     "link_correspondence": result, "reason": reason})
    missing, unexpected = sorted(set(expected) - by_source.keys()), sorted(by_source.keys() - set(expected))
    membership = Result.FAIL if unexpected else Result.UNKNOWN if missing else Result.PASS
    return {"check": "compiled_link_input_correspondence",
            "result": _all((membership, link_proof["result"], *(row["result"] for row in rows))),
            "sources": sorted(expected), "missing_sources": missing, "unexpected_sources": unexpected,
            "source_membership": membership, "compilations": rows, "link_files": link_proof,
            "other_explicit_inputs": [path for path in link["explicit_inputs"] if path not in by_output],
            "input_coverage": Result.UNKNOWN, "linked_selection": Result.UNKNOWN,
            "build_graph_verification": Result.UNKNOWN,
            "persisted": False, "ready_for_collection": False,
            "limitations": ["expected sources and trusted compiler capture require an unchanged enclosing build context",
                            "no argument interpretation, complete include/dependency inventory or build-graph comparison",
                            "supplied object files do not prove retained sections, initializers, common allocation or behavior"]}
