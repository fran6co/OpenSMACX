"""Source-identity restoration coverage, derived from immutable Git history.

Counts are reports, never ledgers. Renaming or rehoming a candidate does not
change its original identity. This checks that obligations remain represented
in source, not that current bytes, references or structure satisfy them.
"""

from collections import defaultdict
from pathlib import Path
import re

from .model import Origin, digest
from .records import parse_json
from .source import read_text
from .source_config import SourceConfiguration
from .source_history import HistoryConfiguration, git, source_versions
from .source_inventory import source_snapshot, assert_snapshot_current


def _baseline(root: Path, head: str, history: HistoryConfiguration) -> str:
    """A new anchor is explicit policy; a recorded anchor cannot be moved/reset.

    Inspect every reachable configuration change, including merged branches.
    Checking only the chosen baseline's descendants would let a later baseline
    hide its own earlier declaration and the obligations it discarded.
    """
    baseline = history.baseline_revision
    if baseline is None:
        raise ValueError("configure source_history.baseline_revision before checking claim identities")
    if git(root, "rev-parse", "--is-shallow-repository").strip() != b"false":
        raise ValueError("claim identity coverage requires complete Git history; shallow history cannot prove coverage")
    revisions = git(root, "log", "--full-history", "--format=%H", head,
                    "--", "recovery/project.json").decode().splitlines()
    anchors = set()
    for revision in dict.fromkeys(revisions):
        entry = git(root, "ls-tree", revision, "--", "recovery/project.json")
        if not entry:
            continue  # A configuration deletion does not erase earlier anchors.
        metadata = entry.split(b"\t", 1)[0].decode().split()
        if metadata[:2] != ["100644", "blob"] and metadata[:2] != ["100755", "blob"]:
            raise ValueError("historical project configuration is not a regular file")
        project = parse_json(git(root, "cat-file", "blob", metadata[2]))
        value = project.get("source_history", {})
        if not isinstance(value, dict):
            raise ValueError("invalid historical source history configuration")
        if "baseline_revision" in value:
            if (not isinstance(value["baseline_revision"], str)
                    or not re.fullmatch(r"[0-9a-f]{40}|[0-9a-f]{64}", value["baseline_revision"])):
                raise ValueError("invalid historical source obligation baseline")
            anchors.add(value["baseline_revision"])
            interpretation = (SourceConfiguration.from_dict(value["before_configuration"])
                              if "before_configuration" in value else None)
            if interpretation != history.before_configuration:
                raise ValueError("established pre-configuration source interpretation changed")
    if anchors - {baseline}:
        raise ValueError("source history baseline changed; established obligations cannot be reset")
    git(root, "merge-base", "--is-ancestor", baseline, head)
    return baseline


def check_claims(root: Path) -> dict:
    """Require every historical byte/shape obligation to remain in source.

    Exact bytes also preserve an older instruction-shape requirement. Multiple
    candidates for an original remain measurement/ownership work, not separate
    identities that force keeping obsolete seed copies. New claim declarations
    still require fresh measurement by the separate binary gate; this function
    neither grants claims nor authenticates which editor wrote a token.
    """
    root = root.resolve()
    config_path = root / "recovery/project.json"
    config_bytes = config_path.read_bytes()
    history = HistoryConfiguration.read(root)
    snapshot = source_snapshot(root, allow_empty=True)
    head = snapshot["revision"]
    if head is None:
        raise ValueError("claim identity coverage requires committed source history")
    baseline = _baseline(root, head, history)
    configuration = SourceConfiguration.from_dict(snapshot["source_configuration"])
    pin = snapshot["image_sha256"]
    versions = source_versions(root, baseline, head, configuration=configuration,
                               history=history, image_sha256=pin)
    required, current = defaultdict(list), defaultdict(list)
    for version in versions:
        for row in read_text(version.content.decode("utf-8", errors="replace"), root / version.path):
            entity = Origin(pin, version.configuration.address_space, row.address)
            for present, dimension in ((row.byte_exact, "bytes"), (row.semantic, "instruction_shape")):
                if present:
                    required[(entity.entity_id, dimension)].append({
                        "address": row.address_hex, "path": version.path, "line": row.line,
                        "revision": version.revision, "blob": version.blob})
    claims = 0
    for row in snapshot["records"]:
        if row["legacy_byte_exact"] or row["legacy_semantic"]:
            claims += 1
        for present, dimension in ((row["legacy_byte_exact"], "bytes"),
                                   (row["legacy_semantic"] or row["legacy_byte_exact"], "instruction_shape")):
            if present:
                current[(row["entity_id"], dimension)].append({
                    "address": f'0x{row["origin"]["address"]:08X}', "path": row["path"], "line": row["line"]})
    missing = [{"entity_id": entity, "dimension": dimension,
                "address": evidence[0]["address"], "source_evidence": evidence}
               for (entity, dimension), evidence in sorted(required.items()) if (entity, dimension) not in current]
    if not snapshot["records"] and not required:
        raise ValueError("no source annotations or historical requirements to check")
    if config_path.read_bytes() != config_bytes or history != HistoryConfiguration.read(root):
        raise ValueError("source history policy changed during claim checking")
    assert_snapshot_current(root, snapshot)
    return {"schema_version": 1, "check": "source_claim_identities",
            "result": "fail" if missing else "pass", "base_revision": baseline,
            "head": head, "image_sha256": pin, "annotations": len(snapshot["records"]),
            "claims": claims, "historical_requirements": len(required), "missing": missing,
            "input_fingerprint": digest({"head": head, "history": history,
                                         "configuration": configuration, "image": pin,
                                         "files": {path: item["sha256"] for path, item in snapshot["files"].items()}}),
            "persisted": False, "ready_for_collection": False,
            "verification": "source_coverage_only"}
