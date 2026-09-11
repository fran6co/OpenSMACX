"""Preserve original mappings across C++ worker edits, not candidate spellings.

Snapshots are ephemeral. Intact mappings may move between authorized source
hosts; seeds, duplicate identities and new claims cannot replace lost candidates.
This checks metadata continuity, not whether a binding describes the right body.
"""

from collections import Counter, defaultdict
from pathlib import Path
from uuid import uuid4

from .model import digest
from .records import publish_json, read_json
from .source_config import SourceConfiguration
from .source_inventory import source_snapshot
from .source_replica import artifact_parent


_FACTS = ("entity_id", "candidate_kind", "catalogued_symbol", "spans", "body_path",
          "kind_hint", "legacy_mode", "legacy_exclusion", "binding_evidence")


def capture(root):
    """Observe native source projects; command-only drivers own other formats."""
    root = Path(root).resolve()
    path = root / "recovery/project.json"
    if not path.exists() and not path.is_symlink():
        return None
    if path.resolve() != path or not path.is_file() or path.stat().st_nlink != 1:
        raise ValueError("worker source configuration must be a canonical regular file")
    configuration = read_json(path)
    if not isinstance(configuration, dict):
        raise ValueError("invalid worker source project configuration")
    if "source" not in configuration:
        return None
    snapshot = source_snapshot(root, allow_empty=True)
    for name in snapshot["files"]:
        source = root / name
        if source.resolve() != source or not source.is_file() or source.stat().st_nlink != 1:
            raise ValueError("worker source mappings require canonical regular files")
    return snapshot


def _key(row, editable):
    # Moving an intact mapping within the authorized group is not deletion.
    # An outside host, however, can never substitute for an editable one.
    return digest({"host": None if row["path"] in editable else row["path"],
                   **{name: row[name] for name in _FACTS}})


def _groups(snapshot, editable):
    groups = defaultdict(Counter)
    for row in snapshot["records"]:
        key = _key(row, editable)
        flags = int(row["legacy_byte_exact"]) | (int(row["legacy_semantic"]) << 1)
        groups[key][flags] += 1
    return groups


def _retained(prior, current):
    # There are two independent protected flags. Consume both-flags demands
    # first, exact single-flag matches next, and unclaimed candidates last.
    # Each post-worker candidate can satisfy only one pre-worker obligation.
    available = current.copy()
    for flags in sorted(prior.elements(), key=int.bit_count, reverse=True):
        options = [value for value, count in available.items() if count and value & flags == flags]
        if not options:
            return False
        selected = min(options, key=lambda value: (value.bit_count(), value))
        available[selected] -= 1
    return True


def _validate(root, before, editable):
    after = capture(root)
    if before is None or after is None:
        if before is not after:
            raise ValueError("worker changed native source configuration")
        return
    if any(before[key] != after[key] for key in ("source_configuration", "image_sha256", "revision")):
        raise ValueError("worker changed original source configuration, image or revision")
    editable = frozenset(editable)
    changed = {name for name in before["files"].keys() | after["files"].keys()
               if before["files"].get(name) != after["files"].get(name)}
    if changed - editable:
        names = ", ".join(sorted(changed - editable))
        raise ValueError("worker exceeded editing scope in configured source; preserve work: " + names)
    current = _groups(after, editable)
    for key, prior in _groups(before, editable).items():
        if not _retained(prior, current.get(key, Counter())):
            row = next(row for row in before["records"] if _key(row, editable) == key)
            raise ValueError("worker removed or changed an original source binding or protected claim; preserve work: "
                             f"{row['entity_id']} (previously {row['path']}:{row['line']})")


def validate(root, before, editable):
    """Refuse loss, preserving pre-worker evidence privately without undoing C++."""
    try:
        _validate(root, before, editable)
    except (OSError, ValueError) as error:
        if before is None:
            raise
        # Uncommitted coordinator bindings may have no Git copy. Save their
        # actual pre-worker source on refusal, not only a now-unusable digest.
        # Never load this cache as an obligation ledger or rollback instruction.
        try:
            root = Path(root).resolve()
            parent = artifact_parent(root, "build/recovery/worker-source-failures",
                                     excluded_roots=SourceConfiguration.from_dict(before["source_configuration"]).roots)
            path = parent / (uuid4().hex + ".json")
            publish_json(path, {"schema_version": 1, "verification": "not_measured",
                                "ready_for_collection": False, "reason": str(error),
                                "before": before})
        except (OSError, ValueError) as backup_error:
            raise ValueError(f"{error}; pre-worker source backup unavailable: {backup_error}") from error
        raise ValueError(f"{error}; pre-worker source saved at {path.relative_to(root)}") from error
