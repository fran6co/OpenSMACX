"""Read-only annotation integrity, independent of a compiler or target image.

Compare actual source text, not merely two already-lossy parsed records. All
mutations below are in-memory damage controls; this command never stamps claims.
"""

from collections import Counter
from concurrent.futures import ProcessPoolExecutor
from dataclasses import replace
from hashlib import sha256
import os
from pathlib import Path
import re

from .model import digest
from .records import parse_json
from .source import Tier, read_text, write
from .source.record import stamped
from .source_config import SourceConfiguration


RULES = (
    "marker_coverage", "noop_identity", "keyword_isolation",
    "demotion_line_count", "demotion_lessons", "lesson_deletion", "binding_evidence",
)
# Independent, deliberately broader than the parser's accepted address grammar.
# Only standalone comment marker heads count here, not quoted documentation,
# string literals, or PROVEN-AGAINST-ORIGINAL provenance.
MARKER_HEAD = re.compile(r"^\s*(?://|/\*|\*)\s*ORIGINAL:")


def check_annotation_text(path: Path, text: str) -> dict:
    """Check a file in memory, preserving its path for parser ownership rules."""
    records = read_text(text, path)
    findings = []

    def fail(rule, line, reason):
        findings.append({"rule": rule, "line": line, "reason": reason})

    from .binding_source import parse as parse_binding_evidence
    try:
        parse_binding_evidence(text, records)
    except ValueError as error:
        fail("binding_evidence", 1, str(error))

    parsed_lines = {record.line for record in records}
    for line, content in enumerate(text.splitlines(), 1):
        if MARKER_HEAD.match(content) and line not in parsed_lines:
            fail("marker_coverage", line, "standalone ORIGINAL marker was not read")
    if not records:
        return {"annotations": 0, "findings": findings}
    if write(text, records) != text:
        fail("noop_identity", records[0].line, "a no-op rewrite changes source text")
    before_lines = text.splitlines(keepends=True)
    for record in records:
        flipped = write(text, [replace(record, byte_exact=not record.byte_exact)])
        after_lines = flipped.splitlines(keepends=True)
        changed = [index for index, (a, b) in enumerate(zip(before_lines, after_lines)) if a != b]
        if (len(before_lines) != len(after_lines) or changed != [record.line - 1]
                or before_lines[record.line - 1].replace(" BYTE_EXACT", "")
                != after_lines[record.line - 1].replace(" BYTE_EXACT", "")):
            fail("keyword_isolation", record.line, "flipping a claim changes more than its marker keyword")

        # Exercise the real transition policy, not a second copy of its lesson
        # rules. This is a hypothetical text operation, never demotion on disk.
        down = stamped(record, Tier.MISMATCH, demote=True)
        after = write(text, [down])
        if len(after.splitlines()) != len(before_lines):
            fail("demotion_line_count", record.line, "demotion changes the source line count")
        else:
            reread = [item for item in read_text(after, path) if item.line == record.line]
            if (len(reread) != 1 or reread[0].address != record.address
                    or Counter(reread[0].ruled_out) != Counter(down.ruled_out)
                    or Counter(reread[0].levers) != Counter(down.levers)
                    or Counter(reread[0].deferred) != Counter(down.deferred)):
                fail("demotion_lessons", record.line, "demotion loses or changes lesson meaning or identity")

        if record.origin:
            bare = replace(record, levers=(), ruled_out=(), unrecoverable=(), deferred=())
            owned = {offset for lesson in record.origin for offset, _ in lesson.lines}
            expected = "".join(line for index, line in enumerate(before_lines) if index not in owned)
            # Stronger than a line-count check: deleting a neighboring line and
            # retaining a lesson would have the same count but must fail.
            if write(text, [bare]) != expected:
                fail("lesson_deletion", record.line, "clearing lessons changes text beyond their owned lines")
    return {"annotations": len(records), "findings": findings}


def _one_file(item):
    path, data = item
    try:
        result = check_annotation_text(Path(path), data.decode("utf-8"))
    except (UnicodeError, ValueError, AssertionError, IndexError) as error:
        return path, {"annotations": 0, "findings": [], "error": str(error)}
    return path, result


def check_source(root: Path, *, jobs: int = 1) -> dict:
    """Check every configured source file, including headers and seed material.

    No Git database, image, build commands, compiler or optional disassembler is
    needed. Missing/changed inputs are unknown, not an empty successful census.
    """
    if type(jobs) is not int or jobs < 1:
        raise ValueError("source-check jobs must be a positive integer")
    root = root.resolve()
    project_path = root / "recovery/project.json"
    if not project_path.resolve().is_relative_to(root):
        raise ValueError("source configuration resolves outside the worktree")
    project = project_path.read_bytes()
    configuration = parse_json(project)
    if (not isinstance(configuration, dict) or type(configuration.get("schema_version")) is not int
            or configuration["schema_version"] != 1):
        raise ValueError("unsupported project configuration version")
    source = SourceConfiguration.from_dict(configuration.get("source"))

    def snapshot():
        for name in source.roots:
            directory = source.path(root, name)
            for path in (directory, *directory.rglob("*")):
                # rglob skips directory symlinks; accepting one could hide an
                # arbitrary population. Avoid rewriting aliases as well.
                if path.is_symlink():
                    raise ValueError(f"source integrity cannot enumerate symlinks: {path.relative_to(root)}")
        return {path: path.read_bytes() for path in source.files(root)}

    before = snapshot()
    if not before:
        raise ValueError("refusing an empty configured source census")
    package = Path(__file__).parent

    def implementation():
        return {path.relative_to(package).as_posix(): sha256(path.read_bytes()).hexdigest()
                for path in sorted(package.rglob("*.py"))}

    code = implementation()
    items = [(str(path), data) for path, data in before.items()]
    if jobs == 1:
        results = list(map(_one_file, items))
    else:
        with ProcessPoolExecutor(max_workers=min(jobs, len(items), os.cpu_count() or 1)) as pool:
            results = list(pool.map(_one_file, items, chunksize=8))
    if project_path.read_bytes() != project or before != snapshot() or implementation() != code:
        raise ValueError("source, configuration or checker changed during annotation integrity verification")
    findings, errors = [], []
    for path, result in results:
        relative = Path(path).relative_to(root).as_posix()
        findings.extend({"path": relative, **finding} for finding in result["findings"])
        if "error" in result:
            errors.append({"path": relative, "reason": result["error"]})
    counts = Counter(finding["rule"] for finding in findings)
    annotations = sum(result["annotations"] for _, result in results)
    if not annotations and not findings and not errors:
        errors.append({"reason": "no ORIGINAL annotations were read; recovery coverage is unestablished"})
    return {
        "check": "source_annotation_integrity", "schema_version": 1,
        "result": "fail" if findings else "unknown" if errors else "pass",
        "files_scanned": len(before), "annotations": annotations,
        "rules": [{"name": name, "failures": counts[name]} for name in RULES],
        "findings": findings, "errors": errors,
        "input_fingerprint": digest({"project": sha256(project).hexdigest(), "implementation": code,
                                     "source": {path.relative_to(root).as_posix(): sha256(data).hexdigest()
                                                for path, data in before.items()}}),
        "persisted": False, "ready_for_collection": False,
        "limitations": ["annotation text integrity only, not current matching, source ownership or full acceptance",
                        "coverage witnesses standalone ORIGINAL comment heads in configured extensions; not a C++ lexer",
                        "duplicate bindings, build participation and historical obligations require separate checks"],
    }
